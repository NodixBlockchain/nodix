
#define PTR_NULL	(void *)0x00000000L
#define PTR_INVALID (void *)0xDEADBEEFLL
#define PTR_FF		(void *)0xFFFFFFFFLL

/* #define NATIVE_ALLOC */

typedef struct
{
	volatile  mem_ptr zone;
}mem_zone_ref;

typedef struct
{
	const_mem_ptr zone;
}mem_zone_const_ref;

typedef mem_zone_ref			*mem_zone_ref_ptr;
typedef const mem_zone_ref		*mem_zone_ref_const_ptr;
typedef mem_zone_const_ref		*mem_zone_const_ref_ptr;

typedef int C_API_FUNC zone_free_func(mem_zone_ref_ptr zone_ref, unsigned int scan_id);
typedef  zone_free_func	*zone_free_func_ptr;

typedef int C_API_FUNC thread_func(mem_zone_ref_ptr p,unsigned int *status);
typedef thread_func *thread_func_ptr;

typedef enum {MEM_TYPE_DATA = 1,MEM_TYPE_TREE = 2}mem_area_type_t;
	
typedef int				vec_2s_t[2];
typedef unsigned int	vec_2_t[2];
typedef unsigned char	vec_4uc_t[4];

struct gfx_rect
{
	vec_2s_t	pos;
	vec_2_t		size;
};


#ifdef __cplusplus
	extern "C" {
#endif
LIBC_API void			C_API_FUNC init_default_mem_area	(size_t size, size_t nzones);

LIBC_API unsigned int	C_API_FUNC init_new_mem_area		(mem_ptr phys_start, mem_ptr phys_end, size_t nzones, mem_area_type_t type);


LIBC_API unsigned int	C_API_FUNC free_mem_area			(unsigned int area_id);
LIBC_API unsigned int	C_API_FUNC allocate_new_zone		(unsigned int area_id,	mem_size zone_size,	mem_zone_ref *zone_ref);
LIBC_API unsigned int	C_API_FUNC allocate_new_empty_zone	(unsigned int area_id,mem_zone_ref *zone_ref);
LIBC_API int			C_API_FUNC expand_zone				(mem_zone_ref *ref,mem_size new_size);
LIBC_API int 			C_API_FUNC realloc_zone				(mem_zone_ref *zone_ref,mem_size new_size);
LIBC_API void 			C_API_FUNC empty_trash				();
		
LIBC_API void			C_API_FUNC copy_zone_ref			(mem_zone_ref_ptr dest_zone_ref, const mem_zone_ref *zone_ref);

LIBC_API unsigned int	C_API_FUNC get_zone_area_type		(mem_zone_ref_const_ptr zone);

LIBC_API void			C_API_FUNC init_mem_system			();
LIBC_API size_t			C_API_FUNC dump_mem_used			(unsigned int area_id);

#ifdef _DEBUG
LIBC_API size_t			C_API_FUNC dump_mem_used_after		(unsigned int area_id, unsigned int time, mem_zone_ref *outs, size_t nOuts);
#endif

LIBC_API int			C_API_FUNC  get_shared_slot			(mem_zone_ref_ptr shared_zone, mem_zone_ref_ptr *zone_ptr);
LIBC_API int			C_API_FUNC release_shared_slot		(mem_zone_ref_ptr zone_ptr);

LIBC_API mem_ptr		C_API_FUNC get_zone_ptr				(mem_zone_ref_const_ptr ref,mem_size ofset);
LIBC_API mem_size		C_API_FUNC get_zone_size			(mem_zone_ref_const_ptr ref);
LIBC_API unsigned int	C_API_FUNC do_mark_sweep			(unsigned int area_id, unsigned int delay);
LIBC_API void			C_API_FUNC mark_zone				(mem_ptr zone, unsigned int scan_id);

LIBC_API unsigned int	C_API_FUNC find_zones_used			(unsigned int area_id);
LIBC_API void			C_API_FUNC do_gdt_real_mode			(mem_ptr new_gdt);
LIBC_API unsigned int	C_API_FUNC get_zone_numref			(mem_zone_ref *zone_ref);
LIBC_API unsigned int	C_API_FUNC area_type				(unsigned int area_id);

LIBC_API void			C_API_FUNC swap_zone_ref					(mem_zone_ref_ptr dest_zone_ref, mem_zone_ref_ptr src_zone_ref);
//LIBC_API int			C_API_FUNC align_zone_memory				(mem_zone_ref *zone_ref, mem_size align);


LIBC_API int			C_API_FUNC set_mem_area_id					(unsigned int area_id);
LIBC_API int			C_API_FUNC set_tree_mem_area_id				(unsigned int area_id);
LIBC_API void			C_API_FUNC aquire_lock_excl					(volatile unsigned int *lock, unsigned int excl);
LIBC_API void			C_API_FUNC release_lock_excl				(volatile unsigned int *lock);

LIBC_API unsigned int	C_API_FUNC get_mem_area_id					(void);
LIBC_API unsigned int	C_API_FUNC get_tree_mem_area_id				(void);
LIBC_API int			C_API_FUNC get_thread_data					(mem_zone_ref_ptr thread_data);
LIBC_API int			C_API_FUNC set_thread_data					(mem_zone_ref_const_ptr thread_data);
LIBC_API int			C_API_FUNC background_func					(thread_func_ptr func, mem_zone_ref_ptr params);
LIBC_API void			C_API_FUNC ptr_to_ref						(mem_ptr ptr, mem_zone_ref_ptr out);
LIBC_API unsigned int	C_API_FUNC get_thread_id					(void);

static __inline unsigned int mem_to_uint(const_mem_ptr ptr)
{
	return *((unsigned int *)&ptr);
}

static __inline unsigned short mem_to_ushort(const_mem_ptr ptr,unsigned char which)
{
	unsigned int ptr_val;
	unsigned int mask;
	unsigned short ret;
	mask	=	(0xFFFF<<which);
	ptr_val	=	*((unsigned int *)&ptr);
	ret		=	(unsigned short )((ptr_val&mask)>>which);
	return ret;
}

static __inline mem_size mem_to_size(const_mem_ptr ptr)
{
	return *((mem_size *)&ptr);
}

static __inline int mem_to_int(const_mem_ptr ptr)
{
	return *((int *)&ptr);
}


static __inline mem_ptr uint_to_mem(unsigned int val)
{
	large_uint_t val64;

	val64.uint32.ints[0]=val;
	val64.uint32.ints[1]=0;
	return ((mem_ptr )((mem_size)(val64.uint64.val)));
}

static __inline mem_ptr size_to_mem(size_t val)
{
	return ((mem_ptr )val);
}

static __inline mem_size mem_sub(const_mem_ptr base,const_mem_ptr end)
{
	mem_size	s_base,s_end;

	s_base	=	mem_to_uint(base);
	s_end	=	mem_to_uint(end);

	return (s_end-s_base);

}

static __inline mem_ptr mem_add(const_mem_ptr base,mem_size size)
{
	mem_size	s_base;

	s_base	=	mem_to_uint(base);
	s_base  =    s_base + size;
	return		size_to_mem(s_base);
}

static __inline mem_ptr mem_dec(const_mem_ptr base,mem_size size)
{
	mem_size	s_base;

	s_base	=	 mem_to_uint(base);
	s_base  =    s_base - size;
	return		size_to_mem(s_base);
}


static __inline void copy_vec4u_c	(vec_4uc_t d,const vec_4uc_t s)
{
	d[0]=s[0];
	d[1]=s[1];
	d[2]=s[2];
	d[3]=s[3];
}

LIBC_API void			C_API_FUNC  release_zone_ref	(mem_zone_ref_ptr zone_ref);
LIBC_API void			C_API_FUNC  dec_zone_ref		(mem_zone_ref_ptr zone_ref);
LIBC_API void		    C_API_FUNC	inc_zone_ref		(mem_zone_ref_ptr zone_ref);
LIBC_API mem_size	    C_API_FUNC	set_zone_free		(mem_zone_ref_ptr ref, zone_free_func_ptr	free_func);


#ifdef __cplusplus
	}
#endif

