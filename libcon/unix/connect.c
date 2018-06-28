/*copyright iadix 2016*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <errno.h>

#include "../base/std_def.h"
#include "../base/std_mem.h"
#include "../base/mem_base.h"
#include "../base/std_str.h"
#include "strs.h"
#include "fsio.h"
#include "mem_stream.h"
#include "tpo_mod.h"


#include <connect.h>
#include <sys_include.h>
#include <ifaddrs.h>
struct read_con
{
	FILE			*wr_file;
	struct con		*rd_con;
	size_t			len;
	struct string	file_name;
};

struct read_group
{
	fd_set				read_set;
	int					max_sock;
	struct read_con		cons[64];
};


struct read_group my_read_group={0};
struct string	  read_done[64]={0};


OS_API_C_FUNC(int) network_init()
{
	sys_add_tpo_mod_func_name("libcon", "network_init", (void_func_ptr)network_init, 0);
	sys_add_tpo_mod_func_name("libcon", "network_free", (void_func_ptr)network_free, 0);
	sys_add_tpo_mod_func_name("libcon", "get_if", (void_func_ptr)get_if, 0);
	sys_add_tpo_mod_func_name("libcon", "init_read_group", (void_func_ptr)init_read_group, 0);
	sys_add_tpo_mod_func_name("libcon", "read_group_has", (void_func_ptr)read_group_has, 0);
	sys_add_tpo_mod_func_name("libcon", "set_tcp_no_delay", (void_func_ptr)set_tcp_no_delay, 0);
	sys_add_tpo_mod_func_name("libcon", "add_read_group", (void_func_ptr)add_read_group, 0);
	sys_add_tpo_mod_func_name("libcon", "get_con_error", (void_func_ptr)get_con_error, 0);
	sys_add_tpo_mod_func_name("libcon", "get_con_lastline", (void_func_ptr)get_con_lastline, 0);
	sys_add_tpo_mod_func_name("libcon", "con_move_data", (void_func_ptr)con_move_data, 0);
	sys_add_tpo_mod_func_name("libcon", "con_consume_data", (void_func_ptr)con_consume_data, 0);
	sys_add_tpo_mod_func_name("libcon", "get_con_hostd", (void_func_ptr)get_con_hostd, 0);
	sys_add_tpo_mod_func_name("libcon", "do_connect", (void_func_ptr)do_connect, 0);
	sys_add_tpo_mod_func_name("libcon", "reconnect", (void_func_ptr)reconnect, 0);
	sys_add_tpo_mod_func_name("libcon", "open_port", (void_func_ptr)open_port, 0);
	sys_add_tpo_mod_func_name("libcon", "do_get_incoming", (void_func_ptr)do_get_incoming, 0);
	sys_add_tpo_mod_func_name("libcon", "read_data", (void_func_ptr)read_data, 0);
	sys_add_tpo_mod_func_name("libcon", "send_data", (void_func_ptr)send_data, 0);
	sys_add_tpo_mod_func_name("libcon", "readline", (void_func_ptr)readline, 0);
	sys_add_tpo_mod_func_name("libcon", "do_read_group", (void_func_ptr)do_read_group, 0);
	sys_add_tpo_mod_func_name("libcon", "pop_read_done", (void_func_ptr)pop_read_done, 0);
	sys_add_tpo_mod_func_name("libcon", "con_close", (void_func_ptr)con_close, 0);
	sys_add_tpo_mod_func_name("libcon", "get_con_saddr", (void_func_ptr)get_con_saddr, 0);
	sys_add_tpo_mod_func_name("libcon", "read_av_data", (void_func_ptr)read_av_data, 0);
	sys_add_tpo_mod_func_name("libcon", "send_data_av", (void_func_ptr)send_data_av, 0);
	sys_add_tpo_mod_func_name("libcon", "create_upnp_broadcast", (void_func_ptr)create_upnp_broadcast, 0);
	sys_add_tpo_mod_func_name("libcon", "send_upnpbroadcast", (void_func_ptr)send_upnpbroadcast, 0);
	sys_add_tpo_mod_func_name("libcon", "get_con_ip", (void_func_ptr)get_con_ip, 0);
	sys_add_tpo_mod_func_name("libcon", "free_con_buffer", (void_func_ptr)free_con_buffer, 0);

}

OS_API_C_FUNC(int) send_data_av(struct con *Con, unsigned char *data, size_t len)
{
	fd_set				fd_write, fd_err;
	struct timeval		timeout;
	int					ret;
	int					s;

	free_string(&Con->error);

	timeout.tv_sec = 0;
	timeout.tv_usec = 1000;

	/* Block until input arrives on one or more active sockets. */
	fd_write = Con->con_set;
	fd_err = Con->con_set;

	ret = select(Con->sock + 1, NULL, &fd_write, &fd_err, &timeout);
	if (ret < 0)
		return -1;

	if (FD_ISSET(Con->sock, &fd_err)){ Con->last_rd = 0; return -1; }
	if (FD_ISSET(Con->sock, &fd_write))
		s = send(Con->sock, data, (int)(len), MSG_NOSIGNAL);
	else
		return 0;

	return s;
}

OS_API_C_FUNC(int) read_av_data(struct con *Con, size_t max)
{
	fd_set			read_fd_set, err_fd_set;
	struct timeval	timeout;
	int				ret;

	free_string(&Con->error);
	if (Con->lastLine.str == NULL)
	{
		Con->lastLine.size = max + 1;
		Con->lastLine.str = malloc_c(Con->lastLine.size);
		Con->lastLine.len = 0;
	}
	else if (Con->lastLine.size<(Con->lastLine.len + max + 1))
	{
		Con->lastLine.size = Con->lastLine.len + max + 1;
		Con->lastLine.str = realloc_c(Con->lastLine.str, Con->lastLine.size);
	}

	timeout.tv_sec = 0;
	timeout.tv_usec = 1000;
	Con->last_rd = 0;

	read_fd_set = Con->con_set;
	err_fd_set = Con->con_set;
	ret = select(Con->sock + 1, &read_fd_set, NULL, &err_fd_set, &timeout);
	if (ret < 0)
		return 0;

	if (FD_ISSET(Con->sock, &err_fd_set)){ make_string(&Con->error, "select error");  Con->last_rd = 0; return 0; }
	if (!FD_ISSET(Con->sock, &read_fd_set))return 0;

	ret = recv(Con->sock, &Con->lastLine.str[Con->lastLine.len], max, MSG_NOSIGNAL);
	if (ret > 0)
	{
		Con->last_rd = ret;
		Con->lastLine.len += Con->last_rd;
	}
	else
		Con->last_rd = 0;

	return ret;

}
OS_API_C_FUNC(int) get_con_saddr(struct con *mycon, ipv4_t addr)
{
	addr[0] = mycon->peer.sin_addr.s_addr & 0xFF;
	addr[1] = (mycon->peer.sin_addr.s_addr >> 8)  & 0xFF;
	addr[2] = (mycon->peer.sin_addr.s_addr >> 16) & 0xFF;
	addr[3] = (mycon->peer.sin_addr.s_addr >> 24) & 0xFF;
	return 1;
}

OS_API_C_FUNC(int) network_free()
{
	 return 0;
}
OS_API_C_FUNC(int) get_if(const char *gw_ip, struct string *name, struct string *ip)
{
	struct ifaddrs *addrs,*tmp;
	struct in_addr gw_addr;

	gw_addr.s_addr=inet_addr(gw_ip);

	getifaddrs(&addrs);
	tmp = addrs;
	while (tmp)
	{
		if (tmp->ifa_addr && tmp->ifa_netmask )
		{
			struct in_addr 	  gw_masked,my_masked,if_addr;
			
			printf("%s[%s]\n",tmp->ifa_name,inet_ntoa(((struct sockaddr_in *)tmp->ifa_addr)->sin_addr));

			if_addr				=	((struct sockaddr_in *)tmp->ifa_addr)->sin_addr;
			if(if_addr.s_addr==0)continue;
			gw_masked.s_addr	=	gw_addr.s_addr&((struct sockaddr_in *)(tmp->ifa_netmask))->sin_addr.s_addr;
			my_masked.s_addr	=	if_addr.s_addr&((struct sockaddr_in *)(tmp->ifa_netmask))->sin_addr.s_addr;

			if(my_masked.s_addr==gw_masked.s_addr)
			{
				make_string(name,tmp->ifa_name);
				make_string(ip,inet_ntoa(((struct sockaddr_in *)tmp->ifa_addr)->sin_addr));
				return 1;
			}
		}
	    tmp = tmp->ifa_next;
	}
	freeifaddrs(addrs);
	return 0;
}

int is_ip_self(const struct in_addr ip)
{
	struct ifaddrs *addrs,*tmp;
	int ret=0;

	getifaddrs(&addrs);
	tmp = addrs;
	while (tmp)
	{
		if (tmp->ifa_addr && tmp->ifa_netmask )
		{
			struct in_addr if_addr=	((struct sockaddr_in *)tmp->ifa_addr)->sin_addr;
			if(if_addr.s_addr==ip.s_addr){ret=1;break;}
		}
	    tmp = tmp->ifa_next;
	}
	freeifaddrs(addrs);
	return ret;
}

OS_API_C_FUNC(const struct string *)get_con_error(struct con *Con)
{
	return &Con->error;
}
OS_API_C_FUNC(struct string *)get_con_lastline(struct con *Con)
{
	return &Con->lastLine;
}
OS_API_C_FUNC(const struct host_def *)get_con_hostd(struct con *Con)
{
	return &Con->host;
}
OS_API_C_FUNC(int) con_consume_data(struct con *Con, size_t mov_len)
{
	if(Con->lastLine.str==NULL)return 0;
	if(Con->lastLine.len==0)return 0;

	if(mov_len>Con->lastLine.len)
		mov_len=Con->lastLine.len;

	Con->lastLine.len-=mov_len;
	if(Con->lastLine.len>0)
		memmove	(Con->lastLine.str,&Con->lastLine.str[mov_len],Con->lastLine.len);

	Con->lastLine.str[Con->lastLine.len]=0;
	return (int)mov_len;
}
OS_API_C_FUNC(int) con_move_data(struct con *Con, struct string *data, size_t mov_len)
{
	size_t		new_len,new_size;

	if(Con->lastLine.str==NULL)return 0;
	if(Con->lastLine.len==0)return 0;
	if(mov_len==0)return 0;

	if(mov_len>Con->lastLine.len)
		mov_len=Con->lastLine.len;

	new_len				=	data->len+mov_len;
	new_size			=	new_len+1;
	
	if((data->str!=NULL)&&(new_size>data->size))
	{
		data->size		=	new_size;
		data->str		=	realloc_c(data->str,data->size);
	}
	else
	{
		data->size		=	new_size;
		data->len		=	0;
		data->str		=	malloc_c(data->size);
	}
	memcpy	(&data->str[data->len],Con->lastLine.str,mov_len);

	data->len			 =	new_len;
	Con->lastLine.len	-=	mov_len;
	
	if(Con->lastLine.len>0)
		memmove	(Con->lastLine.str,&Con->lastLine.str[mov_len],Con->lastLine.len);

	Con->lastLine.str[Con->lastLine.len]=0;
	return (int)mov_len;
}
OS_API_C_FUNC(void) init_read_group()
{
	FD_ZERO(&my_read_group.read_set);
	memset(my_read_group.cons,0,sizeof(my_read_group.cons));
	my_read_group.max_sock=0;
}

OS_API_C_FUNC(int) read_group_has(const char *file)
{
	struct read_con		*rcon;
	for(rcon=my_read_group.cons;rcon->rd_con!=NULL;rcon++)
	{
		if(!strcmp(rcon->file_name.str,file))return 1;
	}
	return 0;
}

void add_read_done(const char *file_name)
{
	struct string		*file_done=	read_done;
	while(file_done->str!=NULL)file_done++;
	make_string(file_done,file_name);
}

OS_API_C_FUNC(int) pop_read_done(struct string *out)
{
	struct string		*file_done=	read_done;

	if(file_done->str==NULL)return 0;
	
	while(((file_done+1)->str)!=NULL)
		file_done++;

	clone_string(out,file_done);
	free_string	(file_done);
	return 1;
}


OS_API_C_FUNC(struct con	*)init_con()
{
	struct con	*newCon;
	newCon		=	malloc_c(sizeof(struct con));
	init_string(&newCon->error);
	init_string(&newCon->lastLine);
	init_string(&newCon->host.host);
	init_string(&newCon->host.port_str);
	newCon->sock=0;
	memset(&newCon->peer,0,sizeof(struct	sockaddr_in ));
	FD_ZERO(&newCon->con_set);
	return newCon;
}

OS_API_C_FUNC(void) con_close(struct con *Con)
{
	FD_ZERO			(&Con->con_set);
	if(Con->sock>0)
	{
		close			(Con->sock);
		Con->sock=0;
	}
	free_string		(&Con->error);
	free_string		(&Con->lastLine);
	free_host_def	(&Con->host);
	free_c			(Con);
}

OS_API_C_FUNC(char *)readline(struct con *Con, ctime_t timeout)
{
	fd_set			 fd_read,fd_err;
	char			 line[1024];
	size_t			 n_char;
	char			 c;
	//struct timeval   start_time,my_time;
	struct timeval	 stimeout;
	ctime_t			 s_time,m_time;

	if(Con->lastLine.str!=NULL)
		free_string(&Con->lastLine);

	if(Con->error.str!=NULL)
		free_string(&Con->error);

	memset_c			(line,0,1024);
	get_system_time_c	(&s_time);

	/*
    gettimeofday	(&start_time, NULL);
	s_time	=	start_time.tv_sec * 1000 + start_time.tv_usec/1000;
	*/

	n_char	=	0;
 	while(n_char<1023)
	{
		int n;
		
		FD_ZERO		(&fd_read);
		FD_SET		(Con->sock, &fd_read);

		fd_err			=	Con->con_set;
		stimeout.tv_sec	=	0;
		stimeout.tv_usec =	1000;

		if (select (Con->sock+1, &fd_read, NULL, &fd_err, &stimeout) < 0)
		{
			make_string(&Con->error,"select");
			return NULL;
		}
		if(FD_ISSET(Con->sock,&fd_err))
		{
			make_string(&Con->error,"error set");
			return NULL;
		}
		
		if(FD_ISSET(Con->sock,&fd_read))
		{
			n	=	recv	(Con->sock,&c,1, MSG_NOSIGNAL);
			if(n<=0)
			{
				if(n<0)
				{
					make_string (&Con->error,"neg read '");
					cat_ncstring(&Con->error,line,n_char);
					cat_cstring	(&Con->error,"' ");
					strcat_int  (&Con->error,n_char);
					cat_cstring	(&Con->error," ");

					close		(Con->sock);
					FD_ZERO		(&Con->con_set);
					Con->sock=0;
				}
				else
				{
					make_string (&Con->error,"zero read '");
					cat_ncstring(&Con->error,line,n_char);
					cat_cstring	(&Con->error,"' ");
					strcat_int  (&Con->error,n_char);
					cat_cstring	(&Con->error," ");
				}
				return NULL;
			}
			if(c==10)break;
			if(c!=13)line[n_char++]=c;
			
		}
		/*
		gettimeofday(&my_time, NULL);
		m_time	=	my_time.tv_sec * 1000 + my_time.tv_usec/1000;
		*/

		get_system_time_c(&m_time);
		if((m_time-s_time)>= timeout){
			make_string(&Con->error,"timeout");
			return NULL;
		}
	}
	if(n_char==0)
	{
		make_string(&Con->error,"empty line");
		return NULL;
	}

	line[n_char] = 0;
	make_string(&Con->lastLine,line);
	return Con->lastLine.str;
}


OS_API_C_FUNC(int) read_data(struct con *Con, size_t max)
{
	fd_set			read_fd_set,err_fd_set;
	size_t			read;
	struct timeval	timeout;
	if(Con->lastLine.str == NULL)
	{
		Con->lastLine.size	=	max+1;
		Con->lastLine.str	=	(char *)malloc_c(Con->lastLine.size);
		Con->lastLine.len	=	0;
	}
	else if(Con->lastLine.size<(Con->lastLine.len+max+1))
	{
		Con->lastLine.size	=	Con->lastLine.len+max+1;
		Con->lastLine.str	=	(char *)realloc_c(Con->lastLine.str,Con->lastLine.size);
	}
	
	read=0;

 	while(read<max)
	{
      	/* Block until input arrives on one or more active sockets. */
		timeout.tv_sec	= 0;
		timeout.tv_usec = 1000;

		read_fd_set = Con->con_set;
		err_fd_set	= Con->con_set;
      	if (select (Con->sock+1, &read_fd_set, NULL, &err_fd_set, &timeout) < 0)
      	{
      	    return 0;
      	}
		if (FD_ISSET (Con->sock, &err_fd_set)){Con->last_rd=0;break;}
		if (FD_ISSET (Con->sock, &read_fd_set))
		{
			Con->last_rd=	recv(Con->sock,&Con->lastLine.str[Con->lastLine.len],(int)(max-read), MSG_NOSIGNAL);
			if(Con->last_rd==0)
				break;
			read					+=	Con->last_rd;
			Con->lastLine.len		+=	Con->last_rd;
		}
	}
	return (int)read;
}


OS_API_C_FUNC(void) add_read_group(struct con *mycon, void *ffile, size_t transfer_len, const struct string *file_name)
{
	FILE				*file = (FILE *)ffile;
	struct read_con		*rcon;
	rcon	=	my_read_group.cons;
	
	while(rcon->rd_con!=NULL)rcon++;

	rcon->rd_con	=mycon;
	rcon->wr_file	=file;
	rcon->len		=transfer_len;
	clone_string	(&rcon->file_name,file_name);

	rcon->rd_con->lastLine.str	=	malloc_c(4*1024);

	FD_SET			(mycon->sock,&my_read_group.read_set);

	if(mycon->sock>my_read_group.max_sock)
		my_read_group.max_sock=mycon->sock;
}

OS_API_C_FUNC(void) do_read_group()
{
	struct timeval		timeout;
	struct read_con		*rcon;
	fd_set				read_fd_set,err_fd_set;
	size_t				rd;

	timeout.tv_sec	=	0;
	timeout.tv_usec =	100000;

	read_fd_set = my_read_group.read_set;
	err_fd_set	= my_read_group.read_set;

	if (select (my_read_group.max_sock+1, &read_fd_set, NULL, NULL, &timeout) < 0)
   	{
   		return ;
  	}
	for(rcon=my_read_group.cons;rcon->rd_con!=NULL;rcon++)
	{
		if(rcon->wr_file==NULL)continue;
		if(FD_ISSET(rcon->rd_con->sock,&read_fd_set))
		{
			int			to_read;
			to_read	=	rcon->len<4*1024?(int)rcon->len:4*1024;
			rd		=	recv(rcon->rd_con->sock,rcon->rd_con->lastLine.str,to_read,0);
			if(rd>0)
			{
				fwrite	 (rcon->rd_con->lastLine.str,rd,1,rcon->wr_file);
				rcon->len-=rd;
			}
			if((rd==0)||(rcon->len==0))
			{	
				struct read_con	*mvcon;

				FD_CLR		 (rcon->rd_con->sock,&my_read_group.read_set);
				fclose		 (rcon->wr_file);
				con_close	 (rcon->rd_con);

				add_read_done(rcon->file_name.str);
				free_string  (&rcon->file_name);
				mvcon		 =rcon;
				while((mvcon+1)->rd_con!=NULL)
				{
					*mvcon				=*(mvcon+1);
					mvcon++;
				}
				mvcon->rd_con=NULL;
				mvcon->wr_file=NULL;
				continue;
			}
			
		}
	}
}


OS_API_C_FUNC(int) send_data (struct con *Con,unsigned char *data,size_t len)
{
	size_t b_sent;
	int		s;

	free_string(&Con->error);

	b_sent=0;
	while(b_sent<len)
	{
		s	= send(Con->sock,&data[b_sent],len-b_sent, MSG_NOSIGNAL);
		if(s<=0)return s;
		b_sent+=s;
	}
	return b_sent;
}

OS_API_C_FUNC(int)get_con_ip(struct con *Con, ipv4_t ip)
{
	if (Con == PTR_NULL)return 0;

	ip[0] = Con->peer.sin_addr.s_addr & 0xFF;
	ip[1] = (Con->peer.sin_addr.s_addr >> 8) & 0xFF;
	ip[2] = (Con->peer.sin_addr.s_addr >> 16) & 0xFF;
	ip[3] = (Con->peer.sin_addr.s_addr >> 24) & 0xFF;

	return 1;
}

OS_API_C_FUNC(struct con *)do_get_incoming(struct con *listen_con, unsigned int time_out)
{
	fd_set		    my_listen,error;
	struct con		*newCon;
	struct timeval	timeout;
	unsigned int	clilen;
	int				new_sock;
	
	
	timeout.tv_sec	=	0;
	timeout.tv_usec =	time_out*1000;

	FD_ZERO(&my_listen);
	FD_SET (listen_con->sock,&my_listen);
	error	=	my_listen;
	
	if(select(listen_con->sock + 1, &my_listen, NULL, &error, &timeout)<0)
		return NULL;

	if (FD_ISSET(listen_con->sock, &my_listen))
	{
		newCon			=	init_con	();
		clilen			=	sizeof(struct sockaddr_in);
    	newCon->sock	=	accept(listen_con->sock, (struct sockaddr *)&newCon->peer, &clilen);
		if(newCon->sock<0)
		{
			make_string		(&newCon->error,"invalid socket");
			listen			(listen_con->sock,10);
		}
		else
		{
			char		*saddr;

			newCon->host.port = ntohs(newCon->peer.sin_port);
			saddr = inet_ntoa(newCon->peer.sin_addr);

			make_string_from_uint(&newCon->host.port_str, newCon->host.port);
			if (saddr != NULL)make_string(&newCon->host.host, saddr);
		}

		FD_SET(newCon->sock,&newCon->con_set);
		return newCon;
	}
	return NULL;
}

OS_API_C_FUNC(struct con	*)open_port(const char *my_addr, unsigned short port)
{
	struct con			*newCon;
	    int reuseaddr = 1; /* True */

	newCon			=	init_con	();

	make_string				(&newCon->host.host		,my_addr);
	make_string_from_uint	(&newCon->host.port_str	,port);

	newCon->host.port					=	port;
	newCon->sock						=	socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	/* fcntl				(newCon->sock, F_SETFL, O_NONBLOCK); */
    /* Enable the socket to reuse the address */
    if (setsockopt(newCon->sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1) {
        perror("setsockopt");
        return NULL;
    }
	/* setup address structure */
    memset	((char *) &newCon->peer, 0, sizeof(struct sockaddr_in));
    newCon->peer.sin_family				=	AF_INET;
    newCon->peer.sin_port				=	htons		(port);
	newCon->peer.sin_addr.s_addr		 =	INADDR_ANY; /*inet_addr	(newCon->host.host.str);*/
	if(bind	(newCon->sock, (struct sockaddr *)&newCon->peer,sizeof(struct sockaddr_in))<0)
	{
		make_string(&newCon->error,"bind error");
		return newCon;
	}
	/* Set up queue for incoming connections. */
	listen			(newCon->sock,10);
	/* Accept actual connection from the client */
	return newCon;
}


OS_API_C_FUNC(int) set_tcp_no_delay(struct con *mycon, int on)
{
	return 	setsockopt	(mycon->sock,IPPROTO_TCP,TCP_NODELAY,(char *) &on,sizeof(int));
}


OS_API_C_FUNC(int) reconnect(struct con *mycon)
{
	/*struct hostent		*iHost;*/
	struct addrinfo hints;
	struct addrinfo *res = NULL;
	int					iResult;
	free_string	(&mycon->lastLine);
	free_string	(&mycon->error);
	
	FD_ZERO		(&mycon->con_set);
	if(mycon->sock>0)
	{
		close	(mycon->sock);
		mycon->sock=0;
	}

	mycon->sock	= socket	 (AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(mycon->sock<=0){
		make_string(&mycon->error,"no socket");
		return 0;
	}
	
	memset_c(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow IPv4 */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_TCP;
	
	iResult = getaddrinfo (mycon->host.host.str,NULL,NULL,&res);
	
	if(iResult!=0){
		make_string(&mycon->error,"host not found");
		freeaddrinfo(res); 
		return 0;
	}
	mycon->peer.sin_addr	=((struct sockaddr_in *)res->ai_addr)->sin_addr;
	
	/*
	iHost = gethostbyname	(mycon->host.host.str);
	if(iHost==NULL){
		make_string(&mycon->error,"host not found");
		return 0;
	}
	mycon->peer.sin_addr.s_addr		= *((unsigned long*) iHost->h_addr);
	*/
	
    mycon->peer.sin_family			= AF_INET;
    freeaddrinfo					(res); 
    mycon->peer.sin_port		    = htons		 (mycon->host.port);
	iResult						    = connect	 (mycon->sock, (struct sockaddr *)&mycon->peer, sizeof(struct sockaddr_in));
	if(iResult!=0){
		make_string(&mycon->error, "connection error ");
		strcat_int(&mycon->error, iResult);
		return 0;
	}
   
	FD_SET(mycon->sock,&mycon->con_set);
	return 1;

}

OS_API_C_FUNC(struct con	*)do_connect(const struct host_def *host)
{
	struct con			*newCon;
	/*struct hostent	*iHost;*/
	struct addrinfo hints;
	struct addrinfo *res = NULL;
	unsigned int		flags;
	int					iResult;
	
	newCon				=	init_con	();
	
	newCon->host.port	=	host->port;
	clone_string			(&newCon->host.port_str	,&host->port_str);
	clone_string			(&newCon->host.host		,&host->host);
	
	log_output("resolving host\n");
	
	memset_c(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow IPv4  */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_TCP;
	
	iResult = getaddrinfo (newCon->host.host.str,NULL,NULL,&res);
	
	if(iResult!=0){
		make_string(&newCon->error,"host not found");
		freeaddrinfo(res); 
		return newCon;
	}
	
	newCon->peer.sin_addr	=((struct sockaddr_in *)res->ai_addr)->sin_addr;
	freeaddrinfo				(res); 
	
	if(is_ip_self(newCon->peer.sin_addr) )
	{
		make_string(&newCon->error,"connection to self skipped");
		return newCon;
	}

    /* Resolve the server address and port */
	newCon->sock					= socket	 (AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(newCon->sock<=0){
		make_string(&newCon->error,"no socket");
		return newCon;
	}

  	newCon->peer.sin_family		= AF_INET;
	newCon->peer.sin_port		= htons(newCon->host.port);

	flags = fcntl(newCon->sock, F_GETFL);
	if (fcntl(newCon->sock, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		make_string(&newCon->error, "fcntl error");
		return newCon;
	}


	log_output				 ("connecting host\n");
  	
	iResult	   = connect	 (newCon->sock, (struct sockaddr *)&newCon->peer, sizeof(struct sockaddr_in));

	if(iResult==-1)
	{
		fd_set			Write, Err;
		struct timeval	Timeout;
		int				TimeoutSec = 2; // timeout after 10 seconds
		int				error;

		if (errno != EINPROGRESS)
		{
			make_string(&newCon->error, "connection error");
			return newCon;
		}

		FD_ZERO	(&Write);
		FD_ZERO	(&Err);
		FD_SET	(newCon->sock, &Write);
		FD_SET	(newCon->sock, &Err);
		
		Timeout.tv_sec = TimeoutSec;
		Timeout.tv_usec = 0;

		error = select(FD_SETSIZE, NULL, &Write, &Err, &Timeout);
		if (error == -1)
		{
			make_string(&newCon->error, "connection error");
			return newCon;
		}
		else if ((FD_ISSET(newCon->sock, &Err)) || (!FD_ISSET(newCon->sock, &Write)))
		{
			make_string(&newCon->error, "connection timeout");
			return newCon;
		}
	}

	fcntl(newCon->sock, F_SETFL, flags);

	log_output("connected\n");
	FD_ZERO	(&newCon->con_set);
	FD_SET	(newCon->sock,&newCon->con_set);
	

	return newCon;
}

OS_API_C_FUNC(void) free_con_buffer(struct con *my_con)
{
	free_string(&my_con->error);
	free_string(&my_con->lastLine);
}


OS_API_C_FUNC(struct con	*)create_upnp_broadcast(struct host_def *host)
{
	struct con			*newCon;
	struct	sockaddr_in upnpControl;
	int					onOff;
	int					ret;

	newCon				=	init_con	();
	newCon->host.port	=	host->port;
	clone_string		(&newCon->host.port_str	,&host->port_str);
	clone_string		(&newCon->host.host		,&host->host);

	/*Resolve the server address and port*/
	newCon->sock= socket(AF_INET, SOCK_DGRAM, 0);
	if(newCon->sock<=0){
		make_string(&newCon->error,"no socket");
		return newCon;
	}

    upnpControl.sin_family			= AF_INET;
    upnpControl.sin_port			= htons(0);
    upnpControl.sin_addr.s_addr		= INADDR_ANY;
    
	ret								=	bind(newCon->sock, (struct sockaddr *)&upnpControl, sizeof(upnpControl));
	onOff							=	1;
	ret								=	setsockopt(newCon->sock, SOL_SOCKET, SO_BROADCAST,(char *)&onOff, sizeof(onOff));

	newCon->peer.sin_family			= AF_INET;
    newCon->peer.sin_port			= htons(newCon->host.port);
	newCon->peer.sin_addr.s_addr	= inet_addr(newCon->host.host.str);
	FD_SET							(newCon->sock,&newCon->con_set);
	return newCon;
}

OS_API_C_FUNC(int) send_upnpbroadcast(struct con *Con, struct string *data)
{
	int				ret,s;
	time_t			start_time,my_time;
	
	
	ret				=	setsockopt	(Con->sock,SOL_SOCKET, SO_BROADCAST, data->str, (int)data->len);
	s				=	sendto		(Con->sock,data->str,(int)data->len, 0, (struct sockaddr  *)&Con->peer, sizeof(Con->peer));
	start_time		=  time(0);
	
	
	while(((my_time=time(0))-start_time)<3)
	{
		fd_set				fd_read,fd_err;
		struct timeval		timeout;
		int					ret;

		prepare_new_data	(&Con->lastLine,512);
	
		timeout.tv_sec	=	0;
		timeout.tv_usec =	10000;

      	/* Block until input arrives on one or more active sockets. */
		fd_read = Con->con_set;
		fd_err	= Con->con_set;
		ret	=	select (Con->sock+1, &fd_read, NULL, &fd_err, &timeout);
      	if (ret < 0)
      	    return 0;

		if (FD_ISSET (Con->sock, &fd_err)){Con->last_rd=0;break;}
		if (FD_ISSET (Con->sock, &fd_read))
		{
			size_t bcLen		=	sizeof(Con->peer);
			Con->last_rd	=	recvfrom(Con->sock,&Con->lastLine.str[Con->lastLine.len],512,0, (struct sockaddr  *)&Con->peer, &bcLen);
			if(Con->last_rd>0)
			{
				Con->lastLine.len+=Con->last_rd;
				Con->lastLine.str[Con->lastLine.len]=0;
			}
		}
	}
	return 1;
}
