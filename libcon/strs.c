/* copyright iadix 2016 */
#define LIBC_API C_EXPORT
#include "base/std_def.h"
#include "base/std_mem.h"
#include "base/mem_base.h"
#include "base/std_str.h"
#include "strs.h"

extern char path_sep;

OS_API_C_FUNC(char *) my_strrev(char *str)
{
    size_t i = strlen_c(str)-1,j=0;
    char ch;
    while(i>j)
    {
        ch = str[i];
        str[i]= str[j];
        str[j] = ch;
        i--;
        j++;
    }
    return str;
}

OS_API_C_FUNC(void) init_string(struct string *str)
{
	str->size		=0;
	str->len		=0;
	str->str		=NULL;
}

OS_API_C_FUNC(int) clone_string(struct string *str, const struct string *str1)
{
	str->len	=	str1->len;
	str->size	=	str->len+1;
	str->str	=	malloc_c(str->size);
	if (str->str == PTR_NULL)return 0;
	memcpy_c	(str->str,str1->str,str->size);

	return 1;
}

OS_API_C_FUNC(int) make_string(struct string *str, const char *toto)
{
	str->len			=	strlen_c(toto);
	if((str->str!=PTR_NULL)&&(str->str!=PTR_INVALID)&&(str->size>0))
		free_c(str->str);
	str->size			=	str->len+1;
	str->str			=	malloc_c(str->size);
	if (str->str == PTR_NULL)return 0;
	memcpy_c				(str->str,toto,str->len+1);
	return 1;
}

OS_API_C_FUNC(int) make_string_l(struct string *str, const char *toto, size_t len)
{
	str->len			=	len;
	if(str->str!=NULL)free_c(str->str);
	str->size			=	str->len+1;
	str->str			=	malloc_c(str->size);
	memcpy_c				(str->str,toto,str->len);
	str->str[str->len]	=	0;
	return 1;
}

OS_API_C_FUNC(int) make_string_from_url(struct string *str, const char *toto, size_t len)
{
	size_t	n = 0;
	int		n_o = 0;

	str->len = len;
	if (str->str != NULL)free_c(str->str);
	str->size = str->len + 1;
	str->str = malloc_c(str->size);
	
	while (n<len)
	{
		if (toto[n] == '%')
		{
			char hex[3];

			hex[0] = toto[n + 1];
			hex[1] = toto[n + 2];
			hex[2] = 0;
			str->str[n_o++] = strtol_c(hex, NULL, 16);
			n += 3;
		}
		else
			str->str[n_o++] = toto[n++];
	}
	str->str[n_o] = 0;
	return n_o;
}

OS_API_C_FUNC(int) cat_cstring(struct string *str, const char *src)
{
	size_t		new_len,src_len;

	src_len				=	strlen_c(src);
	if(src_len==0)return (int)str->len;

	new_len				=	str->len+src_len;
	str->size			=	new_len+1;
	if(str->str!=NULL)
		str->str=realloc_c(str->str,str->size);
	else
	{
		str->len=0;
		str->str=malloc_c(str->size);
	}

	memcpy_c	(&str->str[str->len],src,src_len+1);
	str->len = new_len;
	return (int)str->len;
}


OS_API_C_FUNC(int) cat_cstring_p(struct string *str, const char *src)
{
	size_t		new_len, src_len;

	src_len = strlen_c(src);
	if (src_len == 0)return (int)str->len;

	new_len = str->len + src_len+1;
	str->size = new_len + 1;

	if ((str->str != NULL) && (str->str != PTR_INVALID))
		str->str = realloc_c(str->str, str->size);
	else
	{
		str->len = 0;
		str->str = malloc_c(str->size);
	}

	str->str[str->len] = path_sep;
	memcpy_c(&str->str[str->len+1], src, src_len + 1);
	str->len = new_len;
	return (int)str->len;
}

OS_API_C_FUNC(int) cat_ncstring(struct string *str, const char *src, size_t src_len)
{
	size_t new_len,cpy_len,n;

	if (src_len == 0)return str->len;

	cpy_len = 0;
	n		= 0;
	while (n<src_len)
	{
		if (src[n++] == 0)break;
		cpy_len++;
	}
	if (cpy_len == 0)return 0;
	
	new_len				=	str->len+cpy_len;
	str->size			=	new_len+1;
	if(str->str!=NULL)
		str->str=realloc_c(str->str,str->size);
	else
		str->str=malloc_c(str->size);

	memcpy_c	(&str->str[str->len],src,cpy_len);
	str->len			= new_len;
	str->str[str->len]	= 0;
	return (int)str->len;
}

OS_API_C_FUNC(int) cat_ncstring_p(struct string *str, const char *src, size_t src_len)
{
	size_t		new_len;

	new_len = str->len + 1+src_len;
	str->size = new_len + 1;
	if (str->str != NULL)
		str->str = realloc_c(str->str, str->size);
	else
		str->str = malloc_c(str->size);

	if (str->str == PTR_NULL)
		return 0;

	str->str[str->len] = path_sep;
	memcpy_c(&str->str[str->len+1], src, src_len);
	str->len = new_len;
	str->str[str->len] = 0;
	return (int)str->len;
}


OS_API_C_FUNC(size_t) cat_string(struct string *str, const struct string *src)
{
	size_t new_len, cpy_len, n;

	if(src->len==0)return str->len;

	cpy_len = 0;
	n = 0;

	while (n<src->len)
	{
		if (src->str[n++] == 0)break;
		cpy_len++;
	}
	if (cpy_len == 0)return 0;

	new_len				=	str->len+cpy_len;
	str->size			=	new_len+1;

	if(str->str!=NULL)
		str->str=realloc_c(str->str,str->size);
	else
		str->str=malloc_c	(str->size);

	memcpy_c	(&str->str[str->len],src->str,cpy_len);
	str->len = new_len;
	str->str[str->len]=0;
	return (int)str->len;
}
OS_API_C_FUNC(int) prepare_new_data(struct string *str, size_t len)
{
	size_t		new_len,new_size;

	new_len				=	str->len+len;
	new_size			=	new_len+1;
	
	if((str->str!=NULL)&&(new_size>str->size))
	{
		str->size	=new_size;
		str->str	=realloc_c(str->str,str->size);
	}
	else if(str->str==NULL)
	{
		str->size	=new_size;
		str->str=malloc_c(str->size);
	}
	return (int)str->len;
}

OS_API_C_FUNC(int) str_end_with(const struct string *str, const char *end)
{
	size_t		endLen,strOfs;
	

	endLen = strlen_c(end);

	if (str->len < endLen)
		return 0;

	strOfs = str->len - endLen;

	while (endLen--)
	{
		if (str->str[strOfs+ endLen] != end[endLen])
			return 0;
	}
	return 1;
}

OS_API_C_FUNC(int) str_start_with(const struct string *str, const char *start)
{
	size_t		n=0;

	if (str->len < strlen_c(start))return 0;

	while ((n<str->len)&&(start[n]!=0))
	{
		if (str->str[n] != start[n])
			return 0;
	
		n++;
	}
	return 1;
}

OS_API_C_FUNC(int) vstr_to_str(mem_ptr data_ptr, struct string *str)
{
	if (*((unsigned char *)(data_ptr)) < 0xFD)
	{
		str->len = *((unsigned char *)(data_ptr));
		str->str = mem_add(data_ptr, 1);
	}
	else if (*((unsigned char *)(data_ptr)) == 0xFD)
	{
		str->len = *((unsigned short *)(mem_add(data_ptr, 1)));
		str->str = mem_add(data_ptr, 3);
	}
	else if (*((unsigned char *)(data_ptr)) == 0xFE)
	{
		str->len = *((unsigned int *)(mem_add(data_ptr, 1)));
		str->str = mem_add(data_ptr, 5);
	}
	else if (*((unsigned char *)(data_ptr)) == 0xFF)
	{
		str->len = *((uint64_t *)(mem_add(data_ptr, 1)));
		str->str = mem_add(data_ptr, 9);
	}

	str->size			= str->len + 1;
	str->str[str->len]	= 0;

	return 1;
}

OS_API_C_FUNC(int) strcat_uint(struct string *str, size_t i)
{
	size_t		new_len,src_len;
	char		buff[32];

	uitoa_s				(i,buff,32,16);

	src_len				=	strlen_c(buff);
	new_len				=	str->len+src_len;

	str->size			=	new_len+1;
	if(str->str!=NULL)
		str->str=realloc_c(str->str,str->size);
	else
		str->str=malloc_c(str->size);

	memcpy_c	(&str->str[str->len],buff,src_len+1);
	str->len = new_len;
	return (int)str->len;
}
/*
OS_API_C_FUNC(int) strcat_float(struct string *str, double f)
{
	size_t		new_len,src_len;
	char		buff[32];

	dtoa_c				(buff,'f',32,10,f);

	src_len				=	strlen_c(buff);
	new_len				=	str->len+src_len;

	str->size			=	new_len+1;
	if(str->str!=NULL)
		str->str=realloc_c(str->str,str->size);
	else
		str->str=malloc_c(str->size);

	memcpy_c	(&str->str[str->len],buff,src_len+1);
	str->len = new_len;
	return (int)str->len;
}
*/
OS_API_C_FUNC(int) strcat_int(struct string *str, int i)
{
	size_t		new_len,src_len;
	char		buff[32];

	itoa_s				(i,buff,32,10);

	src_len				=	strlen_c(buff);
	new_len				=	str->len+src_len;

	str->size			=	new_len+1;
	if(str->str!=NULL)
		str->str=realloc_c(str->str,str->size);
	else
		str->str=malloc_c(str->size);

	memcpy_c	(&str->str[str->len],buff,src_len+1);
	str->len = new_len;
	return (int)str->len;
}

OS_API_C_FUNC(int) make_string_url(struct string *str, const char *toto, size_t len)
{
	size_t 	n,n_char;
	size_t	new_len;
	new_len=0;
	n=0;
	while(n<len)
	{
		if((toto[n]==' ')||(toto[n]=='+'))
			new_len+=3;
		else
			new_len++;
		n++;
	}

	str->len			=	new_len;
	if(str->str!=NULL)free_c(str->str);

	str->size			=	new_len+1;
	str->str			=	malloc_c(str->size);

	n=0;
	n_char=0;
	while(n<len)
	{
		if(toto[n]==' ')
		{
			str->str[n_char++]='%';
			str->str[n_char++]='2';
			str->str[n_char++]='0';
		}
		else if(toto[n]=='+')
		{
			str->str[n_char++]='%';
			str->str[n_char++]='2';
			str->str[n_char++]='B';
		}
	
		else
		{
			str->str[n_char]=toto[n];
			n_char++;
		}
		n++;
	}
	str->str[n_char]	=	0;
	return 1;
}

OS_API_C_FUNC(int) make_string_from_uint(struct string *str, size_t i)
{
	char		 int_str[32]={0};

	uitoa_s				(i,int_str,32,10);
	str->len			=	strlen_c(int_str);

	if(str->str!=NULL)free_c(str->str);

	str->size			=	str->len+1;
	str->str			=	malloc_c(str->size);
	memcpy_c				(str->str,int_str,str->len+1);

	return 1;
}

OS_API_C_FUNC(int) make_cstring(const struct string *str, char *toto, size_t len)
{
	size_t	n	=0;
	int		n_o	=0;
	while(n<str->len)
	{
		if(str->str[n]=='%')
		{
			char hex[3];

			hex[0]=str->str[n+1];
			hex[1]=str->str[n+2];
			hex[2]=0;

			toto[n_o++]=strtol_c(hex,NULL,16);
			n+=3;
		}
		else
			toto[n_o++]=str->str[n++];
	}
	toto[n_o]=0;
	return n_o;
}


OS_API_C_FUNC(void) free_string(struct string *str)
{
	if(str->str!=NULL){free_c(str->str);}
	str->str=NULL;
	str->len=0;
	str->size=0;
}




OS_API_C_FUNC(void) cat_tag(struct string *str, const char *tag, const char *val)
{
	cat_cstring		(str,"<");
	cat_cstring		(str,tag);
	cat_cstring		(str,">");
	cat_cstring		(str,val);
	cat_cstring		(str,"</");
	cat_cstring		(str,tag);
	cat_cstring		(str,">");
}
OS_API_C_FUNC(struct host_def *)make_host_def(const char *host, unsigned short port)
{
	const char *ptr;
	struct host_def *new_host;
	
	
	new_host				=	malloc_c(sizeof(struct host_def));

	init_string				(&new_host->port_str);
	init_string				(&new_host->host);
	
	new_host->port			=	port;
	make_string_from_uint	(&new_host->port_str	,new_host->port);
	make_string				(&new_host->host		,host);

	ptr					=	memchr_c(new_host->host.str,'/',new_host->host.len);
	if(ptr!=NULL)
	{
		new_host->host.len=ptr-new_host->host.str;
		new_host->host.str[new_host->host.len]=0;
	}

	return new_host;
}
OS_API_C_FUNC(struct host_def *)make_host_def_url(const struct string *url, struct string *path)
{
	struct host_def			*new_host;
	const char	*port_ptr,*url_ptr,*path_ptr;
	size_t			len;
		
	new_host				=	malloc_c(sizeof(struct host_def));

	init_string				(&new_host->port_str);
	init_string				(&new_host->host);

	url_ptr					=	memchr_c(url->str,':',url->len);
	url_ptr					+=3;
	len						=	strlen_c(url_ptr);
	path_ptr				=	memchr_c(url_ptr,'/',len);

	if(path!=NULL)
		make_string	(path,path_ptr);

	port_ptr				=	memchr_c(url_ptr,':',len);
	if(port_ptr!=NULL)
	{
		make_string_l	(&new_host->port_str	,port_ptr+1,path_ptr-(port_ptr+1));
		make_string_l	(&new_host->host		,url_ptr,port_ptr-url_ptr);
		new_host->port			=	strtol_c(new_host->port_str.str,NULL,10);
	}
	else
	{
		new_host->port			=	80;
		make_string_from_uint	(&new_host->port_str	,new_host->port);
		make_string_l			(&new_host->host,url_ptr,path_ptr-(url_ptr));
	}
	return new_host;
}
OS_API_C_FUNC(void )copy_host_def(struct host_def *dhost, const struct host_def *host)
{
	clone_string	(&dhost->host,&host->host);
	clone_string	(&dhost->port_str,&host->port_str);
	dhost->port	=	host->port;
}

OS_API_C_FUNC(void) free_host_def(struct host_def *host)
{
	free_string (&host->port_str);
	free_string (&host->host);
}



OS_API_C_FUNC(int) find_mem_hash(hash_t hash, unsigned char *mem_hash, unsigned int num)
{
	unsigned int n = 0;
	if (num == 0)return 0;
	if (mem_hash == PTR_NULL)return 0;
	while (n<(num * 32))
	{
		if (!memcmp_c(&mem_hash[n], hash, sizeof(hash_t)))
			return 1;
		n += 32;
	}
	return 0;
}
