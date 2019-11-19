/*copyright iadix 2016*/

#define LIBC_API C_EXPORT
#include "../base/std_def.h"
#include "../base/std_mem.h"
#include "../base/mem_base.h"
#include "../base/std_str.h"
#include "strs.h"
#include "fsio.h"
#include "mem_stream.h"
#include <sys_include.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <utime.h>
#include <pwd.h>
#include <sys/file.h>

char	path_sep				= '/';
struct string log_file_name		= { PTR_INVALID,0,0 };
struct string lck_file_name		= { PTR_INVALID,0,0 };
struct string home_path			= { PTR_INVALID,0,0 };
struct string exe_path			= { PTR_INVALID,0,0 };
unsigned int running			= 1;
FILE	*log_file				= PTR_INVALID ;
int		lock_file				= 0xFFFFFFFF;
unsigned int log_lck = 0xFFFFFFFF;

struct thread
{
	thread_func_ptr		func;
	mem_zone_ref		params;
	mem_zone_ref		thread_data;
	unsigned int		status;
	unsigned int		tree_area_id;
	unsigned int		mem_area_id;
	pthread_t			thread;
	pid_t				h;
};

struct thread threads[MAX_THREADS+1] = { PTR_INVALID };

OS_API_C_FUNC(int) set_mem_exe(mem_zone_ref_ptr zone)
{
	mem_ptr				ptr;
	mem_size			size;
	int					ret;

	ptr = uint_to_mem(mem_to_uint(get_zone_ptr(zone, 0))&(~0xFFF));
	size = (get_zone_size(zone)&(~0xFFF)) + 4096;
	ret = mprotect(ptr,size , PROT_READ | PROT_EXEC | PROT_WRITE);
	
	/*printf("mprotect %X %d, ret %d\n", ptr, size,ret);*/

	return ret;
}

OS_API_C_FUNC(int) move_file(const char *ipath, const char *opath)
{
	return rename(ipath, opath);
}

OS_API_C_FUNC(int) del_file(const char *path)
{
	return unlink(path);
}

OS_API_C_FUNC(int) del_dir(const char *path)
{
	return rmdir(path);
}


void init_threads()
{
	memset_c(threads, 0, sizeof(threads));

}
unsigned int new_thread(pid_t h)
{
	unsigned int	n=0;
	while (n<MAX_THREADS)
	{
		if (compare_z_exchange_c((unsigned int *)&threads[n].h, (unsigned int)h))
			return n;

		n++;
	}
	return 0xFFFFFFFF;
}
unsigned int replace_thread_h(pid_t pold, pid_t pnew)
{
	int n = 0;

	while (n < MAX_THREADS)
	{
		if (pold == threads[n].h)
		{
			threads[n].h = pnew;
			return n;
		}
		n++;
	}
	return 0xFFFFFFFF;
}
unsigned int get_current_thread(pid_t h)
{
	int n = 0;

	while (n < MAX_THREADS)
	{
		if (h == threads[n].h)
			return n;
		n++;
	}
	return 0xFFFFFFFF;
}
pid_t gettid()
{
	return syscall(__NR_gettid);
}

OS_API_C_FUNC(int) set_mem_area_id(unsigned int area_id)
{
	pid_t  h;
	unsigned int cur;
	h = gettid();
	cur = get_current_thread(h);
	if (cur == 0xFFFFFFFF)
		cur = new_thread(h);

	
	threads[cur].mem_area_id = area_id;
	return 1;
}

OS_API_C_FUNC(int) set_tree_mem_area_id(unsigned int area_id)
{
	pid_t  h;
	unsigned int cur;
	h = gettid();
	cur = get_current_thread(h);
	if (cur == 0xFFFFFFFF)
		cur = new_thread(h);

	threads[cur].tree_area_id = area_id;
	return 1;
}

OS_API_C_FUNC(unsigned int) get_tree_mem_area_id()
{
	pid_t  h;
	unsigned int cur;
	h = gettid();
	cur = get_current_thread(h);
	if (cur == 0xFFFFFFFF)
		return cur;

	return threads[cur].tree_area_id;
}
int get_my_thread_flag(unsigned int *flag)
{
	pid_t  h;
	unsigned int cur;
	h = gettid();
	cur = get_current_thread(h);
	if (cur == 0xFFFFFFFF)
		cur = new_thread(h);

	(*flag) = 1 << cur;

	return 1;
}

OS_API_C_FUNC(unsigned int) get_mem_area_id()
{
	pid_t  h;
	unsigned int cur;
	h = gettid();
	cur = get_current_thread(h);
	if (cur == 0xFFFFFFFF)
		return cur;

	return threads[cur].mem_area_id;
}

OS_API_C_FUNC(unsigned int) get_thread_id()
{
	pid_t  h;
	unsigned int cur;
	h = gettid();
	cur = get_current_thread(h);
	return cur;
}

OS_API_C_FUNC(int) get_thread_data(mem_zone_ref_ptr thread_data)
{
	pid_t  h;
	unsigned int cur;
	h = gettid();
	cur = get_current_thread(h);
	if (cur == 0xFFFFFFFF)
		return 0;

	copy_zone_ref(thread_data, &threads[cur].thread_data);

	return 1;
}

OS_API_C_FUNC(int) set_thread_data(mem_zone_ref_const_ptr thread_data)
{
	pid_t  h;
	unsigned int cur;
	h = gettid();
	cur = get_current_thread(h);
	if (cur == 0xFFFFFFFF)
		return 0;

	copy_zone_ref(&threads[cur].thread_data, thread_data);

	return 1;
}
void *thread_start(void *p)
{
	thread_func_ptr		func;
	struct thread	    *thread;
	unsigned int		pn;
	int					ret;

	thread		= (struct thread *)p;
	thread->h	= gettid();
	func		= thread->func;

	init_default_mem_area(16 * 1024 * 1024, 8 * 1024);
	ret = func(&thread->params, &thread->status);
	free_mem_area(0);

	return NULL;
}
unsigned int next_ttid = 1;

OS_API_C_FUNC(int) background_func(thread_func_ptr func, mem_zone_ref_ptr params)
{
	unsigned int			cur;
	int						ret;

	cur = new_thread(next_ttid++);

	copy_zone_ref(&threads[cur].params, params);
	threads[cur].func = func;
	threads[cur].status = 0;

	ret = pthread_create		(&threads[cur].thread, NULL, thread_start, &threads[cur]);
	
	if (!ret)
	{
		while (threads[cur].status == 0)
		{
			struct timespec tim;

			tim.tv_sec = 0;
			tim.tv_nsec = 1000000L;

			nanosleep(&tim,NULL);
		}
	}

	release_zone_ref(&threads[cur].params);

	return (ret==0);
}
OS_API_C_FUNC(int) get_cwd(char *path, size_t len)
{
	if (getcwd(path, len))
		return 1;
	else
		return 0;
}
OS_API_C_FUNC(int) set_cwd(const char *path)
{
	return chdir(path);
}

OS_API_C_FUNC(int) set_exe_path()
{
	char path[512];
	get_cwd(path, 512);

	if (exe_path.str==PTR_INVALID)
		init_string(&exe_path);

	make_string(&exe_path, path);

	log_output("exe path : ");
	log_output(exe_path.str);
	log_output("\n");


	return 1;
}
OS_API_C_FUNC(int) get_exe_path(struct string *outPath)
{
	clone_string(outPath, &exe_path);
	return 1;
}


OS_API_C_FUNC(int) get_ftime(const char *path, ctime_t *time)
{
	struct stat statbuf;

	if (stat(path, &statbuf) < 0) {

		struct	string cpath = { 0 };
		int		ret;
		
		if (!make_string(&cpath, exe_path.str))
			return -1;

		cat_cstring_p(&cpath, path);
		ret = stat(cpath.str, &statbuf);
		free_string(&cpath);

		if (ret < 0)
			return 0;
	}
	*time = statbuf.st_mtime;
	return 1;
}

OS_API_C_FUNC(int) set_ftime(const char *path, ctime_t time)
{
	struct stat		statb;
	struct utimbuf	new_times;
	int	ret;
	
	new_times.actime = time;	 /* set atime to current time */
	new_times.modtime = time;    /* set mtime to current time */
	
	if (stat(path, &statb) == 0)
	{
		ret = utime(path, &new_times);
	}
	else
	{
		struct	string	cpath = { 0 };
		int		sret;

		if (!make_string(&cpath, exe_path.str))
			return 0;

		cat_cstring_p(&cpath, path);
		sret = stat(cpath.str, &statb);


		if (sret == 0)
			ret = utime(cpath.str, &new_times); 
		else
			ret = -1;

		free_string(&cpath);
	}

	if ( ret != 0)
	{
		log_output("utime failed ");
		log_output(path);
		log_output("\n");
		return 0;
	}

	return 1;
}




OS_API_C_FUNC(int) get_home_dir(struct string *path)
{
	const char *homedir;

	if ((homedir = getenv("HOME")) == NULL) {
		homedir = getpwuid(getuid())->pw_dir;
	}

	if (homedir == NULL)
		return 0;
	
	make_string(path, homedir);
	return 1;
}

OS_API_C_FUNC(int) set_data_dir(const struct string *path,const char *name)
{
	clone_string  (&home_path,path);	
	cat_cstring_p (&home_path, name);
	create_dir	  (home_path.str);
	set_cwd		  (home_path.str);

	log_output		("set data path : '");
	log_output		(home_path.str);
	log_output		("'\n");
	return 1;
}

OS_API_C_FUNC(int) set_home_path(const char *name)
{
	init_string	(&home_path);

	if (!get_home_dir(&home_path))return 0;
	cat_cstring_p	(&home_path, name);
	create_dir		(home_path.str);
	set_cwd			(home_path.str);
	return 1;

}
OS_API_C_FUNC(int) stat_file(const char *path)
{
	struct  stat	fileStat;
	struct string t = { 0 };
	int ret;

	ret = stat(path, &fileStat);
	if ((ret != 0) && (exe_path.len > 0))
	{
		clone_string  (&t, &exe_path);
		cat_cstring_p (&t, path);
		ret = stat	  (t.str, &fileStat);
		free_string	  (&t);
	}
	/*
	if (ret == 0)
	{
		if (fileStat.st_mode & S_IREAD)
			return	0;
		else
			return	1;
	}
	*/
	return ret;
}




OS_API_C_FUNC(int) create_dir(const char *path)
{
	return mkdir(path,0775);
}

OS_API_C_FUNC(int) get_sub_dirs(const char *path, struct string *dir_list)
{
	struct dirent *direntp;
	int ret=0;
    DIR *dirp;

	if ((dirp = opendir(path)) == NULL) 
	{
		return ret;
	}

	while ((direntp = readdir(dirp)) != NULL)
	{
	    /*
		struct stat s; 
		stat(direntp->d_name, &s);
		if (!(s.st_mode & S_IFDIR))continue;
		*/
		if(direntp->d_type!=DT_DIR)continue;
		if (direntp->d_name[0] == '.')continue;
		
		cat_cstring (dir_list,direntp->d_name);
		cat_cstring (dir_list,"\n");
		ret++;
		
	}

   closedir(dirp);
   return ret;
}

OS_API_C_FUNC(int) get_sub_files(const char *path, struct string *dir_list)
{
	struct dirent *direntp;
	int ret = 0;
	DIR *dirp;

	if ((dirp = opendir(path)) == NULL)
	{
		return ret;
	}

	while ((direntp = readdir(dirp)) != NULL)
	{
		if (direntp->d_type == DT_DIR)continue;
		cat_cstring(dir_list, direntp->d_name);
		cat_cstring(dir_list, "\n");
		ret++;
	}

	closedir(dirp);
	return ret;
}

OS_API_C_FUNC(int) rm_dir(const char *dir)
{
	char			mdir[512];
	struct string	dir_list = { PTR_NULL };
	const char		*ptr, *optr;
	unsigned int	dir_list_len;
	size_t			cur, nfiles;

	if ((nfiles = get_sub_files(dir, &dir_list)) > 0)
	{
		dir_list_len = dir_list.len;
		optr = dir_list.str;
		cur = 0;
		while (cur < nfiles)
		{
			size_t			sz;
			ptr = memchr_c(optr, 10, dir_list_len);
			sz = mem_sub(optr, ptr);

			strcpy_cs(mdir, 512, dir);
			strncat_c(mdir,&path_sep, 1);
			strncat_c(mdir,optr, sz);
			del_file(mdir);
			cur++;
			optr = ptr + 1;
			dir_list_len -= sz;
		}
	}
	free_string(&dir_list);
	return del_dir(dir);
}

OS_API_C_FUNC(int) put_file(const char *path, const void *data, size_t data_len)
{
	FILE		*f;
	size_t		len;
	
	f	=	fopen	(path,"wb");
	if(f==NULL)return 0;
	if (data_len > 0)
		len = fwrite(data, 1, data_len, f);

	fclose(f);
	return len;
}


OS_API_C_FUNC(int) get_file_range(const char *path, size_t ofset, size_t last, unsigned char **data, size_t *data_len)
{
	struct string	cpath = { PTR_NULL };
	FILE			*f;
	size_t			filesize, len=0;
	uint64_t		start, end;
	int				ret;

	f = fopen(path, "rb");
	if (f == NULL)
	{
		*data_len = 0;
		return -1;
	}

	fseek(f, 0, SEEK_END);
	filesize = ftell(f);

	start = mul64(ofset, 512);
	
	if (start<filesize)
	{
		end = mul64(last, 512);

		if (end > filesize)
			end = filesize;

		(*data_len) = end - start;
		(*data) = (unsigned char *)malloc_c((*data_len));
		if ((*data) != PTR_NULL)
		{
			int rd;
			fseek(f, start, SEEK_SET);
			rd = fread((*data), 1, (*data_len), f);
			if (rd>0)
				len = (*data_len);
			else
				len = 0;

		}
	}

	fclose(f);

	return len;
}

OS_API_C_FUNC(int) get_file_chunk(const char *path, size_t ofset, unsigned char **data, size_t *data_len)
{
	struct string	cpath = { PTR_NULL };
	FILE			*f;
	size_t			filesize, len = 0;
	int				ret;

	f = fopen(path, "rb");
	if (f == NULL)
	{
		*data_len = 0;
		return -1;
	}

	fseek(f, 0, SEEK_END);
	filesize = ftell(f);

	if ((ofset + 4) <= filesize)
	{
		unsigned int chunk_size;

		fseek(f, ofset, SEEK_SET);
		fread(&chunk_size, 1, 4, f);
		if (filesize >= (ofset + 4 + chunk_size))
		{
			(*data_len) = chunk_size;
			(*data) = malloc_c((*data_len) + 1);
			if ((*data) != PTR_NULL)
			{
				ret = fread((*data), 1, (*data_len), f);
				if (ret>0)
					len = (*data_len);
				else
					len = 0;

				(*data)[*data_len] = 0;
			}
		}
	}

	fclose(f);

	return len;
}

OS_API_C_FUNC(int) append_file(const char *path, const void *data, size_t data_len)
{
	FILE		*f;
	int			ret,len;
	f = fopen(path, "ab+");

	if (f == NULL)
		return 0;

	fseek(f, 0, SEEK_END);
	len = fwrite(data, 1, data_len, f);
	fflush(f);
	fclose(f);
	
	if (len > 0)
		return data_len;
	else
		return 0;
}

OS_API_C_FUNC(int) truncate_file(const char *path, size_t ofset, const void *data, size_t data_len)
{
	FILE		*f;
	size_t		len;
	uint64_t	offset;
	int			ret;

	if ((ofset == 0) && (data_len == 0))
	{
		del_file(path);
		return 1;
	}

	ret = truncate(path, ofset);

	if (ret != 0)
		return 0;

	if ((data != PTR_NULL)&&(data_len>0))
	{
		f = fopen(path, "ab+");
		if (f != NULL)
		{
			fseek(f, 0, SEEK_END);
			len = fwrite(data, data_len, 1, f);
			ret = (len > 0) ? 1 : 0;

			fflush(f);
			fclose(f);
		}
		else
			ret = -1;
	}

	return ret;
}


OS_API_C_FUNC(size_t) file_size(const char *path)
{
	FILE		*f;
	size_t		len;
	int			ret;
	f = fopen( path, "ab+");
	if (f == NULL)return 0;
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fclose(f);
	return len;
}
OS_API_C_FUNC(int) get_file_to_memstream(const char *path, mem_stream *stream)
{
	struct string	cpath = { PTR_NULL };
	mem_zone_ref	fileMem = { PTR_NULL };
	FILE			*f;
	size_t			len = 0, data_len;
	int				ret;

	f = fopen(path, "rb");
	if (f == NULL)
	{
		init_string(&cpath);
		if (!make_string(&cpath, exe_path.str))
			return -1;

		cat_cstring_p(&cpath, path);
		f = fopen(cpath.str, "rb");
		free_string(&cpath);

		if (f == NULL)
			return -1;
	}

	fseek(f, 0, SEEK_END);
	data_len = ftell(f);
	rewind(f);
	if (data_len>0)
	{
		unsigned char *data;
		allocate_new_zone(0, data_len+1, &fileMem);
		data = (unsigned char *)get_zone_ptr(&fileMem, 0);
		if (data != PTR_NULL)
		{
			len = fread(data, data_len, 1, f);
			data[data_len] = 0;
			mem_stream_init(stream, &fileMem, 0);
		}
		else
			len = 0;
	}
	fclose(f);
	return (int)len;
}
OS_API_C_FUNC(int) get_file(const char *path, mem_ptr *data, size_t *data_len)
{
	struct string	cpath = { PTR_NULL };
	FILE			*f;
	size_t			len = 0;
	int				ret;
		
	f = fopen(path, "rb");
	if (f == NULL)
	{
		init_string		(&cpath);
		if (!make_string(&cpath, exe_path.str))
			return -1;

		cat_cstring_p	(&cpath, path);
		f = fopen		(cpath.str, "rb");
		free_string		(&cpath);

		if (f == NULL)
		{
			*data_len = 0;
			return -1;
		}
	}
	
	fseek(f, 0, SEEK_END);
	(*data_len) = ftell(f);
	rewind(f);
	if ((*data_len)>0)
	{
		(*data) = malloc_c((*data_len) + 1);
		if ((*data) != PTR_NULL)
		{
			ret = fread((*data),  1,*data_len, f);
			if(ret>0)
				len = *data_len;
			else
				len = 0;

			((char *)(*data))[*data_len] = 0;
		}
		else
			len = 0;
	}
	fclose(f);
	return (int)len;

}



OS_API_C_FUNC(int) get_file_len(const char *path, size_t size, unsigned char **data, size_t *data_len)
{
		struct string	cpath = { PTR_NULL };
	FILE			*f;
	size_t			len = 0;
	int				ret;
		
	f = fopen(path, "rb");
	if (f == NULL)
	{
		init_string		(&cpath);
		if (!make_string(&cpath, exe_path.str))
			return -1;

		cat_cstring_p	(&cpath, path);
		f = fopen		(cpath.str, "rb");
		free_string		(&cpath);

		if (f == NULL)
		{
			*data_len = 0;
			return -1;
		}
	}
	
	fseek(f, 0, SEEK_END);
	(*data_len) = ftell(f);
	rewind(f);
	if ((*data_len)>0)
	{
		if((*data_len)>size)
			(*data_len)=size;

		(*data) = malloc_c((*data_len) + 1);
		if ((*data) != PTR_NULL)
		{
			ret = fread((*data), 1, *data_len, f);

			if(ret>0)
				len = *data_len;
			else
				len = 0;

			(*data)[*data_len] = 0;
		}
		else
			len = 0;
	}
	fclose(f);
	return (int)len;
}

OS_API_C_FUNC(void *) kernel_memory_map_c(unsigned int size)
{
	return malloc(size);
}
OS_API_C_FUNC(int) kernel_memory_free_c(mem_ptr ptr)
{
	free(ptr);
	return 1;
}


OS_API_C_FUNC(void) get_system_time_c(ctime_t *ms)
{
	struct			timespec spec;
	clock_gettime	(CLOCK_REALTIME, &spec);
	*ms = spec.tv_sec * 1000 + spec.tv_nsec / 1000000; /* Convert nanoseconds to milliseconds */
}



OS_API_C_FUNC(ctime_t)	 get_time_c()
{
	return time(0);
}




OS_API_C_FUNC(int) aquire_lock_file(const char *name)
{
	init_string(&lck_file_name);
	make_string(&lck_file_name, name);
	cat_cstring(&lck_file_name, ".pid");

	if ((lock_file = open(lck_file_name.str, O_CREAT | O_RDWR, 0666)) < 0) {
		console_print("could not open lock file\n");
		return 0;
	}
	if (flock(lock_file, LOCK_EX | LOCK_NB) < 0) {
		console_print("could not aquire lock file\n");
		close(lock_file);
		return 0;
	}

	return 1;
}



 OS_API_C_FUNC(int) daemonize(const char *name)
{
   char		    pidbuff[16];
   pid_t		pid, sid;
   pid_t		tid, ttid;
   size_t		pbsz;

   umask			(0);
   init_string		(&log_file_name);
   make_string		(&log_file_name, name);
   cat_cstring		(&log_file_name, ".log");
   
   if (log_output("init log\n") < 0)
	   return -1;
      
   /* Fork off the parent process  */
   tid = gettid();
   pid = fork();
   if (pid < 0) {
       return -1;
   }
   /* If we got a good PID, then we can exit the parent process. */
   if (pid > 0) {
	    printf	("forked %d \n", pid);
		exit	(EXIT_SUCCESS);
   }

   if (log_output("new process\n") < 0)
	   return -1;
   pid  = getpid	();
   ttid = gettid	();
   replace_thread_h (tid, ttid);

   /* Change the file mode mask */
   umask			(0);
   
   if (log_output("umask \n") < 0)
	   return -1;
   /* Open any logs here */
   set_cwd		(home_path.str);

   /* Create a new SID for the child process */
   sid = setsid();
   if (sid < 0) {
	   /* Log any failures here */
	   log_output( "sid failed");
	   return -1;
   }



   uitoa_s			(pid, pidbuff, 16, 10);
   pbsz				= strlen_c(pidbuff);
   pidbuff[pbsz++]	= 10;

   write			(lock_file, pidbuff, pbsz);
   
   return 1;
}

OS_API_C_FUNC(void) console_print(const char *msg)
{
	unsigned int tid = get_thread_id();

	if (tid != log_lck)
	{
		while (!compare_z_exchange_c(&log_lck, tid)) {}
	}

	printf("%s",msg);

	if (strlpos_c(msg, 0, '\n') != INVALID_SIZE)
		log_lck = 0;
}

OS_API_C_FUNC(int) log_output(const char *data)
{
	if ((log_file_name.str != PTR_NULL) && (log_file_name.str != PTR_INVALID))
	{
		int ret;
		unsigned int tid = get_thread_id();

		if (tid != log_lck)
		{
			while (!compare_z_exchange_c(&log_lck, tid)) {}
		}

		ret = append_file(log_file_name.str, data, strlen_c(data));

		if (strlpos_c(data, 0, '\n') != INVALID_SIZE)
			log_lck = 0;

		return ret;
	}

	console_print(data);
	return 1;
	
	
	
}

int extractDate(const char * s){
	unsigned int	y, m, d;
	struct tm t;

	y = strtoul(s, PTR_NULL, 10);
	m = strtoul(&s[5], PTR_NULL, 10);
	d = strtoul(&s[8], PTR_NULL, 10);

	memset_c(&t, 0, sizeof(struct tm));

	t.tm_mday = d;
	t.tm_mon = m - 1;
	t.tm_year = y - 1900;
	t.tm_isdst = -1;

	/* normalize: */
	return mktime(&t);
}
OS_API_C_FUNC(unsigned int) parseDate(const char *date)
{
	int			n;
	n = strlen_c(date);
	if (n < 10)return 0;

	return extractDate(date);
}
OS_API_C_FUNC(int) default_RNG(uint8_t *dest, unsigned size) 
{
	int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	if (fd == -1) {
		fd = open("/dev/random", O_RDONLY | O_CLOEXEC);
		if (fd == -1) {
			return 0;
		}
	}

	char *ptr = (char *)dest;
	size_t left = size;
	while (left > 0) {
		ssize_t bytes_read = read(fd, ptr, left);
		if (bytes_read <= 0) { /* read failed */
			close(fd);
			return 0;
		}
		left -= bytes_read;
		ptr += bytes_read;
	}

	close(fd);
	return 1;
}

OS_API_C_FUNC(int)  time_to_date(ctime_t time, char *date, size_t date_len)
{
	struct tm  ts;
	ts = *localtime(&time);
	return strftime(date, date_len, "%Y-%m-%d", &ts);
}


OS_API_C_FUNC(unsigned int) isRunning()
{
	return running;
}
void init_exit()
{
}
OS_API_C_FUNC(void) strtod_c(const char *str, double *d)
{
	*d = strtod(str, NULL);
}
OS_API_C_FUNC(void) snooze_c(size_t n)
{
	struct timespec tim;

	tim.tv_sec = 0;
	tim.tv_nsec = n;

	nanosleep(&tim, NULL);
}

/*
void exited(void)
{
	running = 0;
	return PTR_NULL;
}
void init_exit()
{
	atexit(exited);
}*/
