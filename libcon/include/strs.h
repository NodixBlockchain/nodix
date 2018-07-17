
struct string
{
	char	*str;		/*pointer to null terminated string*/
	size_t  len;		/*number of characters in the string*/
	size_t  size;		/*memory size of the string*/
};


struct host_def
{
	struct string		host;		/*host name*/
	struct string		port_str;	/*port string*/
	unsigned short		port;		/*port integer*/
};


/* Initialize string structure */
LIBC_API		void				C_API_FUNC init_string			(struct string *str);

/* preallocate string memory */
LIBC_API		int 				C_API_FUNC prepare_new_data		(struct string *str, size_t len);

/* clone input string to target */
LIBC_API		int					C_API_FUNC clone_string			(struct string *str, const struct string *str1);

/* Create string from C string */
LIBC_API		int					C_API_FUNC make_string			(struct string *str, const char *toto);

/* Create string with n chars from C string */
LIBC_API		int 				C_API_FUNC make_string_l		(struct string *str, const char *toto, size_t len);

/* Create url encoded string with n chars from C string */
LIBC_API		int 				C_API_FUNC make_string_url		(struct string *str, const char *toto, size_t len);

/* Create string with n chars from url encoded C string */
LIBC_API		int 				C_API_FUNC make_string_from_url	(struct string *str, const char *toto, size_t len);

/* Create base 10 string from unsigned int */
LIBC_API		int 				C_API_FUNC make_string_from_uint(struct string *str, size_t i);

/* Concatenate input string to target */
LIBC_API		size_t				C_API_FUNC cat_string			(struct string *str, const struct string *src);

/* Concatenate input unsigned integer to target */
LIBC_API		int					C_API_FUNC strcat_uint			(struct string *str, size_t i);

/* Concatenate input float to target */
LIBC_API		int					C_API_FUNC strcat_float			(struct string *str, double f);

/* Concatenate input signed integer to target */
LIBC_API		int 				C_API_FUNC strcat_int			(struct string *str, int i);

/* Concatenate input C string to target */
LIBC_API		int 				C_API_FUNC cat_cstring			(struct string *str, const char *src);

/* Concatenate input n chars from C string to target */
LIBC_API		int 				C_API_FUNC cat_ncstring			(struct string *str, const char *src, size_t src_len);

/* Concatenate path seperator and input n chars from C string to target */
LIBC_API		int					C_API_FUNC cat_ncstring_p		(struct string *str, const char *src, size_t src_len);

/* Concatenate path seperator and input from C string to target */
LIBC_API		int					C_API_FUNC cat_cstring_p		(struct string *str, const char *src);

/* Output string to C string*/
LIBC_API		int 				C_API_FUNC make_cstring			(const struct string *str, char *toto, size_t len);

/* Free string memory */
LIBC_API		void				C_API_FUNC free_string			(struct string *str);

/* return host def structure from hostname and port */
LIBC_API		struct host_def *	C_API_FUNC make_host_def		(const char *host, unsigned short port);

/* return host def structure from url 'xx://hostname:port/' */
LIBC_API		struct host_def	*	C_API_FUNC make_host_def_url	(const struct string *url, struct string *path);

/* clone host def structure */
LIBC_API		void				C_API_FUNC copy_host_def		(struct host_def *dhost, const struct host_def *host);

/* free host def structure memory */
LIBC_API		void				C_API_FUNC free_host_def		(struct host_def *host);

/* generate XML tag with value */
LIBC_API		void				C_API_FUNC cat_tag				(struct string *str, const char *tag, const char *val);

/* create zip file */
LIBC_API		int					C_API_FUNC do_zip				(const char *fileName, struct string *initial_data, const char **files, size_t nFiles, struct string *zipData);


LIBC_API		int					C_API_FUNC str_end_with			(const struct string *str, const char *end);

LIBC_API		int					C_API_FUNC str_start_with		(const struct string *str, const char *start);

LIBC_API		int					C_API_FUNC vstr_to_str			(mem_ptr data_ptr, struct string *str);

/* find mem hash */
LIBC_API		int					C_API_FUNC find_mem_hash		(hash_t hash, unsigned char *mem_hash, unsigned int num);

LIBC_API		int					C_API_FUNC b58enc				(const struct string *in, struct string *out);

#define STRBUFFER_MIN_SIZE  16
#define STRBUFFER_FACTOR    2
#define STRBUFFER_SIZE_MAX  ((size_t)-1)

LIBC_API		int					C_API_FUNC strbuffer_append_bytes(struct string *strbuff, const char *data, size_t size);
LIBC_API		int					C_API_FUNC strbuffer_append_byte(struct string *strbuff, char byte);
LIBC_API		int					C_API_FUNC strbuffer_append(struct string *strbuff, const char *string);

