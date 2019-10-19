/* copyright iadix 2016 */
#define LIBC_API C_EXPORT
#include "base/std_def.h"
#include "base/std_mem.h"
#include "base/mem_base.h"
#include "base/std_str.h"
#include "base/utf.h"
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
	if(str==PTR_NULL)return 0;
	str->len			=	strlen_c(toto);
	if((str->str!=PTR_NULL)&&(str->str!=PTR_INVALID)&&(str->size>0))
		free_c(str->str);
	str->size			=	str->len+1;
	str->str			=	malloc_c(str->size);
	if (str->str == PTR_NULL)return 0;
	memcpy_c				(str->str,toto,str->len+1);
	return 1;
}


OS_API_C_FUNC(int) make_utf8_string(struct string *dst, const struct string *src)
{
	size_t n, totSz;
	totSz = 0;

	for (n = 0; n < src->len; n++)
	{
		char buffer[8];
		size_t size = 8;
		utf8_encode(src->str[n], buffer, &size);
		totSz += size;
	}

	dst->len = totSz;
	dst->size = dst->len + 1;
	dst->str = malloc_c(dst->size);

	totSz = 0;
	for (n = 0; n < src->len; n++)
	{
		size_t size = 8;
		utf8_encode(src->str[n], &dst->str[totSz], &size);
		totSz += size;
	}

	
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
	size_t	src_len;

	src_len				=	strlen_c(src);
	if(src_len==0)return (int)str->len;


	strbuffer_append_bytes(str, src, src_len);

	/*
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
	*/
	return (int)str->len;
}


OS_API_C_FUNC(int) cat_cstring_p(struct string *str, const char *src)
{
	size_t		src_len;

	src_len = strlen_c(src);
	if (src_len == 0)return (int)str->len;

	strbuffer_append_byte (str, path_sep);
	strbuffer_append_bytes(str, src, src_len);
	/*
	new_len = str->len + src_len+1;
	str->size = new_len + 1;

	if (str->str != NULL)
		str->str = realloc_c(str->str, str->size);
	else
	{
		str->len = 0;
		str->str = malloc_c(str->size);
	}

	str->str[str->len] = path_sep;
	memcpy_c(&str->str[str->len+1], src, src_len + 1);
	str->len = new_len;
	*/
	return (int)str->len;
}

OS_API_C_FUNC(int) cat_ncstring(struct string *str, const char *src, size_t src_len)
{
	size_t cpy_len,n;

	if (src_len == 0)return str->len;

	cpy_len = 0;
	n		= 0;
	while (n<src_len)
	{
		if (src[n++] == 0)break;
		cpy_len++;
	}
	if (cpy_len == 0)return 0;


	strbuffer_append_bytes(str, src, cpy_len);
	

	/*
	new_len				=	str->len+cpy_len;
	str->size			=	new_len+1;
	if(str->str!=NULL)
		str->str=realloc_c(str->str,str->size);
	else
		str->str=malloc_c(str->size);

	memcpy_c	(str->str + str->len , src, cpy_len);
	str->len			= new_len;
	str->str[str->len]	= 0;
	*/
	return (int)str->len;
}

OS_API_C_FUNC(int) cat_ncstring_p(struct string *str, const char *src, size_t src_len)
{


	strbuffer_append_byte(str, path_sep);
	strbuffer_append_bytes(str, src, src_len);

	/*
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
	*/
	return (int)str->len;
}


OS_API_C_FUNC(size_t) cat_string(struct string *str, const struct string *src)
{
	size_t cpy_len, n;

	if(src->len==0)return str->len;

	cpy_len = 0;
	n = 0;

	while (n<src->len)
	{
		if (src->str[n++] == 0)break;
		cpy_len++;
	}
	if (cpy_len == 0)return 0;

	strbuffer_append_bytes(str, src->str, cpy_len);


	/*
	new_len				=	str->len+cpy_len;
	str->size			=	new_len+1;

	if(str->str!=NULL)
		str->str=realloc_c(str->str,str->size);
	else
		str->str=malloc_c	(str->size);

	memcpy_c	(&str->str[str->len],src->str,cpy_len);
	str->len = new_len;
	str->str[str->len]=0;
	*/
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
	if (data_ptr == PTR_NULL)
		return 0;

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
	size_t		src_len;
	char		buff[32];

	uitoa_s				(i,buff,32,16);

	src_len				=	strlen_c(buff);


	strbuffer_append_bytes(str, buff, src_len);

	/*
	new_len				=	str->len+src_len;

	str->size			=	new_len+1;

	
	if(str->str!=NULL)
		str->str=realloc_c(str->str,str->size);
	else
		str->str=malloc_c(str->size);

	memcpy_c	(&str->str[str->len],buff,src_len+1);
	str->len = new_len;
	*/
	return (int)str->len;
}

OS_API_C_FUNC(int) strcat_int(struct string *str, int i)
{
	size_t		src_len;
	char		buff[32];

	itoa_s				(i,buff,32,10);

	src_len				=	strlen_c(buff);

	strbuffer_append_bytes(str, buff, src_len);

	/*
	new_len				=	str->len+src_len;

	str->size			=	new_len+1;
	if(str->str!=NULL)
		str->str=realloc_c(str->str,str->size);
	else
		str->str=malloc_c(str->size);

	memcpy_c(&str->str[str->len],buff,src_len+1);
	str->len = new_len;
	*/

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


OS_API_C_FUNC(int) chopChars(struct string *str,size_t mov_len)
{
	if (str->str == NULL)return 0;
	if (str->len == 0)return 0;

	if (mov_len>str->len)
		mov_len = str->len;

	str->len -= mov_len;
	if (str->len>0)
		memmove_c(str->str, &str->str[mov_len], str->len);

	str->str[str->len] = 0;
	return (int)mov_len;
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


static const char b58digits_ordered[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

OS_API_C_FUNC(int) b58enc(const struct string *in, struct string *out)
{
	const unsigned char *bin = in->str;
	unsigned char *buf;
	int carry;
	size_t i, j, high, zcount = 0;
	size_t size;

	while (zcount < in->len && !bin[zcount])
		++zcount;

	size = (in->len - zcount) * 138 / 100 + 1;
	buf = malloc_c(size);
	memset_c(buf, 0, size);

	for (i = zcount, high = size - 1; i < in->len; ++i, high = j)
	{
		for (carry = bin[i], j = size - 1; (j > high) || carry; --j)
		{
			carry += 256 * buf[j];
			buf[j] = carry % 58;
			carry /= 58;
			if (!j) {
				// Otherwise j wraps to maxint which is > high
				break;
			}
		}
	}

	out->len = size+1;
	out->size = out->len + 1;
	out->str = malloc_c(out->size);

	if (zcount)
		memset_c(out->str, '1', zcount);

	for (i = zcount; j < size; ++i, ++j)
		out->str[i] = b58digits_ordered[buf[j]];

	out->str[i] = 0;

	for (i = 0; out->str[i] == '1'; i++);
	{
		zcount++;
	}

	out->len-= zcount;
	memmove_c(out->str, out->str + zcount, out->len);
	out->str[out->len] = 0;

	free_c(buf);
	return 1;
}

static const unsigned char base64_table[65] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

OS_API_C_FUNC(int) base64_decode(const unsigned char *src, size_t len, unsigned char *out, size_t *out_len)
{
	unsigned char dtable[256], *pos, block[4], tmp;
	size_t i, count, olen;
	int pad = 0;

	memset_c(dtable, 0x80, 256);

	for (i = 0; i < sizeof(base64_table) - 1; i++)
		dtable[base64_table[i]] = (unsigned char)i;
	dtable['='] = 0;

	count = 0;
	for (i = 0; i < len; i++) {
		if (dtable[src[i]] != 0x80)
			count++;
	}

	if (count == 0 || count % 4)
		return 0;

	olen = count / 4 * 3;
	pos = out;

	if (out == NULL)
		return 0;

	count = 0;
	for (i = 0; i < len; i++) {
		tmp = dtable[src[i]];
		if (tmp == 0x80)
			continue;

		if (src[i] == '=')
			pad++;
		block[count] = tmp;
		count++;
		if (count == 4) {
			*pos++ = (block[0] << 2) | (block[1] >> 4);
			*pos++ = (block[1] << 4) | (block[2] >> 2);
			*pos++ = (block[2] << 6) | block[3];
			count = 0;
			if (pad) {
				if (pad == 1)
					pos--;
				else if (pad == 2)
					pos -= 2;
				else {
					/* Invalid padding */
					return 0;
				}
				break;
			}
		}
	}

	*out_len = pos - out;
	return 1;
}
OS_API_C_FUNC(int) strbuffer_append(struct string *strbuff, const char *string)
{
	return strbuffer_append_bytes(strbuff, string, strlen_c(string));
}

OS_API_C_FUNC(int) strbuffer_append_byte(struct string *strbuff, char byte)
{
	return strbuffer_append_bytes(strbuff, &byte, 1);
}

OS_API_C_FUNC(int) strbuffer_append_bytes(struct string *strbuff, const char *data, size_t size)
{
	if (strbuff->str == PTR_NULL)
	{
		strbuff->len = 0;
		strbuff->size = size+1;
		strbuff->str = malloc_c(strbuff->size);

	}
	else if ((strbuff->len + size) >= strbuff->size)
	{
		size_t new_size;

		// avoid integer overflow 
		if (strbuff->size > STRBUFFER_SIZE_MAX / STRBUFFER_FACTOR
			|| size > STRBUFFER_SIZE_MAX - 1
			|| strbuff->len > STRBUFFER_SIZE_MAX - 1 - size)
			return -1;

		new_size = strbuff->len + size + 1;

		if ((strbuff->size * STRBUFFER_FACTOR)>new_size)
			new_size = strbuff->size * STRBUFFER_FACTOR;
	

		strbuff->size = new_size;
		strbuff->str = realloc_c(strbuff->str, strbuff->size);
	}

	if (strbuff->str == PTR_NULL)
		return -1;


	memcpy_c(strbuff->str + strbuff->len, data, size);
	strbuff->len += size;
	strbuff->str[strbuff->len] = '\0';
	

	return 0;
}

OS_API_C_FUNC(int) parse_query_line(const struct string *line, size_t *offset, struct key_val *key)
{
	const char  *data;
	enum op_type op;
	size_t		data_sz;
	unsigned int i;

	for (i = ((offset == PTR_NULL) ? 0 : (*offset)); i < line->len; i++)
	{
		if ((line->str[i] == '=') || (line->str[i] == '>') || (line->str[i] == '<'))
		{
			size_t n, klen = 0;
			if (i < 1)continue;
			switch (line->str[i])
			{
			case '=':op = (line->str[i - 1] == '*') ? CMPL_E : CMP_E; break;
			case '>':op = (line->str[i - 1] == '*') ? CMPL_G : CMP_G; break;
			case '<':op = (line->str[i - 1] == '*') ? CMPL_L : CMP_L; break;
			case '!':op = (line->str[i - 1] == '*') ? CMPL_N : CMP_N; break;
			}
			data = line->str + (i + 1);
			data_sz = line->len - (i);
			while ((*data) == ' ') { data++; data_sz--; }

			for (n = 0; n < data_sz; n++)
			{
				if (!isalpha_c(line->str[n]))break;
				key->key[klen++] = line->str[n];
				if (klen >= 31)break;
			}
			key->key[klen] = 0;
			key->kcrc = calc_crc32_c(key->key, 32);

			data = line->str + (i + 1);
			data_sz = line->len - (i + 1);
			key->value.len = 0;
			while ((key->value.len < data_sz) && ((data[key->value.len] == '_') || (data[key->value.len] == '-') || (isalpha_c(data[key->value.len]) || (isdigit_c(data[key->value.len]))))) { key->value.len++; }

			key->value.str = malloc_c(key->value.len + 1);
			memcpy_c(key->value.str, data, key->value.len);
			key->value.str[key->value.len] = 0;
			key->op = op;

			if (offset != PTR_NULL)
				(*offset) = i + 1 + key->value.len;

			return 1;
		}
	}
	return 0;
}