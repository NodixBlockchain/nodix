//copyright antoine bentue-ferrer 2016
#define LIBC_API C_EXPORT


#include "../base/std_def.h"
#include "../base/std_mem.h"
#include "../base/mem_base.h"
#include "../base/std_str.h"
#include "strs.h"
#include "fsio.h"
#include "mem_stream.h"

#include <sys_include.h>
#include <time.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>



#include <shlwapi.h>
#include "shlobj.h"

char path_sep='\\';
struct string log_file_name = { PTR_INVALID,0,0};
struct string lck_file_name = { PTR_INVALID,0,0 };
struct string home_path		= { PTR_INVALID,0,0};
struct string exe_path		= { PTR_INVALID,0,0};
HANDLE		  lockFile		= PTR_INVALID;
unsigned int running		= 1;
struct tm tmtime			= { 0xCD };


struct thread
{
	thread_func_ptr		func;
	mem_zone_ref		params;
	mem_ptr				stack;
	unsigned int		status;
	unsigned int		tree_area_id;
	unsigned int		mem_area_id;
	DWORD				h;
	HANDLE				th;
};


struct thread threads[MAX_THREADS+1] = { PTR_INVALID };


OS_API_C_FUNC(int) set_mem_exe(mem_zone_ref_ptr zone)
{
	unsigned int	old;
	mem_ptr				ptr;
	mem_size			size;

	ptr = uint_to_mem(mem_to_uint(get_zone_ptr(zone, 0))&(~0xFFF));
	size = (get_zone_size(zone)&(~0xFFF)) + 4096*2;
	return VirtualProtect(get_zone_ptr(zone, 0), get_zone_size(zone), PAGE_EXECUTE_READWRITE, &old);
}

OS_API_C_FUNC(int) stat_file(const char *path)
{
	struct string t = { 0 };
	int ret;

	ret = PathFileExists(path) ? 0 : -1;
	if ((ret!=0) && (exe_path.len > 0))
	{
		clone_string(&t, &exe_path);
		cat_cstring_p(&t, path);
		ret = PathFileExists(t.str) ? 0 : -1;
		free_string(&t);
	}
	return ret;
}

OS_API_C_FUNC(int) del_dir(const char *path)
{
	return RemoveDirectory(path);
}
OS_API_C_FUNC(int) create_dir(const char *path)
{
	return CreateDirectory(path, NULL);
}
#define TICKS_PER_SECOND 10000000
#define EPOCH_DIFFERENCE 11644473600LL

void UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;
}

ctime_t FileTime_to_POSIX(FILETIME ft)
{
	struct big64 t;
	uint64_t ti;

	t.m.v[0] = ft.dwLowDateTime;
	t.m.v[1] = ft.dwHighDateTime;
	ti = muldiv64(t.m.v64 - 116444736000000000, 1, 10000000);
	return (ti );
}
ctime_t FileTime_to_Milli(FILETIME ft)
{
	struct big64 t;
	uint64_t ti;

	t.m.v[0] = ft.dwLowDateTime;
	t.m.v[1] = ft.dwHighDateTime;
	ti = muldiv64(t.m.v64 - 116444736000000000, 1, 10000);
	return (ti);
}

OS_API_C_FUNC(int) set_ftime(const char *path,ctime_t time)
{
	int ret;
	HANDLE hFile;
	FILETIME ft;
	if ((hFile = CreateFile(path, FILE_WRITE_ATTRIBUTES, 0, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0)) == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	UnixTimeToFileTime(time, &ft);

	ret = SetFileTime(hFile, &ft, &ft, &ft);
	CloseHandle(hFile);
	return ret;
}
OS_API_C_FUNC(int) get_ftime(const char *path, ctime_t *time)
{
	int ret;
	HANDLE hFile;
	FILETIME ft;
	if ((hFile = CreateFile(path, FILE_READ_ATTRIBUTES, 0, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0)) == INVALID_HANDLE_VALUE)
		return 0;

	ret = GetFileTime(hFile, &ft, NULL, NULL);
	CloseHandle(hFile);
	
	if (ret)
		*time = FileTime_to_POSIX(ft);

	return ret;
}
OS_API_C_FUNC(size_t) file_size(const char *path)
{
	size_t size;
	HANDLE h;
	if ((h = CreateFile(path, FILE_WRITE_ATTRIBUTES, 0, 0, OPEN_EXISTING, 0, 0)) == INVALID_HANDLE_VALUE)
		return 0;

	size = GetFileSize(h, NULL);
	CloseHandle(h);

	return size;
}


OS_API_C_FUNC(int) del_file(const char *path)
{
	return DeleteFile(path);
}

OS_API_C_FUNC(int) move_file(const char *ipath,const char *opath)
{
	return MoveFile(ipath,opath);
}
OS_API_C_FUNC(int) get_sub_dirs(const char *path, struct string *dir_list)
{
	WIN32_FIND_DATA ffd;
	char			toto[128];
	int				ret=0;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	strcpy_cs	 (toto,128,path);
	strcat_cs	 (toto,128,"\\*");

    hFind = FindFirstFile(toto, &ffd);
   if (INVALID_HANDLE_VALUE == hFind) 
   {
      return ret;
   } 
   
   // List all the files in the directory with some info about them.
   do
   {
      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
		  if (ffd.cFileName[0] == '.')continue;
		  
		  cat_cstring (dir_list,ffd.cFileName);
		  cat_cstring (dir_list,"\n");
		  ret++;
	  }
   }
   while (FindNextFile(hFind, &ffd) != 0);

   FindClose(hFind);
   return ret;
}
OS_API_C_FUNC(int) get_sub_files(const char *path, struct string *file_list)
{
	WIN32_FIND_DATA ffd;
	char			toto[128];
	int				ret = 0;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	strcpy_cs(toto, 128, path);
	strcat_cs(toto, 128, "\\*");

	hFind = FindFirstFile(toto, &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		return ret;
	}

	// List all the files in the directory with some info about them.
	do
	{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			cat_cstring(file_list, ffd.cFileName);
			cat_cstring(file_list, "\n");
			ret++;
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
	return ret;
}


OS_API_C_FUNC(int) get_file_to_memstream(const char *path, mem_stream *stream)
{
	HANDLE hFile;
	mem_zone_ref fileMem = { PTR_NULL };
	size_t len, data_len;

	if ((hFile = CreateFile(path, FILE_READ_DATA | FILE_READ_ATTRIBUTES, 0, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0)) == INVALID_HANDLE_VALUE)
	{
		struct string t;
		clone_string(&t, &exe_path);
		cat_cstring_p(&t, path);
		hFile = CreateFile(t.str, FILE_READ_DATA | FILE_READ_ATTRIBUTES, 0, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
		free_string(&t);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			stream->data.zone = PTR_NULL;
			return -1;
		}
	}

	data_len = GetFileSize(hFile, NULL);
	if (data_len>0)
	{
		unsigned char *data;
		allocate_new_zone(0, data_len+1, &fileMem);
		data = (unsigned char *)get_zone_ptr(&fileMem, 0);
		if (data != PTR_NULL)
		{
			SetFilePointer(hFile, 0, 0, FILE_BEGIN);
			ReadFile(hFile, data, data_len, &len, PTR_NULL);
			data[data_len] = 0;
			mem_stream_init(stream, &fileMem, 0);
		}
		else
			len = 0;
	}
	else
		len = 0;
	CloseHandle(hFile);

	return (int)len;
}


OS_API_C_FUNC(int) get_file(const char *path, unsigned char **data, size_t *data_len)
{
	HANDLE hFile;
	size_t len;

	if (path == PTR_NULL)return 0;

	if ((hFile = CreateFile(path, FILE_READ_DATA|FILE_READ_ATTRIBUTES, 0, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0)) == INVALID_HANDLE_VALUE)
	{
		struct string t;
		clone_string		(&t, &exe_path);
		cat_cstring_p		(&t, path);
		hFile = CreateFile	(t.str, FILE_READ_DATA|FILE_READ_ATTRIBUTES, 0, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
		free_string(&t);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			*data_len = 0;
			return -1;
		}
	}
	
	(*data_len) = GetFileSize(hFile, NULL);

	if ((*data_len)>0)
	{
		(*data) = malloc_c((*data_len) + 1);
		if ((*data) != PTR_NULL)
		{
			SetFilePointer	(hFile, 0, 0, FILE_BEGIN);
			ReadFile		(hFile, (*data), (*data_len), &len, PTR_NULL);
			(*data)[*data_len] = 0;
		}
		else
			len = 0;
	}
	else
		len = 0;
	CloseHandle	(hFile);

	return (int)len;
}



OS_API_C_FUNC(int) get_file_len(const char *path, size_t size, unsigned char **data, size_t *data_len)
{
	HANDLE hFile;
	size_t len;

	if ((hFile = CreateFile(path, FILE_READ_DATA|FILE_READ_ATTRIBUTES, 0, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0)) == INVALID_HANDLE_VALUE)
	{
		struct string t;
		clone_string		(&t, &exe_path);
		cat_cstring_p		(&t, path);
		hFile = CreateFile	(t.str, FILE_READ_DATA|FILE_READ_ATTRIBUTES, 0, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
		free_string(&t);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			*data_len = 0;
			return -1;
		}
	}
	
	(*data_len) = GetFileSize(hFile, NULL);

	if ((*data_len)>0)
	{
		if((*data_len)>size)
			(*data_len)=size;

		(*data) = malloc_c((*data_len) + 1);
		if ((*data) != PTR_NULL)
		{
			SetFilePointer	(hFile, 0, 0, FILE_BEGIN);
			ReadFile		(hFile, (*data), (*data_len), &len, PTR_NULL);
			(*data)[*data_len] = 0;
		}
		else
			len = 0;
	}
	else
		len = 0;
	CloseHandle	(hFile);

	return (int)len;
}

OS_API_C_FUNC(int) put_file(const char *path, void *data, size_t data_len)
{
	HANDLE		hFile;
	size_t		len;

	if ((hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE)
		return 0;

	if (data_len > 0)
		WriteFile(hFile,data,data_len,&len,PTR_NULL);
	else
		len = 1;

	CloseHandle(hFile);
	return len;

}

OS_API_C_FUNC(int) get_file_range(const char *path, size_t ofset, size_t last, unsigned char **data, size_t *data_len)
{
	HANDLE hFile;
	uint64_t start, end;
	size_t len, filesize;


	while ((hFile = CreateFile(path, FILE_READ_DATA | FILE_READ_ATTRIBUTES, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0)) == INVALID_HANDLE_VALUE)
	{
		DWORD error;
		error = GetLastError();
		if (error != ERROR_SHARING_VIOLATION)
		{
			*data_len = 0;
			return -1;
		}
	}

	len = 0;
	start = mul64(ofset, 512);
	filesize = GetFileSize(hFile, NULL);

	if(start<filesize)
	{
		
		end = mul64(last, 512);

		if (end > filesize)
			end = filesize;

		SetFilePointer(hFile, start, 0, FILE_BEGIN);

		(*data_len) = end - start;
		(*data) = (unsigned char *)malloc_c((*data_len));
		if ((*data) != PTR_NULL)
			ReadFile(hFile, (*data), (*data_len), &len, PTR_NULL);
		else
			len = 0;

		(*data_len) = len;
		
	}
	CloseHandle(hFile);
	return (int)len;
}

OS_API_C_FUNC(int) get_file_chunk(const char *path, size_t ofset, unsigned char **data, size_t *data_len)
{
	HANDLE hFile;
	size_t len, filesize;

	while ((hFile = CreateFile(path, FILE_READ_DATA | FILE_READ_ATTRIBUTES, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0)) == INVALID_HANDLE_VALUE)
	{
		DWORD error;
		error = GetLastError();
		if (error != ERROR_SHARING_VIOLATION)
		{
			*data_len = 0;
			return -1;
		}
	}

	len = 0;
	filesize = GetFileSize(hFile, NULL);

	if (filesize >= (ofset + 4))
	{
		unsigned int	chunk_size;
		SetFilePointer(hFile, ofset, 0, FILE_BEGIN);
		ReadFile(hFile, &chunk_size, 4, &len, PTR_NULL);

		if (filesize >= (ofset + 4 + chunk_size))
		{
			(*data_len) = chunk_size;
			(*data) = (unsigned char *)malloc_c((*data_len) + 1);
			if ((*data) != PTR_NULL)
			{
				ReadFile(hFile, (*data), (*data_len), &len, PTR_NULL);
				(*data)[*data_len] = 0;
			}
		}
		else
			len = 0;
	}
	CloseHandle(hFile);
	return (int)len;
}

OS_API_C_FUNC(int) append_file(const char *path, const void *data, size_t data_len)
{
	HANDLE		hFile;
	size_t		len;

	if (data_len == 0)return 1;

	while ((hFile = CreateFile(path, FILE_APPEND_DATA | FILE_READ_ATTRIBUTES, FILE_SHARE_READ , NULL, OPEN_ALWAYS, FILE_FLAG_BACKUP_SEMANTICS, NULL)) == INVALID_HANDLE_VALUE)
	{
		DWORD error;
		error = GetLastError();
		if (error != ERROR_SHARING_VIOLATION)
			return 0;
	}

	SetFilePointer		(hFile, 0, NULL, FILE_END);
	WriteFile			(hFile, data, data_len, &len, PTR_NULL);
	CloseHandle			(hFile);
	return len;

}

OS_API_C_FUNC(int) truncate_file(const char *path, size_t ofset, const void *data, size_t data_len)
{
	HANDLE		hFile;
	size_t		len;

	if ((ofset == 0) && (data_len == 0))
	{
		del_file(path);
		return 1;
	}

	while ((hFile = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE)
	{
		DWORD error;
		error = GetLastError();
		if (error != ERROR_SHARING_VIOLATION)
			return 0;
	}

	if (SetFilePointer(hFile, ofset, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		CloseHandle(hFile);
		return 0;
	}

	SetEndOfFile(hFile);

	if ((data != PTR_NULL) && (data_len > 0))
	{
		WriteFile		(hFile, data, data_len, &len, PTR_NULL);
	}
	else
	{
		len = 1;
	}

	CloseHandle(hFile);


	return 1;

}

OS_API_C_FUNC(ctime_t)	 get_time_c()
{
	SYSTEMTIME st;
	FILETIME   ft;
	GetSystemTime				(&st);
	SystemTimeToFileTime		(&st, &ft);
	return FileTime_to_POSIX	(ft);
}
OS_API_C_FUNC(int) get_home_dir(struct string *path)
{
	char szPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath)))
	{
		// Append product-specific path
		make_string(path, szPath);
		return 1;
	}
	return 0;
}
OS_API_C_FUNC(int) set_exe_path()
{
	char path[512];
	get_cwd(path, 512);

	init_string(&exe_path);
	make_string(&exe_path, path);

	return 1;
}

OS_API_C_FUNC(int) get_exe_path(struct string *outPath)
{
	clone_string(outPath, &exe_path);
	return 1;
}

OS_API_C_FUNC(int) set_data_dir(const struct string *path,const char *name)
{
	clone_string  (&home_path,path);	
	cat_cstring_p (&home_path, name);
	create_dir	  (home_path.str);
	set_cwd		  (home_path.str);
	return 1;
}
OS_API_C_FUNC(int) set_home_path(const char *name)
{
	init_string  (&home_path);
	get_home_dir (&home_path);
	cat_cstring_p(&home_path, name);
	create_dir	 (home_path.str);
	set_cwd		 (home_path.str);
	return 1;
}

OS_API_C_FUNC(int) aquire_lock_file(const char *name)
{
	init_string(&lck_file_name);
	make_string(&lck_file_name, name);
	cat_cstring(&lck_file_name, ".pid");

	lockFile = CreateFile(lck_file_name.str, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (lockFile == INVALID_HANDLE_VALUE)
	{
		console_print("unable to open lock file \n");
		return 0;
	}

	return 1;
}

OS_API_C_FUNC(int) daemonize(const char *name)
{
	init_string (&log_file_name);
	make_string	(&log_file_name,name);
	cat_cstring	(&log_file_name,".log");
	return 1;
}



void init_threads()
{
	memset_c(threads, 0, sizeof(threads));

}
unsigned int new_thread(unsigned int h)
{
	unsigned int	n;
	n = 0;
	while (n<MAX_THREADS)
	{
		if (compare_z_exchange_c(&threads[n].h,h))
			return n;

		n++;
	}
	return 0xFFFFFFFF;
}

unsigned int get_current_thread(unsigned int h)
{
	int n=0;

	while (n < MAX_THREADS)
	{
		if (h == threads[n].h)
			return n;
		n++;
	}
	return 0xFFFFFFFF;

}
int get_my_thread_flag(unsigned int *flag)
{
	DWORD  h;
	unsigned int cur;
	h = GetCurrentThreadId();
	cur = get_current_thread(h);
	if (cur == 0xFFFFFFFF)
		cur = new_thread(h);

	(*flag) = 1 << cur;

	return 1;
}



OS_API_C_FUNC(int) set_tree_mem_area_id(unsigned int area_id)
{
	DWORD  h;
	unsigned int cur;
	h = GetCurrentThreadId();
	cur = get_current_thread(h);
	if (cur == 0xFFFFFFFF)
		cur = new_thread(h);

	threads[cur].tree_area_id = area_id;
	return 1;
}

OS_API_C_FUNC(int) set_mem_area_id(unsigned int area_id)
{
	DWORD  h;
	unsigned int cur;
	h = GetCurrentThreadId();
	cur = get_current_thread(h);
	if (cur == 0xFFFFFFFF)
		cur = new_thread(h);
	threads[cur].mem_area_id = area_id;
	return 1;
}
OS_API_C_FUNC(unsigned int) get_tree_mem_area_id()
{
	DWORD  h;
	unsigned int cur;
	h = GetCurrentThreadId();
	cur = get_current_thread(h);
	if (cur == 0xFFFFFFFF)
		return cur;

	return threads[cur].tree_area_id;
}
OS_API_C_FUNC(unsigned int) get_mem_area_id()
{
	DWORD  h;
	unsigned int cur;
	h = GetCurrentThreadId();
	cur = get_current_thread(h);
	if (cur == 0xFFFFFFFF)
		return cur;

	return threads[cur].mem_area_id;
}
unsigned int next_ttid = 1;
extern ASM_API_FUNC scan_stack_c(mem_ptr lower, mem_ptr upper, mem_ptr stack_frame, mem_ptr stack);
extern mem_ptr ASM_API_FUNC get_stack_frame_c();

DWORD WINAPI thread_start(void *p)
{
	thread_func_ptr		func;
	struct thread	    *mythread;
	HANDLE				ph;
	int					ret;

	mythread			= (struct thread *)p;
	mythread->h		= GetCurrentThreadId();
	ph				= GetCurrentProcess();


	DuplicateHandle(ph, GetCurrentThread(), ph, &mythread->th, DUPLICATE_SAME_ACCESS, FALSE, DUPLICATE_SAME_ACCESS);


	func			= mythread->func;

	init_default_mem_area(16 * 1024 * 1024, 8 * 1024);

	ret = func			 (&mythread->params, &mythread->status);
	free_mem_area		 (0);
	
	return ret;
}



void scan_threads_stack(mem_ptr lower, mem_ptr upper)
{
	unsigned int n = 0;
	DWORD h;

	h = GetCurrentThreadId();
	for (n = 1; n < MAX_THREADS; n++)
	{
		CONTEXT ctx;

		if (threads[n].h == 0)continue;
		if (threads[n].h == h)continue;
		if (SuspendThread(threads[n].th) == -1)
		{
			log_output("could not suspend thread \n");
			continue;
		}

		memset_c(&ctx, 0, sizeof(CONTEXT));

		ctx.ContextFlags = CONTEXT_ALL;
		
		if (GetThreadContext(threads[n].th, &ctx))
		{ 
			if ((ctx.Esp != 0) && (ctx.Ebp!=0) && (ctx.Ebp > ctx.Esp))
				scan_stack_c(lower, upper, ctx.Ebp, ctx.Esp);
		}
		else
		{
			log_output("could not get thread ctx\n");
		}

		ResumeThread(threads[n].th);
		
	}
}

void resume_threads()
{
	unsigned int n = 0;

	for (n = 0; n < MAX_THREADS; n++)
	{
		if (threads[n].h == 0)continue;
		ResumeThread(threads[n].th);
	}
}


OS_API_C_FUNC(int) background_func(thread_func_ptr func,mem_zone_ref_ptr params)
{
	unsigned int			cur;

	DWORD					threadid;
	HANDLE					newThread;

	cur = new_thread(next_ttid++);

	copy_zone_ref(&threads[cur].params, params);
	threads[cur].func = func;

	threads[cur].status = 0;


	/*
	SECURITY_ATTRIBUTES		secs;
	SECURITY_DESCRIPTOR		desc;

	desc.Sacl
	desc.Control = THREAD_GET_CONTEXT;

	secs.lpSecurityDescriptor
	*/

	newThread = CreateThread(NULL, 32 * 1024, thread_start, &threads[cur], 0, &threadid);

	while (threads[cur].status == 0)
	{
		SleepEx(1, 1);
	}

	release_zone_ref(&threads[cur].params);
		
	return 1;
}

OS_API_C_FUNC(void) console_print(const char *msg)
{
	HANDLE OUTPUT_HANDLE;
	size_t wrote;
	/* OutputDebugString(msg); */

	OUTPUT_HANDLE = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsole(OUTPUT_HANDLE, msg, strlen_c(msg), &wrote, NULL);

}
OS_API_C_FUNC(void	*)kernel_memory_map_c(unsigned int size)
{
	return HeapAlloc(GetProcessHeap(),0,size);
}
OS_API_C_FUNC(int)kernel_memory_free_c(mem_ptr ptr)
{
	return HeapFree(GetProcessHeap(),0,ptr);
}
OS_API_C_FUNC(void) get_system_time_c(ctime_t *time)
{
	SYSTEMTIME st;
	FILETIME   ft;

	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ft);
	*time=FileTime_to_Milli(ft);
}


 OS_API_C_FUNC(int) get_cwd(char *path,size_t len)
 {
	 return GetCurrentDirectory(len,path);
 }


 OS_API_C_FUNC(int) set_cwd(const char *path)
 {
	 return SetCurrentDirectory(path);
 }

 OS_API_C_FUNC(int) log_output(const char *data)
 {
	console_print(data);
	if ((log_file_name.str != PTR_NULL) && (log_file_name.str!=PTR_INVALID))
		append_file(log_file_name.str, data, strlen_c(data));
	 return 1;
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

			 strcpy_cs	(mdir, 512, dir);
			 strncat_c	(mdir, &path_sep, 1);
			 strncat_c	(mdir, optr, sz);
			 del_file	(mdir);
			 cur++;
			 optr = ptr + 1;
			 dir_list_len -= sz;
		 }
	 }
	 free_string(&dir_list);
	 return del_dir(dir);
 }
 int extractDate(const char * s){
	 unsigned int	y, m, d;
	 SYSTEMTIME st;
	 FILETIME ft;

	 y = strtoul_c(s, PTR_NULL, 10);
	 m = strtoul_c(&s[5], PTR_NULL, 10);
	 d = strtoul_c(&s[8], PTR_NULL, 10);

	 st.wYear = y;
	 st.wMonth = m;
	 st.wDay = d;
	 st.wDayOfWeek = 0;
	 st.wHour = 0;
	 st.wMinute = 0;
	 st.wSecond = 1;
	 st.wMilliseconds = 0;
	 SystemTimeToFileTime(&st, &ft);
	 return FileTime_to_POSIX(ft);
	 /*
	 memset_c(&t, 0, sizeof(struct tm));
	t.tm_mday = d;
	t.tm_mon = m - 1;
	t.tm_year = y - 1900;
	t.tm_isdst = -1;
	 // normalize:
	 return mktime(&t);
	 */
 }
 OS_API_C_FUNC(unsigned int) parseDate(const char *date)
 {
	 int			n;
	 n		= strlen_c(date);
	 if (n < 10)return 0;
	
	 return extractDate(date);
 }
 OS_API_C_FUNC(unsigned int) isRunning()
 {
	 return running;
 }
 
 void init_exit()
 {
	 
 }

OS_API_C_FUNC(int) default_RNG(unsigned char *dest, size_t size) 
{
	  HCRYPTPROV prov;
	  if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		  return 0;
	  }

	  CryptGenRandom(prov, size, (BYTE *)dest);
	  CryptReleaseContext(prov, 0);
	  return 1;
}


OS_API_C_FUNC(void) strtod_c(const char *str,double *d)
{
	*d = strtod(str, NULL);
}

OS_API_C_FUNC(void) snooze_c(size_t n)
{
	SleepEx(n/1000, 1);
}



