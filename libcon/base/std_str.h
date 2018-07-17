#ifndef LIBC_API	
#define LIBC_API	C_IMPORT
#endif

LIBC_API char *			C_API_FUNC strncpy_c				(char *string,const char *src_string,size_t		 cnt);
LIBC_API char *			C_API_FUNC strncpy_cs				(char *string,size_t str_len,const char *src_string,size_t cnt);

LIBC_API int			C_API_FUNC strcpy_cs				(char *string,size_t size,const char *src_string);
LIBC_API char *			C_API_FUNC strcpy_c					(char *string,const char *src_string);
LIBC_API int			C_API_FUNC strcat_cs				(char *string,size_t size,const char *src_string);
LIBC_API int			C_API_FUNC strcat_intval_s			(char *string,size_t size,const char *name,int val,int radix);
LIBC_API int			C_API_FUNC strcat_uintval_s			(char *string,size_t size,const char *name,unsigned int val,int radix);
LIBC_API int			C_API_FUNC strcat_escaped_s			(char *string,size_t size,const char *src_string);
LIBC_API char *			C_API_FUNC str_replace_char_c		(char *string, char c1, char c2);
LIBC_API int			C_API_FUNC strncat_c				(char *string,const char *src_string,size_t max);
LIBC_API int			C_API_FUNC strcat_c					(char *string,const char *src_string);
LIBC_API int			C_API_FUNC strcmp_c					(const char *string1,const char *string2);
LIBC_API int			C_API_FUNC strncmp_c				(const char *string1,const char *string2,size_t len);
LIBC_API int			C_API_FUNC strincmp_c				(const char *string1,const char *string2,size_t len);
LIBC_API int			C_API_FUNC stricmp_c				(const char *string1,const char *string2);

LIBC_API const char	*	C_API_FUNC strrchr_c				(const char *src,int car);
LIBC_API size_t 		C_API_FUNC strlen_c					(const char *string);
LIBC_API size_t			C_API_FUNC strlpos_c				(const char *string,size_t ofset,char c);
LIBC_API size_t			C_API_FUNC strrpos_c				(const char *string,char c);

LIBC_API int			C_API_FUNC itoa_s					(int value, char *string,size_t len, int radix);
LIBC_API int			C_API_FUNC uitoa_s					(size_t value, char *string, size_t len, int radix);
LIBC_API int			C_API_FUNC luitoa_s					(uint64_t value, char *string, size_t len, int radix);
LIBC_API int			C_API_FUNC litoa_s					(int64_t value, char *str, size_t len, int base);

LIBC_API long			C_API_FUNC strtol_c					(const char *nptr, char **endptr,int base);
LIBC_API int64_t		C_API_FUNC strtoll_c				(const char *nptr, char **endptr, int base);
LIBC_API unsigned long	C_API_FUNC strtoul_c				(const char *nptr, char **endptr, int base);
LIBC_API short			C_API_FUNC strtos_c					(const char *nptr, char **endptr, int base);
LIBC_API long			C_API_FUNC atol_c					(const char *str ); 
LIBC_API void			C_API_FUNC dtoll_c					(double dAmount, uint64_t *nAmount);
LIBC_API const char *	C_API_FUNC strstr_c					(const char *buf, const char *sub);
LIBC_API int			C_API_FUNC tolower_c				(int _c);
LIBC_API int			C_API_FUNC toupper_c				(int _c);
LIBC_API int			C_API_FUNC isprint_c				(int _c);
LIBC_API int			C_API_FUNC isalpha_c				(int _c);
LIBC_API int			C_API_FUNC isdigit_c				(int _c);
LIBC_API int			C_API_FUNC isdigit_c				(int _c);
LIBC_API int			C_API_FUNC isxdigit_c				(int _c);
LIBC_API int			C_API_FUNC isspace_c				(int _c);
LIBC_API void			C_API_FUNC snooze_c					(unsigned int micro_sec);
LIBC_API void			C_API_FUNC dtoa_c					(char *buff, char conv, int bsize, int dplace, double value);
LIBC_API unsigned int	C_API_FUNC parseDate				(const char *date);
LIBC_API void			C_API_FUNC store_bigendian			(unsigned char *x, uint64_t u);
LIBC_API uint64_t		C_API_FUNC load_bigendian			(const unsigned char *x);

LIBC_API void			C_API_FUNC strtod_c					(const char *str, double *d);
LIBC_API void			C_API_FUNC strtof_c					(const char *str, float *f);

LIBC_API int			C_API_FUNC b58tobin					(void *bin, size_t *binszp, const char *b58, size_t b58sz);


static const char		hex_chars[]		=	{'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

static INLINE_C_FUNC void bin_2_hex(const unsigned char *bin, size_t len, char *hex)
{
	unsigned int b2hn, b2hnn;
	for(b2hn = 0, b2hnn = 0; b2hn < len; b2hn++, b2hnn += 2)
	{
		hex[b2hnn + 0] = hex_chars[bin[b2hn] >> 0x04];
		hex[b2hnn + 1] = hex_chars[bin[b2hn] & 0x0F];
	}
	hex[b2hnn] = 0;
}

static INLINE_C_FUNC void bin_2_hex_r(const unsigned char *bin, size_t len, char *hex)
{
	unsigned int b2hn, b2hnn;
	for (b2hn = 0, b2hnn = (len - 1) * 2; b2hn < len; b2hn++, b2hnn -= 2)
	{
		hex[b2hnn + 0] = hex_chars[bin[b2hn] >> 0x04];
		hex[b2hnn + 1] = hex_chars[bin[b2hn] & 0x0F];
	}
	hex[len * 2] = 0;
}

static INLINE_C_FUNC void hex_2_bin(const char *chex, unsigned char *bin,size_t len)
{
	unsigned int b2hn, b2hnn;
	char         hex[3];
	hex[2] = 0;
	for (b2hn = 0, b2hnn = 0; chex[b2hn] != 0,b2hn < len; b2hn++, b2hnn += 2)
	{
		hex[0] = chex[b2hnn + 0];
		hex[1] = chex[b2hnn + 1];
		bin[b2hn] = strtoul_c(hex, NULL, 16);
	}
}

static INLINE_C_FUNC void hex_2_bin_r(const char *chex, unsigned char *bin)
{
	unsigned int b2hn, b2hnn;
	char         hex[3];
	hex[2] = 0;

	for (b2hn = strlen_c(chex) / 2 - 1, b2hnn = 0; chex[b2hnn] != 0; b2hn--, b2hnn += 2)
	{
		hex[0] = chex[b2hnn + 0];
		hex[1] = chex[b2hnn + 1];
		bin[b2hn] = strtoul_c(hex, NULL, 16);
	}
}

typedef unsigned char	ipv4_t[4];
typedef	char			bitcore_cmd_t[12];
typedef	unsigned char	hash_t[32];
typedef	char			btc_addr_t[34];
typedef	char			btc_paddr_t[52];

