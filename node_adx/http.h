#ifndef NODE_API
#define NODE_API C_IMPORT
#endif



struct http_hdr
{
	char			key[64];
	enum op_type	op;
	struct string	value;
};

struct http_req
{
	struct string		error;
	struct string		req_line;
	struct key_val		req_headers[32];
	struct key_val		query_vars[32];
	unsigned char		*data;
	size_t				data_len;
};
struct http_infos
{
	struct string		method;
	struct string		path;
	struct string		query;
	struct string		http_version;
};

struct http_resp
{
	struct string		status_code;
	unsigned int		header_sent;
	size_t				data_sent;
	mem_ptr				compressed_data;
	size_t				compressed_data_len;
	struct	key_val		resp_headers[32];
};

#ifndef NODE_API
#define NODE_API IMP_API
#endif

NODE_API struct http_req  *		C_API_FUNC new_http_req();
NODE_API struct http_resp *		C_API_FUNC new_http_resp();
NODE_API struct http_req  *		C_API_FUNC create_request(const struct host_def *host, const char *path, const struct string *data, int is_post, const struct string *cookie);
NODE_API int					C_API_FUNC http_make_request_header(struct http_req *req, struct string *header);
NODE_API int					C_API_FUNC http_make_response_header(const struct http_resp *resp, struct string *response);
NODE_API int					C_API_FUNC parse_http_req_line(const struct string *req_line, struct http_infos *infos);
NODE_API void					C_API_FUNC parse_http_query_vars(struct http_req *req, struct string *query);
NODE_API int					C_API_FUNC parse_http_url_params(const char *params, char sep, mem_zone_ref_ptr vars, unsigned int type);
NODE_API int					C_API_FUNC parse_http_new_url_params(const char *params, char sep, mem_zone_ref_ptr param_list, unsigned int type);
NODE_API int					C_API_FUNC http_add_query_line(const struct string *line, struct key_val *hdrs);
NODE_API void					C_API_FUNC init_http_infos(struct http_infos *infos);
NODE_API void					C_API_FUNC free_http_infos(struct http_infos *infos);
NODE_API char *					C_API_FUNC http_add_header_line(const struct string *line, struct key_val *hdrs, char sep);
NODE_API struct key_val *		C_API_FUNC add_key(struct key_val *hdrs, const char *key, size_t key_len, const char *data, size_t data_len);
NODE_API int					C_API_FUNC del_key(struct key_val *hdrs, const char *key);
NODE_API struct key_val *		C_API_FUNC add_qkey(struct key_val *hdrs, const char *key, size_t key_len, const char *data, size_t data_len);
NODE_API const struct key_val *	C_API_FUNC find_key(const struct key_val *hdrs, const char *key);
NODE_API void					C_API_FUNC free_request(struct http_req *req);
NODE_API void					C_API_FUNC free_response(struct http_resp *resp);

NODE_API struct http_resp *		C_API_FUNC http_request(struct con *mycon, struct http_req *req, unsigned int n_tries, ctime_t timeout);
NODE_API struct http_resp *		C_API_FUNC http_parse_response_header(struct string *data);
NODE_API void					C_API_FUNC http_read_data(struct con *mycon, struct http_resp *resp, struct string *data);
NODE_API int					C_API_FUNC http_write_data(struct con *mycon, struct http_resp *resp, const struct data_buffer *data,unsigned int async);
NODE_API struct  http_req*		C_API_FUNC http_process_request(struct con *new_con, struct  http_infos	 *infos);
NODE_API int					C_API_FUNC fetch_http_url(const struct string *url, struct	string *data, const struct	string *cookie);
NODE_API struct http_req *		C_API_FUNC make_soap_request(const struct string *url, const char *soap_action, const char *soap_body, struct string *data);
NODE_API int					C_API_FUNC json_rpc(struct con *mycon, const char *method, mem_zone_ref_ptr params, unsigned int call_id, unsigned int mode);

NODE_API int					C_API_FUNC parse_multipart(const struct string *post_data, const struct string *boundary, mem_zone_ref_ptr form_parts);


NODE_API int					C_API_FUNC create_write_group(mem_zone_ref_ptr new_write_group);
NODE_API int					C_API_FUNC add_group_listener(mem_zone_ref_ptr group, mem_zone_ref_ptr new_req);
NODE_API int					C_API_FUNC write_group_message(mem_zone_ref_ptr my_group, const struct string *mesage, unsigned int flags);
NODE_API int					C_API_FUNC finish_http_request(mem_zone_ref_ptr http_req);
NODE_API int					C_API_FUNC output_http_req(mem_zone_ref_ptr http_req, const struct data_buffer *inpage,unsigned int async);
NODE_API int					C_API_FUNC delete_write_group(mem_zone_ref_ptr my_group);
