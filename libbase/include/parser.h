#ifndef LIBBASE_API
	#define LIBBASE_API C_IMPORT
#endif

LIBBASE_API int	C_API_FUNC load_script				(const char *file,const char *name, mem_zone_ref_ptr script_vars,unsigned int opts);
LIBBASE_API int	C_API_FUNC get_script_var_value_str (mem_zone_ref_ptr global_vars, const char *var_path, struct string *out, unsigned int radix);
LIBBASE_API int	C_API_FUNC get_script_var_value_i32 (mem_zone_ref_ptr global_vars, const char *var_path, unsigned int *out);
LIBBASE_API int	C_API_FUNC get_script_var_value_ptr	(mem_zone_ref_ptr global_vars, const char *var_path, mem_ptr *out);
LIBBASE_API int	C_API_FUNC resolve_script_var		(mem_zone_ref_const_ptr global_vars, mem_zone_ref_const_ptr script_proc, const char *var_path, unsigned int var_type, mem_zone_ref_ptr out_var, mem_zone_ref_ptr pout_var);
LIBBASE_API int	C_API_FUNC execute_script_proc		(mem_zone_ref_ptr global_vars, mem_zone_ref_ptr script_proc);
LIBBASE_API int	C_API_FUNC load_mod_def				(mem_zone_ref_ptr mod_def,unsigned int flags);
LIBBASE_API int	C_API_FUNC script_add_event_handler (mem_zone_ref_const_ptr global_vars, const struct string *msg_handler_str, const struct string *eval, mem_zone_ref_ptr handlers, mem_zone_ref_ptr handler);
LIBBASE_API int	C_API_FUNC mod_add_event_handler	(mem_zone_ref_const_ptr mod_def	, mem_zone_ref_ptr handlers, const struct string *eval, const char *msg_handler_str, mem_zone_ref_ptr handler);
LIBBASE_API int	C_API_FUNC script_find_event_handler(mem_zone_ref_const_ptr handlers, const char *proc_name, mem_zone_ref_ptr handler);
LIBBASE_API int	C_API_FUNC script_del_event_handler (mem_zone_ref_ptr msg_list, const char *scriptFile);
LIBBASE_API int	C_API_FUNC parse_expression			(const struct string *str, mem_zone_ref_ptr inputs, mem_zone_ref_ptr parse_tree);
LIBBASE_API int	C_API_FUNC eval_tree_to_float		(mem_zone_ref_ptr tree, mem_zone_ref_ptr inputs, float *fOutput, size_t offset);

