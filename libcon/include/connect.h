struct					con;
#ifndef LIBC_API
#define LIBC_API C_IMPORT
#endif

LIBC_API int					C_API_FUNC network_init		();
LIBC_API int					C_API_FUNC network_free();


LIBC_API int					C_API_FUNC get_if(const char *gw_ip, struct string *name, struct string *ip);

LIBC_API void					C_API_FUNC init_read_group();
LIBC_API int					C_API_FUNC read_group_has(const char *file);

LIBC_API int					C_API_FUNC set_tcp_no_delay(struct con *mycon, int on);
LIBC_API void					C_API_FUNC add_read_group(struct con *mycon, void *file, size_t transfer_len, const struct string *file_name);

LIBC_API const struct string*	C_API_FUNC get_con_error(struct con *Con);
LIBC_API void					C_API_FUNC clear_con_error(struct con *Con);
LIBC_API struct string		*	C_API_FUNC get_con_lastline(struct con *Con);
LIBC_API const struct host_def*	C_API_FUNC get_con_hostd(struct con *Con);
LIBC_API int					C_API_FUNC get_con_addr(struct con *mycon, char *addr, size_t len);
LIBC_API int					C_API_FUNC get_con_ip(struct con *Con, ipv4_t ip);

LIBC_API int					C_API_FUNC con_move_data(struct con *Con, struct string *data, size_t mov_len);
LIBC_API int					C_API_FUNC con_consume_data(struct con *Con, size_t mov_len);


LIBC_API struct con *			C_API_FUNC do_connect(const struct host_def *host);
LIBC_API struct con *			C_API_FUNC create_upnp_broadcast(struct host_def *host);
LIBC_API int					C_API_FUNC send_upnpbroadcast(struct con *mycon, struct string *msg);
LIBC_API int					C_API_FUNC reconnect(struct con *mycon);
LIBC_API struct con *			C_API_FUNC open_port(const char *my_addr, unsigned short port);
LIBC_API struct con *			C_API_FUNC do_get_incoming(struct con *listen_con, unsigned int time_out);
LIBC_API int					C_API_FUNC set_non_blocking(struct con *mycon, unsigned int nb);

LIBC_API int					C_API_FUNC read_data(struct con *Con, size_t max);
LIBC_API int					C_API_FUNC read_av_data(struct con *Con, size_t max);
LIBC_API int					C_API_FUNC send_data(struct con *Con, const  unsigned char *data, size_t len);
LIBC_API int					C_API_FUNC send_data_av(struct con *Con, unsigned char *data, size_t len);
LIBC_API char	*				C_API_FUNC readline(struct con *Con, ctime_t timeout);

LIBC_API void					C_API_FUNC do_read_group();
LIBC_API int					C_API_FUNC pop_read_done(struct string *out);
LIBC_API void					C_API_FUNC con_close(struct con *Con);

LIBC_API void					C_API_FUNC free_con_buffer(struct con *my_con);
LIBC_API int					C_API_FUNC  get_write_set(mem_ptr set, unsigned int timems);
LIBC_API int					C_API_FUNC  get_read_set(mem_ptr set, unsigned int timems);
LIBC_API void					C_API_FUNC  add_to_set(mem_ptr set, struct con *mycon);
LIBC_API mem_ptr				C_API_FUNC  create_set();
LIBC_API int					C_API_FUNC  is_write_set(mem_ptr set, struct con *mycon);
LIBC_API int					C_API_FUNC  is_read_set(mem_ptr set, struct con *mycon);
LIBC_API int					C_API_FUNC  is_error_set(mem_ptr set, struct con *mycon);

