//#define _DEBUG
/* copyright antoine bentue-ferrer 2016 */
#include <base/std_def.h>
#include <base/std_mem.h>
#include <base/mem_base.h>
#include <base/std_str.h>

#include <strs.h>
#include <connect.h>
#include <mem_stream.h>
#include <tpo_mod.h>
#include <fsio.h>

typedef int C_API_FUNC app_func						(mem_zone_ref_ptr params);

typedef void C_API_FUNC	tree_manager_init_func		(size_t size, size_t  nzones, unsigned int flags);

typedef int	C_API_FUNC load_script_func				(const char *file,const char *name, mem_zone_ref_ptr script_vars,unsigned int opt);
typedef int	C_API_FUNC resolve_script_var_func		(mem_zone_ref_ptr script_vars, mem_zone_ref_ptr script_proc, const char *var_path, unsigned int var_type, mem_zone_ref_ptr out_var, mem_zone_ref_ptr pout_var);
typedef int C_API_FUNC get_script_var_value_str_func(mem_zone_ref_ptr global_vars, const char *var_path, struct string *out, unsigned int radix);
typedef int C_API_FUNC get_script_var_value_ptr_func(mem_zone_ref_ptr global_vars, const char *var_path, mem_ptr *out);
typedef int C_API_FUNC execute_script_proc_func		(mem_zone_ref_ptr global_vars, mem_zone_ref_ptr proc);

typedef app_func						*app_func_ptr;
typedef load_script_func				*load_script_func_ptr;
typedef resolve_script_var_func			*resolve_script_var_func_ptr;
typedef get_script_var_value_str_func	*get_script_var_value_str_func_ptr;
typedef get_script_var_value_ptr_func	*get_script_var_value_ptr_func_ptr;
typedef execute_script_proc_func		*execute_script_proc_func_ptr;
typedef tree_manager_init_func			*tree_manager_init_func_ptr;

load_script_func_ptr				load_script = PTR_INVALID;
resolve_script_var_func_ptr			resolve_script_var = PTR_INVALID;
get_script_var_value_str_func_ptr	get_script_var_value_str = PTR_INVALID;
get_script_var_value_ptr_func_ptr	get_script_var_value_ptr = PTR_INVALID;
execute_script_proc_func_ptr		execute_script_proc = PTR_INVALID;
tree_manager_init_func_ptr			tree_manager_init = PTR_INVALID;

app_func_ptr						app_init = PTR_INVALID, app_start = PTR_INVALID, app_loop = PTR_INVALID, app_stop = PTR_INVALID;


tpo_mod_file libbase_mod = { 0xDEF00FED };


int main(int argc, const char **argv)
{
	struct string			node_name = { PTR_NULL },data_dir={PTR_NULL};
	mem_zone_ref			nodeDef = { PTR_NULL }, params = { PTR_NULL }, script_vars = { PTR_NULL }, init_node_proc = { PTR_NULL };
	const_mem_ptr			*params_ptr;
	tpo_mod_file			*nodix_mod;
	int						done = 0,n,withGC;


	init_mem_system			();
	init_default_mem_area	(32 * 1024 * 1024, 128 * 1024);
	set_exe_path			();
	network_init			();

	
	log_output("Nodix starting ...\n\n");

	if (daemonize("nodix") <= 0)
	{
		console_print("daemonize failed \n");
		return 0;
	}

	tpo_mod_init			(&libbase_mod);
	load_module				("modz/libbase.tpo", "libbase", &libbase_mod,1);

	withGC = 0;
	
	if (argc > 1)
	{
		allocate_new_zone(0, argc * sizeof(mem_ptr), &params);
		for (n = 0; n < (argc - 1); n++)
		{
			if (!strcmp_c(argv[n + 1], "gc"))
				withGC = 1;

			params_ptr = get_zone_ptr(&params, n * sizeof(mem_ptr));
			(*params_ptr) = argv[n + 1];
		}
		params_ptr = get_zone_ptr(&params, n * sizeof(mem_ptr));
		(*params_ptr) = PTR_NULL;
	}
			

	load_script				 = (load_script_func_ptr)get_tpo_mod_exp_addr_name(&libbase_mod, "load_script", 0);
	resolve_script_var		 = (resolve_script_var_func_ptr)get_tpo_mod_exp_addr_name(&libbase_mod, "resolve_script_var", 0);
	get_script_var_value_str = (get_script_var_value_str_func_ptr)get_tpo_mod_exp_addr_name(&libbase_mod, "get_script_var_value_str", 0);;
	get_script_var_value_ptr = (get_script_var_value_ptr_func_ptr)get_tpo_mod_exp_addr_name(&libbase_mod, "get_script_var_value_ptr", 0);;
	execute_script_proc		 = (execute_script_proc_func_ptr)get_tpo_mod_exp_addr_name(&libbase_mod, "execute_script_proc", 0);;
	tree_manager_init		 = (tree_manager_init_func_ptr)get_tpo_mod_exp_addr_name(&libbase_mod, "tree_manager_init", 0);;

	if (withGC == 1)
	{
		tree_manager_init	(24 * 1024 * 1024, 256 * 1024, 0x10);
		log_output("memory model : M&S \n");
	}
	else
	{
		tree_manager_init	(24 * 1024 * 1024, 256 * 1024, 0x0);
		log_output("memory model : ARC \n");
	}
	
	if (!load_script("nodix.node", "nodix.node", &script_vars, 7))
	{
		log_output("unable to load node script \n");
		return -1;
	}

	if (!get_script_var_value_str(&script_vars, "configuration.name", &node_name, 0))
		make_string(&node_name, "nodix");

	if (get_script_var_value_str(&script_vars, "SelfNode.data_dir", &data_dir, 0))
	{
		set_data_dir(&data_dir, "nodix");
		free_string	(&data_dir);
	}
	else
	{
		if (!set_home_path(node_name.str))
		{
			console_print("could not set home dir 'nodix' \n");
			return 0;
		}
	}

	if (!aquire_lock_file(node_name.str))
	{
		console_print("already running\n");
		return 0;
	}
	
	get_script_var_value_ptr(&script_vars, "nodix.mod_ptr"	, (mem_ptr *)&nodix_mod);
	
	resolve_script_var(&script_vars, PTR_NULL, "init_node", 0xFFFFFFFF, &init_node_proc, PTR_NULL);

	app_init = (app_func_ptr)get_tpo_mod_exp_addr_name(nodix_mod, "app_init", 0);
	app_start = (app_func_ptr)get_tpo_mod_exp_addr_name(nodix_mod, "app_start", 0);
	app_loop = (app_func_ptr)get_tpo_mod_exp_addr_name(nodix_mod, "app_loop", 0);
	app_stop = (app_func_ptr)get_tpo_mod_exp_addr_name(nodix_mod, "app_stop", 0);

	if (!app_init(&script_vars))
	{
		console_print("could not initialize app ");
		console_print(nodix_mod->name);
		console_print("\n");	
		return 0;
	}
	
	if (!execute_script_proc(&script_vars, &init_node_proc))
	{
		console_print("could not execute script initialization routine.");
		return 0;
	}
	

	if (!app_start(&params))
	{
		console_print("could not start app ");
		console_print(nodix_mod->name);
		console_print("\n");
		return 0;
	}
	
	while (isRunning())
	{
	
		app_loop		(PTR_NULL);
		snooze_c		(2000);
	}

	app_stop(PTR_NULL);
}

#ifdef _WIN32
#include <Windows.h>
void mainCRTStartup(void)
{
	char		*command;
	const char	*argv[32];
	int			argc;
	size_t		cmd_len;

	argc	= 0;
	command	=	GetCommandLine();
	if (command != PTR_NULL)
	{
		cmd_len = strlen_c(command);
		if (cmd_len > 0)
		{
			const char *last_cmd = command;
			int			open_quote = 0;
			size_t		n;
			for (n = 0; n < cmd_len;n++)
			{
				if ((open_quote == 0) && (command[n] == '"'))
				{ 
					last_cmd = (command + n + 1); 
					open_quote = 1; 
					continue; 
				}
				
				if (((open_quote == 0)&&(command[n] == ' '))||
					((open_quote == 1)&&(command[n] == '"')))
				{
					if (command[n+1] != 0 )
					{
						argv[argc++]	= last_cmd;

						if (open_quote)
						{
							last_cmd = (command + n + 2);
							command[n] = 0;
							n++;
						}
						else
						{
							last_cmd = (command + n + 1);
							command[n] = 0;
						}
						
					}
					open_quote		= 0;
				}
			}
			argv[argc++] = last_cmd;
		}
	}
	else
	{
		argc	= 0;
		argv[0]	= PTR_NULL;
		argv[1] = PTR_NULL;
	}
	main(argc, argv);
}
#endif