/*copyright iadix 2016*/
#define LIBC_API C_EXPORT
#include "std_def.h"
#include "std_mem.h"
#include "std_str.h"
#include "mem_base.h"

#include "strs.h"


typedef struct
{
	unsigned short	id;
	char			prefix[16];
	unsigned char	color;

}kernel_str_log_t;

extern const char		hex_chars[];


mem_zone_ref		kernel_logs_ref	=	{PTR_INVALID};
unsigned int		num_kern_logs	=	0xFFFFFFFF;
mem_ptr				write_sema		=	{PTR_INVALID};





#define MAX_TEXT_BUFFER_LINES 1024
#define MAX_LINE_CHAR 1024


void init_kernel_log()
{
	num_kern_logs				=	0;
	kernel_logs_ref.zone		=	PTR_NULL;
	allocate_new_zone			(0,sizeof(kernel_str_log_t)*128,&kernel_logs_ref);

	
}





OS_API_C_FUNC(void) strrev_c(unsigned char *str)
{
	int i;
	int j;
	unsigned char a;
	unsigned len = strlen_c((const char *)str);
	for (i = 0, j = len - 1; i < j; i++, j--)
	{
		a = str[i];
		str[i] = str[j];
		str[j] = a;
	}
}


static const unsigned char b58digits_map[] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, -1, -1, -1, -1, -1, -1,
	-1, 9, 10, 11, 12, 13, 14, 15, 16, -1, 17, 18, 19, 20, 21, -1,
	22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, -1, -1, -1, -1, -1,
	-1, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, -1, 44, 45, 46,
	47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, -1, -1, -1, -1, -1,
};

OS_API_C_FUNC(int) b58tobin(void *bin, size_t *binszp, const char *b58, size_t b58sz)
{
	size_t binsz = *binszp;
	const unsigned char *b58u = (void*)b58;
	unsigned char *binu = bin;
	size_t outisz = (binsz + 3) / 4;
	unsigned long *outi;
	uint64_t t;
	unsigned long c;
	size_t i, j;
	unsigned char bytesleft = binsz % 4;
	unsigned long zeromask = bytesleft ? (0xffffffff << (bytesleft * 8)) : 0;
	unsigned zerocount = 0;

	if (!b58sz)
		b58sz = strlen_c(b58);

	outi = malloc_c(outisz * 4 + 1);

	memset_c(outi, 0, outisz * sizeof(*outi));

	// Leading zeros, just count
	for (i = 0; i < b58sz && b58u[i] == '1'; ++i)
		++zerocount;

	for (; i < b58sz; ++i)
	{
		if (b58u[i] & 0x80)
		{
			// High-bit set on invalid digit
			free_c(outi);
			return 0;
		}
		if (b58digits_map[b58u[i]] == -1)
		{
			// Invalid base58 digit
			free_c(outi);
			return 0;
		}
		c = (unsigned)b58digits_map[b58u[i]];
		for (j = outisz; j--;)
		{
			t = ((uint64_t)outi[j]) * 58 + c;
			c = (t & 0x3f00000000) >> 32;
			outi[j] = t & 0xffffffff;
		}
		if (c)
		{
			// Output number too big (carry to the next int32)
			free_c(outi);
			return 0;
		}
		
			
		if (outi[0] & zeromask)
		{
			// Output number too big (last int32 filled too far)
			free_c(outi);
			return 0;
		}
	}

	j = 0;
	switch (bytesleft) {
	case 3:
		*(binu++) = (outi[0] & 0xff0000) >> 16;
	case 2:
		*(binu++) = (outi[0] & 0xff00) >> 8;
	case 1:
		*(binu++) = (outi[0] & 0xff);
		++j;
	default:
		break;
	}

	for (; j < outisz; ++j)
	{
		*(binu++) = (outi[j] >> 0x18) & 0xff;
		*(binu++) = (outi[j] >> 0x10) & 0xff;
		*(binu++) = (outi[j] >> 8) & 0xff;
		*(binu++) = (outi[j] >> 0) & 0xff;
	}

	// Count canonical base58 byte count
	binu = bin;
	for (i = 0; i < binsz; ++i)
	{
		if (binu[i])
			break;
		--*binszp;
	}
	*binszp += zerocount;

	free_c(outi);

	return 1;
}


OS_API_C_FUNC(int) itoa_s(int num, char* str, size_t len, int base)
{
	int sum;
	int i = 0, sign;
	int digit;

	if (len == 0)
		return 0;

	if (num < 0) 
	{
		num = -num;
		sign = 1;
	}
	else
		sign = 0;

	sum = num;

	do
	{
		digit = sum % base;
		if (digit < 0xA)
			str[i++] = '0' + digit;
		else
			str[i++] = 'A' + digit - 0xA;
		sum /= base;
	} while (sum && (i < (len - sign - 2)));
	

	if (sign)
		str[i++] = '-';
	
	str[i] = 0;

	strrev_c(str);

	if (i == (len - sign - 2) && sum)
		return 0;

	
	return 1;
}

OS_API_C_FUNC(int) uitoa_s(unsigned int num, char* str, size_t len, int base)
{
	unsigned int sum = num;
	int i = 0;
	unsigned int digit;

	if (len == 0)
		return 0;
	if (base == 0)
		return 0;

	do
	{
		digit = sum % base;
		if (digit < 0xA)
			str[i++] = '0' + digit;
		else
			str[i++] = 'A' + digit - 0xA;
		sum /= base;
	} while (sum && (i < (len - 2)));

	str[i] = 0;

	strrev_c(str);

	if (i == (len - 2) && sum)
		return 0;



	return 1;
}


OS_API_C_FUNC(int) luitoa_s(uint64_t value, char *str, size_t len, int base)
{
	uint64_t sum = value;
	int i = 0;
	unsigned int digit;

	if (len == 0)
		return 0;
	do
	{
		digit = sum % base;
		if (digit < 0xA)
			str[i++] = '0' + digit;
		else
			str[i++] = 'A' + digit - 0xA;
		sum /= base;
	} while (sum && (i < (len - 2)));

	str[i] = 0;

	strrev_c(str);

	if (i == (len - 2) && sum)
		return 0;

	

	return 1;
}
OS_API_C_FUNC(int) litoa_s(int64_t value, char *str, size_t len, int base)
{
	int64_t sum;
	int i = 0,sign;
	unsigned int digit;

	if (len == 0)
		return 0;

	if (value < 0)
	{
		value = -value;
		sign = 1;
	}
	else
		sign = 0;

	sum = value;

	do
	{
		digit = sum % base;
		if (digit < 0xA)
			str[i++] = '0' + digit;
		else
			str[i++] = 'A' + digit - 0xA;
		sum /= base;
	} while (sum && (i < (len - sign - 2)));


	if (sign)
		str[i++] = '-';
	
	str[i] = 0;

	strrev_c(str);

	if (i == (len - sign - 2) && sum)
		return 0;

	

	return 1;
}


OS_API_C_FUNC(char *)strncpy_c(char *string,const char *src_string,size_t		 cnt)
{
	unsigned int n;
	char src;
	
	n			=0;
	while(((src=src_string[n])!=0))
	{
		if(n>=cnt)break;
		string[n++]=src;
	}
	string[n]=0;

	return string;
}


OS_API_C_FUNC(char *)strncpy_cs(char *string,size_t str_len,const char *src_string,size_t cnt)
{
	unsigned int n;
	char src;
	
	n			=0;
	while(((src=src_string[n])!=0))
	{
		if((n+1)>=str_len)break;
		if(n>=cnt)break;
		string[n]=src;
		n++;
	}
	string[n]=0;

	return string;
}

OS_API_C_FUNC(char *)strcpy_c(char *string,const char *src_string)
{
	unsigned int n;
	char src;
	
	n			=0;
	while(((src=src_string[n])!=0))
	{
		string[n]=src;
		n++;
	}
	string[n]=0;

	return string;
}

OS_API_C_FUNC(char *)str_replace_char_c(char *string, char c1, char c2)
{
	unsigned int n;
	char src;

	n = 0;
	while (((src = string[n]) != 0))
	{
		if (src==c1)
			string[n] = c2;
		n++;
	}
	return string;
}

/*
OS_API_C_FUNC(char *)strcpy(char *string,const char *src_string)
{
	unsigned int n;
	char src;
	
	n			=0;
	while(((src=src_string[n])!=0))
	{
		string[n]=src;
		n++;
	}
	string[n]=0;

	return string;
}
*/
OS_API_C_FUNC(int)  strcpy_cs(char *string,size_t size,const char *src_string)
{
	unsigned int n;
	char src;
	if(src_string==PTR_NULL)return -1;
	if(string==PTR_NULL)return -1;
	if(size==0)return 0;
	n			=0;
	while(((src=src_string[n])!=0))
	{
		if((n+1)>=size)break;
		string[n]=src;
		n++;
	}
	string[n]=0;

	return 0;
}

OS_API_C_FUNC(int)  strcat_cs(char *string,size_t size,const char *src_string)
{
	unsigned int n;
	unsigned int dst_len;
	char src;
	
	if(src_string==PTR_NULL)return -1;
	if(string==PTR_NULL)return -1;
	if(size==0)return 0;

	dst_len=0;
	while(string[dst_len]!=0){ dst_len++; }

	n			=0;
	while(((src=src_string[n])!=0))
	{
		if((dst_len+n+1)>=size)break;
		string[dst_len+n]=src;
		n++;
	}
	string[dst_len+n]=0;

	return 0;
}


OS_API_C_FUNC(int)  strcat_escaped_s(char *string,size_t size,const char *src_string)
{
	unsigned int src_n,dst_n,dst_len;
	char src;
	if(src_string==PTR_NULL)return -1;
	if(string==PTR_NULL)return -1;
	if(size==0)return 0;

	dst_len=0;
	while(string[dst_len]!=0){ dst_len++; }

	src_n	=0;
	dst_n	=dst_len;
	while(((src=src_string[src_n])!=0))
	{
		

		if(isalpha_c(src)||isdigit_c(src)||(src=='_'))
		{
			if((dst_n+1)>=size)break;
			string[dst_n]=src;
			dst_n++;
		}
		else
		{
			if((dst_n+3)>=size)break;
			string[dst_n]  ='\\';
			string[dst_n+1]=hex_chars[(src&0xF0)>>4];
			string[dst_n+2]=hex_chars[(src&0xF)];
			dst_n+=3;
		}
		src_n++;
	}
	string[dst_n]=0;

	return 0;
}


int  strcat_cstrval_s(char *string,size_t size,const char *name,const char *val)
{
	strcat_cs			(string,size,name);
	strcat_cs			(string,size,":\"");
	strcat_escaped_s	(string,size,val);
	strcat_cs			(string,size,"\"");

	return 1;
}

OS_API_C_FUNC(int)  strcat_intval_s(char *string,size_t size,const char *name,int val,int radix)
{
	size_t l;
	strcat_cs	(string,size,name);
	strcat_cs	(string,size,":");
	if(radix==16)
		strcat_cs	(string,size,"0x");

	l=strlen_c	(string);
	itoa_s		(val,&string[l],size-l,radix);

	return 1;
}

OS_API_C_FUNC(int)  strcat_uintval_s(char *string,size_t size,const char *name,unsigned int val,int radix)
{
	size_t l;
	strcat_cs	(string,size,name);
	strcat_cs	(string,size,":");
	if(radix==16)
		strcat_cs	(string,size,"0x");

	l=strlen_c	(string);
	uitoa_s		(val,&string[l],size-l,radix);

	return 1;
}

OS_API_C_FUNC(int)  strcat_c(char *string,const char *src_string)
{
	unsigned int n;
	char src;
	char dst_len;
	if(src_string==PTR_NULL)return -1;
	if(string==PTR_NULL)return -1;

	dst_len=0;
	while(string[dst_len]!=0){ dst_len++; }

	n			=0;
	while(((src=src_string[n])!=0))
	{
		string[dst_len+n]=src;
		n++;
	}
	string[dst_len+n]=0;

	return 0;
}

OS_API_C_FUNC(int)  strncat_c(char *string,const char *src_string,size_t max)
{
	size_t n;
	char src;
	size_t dst_len;
	if(src_string==PTR_NULL)return -1;
	if(string==PTR_NULL)return -1;

	dst_len=0;
	while(string[dst_len]!=0){ dst_len++; }

	n			=0;
	while(((src=src_string[n])!=0)&&(n<max))
	{
		string[dst_len+n]=src;
		n++;
	}
	string[dst_len+n]=0;

	return 0;
}

 OS_API_C_FUNC(int)  strcmp_c(const char *string1,const char *string2)
{
	int		n;
	char	c1='0',c2='0';

	if (string1 == PTR_NULL)return -1;
	n = strlen_c(string1);

	if (string2 == PTR_NULL){
		if (n==0)
			return 0;
		else
			return -1;
	}

	n=0;
	while((c1!=0)&&(c2!=0))
	{
		c1=string1[n];
		c2=string2[n];
		if(c1<c2)return -1;
		if(c1>c2)return 1;
		n++;
	}
	return 0;
}
 OS_API_C_FUNC(int)  stricmp_c(const char *string1,const char *string2)
{
	int		n;
	char	c1 = '0', c2 = '0';

	n=0;
	while((c1!=0)&&(c2!=0))
	{
		c1=toupper_c(string1[n]);
		c2=toupper_c(string2[n]);
		if(c1<c2)return -1;
		if(c1>c2)return 1;
		n++;
	}
	return 0;
}

 OS_API_C_FUNC(int)  strncmp_c(const char *string1,const char *string2,size_t len)
{
	unsigned int		n;
	char	c1 = '0', c2 = '0';

	if (string1 == PTR_NULL)return -1;
	if (string2 == PTR_NULL)return 1;
	

	n=0;
	while((c1!=0)&&(c2!=0))
	{
		if(n>=len)return 0;
		c1=string1[n];
		c2=string2[n];
		if(c1<c2)return -1;
		if(c1>c2)return 1;
		n++;
	}
	return 0;
}

 OS_API_C_FUNC(int)  strincmp_c(const char *string1,const char *string2,size_t len)
{
	unsigned int		n;
	char	c1 = '0', c2 = '0';

	n=0;
	while((c1!=0)&&(c2!=0))
	{
		if(n>=len)return 0;
		c1=toupper_c(string1[n]);
		c2=toupper_c(string2[n]);
		if(c1<c2)return -1;
		if(c1>c2)return 1;
		n++;
	}
	return 0;
}
OS_API_C_FUNC(size_t) strlen_c(const char *string)
{
	int n=0;
	if (string == PTR_NULL)return 0;
	
	while(string[n]!=0){n++;}

	return n;
}

OS_API_C_FUNC(char) first_char(const char *str)
{
	while((*str)!=0)
	{
		if((*str)!=' ')
			return (*str);

		str++;
	}

	return (*str);
}



OS_API_C_FUNC(size_t) strlpos_c(const char *string,size_t ofset,char c)
{
	size_t		n;
	char		sc;

	n=ofset;

	while((sc=string[n])!=0)
	{ 
		if(sc==c)return n;
		n++;
	}
	return INVALID_SIZE;

}
OS_API_C_FUNC(const char *) strrchr_c		(const char *string,int c)
{
	size_t		n;
	
	n	=	strlen_c(string);
	while(n>0)
	{ 
		if(string[n]==c)return &string[n];

		n--;
	}

	return PTR_NULL;


}

OS_API_C_FUNC(size_t) strrpos_c(const char *string,char c)
{
	size_t		n;
	n=strlen_c(string);
	while(n>0)
	{ 
		if(string[n]==c)return (n+1);

		n--;
	}

	return 0;

}


OS_API_C_FUNC(const char *) strstr_c(const char *buf, const char *sub)
{
    const char *bp;
    const char *sp;

    if (!*sub)
	return buf;
    while (*buf) {
	bp = buf;
	sp = sub;
	do {
	    if (!*sp)
		return buf;
	} while (*bp++ == *sp++);
	buf += 1;
    }
	return PTR_NULL;
}





OS_API_C_FUNC(int) isupper_c(int c)
{
    return (c >= 'A' && c <= 'Z');
}

OS_API_C_FUNC(int) islower_c(int c)
{
    return (c >= 'a' && c <= 'z');
}


OS_API_C_FUNC(int) isalpha_c(int c)
{
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}


OS_API_C_FUNC(int) isspace_c(int c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\12');
}

OS_API_C_FUNC(int) isdigit_c(int c)
{
    return (c >= '0' && c <= '9');
}
OS_API_C_FUNC(int) isxdigit_c(int c)
{
	c = toupper_c(c);
	  return ((c >= '0' && c <= '9')||(c >= 'A' && c <= 'F'));
}

OS_API_C_FUNC(int) toupper_c(int c)
{

	if (islower_c(c)) return(c - ('a'-'A'));
	return(c);
}




/*
 * Convert a string to a long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
OS_API_C_FUNC(unsigned long) strtoul_c(const char *nptr, char **endptr, int base)
{
	register const char *s = nptr;
	register unsigned long acc;
	register unsigned long cutoff;
	register int c;
	register int any, cutlim;

	/*
	 * Skip white space and pick up leading +/- sign if any.
	 * If base is 0, allow 0x for hex and 0 for octal, else
	 * assume decimal; if base is already 16, allow 0x.
	 */
	do {
		c = *s++;
	} while (isspace_c(c));
	

	if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	} else if ((base == 0 || base == 2) &&  c == '0' && (*s == 'b' || *s == 'B')) {
		c = s[1];
		s += 2;
		base = 2;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;

	/*
	 * Compute the cutoff value between legal numbers and illegal
	 * numbers.  That is the largest legal value, divided by the
	 * base.  An input number that is greater than this value, if
	 * followed by a legal input character, is too big.  One that
	 * is equal to this value may be valid or not; the limit
	 * between valid and invalid numbers is then based on the last
	 * digit.  For instance, if the range for longs is
	 * [-2147483648..2147483647] and the input base is 10,
	 * cutoff will be set to 214748364 and cutlim to either
	 * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
	 * a value > 214748364, or equal but the next digit is > 7 (or 8),
	 * the number is too big, and we will return a range error.
	 *
	 * Set any if any `digits' consumed; make it negative to indicate
	 * overflow.
	 */
	cutoff = ULONG_MAX;
	cutlim = cutoff % (unsigned long)base;
	cutoff /= (unsigned long)base;
	for (acc = 0, any = 0;; c = *s++) {
		if (isdigit_c(c))
			c -= '0';
		else if (isalpha_c(c))
			c -= isupper_c(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;


		if (any < 0 || acc > cutoff || acc == cutoff && c > cutlim)
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = ULONG_MAX;
		/* errno = ERANGE; */
	}
	if (endptr != 0)
		*endptr = (char *)(any ? s - 1 : nptr);
	return (acc);
}


/*
* Convert a string to a long integer.
*
* Ignores `locale' stuff.  Assumes that the upper and lower case
* alphabets and digits are each contiguous.
*/
OS_API_C_FUNC(int64_t) strtoll_c(const char *nptr, char **endptr, int base)
{
	register const char *s = nptr;
	register uint64_t acc;
	register int c;
	register uint64_t cutoff;
	register int neg = 0, any, cutlim;

	/*
	* Skip white space and pick up leading +/- sign if any.
	* If base is 0, allow 0x for hex and 0 for octal, else
	* assume decimal; if base is already 16, allow 0x.
	*/
	do {
		c = *s++;
	} while (isspace_c(c));
	if (c == '-') {
		neg = 1;
		c = *s++;
	}
	else if (c == '+')
		c = *s++;
	if ((base == 0 || base == 16) &&
		c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	else if ((base == 0 || base == 2) &&
		c == '0' && (*s == 'b' || *s == 'B')) {
		c = s[1];
		s += 2;
		base = 2;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;

	/*
	* Compute the cutoff value between legal numbers and illegal
	* numbers.  That is the largest legal value, divided by the
	* base.  An input number that is greater than this value, if
	* followed by a legal input character, is too big.  One that
	* is equal to this value may be valid or not; the limit
	* between valid and invalid numbers is then based on the last
	* digit.  For instance, if the range for longs is
	* [-2147483648..2147483647] and the input base is 10,
	* cutoff will be set to 214748364 and cutlim to either
	* 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
	* a value > 214748364, or equal but the next digit is > 7 (or 8),
	* the number is too big, and we will return a range error.
	*
	* Set any if any `digits' consumed; make it negative to indicate
	* overflow.
	*/
	cutoff = neg ? -(unsigned long)LONGLONG_MIN : LONGLONG_MAX;
	cutlim = cutoff % (unsigned long)base;
	cutoff /= (unsigned long)base;
	for (acc = 0, any = 0;; c = *s++) {
		if (isdigit_c(c))
			c -= '0';
		else if (isalpha_c(c))
			c -= isupper_c(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || acc == cutoff && c > cutlim)
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = neg ? LONGLONG_MIN : LONGLONG_MAX;
		/*errno = ERANGE;*/
	}
	else if (neg)
		acc = -acc;
	if (endptr != 0)
		*endptr = (char *)(any ? s - 1 : nptr);
	return (acc);
}

/*
 * Convert a string to a long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
OS_API_C_FUNC(long) strtol_c(const char *nptr, char **endptr, int base)
{
	register const char *s = nptr;
	register unsigned long acc;
	register int c;
	register unsigned long cutoff;
	register int neg = 0, any, cutlim;

	/*
	 * Skip white space and pick up leading +/- sign if any.
	 * If base is 0, allow 0x for hex and 0 for octal, else
	 * assume decimal; if base is already 16, allow 0x.
	 */
	do {
		c = *s++;
	} while (isspace_c(c));
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else if (c == '+')
		c = *s++;
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	} else if ((base == 0 || base == 2) &&
	    c == '0' && (*s == 'b' || *s == 'B')) {
		c = s[1];
		s += 2;
		base = 2;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;

	/*
	 * Compute the cutoff value between legal numbers and illegal
	 * numbers.  That is the largest legal value, divided by the
	 * base.  An input number that is greater than this value, if
	 * followed by a legal input character, is too big.  One that
	 * is equal to this value may be valid or not; the limit
	 * between valid and invalid numbers is then based on the last
	 * digit.  For instance, if the range for longs is
	 * [-2147483648..2147483647] and the input base is 10,
	 * cutoff will be set to 214748364 and cutlim to either
	 * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
	 * a value > 214748364, or equal but the next digit is > 7 (or 8),
	 * the number is too big, and we will return a range error.
	 *
	 * Set any if any `digits' consumed; make it negative to indicate
	 * overflow.
	 */
	cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
	cutlim = cutoff % (unsigned long)base;
	cutoff /= (unsigned long)base;
	for (acc = 0, any = 0;; c = *s++) {
		if (isdigit_c(c))
			c -= '0';
		else if (isalpha_c(c))
			c -= isupper_c(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || acc == cutoff && c > cutlim)
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = neg ? LONG_MIN : LONG_MAX;
		/*errno = ERANGE;*/
	} else if (neg)
		acc = -acc;
	if (endptr != 0)
		*endptr = (char *)(any ? s - 1 : nptr);
	return (acc);
}


OS_API_C_FUNC(short) strtos_c(const char *nptr, char **endptr, int base)
{
	unsigned int ret;
	
	ret	=	strtol_c(nptr,endptr,base);

	return ((short)(ret));
}
OS_API_C_FUNC(long)		atol_c			(const char *str )
{
	return strtol_c(str,PTR_NULL,10);
}


OS_API_C_FUNC(unsigned int)	get_new_kern_log_id(const char *pref,unsigned char color)
{
	kernel_str_log_t    *new_kern_log;
	kernel_str_log_t    *kern_log,*kern_end;

	kern_log		=	get_zone_ptr(&kernel_logs_ref,0);
	if(kern_log	== PTR_NULL)return 0;
	kern_end		=	get_zone_ptr(&kernel_logs_ref,sizeof(kernel_str_log_t)*num_kern_logs);

	while(kern_log<kern_end)
	{
		if( (kern_log->color==color)&&
			(!strcmp_c(kern_log->prefix,pref)))
		{
			return kern_log->id;
		}

		kern_log++;
	}

	new_kern_log		=	get_zone_ptr(&kernel_logs_ref,sizeof(kernel_str_log_t)*num_kern_logs);
	new_kern_log->id	=	(num_kern_logs+1);
	new_kern_log->color	=	color;
	strcpy_cs				(new_kern_log->prefix,16,pref);
	num_kern_logs++;
	
	return new_kern_log->id;
}


OS_API_C_FUNC(kernel_str_log_t    *) get_kern_log(unsigned int id)
{
	kernel_str_log_t    *kern_log,*kern_end;

	if(id==0)return PTR_NULL;
	if(id==0xFFFFFFFF)return PTR_NULL;

	kern_log		=	get_zone_ptr(&kernel_logs_ref,0);

	if(kern_log	== PTR_NULL)return PTR_NULL;

	kern_end		=	get_zone_ptr(&kernel_logs_ref,sizeof(kernel_str_log_t)*num_kern_logs);

	while(kern_log<kern_end)
	{
		if(kern_log->id==id)
			return kern_log;

		kern_log++;
	}

	return PTR_NULL;
}




/*
OS_API_C_FUNC(int)  writestr_color(const char *text,unsigned char color)
{
	unsigned char original_color;

	original_color	=	get_text_color_c();

	task_manager_aquire_semaphore		(write_sema,0);

	set_text_color_c					(color);
	draw_car_c							(text);
	set_text_color_c					(original_color);

	if(strlpos_c(text,0,'\n')!=INVALID_SIZE)
		task_manager_release_semaphore		(write_sema,1);	

	return 0;

}
*/







OS_API_C_FUNC(unsigned int) write_bits	 (unsigned int data_orig,unsigned int data_bits,unsigned int ofset,unsigned int n_bits)
{
	unsigned int dest_data_mask;
	unsigned int bit_mask;
	unsigned int data_ret;
	unsigned int data_sf;



	bit_mask		 =	 ~(0xFFFFFFFF << n_bits);
	dest_data_mask	 =	 ~(bit_mask << ofset);
	data_ret		 =	 data_orig&dest_data_mask;
	data_sf			 =	 ((data_bits&bit_mask)<<ofset); 
	data_ret		|=	 (data_sf);
	return data_ret;
}


OS_API_C_FUNC(unsigned char ) write_bits_8	 (unsigned char data_orig,unsigned char data_bits,unsigned char ofset,unsigned char n_bits)
{
	unsigned char dest_data_mask;
	unsigned char bit_mask;
	unsigned char data_ret;
	unsigned char data_sf;

	bit_mask		 =	 ~(0xFF << n_bits);
	dest_data_mask	 =	 ~(bit_mask << ofset);
	data_ret		 =	 data_orig&dest_data_mask;
	data_sf			 =	 ((data_bits&bit_mask)<<ofset); 
	data_ret		|=	 (data_sf);
	return data_ret;
}

OS_API_C_FUNC(unsigned int) set_bit	 (unsigned int data_orig,unsigned int value,unsigned int ofset)
{
	unsigned int dest_data_mask;
	unsigned int data_ret;

	dest_data_mask	 =	 ~(0x01 << ofset);
	data_ret		 =	 data_orig&dest_data_mask;
	value			 =	 value&0x01;
	data_ret		|=	 (value<<ofset);

	return data_ret;
}



OS_API_C_FUNC(int) tolower_c(int _c)
{
	return 1;
}


OS_API_C_FUNC(int) isprint_c(int _C)
{
	return 1;
}

#if 1
#include <math.h>
#define PZERO 38		/* index of 1e0 in powten[]	*/
#define PMAX 76			/* highest index in powten[]	*/
#define SIGFIGS 8
double powten[] = {1e-38, 1e-37, 1e-36, 1e-35, 1e-34, 1e-33,
	1e-32, 1e-31, 1e-30, 1e-29, 1e-28, 1e-27, 1e-26, 1e-25, 1e-24,
	1e-23, 1e-22, 1e-21, 1e-20, 1e-19, 1e-18, 1e-17, 1e-16, 1e-15,
	1e-14, 1e-13, 1e-12, 1e-11, 1e-10, 1e-9, 1e-8, 1e-7, 1e-6, 1e-5,
	1e-4, 1e-3, 1e-2, 1e-1, 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7,
	1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18,
	1e19, 1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29,
	1e30, 1e31, 1e32, 1e33, 1e34, 1e35, 1e36, 1e37, 1e38};


OS_API_C_FUNC(void) dtoa_c	(char *buff, char conv, int bsize, int dplace, double value)
{
    extern double powten[];
    double modf(), v;
    int i, imax, j, exp, ndigits, nlead;

/* set default value of dplace */
    if (dplace < 0)
	dplace = 6;

/* strip off sign */
    if (value < 0.0){
	value = -value;
	*buff++ = '-';
    }

/* scale and compute no of leading digits */
    if (value == 0.0)
	imax = PZERO;
    else {
	for (imax = PMAX; value < powten[imax] && imax > 0; imax--) {
	    if (conv == 'f' && imax <= PZERO)
		break;
	}
    }
    exp = imax - PZERO;
    nlead = exp + 1;

/* decide if 'g' goes to 'e' or 'f' */
    if (conv == 'g') {
	if (nlead > 6)
	    conv = 'e';
	else
	    conv = 'f';
    }

/* compute no of digits to print */
/* change 'f' to 'e' if insufficient space for result */
    if (conv == 'f') {
	ndigits = dplace + nlead;
	if (ndigits + 3 > bsize) {
	    conv = 'e';
	    dplace = SIGFIGS;
	}
    }
    if (conv != 'f') {
	nlead  = 1;
	ndigits = dplace + nlead;
	if (ndigits > SIGFIGS)
	    ndigits = SIGFIGS;
    }

/* scale to range 1.0 - 10.0 and round up */
    if (conv == 'e' && imax == 0 && value < powten[0]) {
	value *= 10.0;
	exp--;
    }
    value = value / powten[imax] + 0.5 * powten[PZERO - ndigits + 1];
    value = modf(value, &v);
    if (v >= 10.0) {
	*buff++ = '1';
	v -= 10.0;
    }

/* build digit string */
    for (i = 0; i < ndigits; i++) {
	if (i == nlead)
	    *buff++ = '.';
	*buff++ = (int)v + '0';
	value = modf(value * 10.0, &v);
    }

/* if not 'f' print exponent part */
    if (conv != 'f'){
	*buff++ = 'e';
	if (exp < 0){
	    exp = -exp;
	    *buff++ = '-';
	}
	else *buff++ = '+';
	j = exp/10;
	*buff++ = '0' + j;
	*buff++ = '0' + exp - 10*j;
    }
/* print final null */
    *buff++ = '\0';
    return;
}
#endif


OS_API_C_FUNC(uint64_t) load_bigendian(const unsigned char *x)
{
	return
		(uint64_t)(x[7]) \
		| (((uint64_t)(x[6])) << 8) \
		| (((uint64_t)(x[5])) << 16) \
		| (((uint64_t)(x[4])) << 24) \
		| (((uint64_t)(x[3])) << 32) \
		| (((uint64_t)(x[2])) << 40) \
		| (((uint64_t)(x[1])) << 48) \
		| (((uint64_t)(x[0])) << 56)
		;
}

OS_API_C_FUNC(void) store_bigendian(unsigned char *x, uint64_t u)
{
	x[7] = u; u >>= 8;
	x[6] = u; u >>= 8;
	x[5] = u; u >>= 8;
	x[4] = u; u >>= 8;
	x[3] = u; u >>= 8;
	x[2] = u; u >>= 8;
	x[1] = u; u >>= 8;
	x[0] = u;
}

OS_API_C_FUNC(void) dtoll_c(double dAmount, uint64_t *nAmount)
{
	(*nAmount) = dAmount;
}