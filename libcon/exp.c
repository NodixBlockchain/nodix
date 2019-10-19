/* copyright iadix 2016 */
#define LIBC_API C_EXPORT
#include "base/std_def.h"
#include "base/std_mem.h"
#include "base/mem_base.h"
#include "base/std_str.h"
#include "base/utf.h"

#include "strs.h"


#define KERNEL_API C_EXPORT
#include "mem_stream.h"
#include "tpo_mod.h"
#include "fsio.h"
#include "xmlparse.h"

extern int init_zfuncs();



void init_funcs()
{
	memset_c(modz,0,sizeof(modz));
	n_modz=0;

	#ifndef _NATIVE_LINK_
	sys_add_tpo_mod_func_name("libcon", "utf8_encode", (void_func_ptr)utf8_encode, 0);
	sys_add_tpo_mod_func_name("libcon", "utf8_check_first", (void_func_ptr)utf8_check_first, 0);
	sys_add_tpo_mod_func_name("libcon", "utf8_check_full", (void_func_ptr)utf8_check_full, 0);
	sys_add_tpo_mod_func_name("libcon", "default_RNG", (void_func_ptr)default_RNG, 0);
	sys_add_tpo_mod_func_name("libcon", "console_print", (void_func_ptr)console_print, 0);
	sys_add_tpo_mod_func_name("libcon", "stat_file", (void_func_ptr)stat_file, 0);
	sys_add_tpo_mod_func_name("libcon", "create_dir", (void_func_ptr)create_dir, 0);
	sys_add_tpo_mod_func_name("libcon", "del_dir", (void_func_ptr)del_dir, 0);
	sys_add_tpo_mod_func_name("libcon", "rm_dir", (void_func_ptr)rm_dir, 0);
	sys_add_tpo_mod_func_name("libcon", "put_file", (void_func_ptr)put_file, 0);
	sys_add_tpo_mod_func_name("libcon", "append_file", (void_func_ptr)append_file, 0);
	sys_add_tpo_mod_func_name("libcon", "truncate_file", (void_func_ptr)truncate_file, 0);
	sys_add_tpo_mod_func_name("libcon", "get_file", (void_func_ptr)get_file, 0);
	sys_add_tpo_mod_func_name("libcon", "get_file_len", (void_func_ptr)get_file_len, 0);
	sys_add_tpo_mod_func_name("libcon", "get_file_chunk", (void_func_ptr)get_file_chunk, 0);
	sys_add_tpo_mod_func_name("libcon", "get_file_range", (void_func_ptr)get_file_range, 0);
	

	sys_add_tpo_mod_func_name("libcon", "get_file_to_memstream", (void_func_ptr)get_file_to_memstream, 0);
	sys_add_tpo_mod_func_name("libcon", "del_file", (void_func_ptr)del_file, 0);
	sys_add_tpo_mod_func_name("libcon", "move_file", (void_func_ptr)move_file, 0);
	sys_add_tpo_mod_func_name("libcon", "file_size", (void_func_ptr)file_size, 0);
	sys_add_tpo_mod_func_name("libcon", "get_sub_dirs", (void_func_ptr)get_sub_dirs, 0);
	sys_add_tpo_mod_func_name("libcon", "get_sub_files", (void_func_ptr)get_sub_files, 0);
	sys_add_tpo_mod_func_name("libcon", "log_output", (void_func_ptr)log_output, 0);
	sys_add_tpo_mod_func_name("libcon", "set_ftime", (void_func_ptr)set_ftime, 0);
	sys_add_tpo_mod_func_name("libcon", "get_ftime", (void_func_ptr)get_ftime, 0);
	sys_add_tpo_mod_func_name("libcon", "get_home_dir", (void_func_ptr)get_home_dir, 0);
	sys_add_tpo_mod_func_name("libcon", "set_home_path", (void_func_ptr)set_home_path, 0);
	sys_add_tpo_mod_func_name("libcon", "set_cwd", (void_func_ptr)set_cwd, 0);
	sys_add_tpo_mod_func_name("libcon", "get_cwd", (void_func_ptr)get_cwd, 0);

	
	
	sys_add_tpo_mod_func_name("libcon", "background_func", (void_func_ptr)background_func, 0);

	sys_add_tpo_mod_func_name("libcon", "daemonize", (void_func_ptr)daemonize, 0);
	sys_add_tpo_mod_func_name("libcon", "get_time_c", (void_func_ptr)get_time_c, 0);
	sys_add_tpo_mod_func_name("libcon", "get_system_time_c", (void_func_ptr)get_system_time_c, 0);
	sys_add_tpo_mod_func_name("libcon", "XML_ParserCreate", (void_func_ptr)XML_ParserCreate, 0);
	sys_add_tpo_mod_func_name("libcon", "XML_SetElementHandler", (void_func_ptr)XML_SetElementHandler, 0);
	sys_add_tpo_mod_func_name("libcon", "XML_SetCharacterDataHandler", (void_func_ptr)XML_SetCharacterDataHandler, 0);
	sys_add_tpo_mod_func_name("libcon", "XML_SetUserData", (void_func_ptr)XML_SetUserData, 0);
	sys_add_tpo_mod_func_name("libcon", "XML_Parse", (void_func_ptr)XML_Parse, 0);
	sys_add_tpo_mod_func_name("libcon", "XML_ParserFree", (void_func_ptr)XML_ParserFree, 0);

	init_zfuncs				();	
#endif

	
}
