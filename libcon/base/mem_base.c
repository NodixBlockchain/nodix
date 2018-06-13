/*copyright antoine bentue-ferrer 2016*/
#define LIBC_API C_EXPORT
#include "std_def.h"
#include "std_mem.h"
#include "mem_base.h"
#include "std_str.h"
#include "strs.h"
#include <bin_tree.h>

#define KERNEL_API C_EXPORT
#include "mem_stream.h"
#include "tpo_mod.h"
#include "fsio.h"

#include "libc_math.h"

#include <math.h>
#include <stdlib.h>

#define MAX_MEM_AREAS    64
#define MAX_FREE_ZONES   1024*64

//#define MAX_MEM_ZONES  1024*128



extern mem_ptr ASM_API_FUNC get_stack_frame_c();
extern mem_ptr ASM_API_FUNC get_stack_c();
extern mem_ptr ASM_API_FUNC memset_asm(mem_ptr ptr, int value, unsigned int size);
extern mem_ptr ASM_API_FUNC memcpy_asm(mem_ptr ptr, int value, unsigned int size);

extern void	   ASM_API_FUNC scan_threads_stack(mem_ptr lower, mem_ptr upper);
extern void	   ASM_API_FUNC scan_stack_c(mem_ptr lower, mem_ptr upper, mem_ptr stack_frame, mem_ptr stack);

extern void mark_modz_zones(mem_ptr lower_bound, mem_ptr higher_bound);
extern void init_funcs(void);
extern void resume_threads();
extern void init_exit();

LIBC_API void	*	C_API_FUNC kernel_memory_map_c				(unsigned int size);
LIBC_API void		C_API_FUNC kernel_memory_free_c				(mem_ptr ptr);
 
typedef struct
{
	mem_ptr			ptr;
	mem_size		size;
}mem_zone_desc;

typedef struct
{
	mem_zone_desc			mem;
	unsigned short			area_id;
	volatile short			n_refs;
	zone_free_func_ptr		free_func;
#ifdef _DEBUG
	unsigned int			time;
#endif
}mem_zone;

typedef			mem_zone			*mem_zone_ptr;
typedef const	mem_zone_desc		*mem_zone_desc_ptr;

typedef struct
{
	unsigned int			area_id;
	unsigned int			last_used_zone;
	unsigned int			nzones;
	ctime_t					last_gc_time;
	mem_area_type_t			type;
#ifndef NATIVE_ALLOC
	mem_ptr					area_start;
	mem_ptr					area_end;
	mem_zone_desc			zones_free[MAX_FREE_ZONES];
#endif
	mem_zone_ptr			zones_buffer;
	mem_zone_ptr			trash[256];

}mem_area;

mem_area	   *__global_mem_areas = PTR_INVALID;
unsigned int   area_lock = 0xCDCDCDCD;
unsigned int   gc_lock = 0xFFFFFFF;


OS_API_C_FUNC(mem_ptr) memset_c(mem_ptr ptr,unsigned char v,mem_size size)
{
	unsigned char *cptr=ptr;
	while(size--){cptr[size]=v;  }
	return ptr;
}

OS_API_C_FUNC(mem_ptr) memset_32_c(mem_ptr ptr,unsigned int v,mem_size size)
{
	unsigned int *cptr=ptr;
	unsigned int cnt = 0;

	size>>=2;

	for (cnt = 0; cnt < size;cnt ++) { cptr[cnt] = v; }
	return ptr;

}

OS_API_C_FUNC(size_t) memchr_32_c(const_mem_ptr ptr,unsigned int value,mem_size size)
{
	const unsigned int *uint;
	const unsigned int *last_uint;

	if ((size & (~0x03))<4)return INVALID_SIZE;

	uint		=ptr;
	last_uint	=mem_add(ptr,(size & (~0x03))-4);
	
	while(uint<last_uint){ if((*uint)==value)return mem_sub(ptr,uint); uint++;}

	return INVALID_SIZE;

}

OS_API_C_FUNC(const_mem_ptr) memchr_c(const_mem_ptr ptr,int value,mem_size size)
{
	unsigned int n;
	const unsigned char *uchar;

	uchar	=ptr;
	n		=0;
	while(n<size){ if(uchar[n]==value)return &uchar[n]; n++;}

	return PTR_NULL;

}

OS_API_C_FUNC(int) memcmp_c(const_mem_ptr ptr_1,const_mem_ptr ptr_2,size_t size)
{
	unsigned int n;
	const unsigned char *ptr1,*ptr2;
	n=0;
	ptr1=ptr_1;
	ptr2=ptr_2;

	while(n<size){ if(ptr1[n]>ptr2[n])return 1; if(ptr1[n]<ptr2[n])return -1; n++;}
	return 0;

}
OS_API_C_FUNC(mem_ptr) memmove_c(mem_ptr dst_ptr, const_mem_ptr src_ptr, mem_size size)
{
	const unsigned char *sptr = src_ptr;
	unsigned char *dptr = dst_ptr;

	if (mem_to_uint(dptr) < mem_to_uint(sptr))
		memcpy_c(dst_ptr, src_ptr, size);
	else
	{
		unsigned int	n = size;;
		while (n--){ dptr[n] = sptr[n]; }
	}

	return dst_ptr;

}


OS_API_C_FUNC(mem_ptr) memcpy_c(mem_ptr dst_ptr,const_mem_ptr src_ptr,mem_size size)
{
	const unsigned char *sptr	=src_ptr;
	unsigned char *dptr			=dst_ptr;
	unsigned int	n			=0;;

	while(n<size){dptr[n]=sptr[n];n++;}

	return dst_ptr;
	
}

OS_API_C_FUNC(mem_area *) get_area(unsigned int area_id)
{
	int n;
	n=0;
	if(__global_mem_areas==PTR_NULL)return PTR_NULL;
//	if(__global_mem_zones==PTR_NULL)return PTR_NULL;

	if(area_id==0)
	{
		unsigned int m_area_id;
		m_area_id = get_mem_area_id();
		n=0;
		while(n<MAX_MEM_AREAS)
		{
			if (__global_mem_areas[n].area_id == m_area_id)
				return &__global_mem_areas[n];
			n++;
		}
		return PTR_NULL;
	}


	while(n<MAX_MEM_AREAS)
	{
		if(__global_mem_areas[n].area_id==area_id)
			return &__global_mem_areas[n];
		n++;
	}
	return PTR_NULL;
}


int check_zone	(const mem_zone *zone)
{
	mem_area				*area_ptr;
	if(zone->area_id==0xFFFF)return 1;
	

	area_ptr			=	get_area(zone->area_id);
	if(area_ptr==PTR_NULL)
	{
		return 0;
	}
	if((zone<area_ptr->zones_buffer)||(zone>=&area_ptr->zones_buffer[area_ptr->nzones]))
	{
		return 0;
	}
	if((zone->mem.ptr==uint_to_mem(0xFFFFFFFF))&&(zone->mem.size==0))return 1;


	if((zone->mem.ptr==PTR_NULL)&&(zone->mem.size==0))
	{
		return 0;
	}
#ifndef NATIVE_ALLOC
	if((zone->mem.ptr<area_ptr->area_start)||(zone->mem.ptr>area_ptr->area_end))
	{
		return 0;
	}
#endif

	return 1;
}



OS_API_C_FUNC(mem_ptr)	get_zone_ptr(mem_zone_ref_const_ptr ref, mem_size ofset)
{

	mem_ptr			 ret= PTR_INVALID;
	mem_zone_ref	 tmpref = { PTR_NULL };

	if (ref == PTR_NULL)
		return PTR_INVALID;

	while (tmpref.zone != ref->zone)
	{
		tmpref.zone = ref->zone;

		if (tmpref.zone == PTR_NULL)
			return PTR_INVALID;

		if ((((mem_zone *)(tmpref.zone))->mem.size) == 0)return PTR_INVALID;

		if (ofset == 0xFFFFFFFF)
			return mem_add(((mem_zone *)(tmpref.zone))->mem.ptr, ((mem_zone *)(tmpref.zone))->mem.size);

		if ((ofset >= ((mem_zone *)(tmpref.zone))->mem.size))
			return PTR_INVALID;

		ret = mem_add(((mem_zone *)(tmpref.zone))->mem.ptr, ofset);
	}

	return ret;
}

OS_API_C_FUNC(mem_size) get_zone_size(mem_zone_ref_const_ptr ref)
{
	mem_size		 ret = 0;
	mem_zone_ref	 tmpref = { PTR_NULL };

	if (ref == PTR_NULL)
		return PTR_INVALID;

	while (tmpref.zone != ref->zone)
	{
		tmpref.zone = ref->zone;

		if (tmpref.zone == PTR_NULL)
			return 0;

		ret = ((mem_zone *)(tmpref.zone))->mem.size;
	}


	return ret;
}


OS_API_C_FUNC(mem_size) set_zone_free(mem_zone_ref_ptr ref,zone_free_func_ptr	free_func)
{
	if(ref==PTR_NULL) return 0;
	if(ref->zone==PTR_NULL) return 0;

	((mem_zone *)(ref->zone))->free_func=free_func;

	return 1;
}

OS_API_C_FUNC(void) init_mem_system()
{
	if(__global_mem_areas	!= PTR_INVALID)return;

	__global_mem_areas = get_next_aligned_ptr(kernel_memory_map_c(MAX_MEM_AREAS*sizeof(mem_area)+8));
	
	memset_c	(__global_mem_areas,0,MAX_MEM_AREAS*sizeof(mem_area));
	
	area_lock =	0;
	gc_lock = 0;

#ifndef _DEBUG
	sys_add_tpo_mod_func_name("libcon", "init_new_mem_area",(void_func_ptr)init_new_mem_area, 0);
	sys_add_tpo_mod_func_name("libcon", "get_tree_mem_area_id",(void_func_ptr)get_tree_mem_area_id, 0);
	sys_add_tpo_mod_func_name("libcon", "set_tree_mem_area_id",(void_func_ptr)set_tree_mem_area_id, 0);
	sys_add_tpo_mod_func_name("libcon", "get_mem_area_id",(void_func_ptr)get_mem_area_id, 0);
	sys_add_tpo_mod_func_name("libcon", "free_mem_area",(void_func_ptr)free_mem_area, 0);

	sys_add_tpo_mod_func_name("libcon", "dtoll_c", (void_func_ptr)dtoll_c, 0);

	sys_add_tpo_mod_func_name("libcon", "realloc_zone",(void_func_ptr)realloc_zone, 0);
	sys_add_tpo_mod_func_name("libcon", "malloc_c",(void_func_ptr)malloc_c, 0);
	sys_add_tpo_mod_func_name("libcon", "calloc_c",(void_func_ptr)calloc_c, 0);
	sys_add_tpo_mod_func_name("libcon", "memset_c",(void_func_ptr)memset_c, 0);

	sys_add_tpo_mod_func_name("libcon", "do_mark_sweep", (void_func_ptr)do_mark_sweep, 0);
	sys_add_tpo_mod_func_name("libcon", "mark_zone", (void_func_ptr)mark_zone, 0);
	sys_add_tpo_mod_func_name("libcon", "area_type", (void_func_ptr)area_type, 0);
	sys_add_tpo_mod_func_name("libcon", "get_shared_slot", (void_func_ptr)get_shared_slot, 0);
	sys_add_tpo_mod_func_name("libcon", "release_shared_slot", (void_func_ptr)release_shared_slot, 0);
	sys_add_tpo_mod_func_name("libcon", "get_zone_area_type", (void_func_ptr)get_zone_area_type, 0);
	
	sys_add_tpo_mod_func_name("libcon", "mfence_c", (void_func_ptr)mfence_c, 0);

	sys_add_tpo_mod_func_name("libcon", "memset", (void_func_ptr)memset_asm, 0);
	sys_add_tpo_mod_func_name("libcon", "memset_32_c", (void_func_ptr)memset_32_c, 0);
	sys_add_tpo_mod_func_name("libcon", "memcpy", (void_func_ptr)memcpy_asm, 0);

	sys_add_tpo_mod_func_name("libcon", "memcpy_c",(void_func_ptr)memcpy_c, 0);
	sys_add_tpo_mod_func_name("libcon", "memcmp_c",(void_func_ptr)memcmp_c, 0);
	sys_add_tpo_mod_func_name("libcon", "memmove_c",(void_func_ptr)memmove_c, 0);
	sys_add_tpo_mod_func_name("libcon", "memchr_c",(void_func_ptr)memchr_c, 0);
	sys_add_tpo_mod_func_name("libcon", "memchr_32_c",(void_func_ptr)memchr_32_c, 0);




	sys_add_tpo_mod_func_name("libcon", "empty_trash", (void_func_ptr)empty_trash, 0);

	sys_add_tpo_mod_func_name("libcon", "store_bigendian",(void_func_ptr)store_bigendian, 0);
	sys_add_tpo_mod_func_name("libcon", "load_bigendian",(void_func_ptr)load_bigendian, 0);
	
	sys_add_tpo_mod_func_name("libcon", "libc_sinf", (void_func_ptr)libc_sinf, 0);
	sys_add_tpo_mod_func_name("libcon", "libc_cosf", (void_func_ptr)libc_cosf, 0);
	sys_add_tpo_mod_func_name("libcon", "libc_atanf", (void_func_ptr)libc_atanf, 0);
	sys_add_tpo_mod_func_name("libcon", "libc_ftol", (void_func_ptr)libc_ftol, 0);

	sys_add_tpo_mod_func_name("libcon", "libc_sqrtd", (void_func_ptr)libc_sqrtd, 0);
	sys_add_tpo_mod_func_name("libcon", "libc_sind", (void_func_ptr)libc_sind, 0);
	sys_add_tpo_mod_func_name("libcon", "powd_c", (void_func_ptr)powd_c, 0);
	sys_add_tpo_mod_func_name("libcon", "libc_cosd", (void_func_ptr)libc_cosd, 0);
	sys_add_tpo_mod_func_name("libcon", "libc_atand", (void_func_ptr)libc_atand, 0);

	sys_add_tpo_mod_func_name("libcon", "allocate_new_zone",(void_func_ptr)allocate_new_zone, 0);
	sys_add_tpo_mod_func_name("libcon", "allocate_new_empty_zone",(void_func_ptr)allocate_new_empty_zone, 0);
	sys_add_tpo_mod_func_name("libcon", "expand_zone",(void_func_ptr)expand_zone, 0);
	sys_add_tpo_mod_func_name("libcon", "get_next_aligned_ptr",(void_func_ptr)get_next_aligned_ptr, 0);
	sys_add_tpo_mod_func_name("libcon", "kernel_memory_map_c",(void_func_ptr)kernel_memory_map_c, 0);
	sys_add_tpo_mod_func_name("libcon", "inc_zone_ref",(void_func_ptr)inc_zone_ref, 0);
	sys_add_tpo_mod_func_name("libcon", "swap_zone_ref",(void_func_ptr)swap_zone_ref, 0);

	sys_add_tpo_mod_func_name("libcon", "set_zone_free",(void_func_ptr)set_zone_free, 0);
	sys_add_tpo_mod_func_name("libcon", "free_c",(void_func_ptr)free_c, 0);
	sys_add_tpo_mod_func_name("libcon", "find_zones_used", (void_func_ptr)find_zones_used, 0);

	sys_add_tpo_mod_func_name("libcon", "dec_zone_ref",(void_func_ptr)dec_zone_ref, 0);
	sys_add_tpo_mod_func_name("libcon", "copy_zone_ref",(void_func_ptr)copy_zone_ref, 0);
	sys_add_tpo_mod_func_name("libcon", "get_zone_ptr",(void_func_ptr)get_zone_ptr, 0);
	sys_add_tpo_mod_func_name("libcon", "get_zone_size",(void_func_ptr)get_zone_size, 0);
	sys_add_tpo_mod_func_name("libcon", "release_zone_ref",(void_func_ptr)release_zone_ref, 0);
	sys_add_tpo_mod_func_name("libcon", "strcpy_c",(void_func_ptr)strcpy_c, 0);
	sys_add_tpo_mod_func_name("libcon", "strcpy_cs",(void_func_ptr)strcpy_cs, 0);
	sys_add_tpo_mod_func_name("libcon", "strncpy_c",(void_func_ptr)strncpy_c, 0);
	sys_add_tpo_mod_func_name("libcon", "strncpy_cs",(void_func_ptr)strncpy_cs, 0);
	sys_add_tpo_mod_func_name("libcon", "strcat_cs",(void_func_ptr)strcat_cs, 0);
	sys_add_tpo_mod_func_name("libcon", "strncat_c",(void_func_ptr)strncat_c, 0);
	sys_add_tpo_mod_func_name("libcon", "strcmp_c",(void_func_ptr)strcmp_c, 0);
	sys_add_tpo_mod_func_name("libcon", "strncmp_c",(void_func_ptr)strncmp_c, 0);
	sys_add_tpo_mod_func_name("libcon", "strincmp_c",(void_func_ptr)strincmp_c, 0);
	sys_add_tpo_mod_func_name("libcon", "strlen_c",(void_func_ptr)strlen_c, 0);
	sys_add_tpo_mod_func_name("libcon", "strlpos_c",(void_func_ptr)strlpos_c, 0);
	sys_add_tpo_mod_func_name("libcon", "strrpos_c", (void_func_ptr)strrpos_c, 0);
	
	sys_add_tpo_mod_func_name("libcon", "strtol_c",(void_func_ptr)strtol_c, 0);
	sys_add_tpo_mod_func_name("libcon", "strtod_c",(void_func_ptr)strtod_c, 0);

	sys_add_tpo_mod_func_name("libcon", "strtoll_c",(void_func_ptr)strtoll_c, 0);
	sys_add_tpo_mod_func_name("libcon", "str_replace_char_c",(void_func_ptr)str_replace_char_c, 0);
	sys_add_tpo_mod_func_name("libcon", "parseDate",(void_func_ptr)parseDate, 0);
	sys_add_tpo_mod_func_name("libcon", "strtoul_c",(void_func_ptr)strtoul_c, 0);
	sys_add_tpo_mod_func_name("libcon", "stricmp_c",(void_func_ptr)stricmp_c, 0);
	sys_add_tpo_mod_func_name("libcon", "uitoa_s",(void_func_ptr)uitoa_s, 0);
	sys_add_tpo_mod_func_name("libcon", "luitoa_s",(void_func_ptr)luitoa_s, 0);
	sys_add_tpo_mod_func_name("libcon", "itoa_s",(void_func_ptr)itoa_s, 0);
	sys_add_tpo_mod_func_name("libcon", "litoa_s", (void_func_ptr)litoa_s, 0);
	
	sys_add_tpo_mod_func_name("libcon", "isalpha_c",(void_func_ptr)isalpha_c, 0);
	sys_add_tpo_mod_func_name("libcon", "isdigit_c",(void_func_ptr)isdigit_c, 0);
	sys_add_tpo_mod_func_name("libcon", "isxdigit_c", (void_func_ptr)isxdigit_c, 0);
	
	sys_add_tpo_mod_func_name("libcon", "dtoa_c",(void_func_ptr)dtoa_c, 0);

	sys_add_tpo_mod_func_name("libcon", "b58tobin", (void_func_ptr)b58tobin, 0);

	sys_add_tpo_mod_func_name("libcon", "muldiv64",(void_func_ptr)muldiv64, 0);
	sys_add_tpo_mod_func_name("libcon", "mul64",(void_func_ptr)mul64, 0);
	sys_add_tpo_mod_func_name("libcon", "shl64",(void_func_ptr)shl64, 0);
	sys_add_tpo_mod_func_name("libcon", "shr64",(void_func_ptr)shr64, 0);
	sys_add_tpo_mod_func_name("libcon", "big128_mul",(void_func_ptr)big128_mul, 0);

	sys_add_tpo_mod_func_name("libcon", "calc_crc32_c",(void_func_ptr)calc_crc32_c, 0);
	sys_add_tpo_mod_func_name("libcon", "compare_z_exchange_c",(void_func_ptr)compare_z_exchange_c, 0);
	sys_add_tpo_mod_func_name("libcon", "fetch_add_c",(void_func_ptr)fetch_add_c, 0);

	sys_add_tpo_mod_func_name("libcon", "init_string",(void_func_ptr)init_string, 0);
	sys_add_tpo_mod_func_name("libcon", "make_string",(void_func_ptr)make_string, 0);
	sys_add_tpo_mod_func_name("libcon", "cat_string",(void_func_ptr)cat_string, 0);
	sys_add_tpo_mod_func_name("libcon", "prepare_new_data",(void_func_ptr)prepare_new_data, 0);
	sys_add_tpo_mod_func_name("libcon", "strcat_int",(void_func_ptr)strcat_int, 0);
	sys_add_tpo_mod_func_name("libcon", "cat_cstring",(void_func_ptr)cat_cstring, 0);
	sys_add_tpo_mod_func_name("libcon", "cat_ncstring",(void_func_ptr)cat_ncstring, 0);
	sys_add_tpo_mod_func_name("libcon", "cat_cstring_p",(void_func_ptr)cat_cstring_p, 0);
	sys_add_tpo_mod_func_name("libcon", "cat_ncstring_p",(void_func_ptr)cat_ncstring_p, 0);
	sys_add_tpo_mod_func_name("libcon", "str_start_with", (void_func_ptr)str_start_with, 0);
	sys_add_tpo_mod_func_name("libcon", "str_end_with", (void_func_ptr)str_end_with, 0);
	sys_add_tpo_mod_func_name("libcon", "vstr_to_str", (void_func_ptr)vstr_to_str, 0);
	sys_add_tpo_mod_func_name("libcon", "make_cstring",(void_func_ptr)make_cstring, 0);
	sys_add_tpo_mod_func_name("libcon", "make_string_l",(void_func_ptr)make_string_l, 0);
	sys_add_tpo_mod_func_name("libcon", "make_string_url",(void_func_ptr)make_string_url, 0);
	sys_add_tpo_mod_func_name("libcon", "make_string_from_uint",(void_func_ptr)make_string_from_uint, 0);
	sys_add_tpo_mod_func_name("libcon", "make_string_from_url",(void_func_ptr)make_string_from_url, 0);
	sys_add_tpo_mod_func_name("libcon", "clone_string",(void_func_ptr)clone_string, 0);
	sys_add_tpo_mod_func_name("libcon", "free_string",(void_func_ptr)free_string, 0);
	sys_add_tpo_mod_func_name("libcon", "make_host_def",(void_func_ptr)make_host_def, 0);
	sys_add_tpo_mod_func_name("libcon", "make_host_def_url",(void_func_ptr)make_host_def_url, 0);
	sys_add_tpo_mod_func_name("libcon", "cat_tag",(void_func_ptr)cat_tag, 0);
	sys_add_tpo_mod_func_name("libcon", "free_host_def",(void_func_ptr)free_host_def, 0);
	sys_add_tpo_mod_func_name("libcon", "strcat_uint",(void_func_ptr)strcat_uint, 0);
	sys_add_tpo_mod_func_name("libcon", "copy_host_def",(void_func_ptr)copy_host_def, 0);

	sys_add_tpo_mod_func_name("libcon", "find_mem_hash",(void_func_ptr)find_mem_hash, 0);

	sys_add_tpo_mod_func_name("libcon", "mem_stream_init",(void_func_ptr)mem_stream_init, 0);
	sys_add_tpo_mod_func_name("libcon", "mem_stream_decomp",(void_func_ptr)mem_stream_decomp, 0);
	sys_add_tpo_mod_func_name("libcon", "mem_stream_read_8",(void_func_ptr)mem_stream_read_8, 0);
	sys_add_tpo_mod_func_name("libcon", "mem_stream_peek_8",(void_func_ptr)mem_stream_peek_8, 0);
	sys_add_tpo_mod_func_name("libcon", "mem_stream_read_16",(void_func_ptr)mem_stream_read_16, 0);
	sys_add_tpo_mod_func_name("libcon", "mem_stream_read_32",(void_func_ptr)mem_stream_read_32, 0);
	sys_add_tpo_mod_func_name("libcon", "mem_stream_peek_32",(void_func_ptr)mem_stream_peek_32, 0);
	sys_add_tpo_mod_func_name("libcon", "mem_stream_read",(void_func_ptr)mem_stream_read, 0);
	sys_add_tpo_mod_func_name("libcon", "mem_stream_skip",(void_func_ptr)mem_stream_skip, 0);
	sys_add_tpo_mod_func_name("libcon", "mem_stream_skip_to",(void_func_ptr)mem_stream_skip_to, 0);
	sys_add_tpo_mod_func_name("libcon", "mem_stream_write",(void_func_ptr)mem_stream_write, 0);
	sys_add_tpo_mod_func_name("libcon", "mem_stream_write_8", (void_func_ptr)mem_stream_write_8, 0);
	sys_add_tpo_mod_func_name("libcon", "mem_stream_write_16", (void_func_ptr)mem_stream_write_16, 0);
	sys_add_tpo_mod_func_name("libcon", "mem_stream_write_32", (void_func_ptr)mem_stream_write_32, 0);
	sys_add_tpo_mod_func_name("libcon", "mem_stream_get_pos", (void_func_ptr)mem_stream_get_pos, 0);
	sys_add_tpo_mod_func_name("libcon", "mem_stream_close",(void_func_ptr)mem_stream_close, 0);

	sys_add_tpo_mod_func_name("libcon", "tpo_mod_load_tpo",(void_func_ptr)tpo_mod_load_tpo, 0);
	sys_add_tpo_mod_func_name("libcon", "tpo_mod_init",(void_func_ptr)tpo_mod_init, 0);
	sys_add_tpo_mod_func_name("libcon", "tpo_mod_find", (void_func_ptr)tpo_mod_find, 0);

	sys_add_tpo_mod_func_name("libcon", "execute_script_mod_call", (void_func_ptr)execute_script_mod_call, 0);
	sys_add_tpo_mod_func_name("libcon", "execute_script_mod_rcall", (void_func_ptr)execute_script_mod_rcall, 0);
	sys_add_tpo_mod_func_name("libcon", "execute_script_mod_rwcall", (void_func_ptr)execute_script_mod_rwcall, 0);

	sys_add_tpo_mod_func_name("libcon", "load_module",(void_func_ptr)load_module, 0);
	sys_add_tpo_mod_func_name("libcon", "register_tpo_exports",(void_func_ptr)register_tpo_exports, 0);
	sys_add_tpo_mod_func_name("libcon", "get_tpo_mod_exp_addr_name",(void_func_ptr)get_tpo_mod_exp_addr_name, 0);
	sys_add_tpo_mod_func_name("libcon", "isRunning",(void_func_ptr)isRunning, 0);

	sys_add_tpo_mod_func_name("libcon", "snooze_c", (void_func_ptr)snooze_c, 0);
	sys_add_tpo_mod_func_name("libcon", "find_mod_ptr", (void_func_ptr)find_mod_ptr, 0);

	sys_add_tpo_mod_func_name("libcon", "get_exe_path", (void_func_ptr)get_exe_path, 0);
	sys_add_tpo_mod_func_name("libcon", "aquire_lock_file", (void_func_ptr)aquire_lock_file, 0);

	sys_add_tpo_mod_func_name("libcon", "bt_insert", (void_func_ptr)bt_insert, 0);
	sys_add_tpo_mod_func_name("libcon", "bt_search", (void_func_ptr)bt_search, 0);
	sys_add_tpo_mod_func_name("libcon", "bt_deltree", (void_func_ptr)bt_deltree, 0);
	
#endif
	
	init_funcs();
}


OS_API_C_FUNC(unsigned int) area_type(unsigned int area_id)
{
	return get_area(area_id)->type;
}

OS_API_C_FUNC(mem_ptr)	get_next_aligned_ptr(mem_ptr ptr)
{
	unsigned int val_addr;

	val_addr=mem_to_uint(ptr);
	if((val_addr&0x0000000F)==0)return ptr;

	return uint_to_mem(((val_addr&0xFFFFFFF0)+16));
}


mem_ptr	get_next_seg_aligned_ptr(mem_ptr ptr)
{
	unsigned int val_addr;

	val_addr=mem_to_uint(ptr);
	if((val_addr&0x0000FFFF)==0)return ptr;

	return uint_to_mem(((val_addr&0xFFFF0000)+0x10000));
}

mem_size get_aligned_size(mem_ptr ptr,mem_ptr end)
{
	mem_ptr			start_align;
	mem_size		size_align;
	mem_size		size_av;
	
	start_align		=	get_next_aligned_ptr(ptr);
	if(start_align>=end)return 0;
	
	size_av			=	mem_sub(start_align,end);
	size_align		=	size_av&0xFFFFFFF0;

	return (size_align);
}



OS_API_C_FUNC(unsigned int) init_new_mem_area(mem_ptr phys_start,mem_ptr phys_end,size_t nzones,mem_area_type_t type)
{
	int n;
	if(__global_mem_areas==PTR_NULL)return 0xFFFFFFFF;

	n = 0;
	while (!compare_z_exchange_c(&area_lock, 1))
		if ((n++) >= 1000)return 0;
	
	n = 0;
	while(n<MAX_MEM_AREAS)
	{
		if(__global_mem_areas[n].type == 0x00000000)
		{
			__global_mem_areas[n].area_id			=n+1;
			__global_mem_areas[n].type				=type;
			__global_mem_areas[n].nzones			=nzones;
			__global_mem_areas[n].zones_buffer		=get_next_aligned_ptr(kernel_memory_map_c(nzones * sizeof(mem_zone)));
			/*__global_mem_areas[n].lock_sema		=PTR_NULL;*/
			__global_mem_areas[n].last_gc_time		=0;
			__global_mem_areas[n].last_used_zone	=0;

			if (type & 0x10)
				memset_32_c	(__global_mem_areas[n].trash, 0xFFFFFFFF, sizeof(__global_mem_areas[n].trash));
			else
				memset_c	(__global_mem_areas[n].trash, 0, sizeof(mem_zone_ptr) * 256);

#ifndef NATIVE_ALLOC
			__global_mem_areas[n].area_start = phys_start;
			__global_mem_areas[n].area_end = phys_end;

			memset_c	(__global_mem_areas[n].zones_free, 0, MAX_FREE_ZONES*sizeof(mem_zone_desc));

			__global_mem_areas[n].zones_free[0].ptr		=	get_next_aligned_ptr(__global_mem_areas[n].area_start);
			__global_mem_areas[n].zones_free[0].size	=	get_aligned_size	( __global_mem_areas[n].area_start, __global_mem_areas[n].area_end);
#endif

			area_lock = 0;
			return __global_mem_areas[n].area_id;
		}
		n++;
	}
	area_lock = 0;
	return 0xFFFFFFFF;
}


OS_API_C_FUNC(void) init_default_mem_area(size_t size, size_t nzones)
{
	
	unsigned int		default_mem_area_id;

#ifndef NATIVE_ALLOC
	mem_ptr				start = PTR_NULL, end = PTR_NULL;
	start = kernel_memory_map_c(size + 8);
	end = mem_add(start, size);

	default_mem_area_id = init_new_mem_area(start, end, nzones, MEM_TYPE_DATA);
	memset_c(start, 0, mem_sub(start, end));
#else
	default_mem_area_id = init_new_mem_area(PTR_NULL, PTR_NULL, nzones, MEM_TYPE_DATA);
#endif
	set_mem_area_id(default_mem_area_id);

	return;
}


OS_API_C_FUNC(unsigned int) free_mem_area(unsigned int area_id)
{
	mem_area	*area_ptr;
	int			n;
	if (__global_mem_areas == PTR_NULL)return 0xFFFFFFFF;

	n = 0;
	while (!compare_z_exchange_c(&area_lock, 1))
		if ((n++) >= 1000)return 0;
	
	area_ptr = get_area(area_id);

	memset_c (area_ptr ->zones_buffer,0, area_ptr->nzones *sizeof(mem_zone_desc));

#ifndef NATIVE_ALLOC
	kernel_memory_free_c(area_ptr->area_start);
	memset_c(area_ptr->zones_free, 0, MAX_FREE_ZONES*sizeof(mem_zone_desc));
#endif

	memset_c	(area_ptr, 0, sizeof(mem_area));

	area_lock = 0;

	return 1;

}


void add_trashed_zone(mem_zone_ptr zone)
{
	unsigned int n;
	mem_area *area;

	area = get_area(zone->area_id);
	if (area == PTR_NULL)return;

	for (n = 0; n < 256; n++)
	{
		if (compare_z_exchange_c((unsigned int *)(&area->trash[n]), mem_to_uint(zone)))break;
	}
}



#ifndef NATIVE_ALLOC

mem_area * find_area(mem_ptr ptr, mem_size size)
{
	int n;
	mem_ptr	 end_zone;

	if (__global_mem_areas == PTR_NULL)return PTR_NULL;

	end_zone = mem_add(ptr, size);

	n = 0;
	while (n<MAX_MEM_AREAS)
	{
		if ((ptr >= __global_mem_areas[n].area_start) && (end_zone<__global_mem_areas[n].area_end))
			return &__global_mem_areas[n];
		n++;
	}


	return PTR_NULL;
}


mem_area	*get_area_by_zone_ptr(const mem_zone *zone)
{
	unsigned int n;
	if (__global_mem_areas == PTR_NULL)return PTR_NULL;
	
	for(n = 0; n<MAX_MEM_AREAS;n++)
	{
		if (__global_mem_areas[n].type == 0x00000000)
			return PTR_NULL;

		if ((zone >= __global_mem_areas[n].zones_buffer) && (zone<&__global_mem_areas[n].zones_buffer[MAX_MEM_AREAS]))
			return &__global_mem_areas[n];
		
	}
	return PTR_NULL;
}


int find_free_zone		(const mem_area *area,mem_size size,mem_zone_desc	*desc)
{
	int	n = 0;
	
	while(area->zones_free[n].ptr!= PTR_NULL)
	{
		mem_ptr	 start_free_zone;
		mem_ptr	 end_free_zone;
		mem_ptr	 start_aligned_free_zone;
		mem_size size_aligned_free_zone;

		start_free_zone			=	area->zones_free[n].ptr;
		end_free_zone			=	mem_add(area->zones_free[n].ptr,area->zones_free[n].size);

		start_aligned_free_zone	=	get_next_aligned_ptr(start_free_zone);
		if(start_aligned_free_zone!=0x0)
		{
			size_aligned_free_zone	=	mem_sub	(start_aligned_free_zone,end_free_zone);

			if(size_aligned_free_zone>=size)
			{
				desc->ptr	=start_aligned_free_zone;
				desc->size	=size;
				return 1;
			}
		}
		n++;
		if (n >= MAX_FREE_ZONES)return 0;
	}
	desc->ptr	=	PTR_NULL;
	desc->size	=	0;

	return 0;
}


int allocate_zone(mem_area *area,const mem_zone_desc *desc)
{
	int						n;
	mem_zone_desc_ptr		ret;
	
	n	=	0;
	ret	=	PTR_NULL;

	while(area->zones_free[n].ptr!=	PTR_NULL)
	{
		mem_ptr		start_aligned_free_zone,end_free_zone;
		mem_size	size_aligned_free_zone;

		end_free_zone			=	mem_add(area->zones_free[n].ptr,area->zones_free[n].size);

		start_aligned_free_zone	=	get_next_aligned_ptr(area->zones_free[n].ptr);
		size_aligned_free_zone	=	mem_sub	(start_aligned_free_zone,end_free_zone);

		if(	(start_aligned_free_zone==desc->ptr)&&(size_aligned_free_zone>=desc->size))
		{
			mem_ptr					free_zone_end_ptr;
			mem_ptr					zone_end_ptr;
			mem_ptr					zone_end_aligned_ptr;

			free_zone_end_ptr		= 	mem_add	 (desc->ptr,area->zones_free[n].size);
			zone_end_ptr			=	mem_add	 (desc->ptr,desc->size);
			zone_end_aligned_ptr	=	get_next_aligned_ptr(zone_end_ptr);
			if(zone_end_aligned_ptr<free_zone_end_ptr)
			{
				area->zones_free[n].ptr		=  zone_end_aligned_ptr;
				area->zones_free[n].size	=  mem_sub	(zone_end_aligned_ptr,free_zone_end_ptr);
			}
			else
			{
				while(area->zones_free[n+1].ptr	!=	PTR_NULL)
				{
					area->zones_free[n].ptr		=area->zones_free[n+1].ptr;
					area->zones_free[n].size	=area->zones_free[n+1].size;
					n++;
				}
				area->zones_free[n].ptr		=PTR_NULL;
				area->zones_free[n].size	=0;
			}
		
			return 1;
		}
		n++;
	}
	return 0;
}

int do_allocate(const mem_area *area, mem_size size, mem_zone_desc	*desc)
{
	if (!find_free_zone(area, size, desc))
		return 0;

	return allocate_zone(area, desc);
		
}

int	free_zone_area		(unsigned int area_id, mem_zone_desc *mem)
{
	unsigned int			n,cnt;
	mem_ptr					start_zone,end_zone;
	mem_zone_desc			new_free_zone;
	mem_area				*area_ptr;


	if(area_id	==0xFFFF)return 1;
	if(mem->ptr	==PTR_INVALID)return 1;

	area_ptr			=	get_area(area_id);
	if(area_ptr==PTR_NULL)return 1;

	if(mem->size>0)
	{
		start_zone			=	mem->ptr;
		end_zone			=	mem_add(start_zone,mem->size);

		new_free_zone.ptr	=	start_zone;
		new_free_zone.size	=	mem_sub(start_zone,end_zone);

		n					=  0;
		while((area_ptr->zones_free[n].ptr!=PTR_NULL))
		{
			mem_ptr	start_free_zone, end_free_zone;

			start_free_zone	=	area_ptr->zones_free[n].ptr;
			end_free_zone	=	mem_add(start_free_zone,area_ptr->zones_free[n].size);

			if(end_zone==start_free_zone)
			{
				new_free_zone.ptr	=start_zone;
				new_free_zone.size	=mem_sub(start_zone,end_free_zone);

				end_zone			=end_free_zone;

				cnt=n;
				while(area_ptr->zones_free[cnt+1].ptr!=PTR_NULL)
				{
					area_ptr->zones_free[cnt]=area_ptr->zones_free[cnt+1];
					cnt++;
				}
				area_ptr->zones_free[cnt].ptr=PTR_NULL;
				area_ptr->zones_free[cnt].size = 0;
				
			}
			else if(start_zone==end_free_zone)
			{
				new_free_zone.ptr	=start_free_zone;
				new_free_zone.size	=mem_sub(new_free_zone.ptr,end_zone);
				start_zone			=start_free_zone;

				cnt=n;
				while(area_ptr->zones_free[cnt+1].ptr!=PTR_NULL)
				{
					area_ptr->zones_free[cnt]=area_ptr->zones_free[cnt+1];
					cnt++;
				}
				area_ptr->zones_free[cnt].ptr = PTR_NULL;
				area_ptr->zones_free[cnt].size = 0;
			}
			else
			{
				n++;
			}
		}

		//memset_32_c(new_free_zone.ptr,0xDEF0DEF0,new_free_zone.size);

		for(n = 0;n<MAX_FREE_ZONES;n++)
		{
			if(area_ptr->zones_free[n].ptr==PTR_NULL)
			{
				area_ptr->zones_free[n].ptr		=new_free_zone.ptr;
				area_ptr->zones_free[n].size	=new_free_zone.size;
				break;
			}
		}
	}

	mem->ptr	=PTR_NULL;
	mem->size	=0;

	return 1;
}


OS_API_C_FUNC(int) 	realloc_zone(mem_zone_ref *zone_ref, mem_size new_size)
{
	unsigned int			n, cnt;
	mem_zone_desc			new_zone;
	mem_zone_desc			*mem;
	mem_area				*area_ptr;
	mem_zone				*src_zone;

	src_zone = zone_ref->zone;
	if (src_zone == PTR_NULL)return 0;

	area_ptr = get_area(src_zone->area_id);
	if (area_ptr == PTR_NULL)
	{
		return 0;
	}

	if ((area_ptr->area_id != get_tree_mem_area_id()) && (area_ptr->area_id != get_mem_area_id()))
	{
		mem_area				*new_area_ptr;
		unsigned char			*src_ptr;;

		if ((area_ptr->type & 0xF) == MEM_TYPE_DATA)
			new_area_ptr = get_area(get_mem_area_id());
		else
			new_area_ptr = get_area(get_tree_mem_area_id());

		if (src_zone->mem.size > 0)
			src_ptr = get_zone_ptr(zone_ref, 0);

		if (!allocate_new_zone(new_area_ptr->area_id, new_size, zone_ref))
		{
			return 0;
		}

		if (src_zone->mem.size > 0)
		{
			memcpy_c(get_zone_ptr(zone_ref, 0), src_ptr, src_zone->mem.size);
		}

		add_trashed_zone(src_zone);
		return 1;
	}

	/*task_manager_aquire_semaphore(area_ptr->lock_sema,0);*/

	if (new_size & 0x0000000F)
		new_size = ((new_size & 0xFFFFFFF0) + 16);

	mem = &src_zone->mem;


	if (mem->size>0)
	{
		mem_ptr					start_zone, end_zone, end_zone_aligned, new_end_zone;
		mem_ptr					new_end_zone_aligned;
		mem_zone_desc			new_free_zone;

		start_zone = mem->ptr;
		end_zone = mem_add(start_zone, mem->size);
		end_zone_aligned = get_next_aligned_ptr(end_zone);
		new_end_zone = mem_add(start_zone, new_size);
		new_end_zone_aligned = get_next_aligned_ptr(new_end_zone);

		n = 0;

		/*try to find free zone contigous to the end of the current memory*/

		while (area_ptr->zones_free[n].ptr != PTR_NULL)
		{
			mem_ptr		start_free_zone, end_free_zone;

			if (n >= MAX_FREE_ZONES)
			{
				/*task_manager_release_semaphore(area_ptr->lock_sema,0);*/
				return -1;
			}

			/*current free zone to test*/
			start_free_zone = area_ptr->zones_free[n].ptr;
			end_free_zone = mem_add(start_free_zone, area_ptr->zones_free[n].size);


			if ((end_zone_aligned == start_free_zone) && (new_size <= area_ptr->zones_free[n].size))
			{
				/*free zone big enough contigous to current memory block*/
				new_free_zone.ptr = new_end_zone_aligned;
				new_free_zone.size = mem_sub(new_free_zone.ptr, end_free_zone);

				if (new_free_zone.ptr >= end_free_zone)
				{
					/*contigous free zone entirely consumed by the new alloc, remove it*/
					cnt = n;
					while (area_ptr->zones_free[cnt + 1].ptr != PTR_NULL)
					{
						area_ptr->zones_free[cnt] = area_ptr->zones_free[cnt + 1];
						cnt++;
					}
					area_ptr->zones_free[cnt].ptr = PTR_NULL;
				}
				else
				{
					/*update the free zone to start at the end of the reallocated block*/
					area_ptr->zones_free[n] = new_free_zone;
				}

				/*reset newly allocated memory*/
				memset_c(end_zone, 0, mem_sub(end_zone, new_end_zone));
				((mem_zone *)(zone_ref->zone))->mem.size = new_size;

#ifdef _DEBUG
				((mem_zone *)(zone_ref->zone))->time = get_time_c();
#endif
				/*task_manager_release_semaphore(area_ptr->lock_sema,0);*/
				return 1;
			}
			n++;
		}
	}

	if (!do_allocate(area_ptr, new_size, &new_zone))
		return -1;

	if (new_zone.size>src_zone->mem.size)
	{
		mem_ptr		start_new;
		mem_size	size_new;

		start_new = mem_add(new_zone.ptr, src_zone->mem.size);
		size_new = new_zone.size - src_zone->mem.size;
		memset_c(start_new, 0x00, size_new);
	}

	if (src_zone->mem.size>0)
		memcpy_c(new_zone.ptr, get_zone_ptr(zone_ref, 0), src_zone->mem.size);


	if (src_zone->free_func != PTR_NULL)
	{
		mem_zone_ref tmp = { src_zone };
		src_zone->free_func(&tmp, 1);
	}
	free_zone_area(src_zone->area_id, &src_zone->mem);

#ifdef _DEBUG
	src_zone->time = get_time_c();
#endif
	src_zone->mem.ptr = new_zone.ptr;
	src_zone->mem.size = new_zone.size;

	/*task_manager_release_semaphore(area_ptr->lock_sema,0);*/

	return 1;
}

#else

int aalloc(size_t size)
{
	void* p1; // original block
	void** p2; // aligned block
	int offset = 16 - 1 + sizeof(void*);
	if ((p1 = (void*)malloc(size + offset)) == NULL)
	{
		return NULL;
	}
	p2 = (void**)(((size_t)(p1)+offset) & ~(16 - 1));
	p2[-1] = p1;
	return p2;
}


int do_allocate(const mem_area *area, mem_size size, mem_zone_desc	*desc)
{
	mem_ptr new_ptr;

	new_ptr = _aligned_malloc(size,16);

	if (new_ptr == PTR_NULL)return 0;

	desc->ptr  = new_ptr;
	desc->size = size;
	return 1;
}

int	free_zone_area(unsigned int area_id, mem_zone_desc *mem)
{
	if (mem->ptr == 0xFFFFFFFF)
		return 0;

	_aligned_free(mem->ptr);

	mem->ptr = PTR_NULL;
	mem->size = 0;

	return 1;
}

OS_API_C_FUNC(int) 	realloc_zone(mem_zone_ref *zone_ref, mem_size new_size)
{
	mem_ptr new_ptr;

	if ((((mem_zone *)(zone_ref->zone))->mem.ptr == 0xFFFFFFFF)||
		(((mem_zone *)(zone_ref->zone))->mem.ptr == PTR_NULL))
		new_ptr = _aligned_malloc(new_size,16);
	else
	{
		new_ptr = _aligned_realloc(((mem_zone *)(zone_ref->zone))->mem.ptr, new_size,16);
	}

	if (new_ptr == PTR_NULL)
		return 0;

	((mem_zone *)(zone_ref->zone))->mem.ptr = new_ptr;
	((mem_zone *)(zone_ref->zone))->mem.size = new_size;

	return 1;
}

OS_API_C_FUNC(unsigned int) do_mark_sweep(unsigned int area_id, unsigned int delay)
{
	return 0;
}

#endif


void	free_zone		(mem_zone_ref_ptr zone_ref)
{
	
	mem_zone	*src_zone=zone_ref->zone;

	if(src_zone==PTR_NULL)return;

	if ((src_zone->area_id != get_mem_area_id()) && (src_zone->area_id != get_tree_mem_area_id()))
	{
		add_trashed_zone(src_zone);
		return ;
	}

	if (((area_type(src_zone->area_id) & 0x10) == 0) && (src_zone->mem.ptr != PTR_FF) && (src_zone->free_func != PTR_NULL))
	{
		mem_zone_ref tmp = { src_zone };
		src_zone->free_func(&tmp, 0);
	}
	free_zone_area(src_zone->area_id, &src_zone->mem);

	src_zone->n_refs = 0;
	src_zone->free_func = PTR_NULL;
	src_zone->area_id = 0;

}


OS_API_C_FUNC(void) empty_trash()
{
	unsigned int	n = 0;
	mem_area		*area;

	area = get_area(get_mem_area_id());
	if (area != PTR_NULL)
	{
		if ((area->type & 0x10) == 0)
		{
			for (n = 0; n < 256; n++)
			{
				mem_zone_ptr src_zone = area->trash[n];
				if (src_zone == PTR_NULL)continue;

				if ( (src_zone->mem.ptr != PTR_FF) && (src_zone->free_func != PTR_NULL))
				{
					mem_zone_ref tmp = { src_zone };
					src_zone->free_func(&tmp, 0);
				}

				free_zone_area(area->area_id, &src_zone->mem);
				area->trash[n] = PTR_NULL;
			}
		}
	}

	area = get_area(get_tree_mem_area_id());
	if (area != PTR_NULL)
	{
		if ((area->type & 0x10) == 0)
		{
			for (n = 0; n < 256; n++)
			{
				mem_zone_ptr src_zone = area->trash[n];
				if (src_zone == PTR_NULL)continue;
				if (src_zone->mem.ptr == PTR_NULL)continue;
				if ((src_zone->mem.ptr != PTR_FF) && (src_zone->free_func != PTR_NULL))
				{
					mem_zone_ref tmp = { src_zone };
					src_zone->free_func(&tmp, 0);
				}
				free_zone_area(area->area_id, &src_zone->mem);
				area->trash[n] = PTR_NULL;
			}
		}
	}
}

OS_API_C_FUNC(void) swap_zone_ref(mem_zone_ref_ptr dest_zone_ref, mem_zone_ref_ptr src_zone_ref)
{
	mem_zone *dst_zone = dest_zone_ref->zone;

	dest_zone_ref->zone = src_zone_ref->zone;
	src_zone_ref->zone = dst_zone;
}


OS_API_C_FUNC(unsigned int) get_zone_numref(mem_zone_ref *zone_ref)
{
	if(zone_ref==PTR_NULL)return 0;
	if(zone_ref->	zone==PTR_NULL)return 0;
	return ((mem_zone *)(zone_ref->	zone))->n_refs;
}

OS_API_C_FUNC(unsigned int) inc_zone_ref(mem_zone_ref_ptr zone_ref)
{
	if (zone_ref->zone == PTR_NULL)return 0;
	
	if ((get_area(((mem_zone *)(zone_ref->zone))->area_id)->type & 0xF0) != 0)
		return 0;

	if (fetch_add_c(&((mem_zone *)(zone_ref->zone))->n_refs, 1) == 0)return 0;
	return 1;

}

OS_API_C_FUNC(void) dec_zone_ref(mem_zone_ref *zone_ref)
{
	if(zone_ref			==PTR_NULL)return;
	if(zone_ref->zone	==PTR_NULL)return;
	
	if (((mem_zone *)(zone_ref->zone))->area_id == 0xFFFF)return;
	
	if ((get_area(((mem_zone *)(zone_ref->zone))->area_id)->type & 0x10) != 0)
		return;

	if (fetch_add_c(&((mem_zone *)(zone_ref->zone))->n_refs, -1) == 1)
	{
		if (((mem_zone *)(zone_ref->zone))->mem.ptr != PTR_NULL)
			free_zone(zone_ref);
	}
}

OS_API_C_FUNC(void) release_zone_ref(mem_zone_ref *zone_ref)
{
	if (zone_ref == PTR_NULL)return;
	if (zone_ref->zone == PTR_NULL)return;

	if ((get_area(((mem_zone *)(zone_ref->zone))->area_id)->type & 0x10) != 0)
		return;

	dec_zone_ref(zone_ref);
	zone_ref->zone=PTR_NULL;
}



OS_API_C_FUNC(void) copy_zone_ref(mem_zone_ref_ptr dest_zone_ref,const mem_zone_ref *zone_ref)
{
	mem_zone *zone = dest_zone_ref->zone;
	int area_type;

	if ((zone_ref->zone == PTR_NULL) && (dest_zone_ref->zone == PTR_NULL))return;

	if (zone_ref->zone == PTR_NULL)
		area_type = get_area(((mem_zone *)(dest_zone_ref->zone))->area_id)->type;
	else
		area_type = get_area(((mem_zone *)(zone_ref->zone))->area_id)->type;
	
	
	if ((area_type & 0x10) != 0)
	{
		dest_zone_ref->zone = zone_ref->zone;
		return;
	}
	

	if (zone_ref->zone != PTR_NULL)
	{
		while(1)
		{
			mem_zone *srcz = (mem_zone *)zone_ref->zone;
			if (fetch_add_c(&srcz->n_refs, 1) > 0)
			{
				dest_zone_ref->zone = srcz;
				break;
			}
		}
	}
	else
		dest_zone_ref->zone = PTR_NULL;

	mfence_c();

	if (zone != PTR_NULL)
	{
		if (fetch_add_c(&zone->n_refs, -1) == 1)
		{
			if (zone->mem.ptr != PTR_NULL)
			{
				if ((zone->area_id != get_tree_mem_area_id()) && (zone->area_id != get_mem_area_id()))
				{
					add_trashed_zone(zone);
					return;
				}
				if ((zone->mem.ptr != PTR_FF) && (zone->free_func != PTR_NULL))
				{
					mem_zone_ref tmp = { zone };
					zone->free_func(&tmp, 0);
				}
				free_zone_area(zone->area_id, &zone->mem);
			}
		}
	}


}





OS_API_C_FUNC(int) get_shared_slot(mem_zone_ref_ptr shared_zone, mem_zone_ref_ptr *zone_ptr)
{
	mem_area		*area_ptr;
	unsigned int	n;

	if (shared_zone->zone == PTR_NULL)
		return 0;

	area_ptr = get_area(((mem_zone *)(shared_zone->zone))->area_id);

	if ((area_ptr->type & 0x10) == 0)
	{
		copy_zone_ref((*zone_ptr), shared_zone);
		return 1;
	}
	
	for (n = 0; n < 256; n++)
	{
		if (compare_exchange_c((unsigned int *)&area_ptr->trash[n], 0xFFFFFFFF, mem_to_uint(shared_zone->zone)))
		{
			(*zone_ptr) = &area_ptr->trash[n];
			return 1;
		}
	}

	return 0;
}

OS_API_C_FUNC(int) release_shared_slot(mem_zone_ref_ptr zone_ptr)
{
	mem_area		*area_ptr;

	if (zone_ptr == PTR_NULL)return 0;
	if (zone_ptr->zone == PTR_NULL)return 0;
	if (zone_ptr->zone == PTR_FF)return 0;

	area_ptr = get_area(((mem_zone *)(zone_ptr->zone))->area_id);

	if ((area_ptr->type & 0x10)!=0)
		zone_ptr->zone = PTR_FF;
	else
		release_zone_ref(zone_ptr);

	return 1;
}

OS_API_C_FUNC(void) mark_zone(mem_ptr zone_ptr, unsigned int scan_id)
{
	mem_zone *zone = (mem_zone *)zone_ptr;
	mem_area *area_ptr;
	if (zone == PTR_NULL)return;
	if (zone == PTR_FF)return;
	if (zone->mem.ptr == PTR_NULL)return;
	if (zone->mem.ptr == PTR_INVALID)return;
	if (zone->area_id == 0)return;
	if (zone->n_refs == scan_id)return;
	if ((area_ptr=get_area(zone->area_id)) == PTR_NULL)return;
	//if ((zone->mem.ptr < area_ptr->area_start) || (zone->mem.ptr > area_ptr->area_end))return;

	zone->n_refs = scan_id;

	if (zone->free_func)
	{
		mem_zone_ref tmp = { zone };
		zone->free_func(&tmp, scan_id);
	}
}


OS_API_C_FUNC(void) reset_zones(mem_area *area_ptr)
{
	unsigned int n;
	for (n = 0; n <= area_ptr->last_used_zone; n++)
	{
		mem_zone *zone = &area_ptr->zones_buffer[n];
		zone->n_refs = 0;
	}
}

OS_API_C_FUNC(void) sweep_zones(mem_area *area_ptr)
{
	unsigned int n;
	unsigned int new_used_zones = 0;

	new_used_zones = 0;

	for(n=0; n <=  area_ptr->last_used_zone;n++)
	{
		mem_zone *zone = &area_ptr->zones_buffer[n];

		if (zone->mem.ptr == PTR_NULL)continue;

		if(zone->n_refs == 0)
		{
			free_zone_area(area_ptr->area_id, &zone->mem);
		}
		else
		{
			zone->n_refs   = 0;
			new_used_zones = n+1;
		}
	}

	 area_ptr->last_used_zone = new_used_zones;
}

#ifndef NATIVE_ALLOC

OS_API_C_FUNC(unsigned int) do_mark_sweep(unsigned int area_id, unsigned int delay)
{
	unsigned int n;
	mem_area	*area_ptr;
	mem_ptr		cur_stack_frm, cur_stack;
	ctime_t		time;

	area_ptr = get_area(area_id);

	if ((area_ptr->type & 0x10) == 0)
		return 0;

	get_system_time_c(&time);
	if ((area_ptr->last_used_zone<((area_ptr->nzones * 80) / 100)) && ((time - area_ptr->last_gc_time) < delay))return 0;


	if (delay == 0)
	{
		while (!compare_z_exchange_c(&gc_lock, 1))
		{
			snooze_c(1);
		}
	}
	else
	{
		if (!compare_z_exchange_c(&gc_lock, 1))return 0;
	}

	area_ptr->last_gc_time = time;
	cur_stack_frm = get_stack_frame_c();
	cur_stack = get_stack_c();
	n = 0;

	for (n = 0; n < 256; n++)
	{
		if (area_ptr->trash[n] != PTR_FF)
			mark_zone(area_ptr->trash[n], 1);
	}

	for (n = 0; n < MAX_MEM_AREAS; n++)
	{
		if ((__global_mem_areas[n].area_start != 0x00000000) && (__global_mem_areas[n].type == (MEM_TYPE_TREE | 0x10)))
		{
			mark_modz_zones(__global_mem_areas[n].zones_buffer, mem_add(__global_mem_areas[n].zones_buffer, __global_mem_areas[n].nzones * sizeof(mem_zone)));
			scan_stack_c(__global_mem_areas[n].zones_buffer, mem_add(__global_mem_areas[n].zones_buffer, __global_mem_areas[n].nzones * sizeof(mem_zone)), cur_stack_frm, cur_stack);
		}
	}
	//scan_stack_c			(area_ptr->zones_buffer,mem_add(area_ptr->zones_buffer,MAX_MEM_ZONES*sizeof(mem_zone)), cur_stack_frm, cur_stack);
	//scan_threads_stack	(area_ptr->zones_buffer,mem_add(area_ptr->zones_buffer,MAX_MEM_ZONES*sizeof(mem_zone)));
	sweep_zones(area_ptr);

	for (n = 0; n < MAX_MEM_AREAS; n++)
	{
		if (__global_mem_areas[n].area_id == get_tree_mem_area_id())continue;
		if ((__global_mem_areas[n].area_start != 0x00000000) && (__global_mem_areas[n].type == (MEM_TYPE_TREE | 0x10)))
		{
			reset_zones(&__global_mem_areas[n]);
		}
	}

	//resume_threads		();

	gc_lock = 0;

	return 1;
}

#endif



OS_API_C_FUNC(unsigned int) get_zone_area_type(mem_zone_ref_const_ptr zone)
{
	return get_area(((mem_zone *)(zone->zone))->area_id)->type;
}

OS_API_C_FUNC(unsigned int) allocate_new_empty_zone(unsigned int area_id, mem_zone_ref *zone_ref)
{
	unsigned int n, ret;
	mem_area	*area_ptr;

	area_ptr = get_area(area_id);
	if (area_ptr == PTR_NULL)
	{
		return 0;
	}
	/* task_manager_aquire_semaphore	(area_ptr->lock_sema,0); */

	release_zone_ref(zone_ref);

	ret = 0;
	n = 0;
	while ((n + 1)<area_ptr->nzones)
	{
		if (area_ptr->zones_buffer[n].mem.ptr == PTR_NULL)
		{
			mem_zone		*nzone;

			nzone = &area_ptr->zones_buffer[n];
			nzone->mem.ptr = uint_to_mem(0xFFFFFFFF);
			nzone->mem.size = 0;
			nzone->area_id = area_ptr->area_id;

			if ((area_ptr->type&0x10)==0)
				nzone->n_refs = 1;
			else
				nzone->n_refs = 0;

#ifdef _DEBUG
			nzone->time = get_time_c();
#endif
			nzone->free_func = PTR_NULL;
			zone_ref->zone = nzone;

			if (n > area_ptr->last_used_zone)
				area_ptr->last_used_zone = n+1;

			ret = 1;
			break;
		}
		n++;
	}

	/* task_manager_release_semaphore(area_ptr->lock_sema,0); */

	if (ret == 0)
	{
		/* dump_task_infos_c	();	*/
	}



	return ret;
}



OS_API_C_FUNC(unsigned int) allocate_new_zone(unsigned int area_id,mem_size zone_size,mem_zone_ref *zone_ref)
{
	mem_zone_ref ezone;
	mem_area	*area_ptr;
	unsigned int n;
	int			 ret = 0, gc;
	

	area_ptr		=	get_area(area_id);
	if(area_ptr==PTR_NULL)
	{
		return 0;
	}

	gc = 1;

	/* task_manager_aquire_semaphore(area_ptr->lock_sema,0); */

	while (ret == 0)
	{
		ezone.zone = zone_ref->zone;
		zone_size  = ((zone_size & 0xFFFFFFF0) + 16);

		for(n = 0; n < area_ptr->nzones;n++)
		{
			mem_zone		*nzone;
			if (area_ptr->zones_buffer[n].mem.ptr != PTR_NULL)continue;

			nzone = &area_ptr->zones_buffer[n];
			/*
			if (find_free_zone(area_ptr, zone_size, &nzone->mem) == 1)
			{
				if (allocate_zone(area_ptr, &nzone->mem) == 1)
				{
			*/

			if(do_allocate(area_ptr, zone_size, &nzone->mem)==1)
			{
				nzone->area_id = area_ptr->area_id;

				if ((area_ptr->type & 0x10) == 0)
					nzone->n_refs = 1;
				else
					nzone->n_refs = 0;

#ifdef _DEBUG
				nzone->time = get_time_c();
#endif
				nzone->free_func = PTR_NULL;
				//memset_c(nzone->mem.ptr, 0x00, nzone->mem.size);
				zone_ref->zone = nzone;

				if (n > area_ptr->last_used_zone)
					area_ptr->last_used_zone = n + 1;

				release_zone_ref(&ezone);
				/* task_manager_release_semaphore(area_ptr->lock_sema,0); */

				ret = 1;
				break;
			}
			else
			{
				/*task_manager_release_semaphore(area_ptr->lock_sema,0);*/
				ret = 0;
				break;
			}
		}

		if (ret == 0)
		{
			if(gc == 0)break;
			if ((area_ptr->type & 0x10) == 0)
				break;
			
			if (do_mark_sweep(area_id, 0))
			{
				gc = 0;
			}
		}
	}

	/*task_manager_release_semaphore(area_ptr->lock_sema,0);*/
	
	return ret;
}





OS_API_C_FUNC(int) expand_zone			(mem_zone_ref *ref,mem_size new_size)
{
	size_t ns;
	
	if(ref->zone==PTR_NULL)return 0;
	if(((mem_zone *)(ref->zone))->mem.size>=new_size)return 0;
	
	if(((mem_zone *)(ref->zone))->mem.size==0)
		ns		=	16;
	else
		ns		=	((mem_zone *)(ref->zone))->mem.size;

	while(ns<new_size)
		ns=ns<<1;

	new_size=ns;

	return realloc_zone	(ref,new_size);
}


OS_API_C_FUNC(mem_ptr) malloc_c(mem_size sz)
{
	mem_zone_ref	ref;
	mem_ptr			m_ptr,ret_ptr;

	ref.zone=PTR_NULL;
	
	if(allocate_new_zone(get_mem_area_id(),sz+16,&ref)!=1)
	{
		return PTR_NULL;
	}
	
	m_ptr	=	get_zone_ptr(&ref,0);

	*((mem_ptr *)(m_ptr))=ref.zone;

	ret_ptr	=	mem_add(m_ptr,16);

	return ret_ptr;
}


OS_API_C_FUNC(mem_ptr) realloc_c(mem_ptr ptr,mem_size sz)
{
	mem_zone_ref	ref;
	mem_ptr			m_ptr,ret_ptr;

	m_ptr		=	mem_dec(ptr,16);
	ref.zone	=	*((mem_ptr *)(m_ptr));

	if(realloc_zone	(&ref,sz+16)!=1)
	{
		return PTR_NULL;
	}

	m_ptr		=	get_zone_ptr(&ref,0);

	*((unsigned int *)(m_ptr))=mem_to_uint(ref.zone);

	ret_ptr		=	mem_add(m_ptr,16);

	return ret_ptr;
}


OS_API_C_FUNC(void) free_c(mem_ptr ptr)
{
	
	mem_zone_ref	ref;
	mem_ptr			m_ptr;
	if(ptr==PTR_NULL)return;

	m_ptr		=	mem_dec(ptr,16);
	ref.zone	=	*((mem_ptr *)(m_ptr));
	release_zone_ref(&ref);
	
	

}

OS_API_C_FUNC(mem_ptr) calloc_c(mem_size sz,mem_size blk)
{
	return malloc_c(sz*blk);
}


OS_API_C_FUNC(uint64_t) mul64(uint64_t a, uint64_t b)
{
	return a * b;
}

OS_API_C_FUNC(uint64_t) shl64(uint64_t a, unsigned char n)
{
	return a << n;
}
OS_API_C_FUNC(uint64_t) shr64(uint64_t a, unsigned char n)
{
	return a >> n;
}
OS_API_C_FUNC(uint64_t) muldiv64(uint64_t a, uint64_t b, uint64_t c)
{
	uint64_t tmp;
	tmp = a	* b;
	tmp = tmp / c;
	return tmp;
}

#define UINT32_MAX 0xFFFFFFFF
OS_API_C_FUNC(void) big128_mul(unsigned int x, struct big64 y, struct big128 *out)
{
	/* x * y = (z2 << 64) + (z1 << 32) + z0
	* where z2 = x1 * y1
	*       z1 = x0 * y1 + x1 * y0
	*       z0 = x0 * y0
	*/
	uint64_t x0 = x, x1 = 0, y0 = y.m.v[0], y1 = y.m.v[1];
	uint64_t z0 = x0 * y0;
	uint64_t z1a = x1 * y0;
	uint64_t z1b = x0 * y1;
	uint64_t z2 = x1 * y1;

	unsigned int z0l = z0 & UINT32_MAX;
	unsigned int z0h = z0 >> 32u;

	uint64_t z1al = z1a & UINT32_MAX;
	uint64_t z1bl = z1b & UINT32_MAX;
	uint64_t z1l = z1al + z1bl + z0h;

	uint64_t z1h = (z1a >> 32u) + (z1b >> 32u) + (z1l >> 32u);
	z2 += z1h;

	out->v[0] = z0l;
	out->v[1] = z1l & UINT32_MAX;
	out->v[2] = z2 & UINT32_MAX;
	out->v[3] = z2 >> 32u;
}

OS_API_C_FUNC(double) powf_c(double a, double b)
{
	double result;
	powd_c(a, b, &result);
	return result;

}

#ifdef _DEBUG
OS_API_C_FUNC(size_t) dump_mem_used_after(unsigned int area_id, unsigned int time, mem_zone_ref *outs, size_t nOuts)
{
	int						n;
	size_t					mem_size;
	size_t					n_zones;
	mem_area				*area_ptr;

	area_ptr = get_area(area_id);
	if (area_ptr == PTR_NULL)return 0;

	mem_size = 0;
	n_zones = 0;
	n = 0;
	while (n<area_ptr->nzones)
	{
		if (area_ptr->zones_buffer[n].mem.ptr != PTR_NULL)
		{
			if (area_ptr->zones_buffer[n].time>time)
			{
				outs[n_zones++].zone = &area_ptr->zones_buffer[n];
				if (n_zones >= nOuts)break;
				/*
				unsigned int	*data;
				int _n;
				if(area_ptr->zones_buffer[n].mem.size>0)
				{
				data=area_ptr->zones_buffer[n].mem.ptr;
				for(_n=0;_n<4;_n++)
				{
				writeint(data[_n],16);
				writestr(",");
				}
				writestr("\n");
				}
				*/
			}
		}
		n++;
	}

	return n_zones;
}
#endif

OS_API_C_FUNC(unsigned int) find_zones_used(unsigned int area_id)
{
	unsigned int n, nfree;
	mem_area	*area_ptr;

	area_ptr = get_area(area_id);
	if (area_ptr == PTR_NULL)
	{
		return 0;
	}

	nfree = 0;
	n = 0;
	while (n<area_ptr->nzones)
	{
		if ((area_ptr->zones_buffer[n].mem.ptr == PTR_NULL) || (area_ptr->zones_buffer[n].mem.size == 0))
		{
			nfree++;
		}
		n++;
	}
	return (area_ptr->nzones - nfree);
}

OS_API_C_FUNC(size_t) dump_mem_used(unsigned int area_id)
{
	int						n;
	size_t					mem_size;
	int						n_zones;
	mem_area				*area_ptr;

	if (__global_mem_areas == PTR_NULL)return 0;
	

	if (area_id == 0xFFFFFFFF)
	{
		int n = 0;
		while (n<MAX_MEM_AREAS)
		{
			unsigned int _n;

			area_ptr = &__global_mem_areas[n];
			if (area_ptr->type != 0x00000000)
			{
				mem_size = 0;
				n_zones = 0;
				_n = 0;
				while (_n<area_ptr->nzones)
				{
					if (area_ptr->zones_buffer[_n].mem.ptr != PTR_NULL)
					{
						mem_size += area_ptr->zones_buffer[_n].mem.size;
						n_zones++;
					}
					_n++;
				}

			}
			n++;
		}
		return mem_size;
	}

	area_ptr = get_area(area_id);
	if (area_ptr == PTR_NULL)return 0;

	mem_size = 0;
	n_zones = 0;
	n = 0;
	while (n<area_ptr->nzones)
	{
		if (area_ptr->zones_buffer[n].mem.ptr != PTR_NULL)
		{
			mem_size += area_ptr->zones_buffer[n].mem.size;
			n_zones++;
		}
		n++;
	}
	return mem_size;
}


/*
OS_API_C_FUNC(int) 	align_zone_memory(mem_zone_ref *zone_ref, mem_size align)
{
mem_area				*area_ptr;
mem_zone_desc			new_free_zone;
size_t					new_size;
unsigned int			mask;
mem_zone				*src_zone;
mem_zone_desc			new_zone;
mem_zone_desc			*mem;
mem_ptr					aligned_ptr;
src_zone = zone_ref->zone;
mask	 = align - 1;
if ((mem_to_uint(src_zone->mem.ptr) & mask) == 0)return 1;

new_size = src_zone->mem.size + align;

if (new_size & mask)
new_size = ((new_size & (~mask)) + align);

area_ptr = get_area(src_zone->area_id);
if (area_ptr == PTR_NULL)
{
return 0;
}
mem = &src_zone->mem;

if (find_free_zone(area_ptr, new_size, &new_zone) == 0)
{
//task_manager_release_semaphore(area_ptr->lock_sema,0);
return -1;
}

if (mem_to_uint(new_zone.ptr) & mask)
{
aligned_ptr		= uint_to_mem(((mem_to_uint(new_zone.ptr) & (~mask)) + align));
new_zone.ptr	= aligned_ptr;

new_free_zone.size = mem_sub(new_zone.ptr, aligned_ptr);
if (new_free_zone.size > 0)
{
new_zone.size	 -= new_free_zone.size;
new_free_zone.ptr = new_zone.ptr;
free_zone_area(src_zone->area_id, &new_free_zone);
}
}
memcpy_c		(new_zone.ptr, src_zone->mem.ptr, src_zone->mem.size);
free_zone_area	(src_zone->area_id, &src_zone->mem);
src_zone->mem.ptr = new_zone.ptr;
src_zone->mem.size = new_zone.size;

return 1;
}


OS_API_C_FUNC(unsigned int) create_zone_ref(mem_zone_ref *dest_zone_ref,mem_ptr ptr,mem_size size)
{
mem_zone		*new_zone;

if(n_mapped_zones>=32)
{

}

new_zone			=&mapped_zones[n_mapped_zones];

new_zone->area_id	=0xFFFF;
new_zone->n_refs	=1;
new_zone->mem.ptr	=ptr;
new_zone->mem.size	=size;
new_zone->time		= get_time_c();
new_zone->free_func =PTR_NULL;
dest_zone_ref->zone=new_zone;

n_mapped_zones++;
return 1;
}

*/