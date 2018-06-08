#ifndef LIBC_API	
#define LIBC_API	C_IMPORT
#endif

typedef void			*mem_ptr;
typedef const void		*const_mem_ptr;

typedef size_t			mem_size;

struct big64
{
	union N
	{
		uint64_t v64;
		unsigned int v[2];
	}n;

	union N m;
}struct_packed;

struct big128
{
	unsigned int v[4];
}struct_packed;

#ifdef __cplusplus
	extern "C" {
#endif

		
LIBC_API mem_ptr		C_API_FUNC memcpy_c				(mem_ptr dst_ptr,const_mem_ptr src_ptr,mem_size size);


LIBC_API mem_ptr		C_API_FUNC memmove_c			(mem_ptr dst_ptr,const_mem_ptr src_ptr,mem_size size);
LIBC_API mem_ptr		C_API_FUNC memset_c				(mem_ptr ptr,unsigned char v,mem_size size);
LIBC_API mem_ptr		C_API_FUNC memset_32_c			(mem_ptr ptr,unsigned int v,mem_size size);
LIBC_API int			C_API_FUNC memcmp_c				(const_mem_ptr ptr1,const_mem_ptr ptr2,size_t size);
LIBC_API const_mem_ptr	C_API_FUNC memchr_c				(const_mem_ptr ptr,int value,mem_size size);
LIBC_API size_t			C_API_FUNC memchr_32_c			(const_mem_ptr ptr,unsigned int value,mem_size size);
LIBC_API void			C_API_FUNC qsort_c				(mem_ptr base, mem_size num, mem_size width,  int ( *comp)(const_mem_ptr,const_mem_ptr));

LIBC_API mem_ptr		C_API_FUNC realloc_c			(mem_ptr ptr,mem_size sz);
LIBC_API void			C_API_FUNC free_c				(mem_ptr ptr);
LIBC_API mem_ptr		C_API_FUNC malloc_c				(mem_size sz);
LIBC_API mem_ptr		C_API_FUNC calloc_c				(mem_size sz,mem_size blk);
LIBC_API mem_ptr		C_API_FUNC get_next_aligned_ptr (mem_ptr ptr);

LIBC_API uint64_t		C_API_FUNC mul64(uint64_t a, uint64_t b);
LIBC_API uint64_t		C_API_FUNC muldiv64(uint64_t a, uint64_t b, uint64_t c);
LIBC_API uint64_t		C_API_FUNC shl64(uint64_t a, unsigned char n);
LIBC_API uint64_t		C_API_FUNC shr64(uint64_t a, unsigned char n);
LIBC_API double			C_API_FUNC exp_c(double a);
LIBC_API void			C_API_FUNC big128_mul(unsigned int x, struct big64 y, struct big128 *out);




LIBC_API unsigned int	ASM_API_FUNC		compare_z_exchange_c(volatile unsigned int *data, unsigned int new_value);
LIBC_API unsigned int	ASM_API_FUNC		compare_exchange_c  (volatile unsigned int *data,unsigned int value, unsigned int new_value);


LIBC_API short			ASM_API_FUNC		fetch_add_c(volatile short *data, volatile short new_value);
LIBC_API char			ASM_API_FUNC		mfence_c();


LIBC_API unsigned int	ASM_API_FUNC		calc_crc32_c(const char *, size_t);



#ifdef __cplusplus
	}
#endif

#ifndef NULL 
#define NULL (void *)0x00000000
#endif


