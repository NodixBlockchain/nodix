#ifndef LIBBASE_API
#define LIBBASE_API C_IMPORT
#endif

/* Read one byte from stream, convert to unsigned char, then int, and
   return. return EOF on end of file. This corresponds to the
   behaviour of fgetc(). */
typedef int (*get_func)(void *data);

typedef int64_t json_int_t;

typedef struct {
	char value_ptr[2048];
    size_t length;   /* bytes used */
    size_t size;     /* bytes allocated */
} strbuffer_t;

typedef struct
{
    const char *data;
    size_t len;
    size_t pos;
} buffer_data_t;

typedef struct {
    get_func get;
    void *data;
    char buffer[5];
    size_t buffer_pos;
    int state;
    int line;
    int column, last_column;
    size_t position;
} stream_t;


typedef struct {
    stream_t stream;
    strbuffer_t saved_text;
	struct string value_string;
    int	token;
    union {
		struct string	*string;
        json_int_t		integer;
        double			real;
    } value;
} lex_t;


#define TOKEN_INVALID         -1
#define TOKEN_EOF              0

#define TOKEN_STRING         256
#define TOKEN_INTEGER        257
#define TOKEN_REAL           258
#define TOKEN_TRUE           259
#define TOKEN_FALSE          260
#define TOKEN_NULL           261

#define STREAM_STATE_OK        0
#define STREAM_STATE_EOF      -1
#define STREAM_STATE_ERROR    -2

int strbuffer_init(strbuffer_t *strbuff);
void strbuffer_close(strbuffer_t *strbuff);

void strbuffer_clear(strbuffer_t *strbuff);

const char *strbuffer_value(const strbuffer_t *strbuff);

/* Steal the value and close the strbuffer */
char *strbuffer_steal_value(strbuffer_t *strbuff);

int strbuffer_append(strbuffer_t *strbuff, const char *string);
int strbuffer_append_byte(strbuffer_t *strbuff, char byte);
int strbuffer_append_bytes(strbuffer_t *strbuff, const char *data, size_t size);

char strbuffer_pop(strbuffer_t *strbuff);

int lex_set_value_string(lex_t *lex, struct string *str);
int  lex_init		(lex_t *lex,  void *data);
void lex_close		(lex_t *lex);
int  lex_scan		(lex_t *lex);
extern int  parse_value	(lex_t *lex,const char *name,unsigned int type,mem_zone_ref_ptr out);
void lex_steal_string(lex_t *lex, char *str, unsigned int str_len);


LIBBASE_API void C_API_FUNC write_json(mem_zone_ref_ptr params, unsigned int mode, struct string *json_req);