#ifndef LIBC_API
#define LIBC_API C_IMPORT
#endif

#define MOD_HASH(name) calc_crc32_c(name,64)
#define FUNC_HASH(name) calc_crc32_c(name,256)

typedef struct
{
	unsigned int		crc_32;
	unsigned int		sym_addr;
	unsigned int		string_idx;
}tpo_export;

typedef struct
{
	size_t				reloc_addr;
	unsigned int		sym_addr;
}tpo_import;

typedef struct
{
	size_t				section_ptr;
	size_t				section_size;
	size_t				section_img_ofset;
	unsigned int		is_code;
	mem_zone_ref		exports_fnc;
	mem_zone_ref		imports_fnc;

}tpo_section;

typedef struct
{
	unsigned int			mod_idx;
	mod_name_decoration_t	deco_type;
	char					name[64];
	unsigned int			name_hash;
	mem_zone_ref			string_buffer_ref;
	unsigned int			string_buffer_len;
	mem_zone_ref			data_sections;
	tpo_section				sections[32];
}tpo_mod_file;

struct kern_mod_fn_t
{
	unsigned int	func_hash;
	unsigned int	func_addr;
	unsigned int	section_start_addr;
	unsigned int	string_idx;
}struct_packed;

struct kern_mod_t
{
	unsigned int	mod_hash;
	unsigned char	n_funcs;
	unsigned char	n_sections;
	unsigned short  fn_ofset;
	mem_ptr			mod_addr;
	mem_ptr			string_tbl;
}struct_packed;

typedef const tpo_mod_file *const_tpo_mod_file_ptr ;



LIBC_API void			C_API_FUNC tpo_mod_init(tpo_mod_file *driver);

LIBC_API int			C_API_FUNC tpo_mod_load_tpo(mem_stream *file_stream, tpo_mod_file *tpo_file, unsigned int flags);
LIBC_API tpo_mod_file *  C_API_FUNC find_mod_ptr(unsigned int name_hash);

LIBC_API void_func_ptr	C_API_FUNC tpo_mod_get_exp_addr(mem_stream *file_stream, const char *sym);
LIBC_API void_func_ptr	C_API_FUNC get_tpo_mod_exp_addr_name(const tpo_mod_file *tpo_mod, const char *name, unsigned int deco_type);
LIBC_API int			C_API_FUNC set_tpo_mod_exp_value32_name(const tpo_mod_file *tpo_mod, const char *name, unsigned int value);
LIBC_API void			C_API_FUNC register_tpo_exports(tpo_mod_file *tpo_mod, const char *mod_name);

LIBC_API int				C_API_FUNC load_module(const char *file, const char *mod_name, tpo_mod_file *mod,unsigned int flags);
/*LIBC_API struct kern_mod_t	*C_API_FUNC tpo_mod_find(const char *name);*/




LIBC_API int C_API_FUNC execute_script_mod_call(tpo_mod_file		*tpo_mod, const char *method);
LIBC_API int C_API_FUNC execute_script_mod_rcall(tpo_mod_file		*tpo_mod, const char *method, mem_zone_ref_ptr input);
LIBC_API int C_API_FUNC execute_script_mod_rwcall(tpo_mod_file		*tpo_mod, const char *method, mem_zone_ref_ptr input, mem_zone_ref_ptr output);

typedef int C_API_FUNC message_handler(mem_zone_ref_ptr node, mem_zone_ref_ptr payload);
typedef message_handler *message_handler_ptr;

typedef int C_API_FUNC message_handler_res(mem_zone_ref_ptr node, mem_zone_ref_ptr handler, mem_zone_ref_ptr payload, mem_zone_ref_ptr result);
typedef message_handler_res *message_handler_res_ptr;


typedef int C_API_FUNC module_proc();
typedef module_proc *module_proc_ptr;


typedef int C_API_FUNC module_rproc(mem_zone_ref_ptr input);
typedef module_rproc *module_rproc_ptr;

typedef int C_API_FUNC module_rwproc(mem_zone_ref_ptr input, mem_zone_ref_ptr output);
typedef module_rwproc *module_rwproc_ptr;

#ifdef _NATIVE_LINK_
LIBC_API int C_API_FUNC set_dbg_ptr2(module_rwproc_ptr  a, module_rwproc_ptr b, module_rproc_ptr  c, module_rwproc_ptr  d, module_rwproc_ptr e, module_rproc_ptr f, module_rwproc_ptr g, module_rproc_ptr h, module_rproc_ptr i, module_rproc_ptr j, module_rproc_ptr k, module_rproc_ptr l, module_rproc_ptr m, module_rproc_ptr n, module_rproc_ptr o, module_rwproc_ptr p, module_rwproc_ptr q, module_rproc_ptr r, module_rproc_ptr s, module_rproc_ptr t, module_rproc_ptr u, module_rproc_ptr v, module_rproc_ptr w, module_rproc_ptr x, module_rwproc_ptr y, module_rwproc_ptr z, module_rproc_ptr zz, module_rwproc_ptr zz2, module_rproc_ptr zz3, module_rwproc_ptr zz4, module_rwproc_ptr  zz5, module_rproc_ptr  zz6, module_proc_ptr zz7, module_rproc_ptr zz8, module_rwproc_ptr zz9, module_rwproc_ptr zz10, module_rwproc_ptr zz11, module_rwproc_ptr zz12, module_rwproc_ptr zz13, module_rproc_ptr zz14, module_rwproc_ptr zz15, module_rwproc_ptr zz16, module_proc_ptr zz17, module_rwproc_ptr zz18, module_rwproc_ptr zz19, module_rwproc_ptr zz20, module_rproc_ptr zz21, module_rwproc_ptr zz22, module_rwproc_ptr zz23, module_rwproc_ptr zz24, module_rwproc_ptr zz25, module_rwproc_ptr zz26, module_rproc_ptr  zz27, module_rproc_ptr   zz28);
LIBC_API int C_API_FUNC set_dbg_ptr(module_rproc_ptr a, module_rwproc_ptr b, module_rproc_ptr c, module_proc_ptr d, module_rwproc_ptr  e, module_proc_ptr f, module_rproc_ptr  g, module_rproc_ptr h, module_rproc_ptr  i, module_rproc_ptr  j, module_rproc_ptr  k, module_rproc_ptr  l, module_rwproc_ptr  m, module_rwproc_ptr  n, module_rwproc_ptr o, module_rproc_ptr p, module_rwproc_ptr q, module_rproc_ptr r, module_rproc_ptr s, module_rproc_ptr t, module_rproc_ptr u, module_rproc_ptr v, module_rproc_ptr w, module_rproc_ptr x, module_rproc_ptr y, module_rproc_ptr z, module_rwproc_ptr zz, module_proc_ptr zzz);
LIBC_API int C_API_FUNC set_pos_dbg_ptr(module_rproc_ptr a, module_rproc_ptr b, module_rproc_ptr c, module_rproc_ptr d, module_rwproc_ptr e, module_rproc_ptr f, module_rwproc_ptr g);
#endif


#ifndef KERNEL_API
#define KERNEL_API	C_IMPORT
#endif

KERNEL_API unsigned int			KERN_API_FUNC	sys_add_tpo_mod_func_name(const char *name, const char *fn_name, void_func_ptr addr, unsigned int deco);
KERNEL_API struct kern_mod_t	*KERN_API_FUNC 	tpo_get_mod_entry_hash_c(unsigned int mod_hash);
KERNEL_API struct kern_mod_fn_t *KERN_API_FUNC 	tpo_get_fn_entry_name_c(unsigned int mod_idx, unsigned int mod_hash, unsigned int str_idx, unsigned int deco_type);



typedef unsigned int	C_API_FUNC defaut_import_func_ptr		(void *data);
typedef int			    C_API_FUNC run_func_fn					();
typedef int			    C_API_FUNC init_func_fn					(mem_zone_ref_ptr	init_data);

typedef run_func_fn *run_func_fn_ptr;
typedef init_func_fn *init_func_fn_ptr;

extern tpo_mod_file			*modz[64];
extern short				n_modz;

