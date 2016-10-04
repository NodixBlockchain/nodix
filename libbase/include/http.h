struct http_hdr
{
	char			key[64];
	struct string	value;
};

struct http_req
{
	struct string		error;
	struct string		req_line;
	struct	http_hdr	req_headers[32];
	struct	http_hdr	query_vars[32];
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
	struct	http_hdr	resp_headers[32];
};

#ifndef LIBBASE_API
#define LIBBASE_API IMP_API
#endif

LIBBASE_API struct http_req  *		C_API_FUNC new_http_req();
LIBBASE_API struct http_resp *		C_API_FUNC new_http_resp();
LIBBASE_API struct http_req  *		C_API_FUNC create_request(const struct host_def *host, const char *path, const struct string *data, int is_post, const struct string *cookie);
LIBBASE_API int						C_API_FUNC http_make_request_header(struct http_req *req, struct string *header);
LIBBASE_API int						C_API_FUNC http_make_response_header(struct http_resp *resp, struct string *response);
LIBBASE_API int						C_API_FUNC parse_http_req_line(const struct string *req_line, struct http_infos *infos);
LIBBASE_API void					C_API_FUNC parse_http_query_vars(struct http_req *req, struct string *query);
LIBBASE_API void					C_API_FUNC init_http_infos(struct http_infos *infos);
LIBBASE_API void					C_API_FUNC free_http_infos(struct http_infos *infos);
LIBBASE_API char *					C_API_FUNC http_add_header_line(const struct string *line, struct http_hdr *hdrs, char sep);
LIBBASE_API struct http_hdr *		C_API_FUNC add_key(struct http_hdr *hdrs, const char *key, size_t key_len, const char *data, size_t data_len);
LIBBASE_API const struct http_hdr *	C_API_FUNC find_key(const struct http_hdr *hdrs, const char *key);
LIBBASE_API void					C_API_FUNC free_request(struct http_req *req);
LIBBASE_API void					C_API_FUNC free_response(struct http_resp *resp);

LIBBASE_API struct http_resp *		C_API_FUNC http_request(struct con *mycon, struct http_req *req, unsigned int n_tries, ctime_t timeout);
LIBBASE_API struct http_resp *		C_API_FUNC http_parse_response_header(struct string *data);
LIBBASE_API void					C_API_FUNC http_read_data(struct con *mycon, struct http_resp *resp, struct string *data);
LIBBASE_API struct  http_req*		C_API_FUNC http_process_request(struct con *new_con, struct  http_infos	 *infos);
LIBBASE_API int						C_API_FUNC fetch_http_url(const struct string *url, struct	string *data, const struct	string *cookie);
LIBBASE_API struct http_req *		C_API_FUNC make_soap_request(const struct string *url, const char *soap_action, const char *soap_body, struct string *data);
LIBBASE_API int						C_API_FUNC json_rpc(struct con *mycon, const char *method, mem_zone_ref_ptr params, unsigned int call_id, unsigned int mode);
LIBBASE_API void					C_API_FUNC write_json(mem_zone_ref_ptr params, unsigned int mode, struct string *json_req);


