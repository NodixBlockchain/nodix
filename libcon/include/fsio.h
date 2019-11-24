#ifndef LIBC_API
#define LIBC_API C_IMPORT
#endif


#define MAX_THREADS 31

LIBC_API size_t			C_API_FUNC file_size(const char *path);
LIBC_API int			C_API_FUNC append_file(const char *path, const void *data, size_t data_len);
LIBC_API int			C_API_FUNC truncate_file(const char *path, size_t ofset, const void *data, size_t data_len);
LIBC_API int			C_API_FUNC stat_file(const char *path);
LIBC_API int			C_API_FUNC create_dir(const char *path);
LIBC_API int			C_API_FUNC del_dir(const char *path);
LIBC_API int			C_API_FUNC put_file(const char *path, const void *data, size_t data_len);
LIBC_API int			C_API_FUNC get_sub_dirs(const char *path, struct string *dir_list);
LIBC_API int			C_API_FUNC get_sub_files(const char *path, struct string *file_list);
LIBC_API int			C_API_FUNC get_file(const char *path, mem_ptr *data, size_t *data_len);
LIBC_API int			C_API_FUNC get_file_range(const char *path, size_t ofset, size_t end, unsigned char **data, size_t *data_len);
LIBC_API int			C_API_FUNC get_file_len(const char *path, size_t size, unsigned char **data, size_t *data_len);
LIBC_API int			C_API_FUNC get_file_chunk(const char *path, size_t ofset, unsigned char **data, size_t *data_len);

//LIBC_API int			C_API_FUNC get_hash_idx(const char *path, size_t idx, hash_t hash);
LIBC_API int			C_API_FUNC set_home_path(const char *name);
LIBC_API int			C_API_FUNC set_data_dir(const struct string *path,const char *name);
LIBC_API int			C_API_FUNC set_exe_path();
LIBC_API int			C_API_FUNC get_exe_path(struct string *outPath);
LIBC_API int			C_API_FUNC daemonize(const char *name);
LIBC_API int			C_API_FUNC aquire_lock_file(const char *name);
LIBC_API ctime_t		C_API_FUNC get_time_c(void);
LIBC_API void			C_API_FUNC get_system_time_c(ctime_t *time);
LIBC_API void			C_API_FUNC console_print(const char *msg);
LIBC_API int			C_API_FUNC log_output(const char *data);
LIBC_API int			C_API_FUNC set_mem_exe(mem_zone_ref_ptr zone);
LIBC_API int			C_API_FUNC move_file(const char *ipath,const char *opath);
LIBC_API int			C_API_FUNC del_file(const char *path);
LIBC_API int			C_API_FUNC set_ftime(const char *path, ctime_t time);
LIBC_API int			C_API_FUNC get_ftime(const char *path, ctime_t *time);
LIBC_API int			C_API_FUNC get_home_dir(struct string *path);
LIBC_API int			C_API_FUNC set_cwd(const char *path);
LIBC_API int			C_API_FUNC get_cwd(char *path, size_t len);
LIBC_API int			C_API_FUNC rm_dir(const char *dir);

LIBC_API unsigned int	C_API_FUNC get_mem_area_id(void);
LIBC_API unsigned int	C_API_FUNC isRunning(void);
LIBC_API int C_API_FUNC default_RNG(unsigned char *dest, size_t size);

extern unsigned int log_lck;
