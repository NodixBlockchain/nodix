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

//libcon.lib;libbase.lib;nodix.lib;kernel32.lib;

#ifdef NDEBUG
#undef _DEBUG
#endif

#ifdef _DEBUG

#include "../libbase/include/parser.h"
#include "../libbase/include/tree.h"

C_IMPORT int C_API_FUNC	init_pos(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC	store_blk_staking(mem_zone_ref_ptr params);
			 
C_IMPORT int C_API_FUNC	set_block_hash(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC	add_money_supply(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC	truncate_chain_to(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC	sub_money_supply(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC	node_check_block_index(mem_zone_ref_ptr p1, mem_zone_ref_ptr p2);
C_IMPORT int C_API_FUNC	block_has_pow(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC	set_next_check(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC	store_blk_tx_staking(mem_zone_ref_ptr params);

C_IMPORT int C_API_FUNC node_get_root_app_fee(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC node_get_apps(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC node_get_app(mem_zone_ref_ptr p1, mem_zone_ref_ptr p2);
C_IMPORT int C_API_FUNC node_get_app_types_def(mem_zone_ref_ptr p1, mem_zone_ref_ptr p2);
C_IMPORT int C_API_FUNC node_get_app_objs(mem_zone_ref_ptr p1, mem_zone_ref_ptr p2);
C_IMPORT int C_API_FUNC blog_get_cat(mem_zone_ref_ptr p1, mem_zone_ref_ptr p2);
C_IMPORT int C_API_FUNC blog_get_post(mem_zone_ref_ptr p1, mem_zone_ref_ptr p2);
C_IMPORT int C_API_FUNC blog_get_accounts(mem_zone_ref_ptr p1, mem_zone_ref_ptr p2);
C_IMPORT int C_API_FUNC blog_get_msgs(mem_zone_ref_ptr p1, mem_zone_ref_ptr p2);




C_IMPORT int C_API_FUNC node_get_types_def(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC node_add_tmp_file(mem_zone_ref_ptr params);



C_IMPORT int C_API_FUNC	queue_getblock_hdrs_message(mem_zone_ref_ptr params);
			 
C_IMPORT int C_API_FUNC	stake_get_reward(mem_zone_ref_ptr in, mem_zone_ref_ptr out);
			 
C_IMPORT int C_API_FUNC	load_last_pos_blk(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC	find_last_pos_block(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC	compute_last_pos_diff(mem_zone_ref_ptr in, mem_zone_ref_ptr out);
C_IMPORT int C_API_FUNC	store_block(mem_zone_ref_ptr in, mem_zone_ref_ptr out);
C_IMPORT int C_API_FUNC	node_is_next_block(mem_zone_ref_const_ptr in);

C_IMPORT int C_API_FUNC	node_list_accounts(mem_zone_ref_ptr account_list);
C_IMPORT int C_API_FUNC	node_list_addrs(mem_zone_ref_ptr account_name, mem_zone_ref_ptr addr_list);

			 
C_IMPORT int C_API_FUNC	accept_block(mem_zone_ref_ptr in, mem_zone_ref_ptr out);
C_IMPORT int C_API_FUNC	node_compute_pow_diff_after(mem_zone_ref_ptr in);
C_IMPORT int C_API_FUNC	get_pow_reward(mem_zone_ref_ptr in, mem_zone_ref_ptr out);
C_IMPORT int C_API_FUNC	check_tx_files(mem_zone_ref_ptr in, mem_zone_ref_ptr out);
			 
C_IMPORT int C_API_FUNC	init_protocol(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC	init_blocks(mem_zone_ref_ptr p1, mem_zone_ref_ptr p2);
			 
C_IMPORT int C_API_FUNC	connect_peer_node(mem_zone_ref_ptr in);
C_IMPORT int C_API_FUNC	node_get_script_modules(mem_zone_ref_ptr in);
C_IMPORT int C_API_FUNC	node_get_script_msg_handlers(mem_zone_ref_ptr in);

C_IMPORT int C_API_FUNC	node_get_mempool_hashes(mem_zone_ref_ptr in);
			 
C_IMPORT int C_API_FUNC	node_add_block_header(mem_zone_ref_ptr in, mem_zone_ref_ptr out);
C_IMPORT int C_API_FUNC	node_check_chain(mem_zone_ref_ptr in, mem_zone_ref_ptr out);
C_IMPORT int C_API_FUNC	node_set_last_block(mem_zone_ref_ptr header);
C_IMPORT int C_API_FUNC	node_init_service(mem_zone_ref_ptr in, mem_zone_ref_ptr out);
C_IMPORT int C_API_FUNC	make_genesis_block(mem_zone_ref_ptr in, mem_zone_ref_ptr out);
C_IMPORT int C_API_FUNC	get_sess_account(mem_zone_ref_ptr in, mem_zone_ref_ptr out);
C_IMPORT int C_API_FUNC	node_has_service_module(mem_zone_ref_ptr module_name);
C_IMPORT int C_API_FUNC	node_load_bookmark();



C_IMPORT int C_API_FUNC	get_nscene(mem_zone_ref_ptr in);
C_IMPORT int C_API_FUNC	get_scene_list(mem_zone_ref_ptr in, mem_zone_ref_ptr in2);
C_IMPORT int C_API_FUNC	blog_get_cat_posts(mem_zone_ref_ptr account_name, mem_zone_ref_ptr addr_list);
C_IMPORT int C_API_FUNC blog_get_last_posts(mem_zone_ref_ptr account_name, mem_zone_ref_ptr addr_list);

C_IMPORT int C_API_FUNC	blog_get_ncats(mem_zone_ref_ptr in);
C_IMPORT int C_API_FUNC	blog_get_cats(mem_zone_ref_ptr in, mem_zone_ref_ptr in2);

C_IMPORT int C_API_FUNC	node_log_version_infos(mem_zone_ref_ptr in);
C_IMPORT int C_API_FUNC	queue_verack_message(mem_zone_ref_ptr in);
C_IMPORT int C_API_FUNC	queue_verack_message(mem_zone_ref_ptr in);
C_IMPORT int C_API_FUNC	queue_mempool_message(mem_zone_ref_ptr node);
C_IMPORT int C_API_FUNC	queue_getaddr_message(mem_zone_ref_ptr in);
C_IMPORT int C_API_FUNC	queue_version_message(mem_zone_ref_ptr in);
C_IMPORT int C_API_FUNC	queue_ping_message(mem_zone_ref_ptr in);
C_IMPORT int C_API_FUNC	queue_pong_message(mem_zone_ref_ptr in, mem_zone_ref_ptr nonce);
C_IMPORT int C_API_FUNC	queue_getdata_message(mem_zone_ref_ptr in, mem_zone_ref_ptr nonce);
C_IMPORT int C_API_FUNC	queue_inv_message(mem_zone_ref_ptr in, mem_zone_ref_ptr hash_list);
C_IMPORT int C_API_FUNC	node_get_mem_pool(mem_zone_ref_ptr tx_list);
C_IMPORT int C_API_FUNC	node_init_self(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC	node_store_last_pos_hash(mem_zone_ref_ptr blk);
C_IMPORT int C_API_FUNC	node_add_tx_to_mempool(mem_zone_ref_ptr tx);
C_IMPORT int C_API_FUNC	node_broadcast_addr(mem_zone_ref_ptr p2p_addr);
C_IMPORT int C_API_FUNC	node_add_remote_peers(mem_zone_ref_ptr node, mem_zone_ref_ptr addrs);
C_IMPORT int C_API_FUNC	node_set_bookmark(mem_zone_ref_ptr p2p_addr, mem_zone_ref_ptr state_node);

C_IMPORT int C_API_FUNC node_get_root_app(mem_zone_ref_ptr txh);

C_IMPORT int C_API_FUNC node_get_root_app_addr(mem_zone_ref_ptr addr);

C_IMPORT int C_API_FUNC	node_load_block_indexes(void);
C_IMPORT int C_API_FUNC	node_load_last_blks(void);
C_IMPORT int C_API_FUNC	node_del_btree_from_mempool(void);
C_IMPORT int C_API_FUNC node_del_txs_from_mempool(mem_zone_ref_ptr tx_list);

C_IMPORT int C_API_FUNC	store_wallet_tx(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC	store_wallet_txs(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC init_staking();

			 
C_IMPORT int C_API_FUNC app_init(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC app_start(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC app_loop(mem_zone_ref_ptr params);
C_IMPORT int C_API_FUNC app_stop(mem_zone_ref_ptr params);

C_IMPORT void C_API_FUNC tree_manager_init(size_t size, size_t nzones,unsigned int flags);



typedef int C_API_FUNC init_func();
typedef init_func *init_func_ptr;

#else

typedef int C_API_FUNC app_func(mem_zone_ref_ptr params);

typedef void C_API_FUNC	tree_manager_init_func(size_t size, size_t  nzones, unsigned int flags);

typedef int	C_API_FUNC load_script_func(const char *file,const char *name, mem_zone_ref_ptr script_vars,unsigned int opt);
typedef int	C_API_FUNC resolve_script_var_func(mem_zone_ref_ptr script_vars, mem_zone_ref_ptr script_proc, const char *var_path, unsigned int var_type, mem_zone_ref_ptr out_var, mem_zone_ref_ptr pout_var);
typedef int C_API_FUNC get_script_var_value_str_func(mem_zone_ref_ptr global_vars, const char *var_path, struct string *out, unsigned int radix);
typedef int C_API_FUNC get_script_var_value_ptr_func(mem_zone_ref_ptr global_vars, const char *var_path, mem_ptr *out);
typedef int C_API_FUNC execute_script_proc_func(mem_zone_ref_ptr global_vars, mem_zone_ref_ptr proc);

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

#endif

tpo_mod_file libbase_mod = { 0xDEF00FED };


void create_def()
{
#if 0
	#define HANDLE_TYPE(type) cat_cstring(&def,"#define "#type"_crc 0x"); strcat_uint(&def,NODE_HASH(#type)); cat_cstring(&def,"\n");

	struct string def = { PTR_NULL };

	HANDLE_TYPE(NODE_GFX_STR)
	HANDLE_TYPE(NODE_GFX_INT)
	HANDLE_TYPE(NODE_GFX_BOOL)
	HANDLE_TYPE(NODE_GFX_SIGNED_INT)
	HANDLE_TYPE(NODE_GFX_RECT)
	HANDLE_TYPE(NODE_GFX_DATA)
	HANDLE_TYPE(NODE_GFX_PTR)
	HANDLE_TYPE(NODE_GFX_4UC)
	HANDLE_TYPE(NODE_GFX_BINT)
	HANDLE_TYPE(NODE_GFX_NULL)
	HANDLE_TYPE(NODE_GFX_OBJECT)
	HANDLE_TYPE(NODE_GFX_SIGNED_BINT)
	HANDLE_TYPE(NODE_GFX_FLOAT)
	HANDLE_TYPE(NODE_GFX_SHORT)

	HANDLE_TYPE(NODE_GFX_SCENE)
	HANDLE_TYPE(NODE_GFX_TEXT)
	HANDLE_TYPE(NODE_GFX_IMAGE_OBJ)
	HANDLE_TYPE(NODE_GFX_CTRL_DATA_COLUMN)
	HANDLE_TYPE(NODE_GFX_CTRL_DATA_COLUMN_LIST)
	HANDLE_TYPE(NODE_GFX_STYLE)
	HANDLE_TYPE(NODE_GFX_CTRL)
	HANDLE_TYPE(NODE_GFX_CTRL_ITEM)
	HANDLE_TYPE(NODE_GFX_TEXT_LIST)
	HANDLE_TYPE(NODE_GFX_TEXT_LIST_ENTRY)
	HANDLE_TYPE(NODE_GFX_CTRL_ITEM_DATA)
	HANDLE_TYPE(NODE_GFX_CTRL_ITEM_LIST)
	HANDLE_TYPE(NODE_GFX_EVENT_LIST)
	HANDLE_TYPE(NODE_GFX_EVENT)
	HANDLE_TYPE(NODE_GFX_RECT_OBJ)
	HANDLE_TYPE(NODE_GFX_IMAGE)
	HANDLE_TYPE(NODE_GFX_IMAGE_LIST)

	HANDLE_TYPE(NODE_REQUEST)
	HANDLE_TYPE(NODE_MEM_AREA_LIST)
	HANDLE_TYPE(NODE_MEM_AREA)
	HANDLE_TYPE(NODE_MEM_AREA_DESC)
	HANDLE_TYPE(NODE_TREE_AREA_DESC)
	HANDLE_TYPE(NODE_TREE_NODE_DESC)

	HANDLE_TYPE(NODE_TASK_LIST)
	HANDLE_TYPE(NODE_TASK)
	HANDLE_TYPE(NODE_TASK_DATA)
	HANDLE_TYPE(NODE_SEMAPHORE)
	HANDLE_TYPE(NODE_MODULE_DEF)
	HANDLE_TYPE(NODE_MODULE_PROC)
	HANDLE_TYPE(NODE_MODULE_RPROC)
	HANDLE_TYPE(NODE_MODULE_RWPROC)
	HANDLE_TYPE(NODE_HTTP_REQUEST)
	HANDLE_TYPE(NODE_SERVICE)

	HANDLE_TYPE(NODE_SCRIPT_HANDLER_PROC)
	HANDLE_TYPE(NODE_SCRIPT_PROC)
	HANDLE_TYPE(NODE_SCRIPT_ACUM_PROC)
	HANDLE_TYPE(NODE_SCRIPT)
	HANDLE_TYPE(NODE_MSG_HANDLER)
	HANDLE_TYPE(NODE_MSG_HANDLER_LIST)
	HANDLE_TYPE(NODE_SCRIPT_PAGE_PROC)
	HANDLE_TYPE(NODE_SCRIPT_PAGE_PARAM)

	HANDLE_TYPE(NODE_JSON_ARRAY)

	HANDLE_TYPE(NODE_TYPE_POOL_LIST)
	HANDLE_TYPE(NODE_TYPE_POOL)
	HANDLE_TYPE(NODE_TYPE_POOL_TUPPLE_LIST)
	HANDLE_TYPE(NODE_POOL_JOB_LIST)
	HANDLE_TYPE(NODE_POOL_JOB)
	HANDLE_TYPE(NODE_CON_LIST)
	HANDLE_TYPE(NODE_CON)
	HANDLE_TYPE(NODE_LOG_PARAMS)
	HANDLE_TYPE(NODE_MD5_HASH)

	HANDLE_TYPE(NODE_BITCORE_NODE)
	HANDLE_TYPE(NODE_BITCORE_NODE_LIST)

	HANDLE_TYPE(NODE_BITCORE_MSG)
	HANDLE_TYPE(NODE_BITCORE_MSG_LIST)

	HANDLE_TYPE(NODE_BITCORE_ADDR)
	HANDLE_TYPE(NODE_BITCORE_ADDR_LIST)

	HANDLE_TYPE(NODE_BITCORE_PAYLOAD)

	HANDLE_TYPE(NODE_BITCORE_ADDRT)
	HANDLE_TYPE(NODE_BITCORE_VSTR)
	HANDLE_TYPE(NODE_BITCORE_VINT)

	HANDLE_TYPE(NODE_BITCORE_BLK_HDR)
	HANDLE_TYPE(NODE_BITCORE_BLK_HDR_LIST)

	HANDLE_TYPE(NODE_BITCORE_BLOCK_LIST)
	HANDLE_TYPE(NODE_BITCORE_BLOCK)

	HANDLE_TYPE(NODE_BITCORE_HASH)
	HANDLE_TYPE(NODE_BITCORE_BLOCK_HASH)
	HANDLE_TYPE(NODE_BITCORE_TX_HASH)
	HANDLE_TYPE(NODE_BITCORE_HASH_LIST)

	HANDLE_TYPE(NODE_BITCORE_WALLET_ADDR)
	HANDLE_TYPE(NODE_BITCORE_WALLET_ADDR_LIST)

	HANDLE_TYPE(NODE_BITCORE_TX)
	HANDLE_TYPE(NODE_BITCORE_TX_LIST)

	HANDLE_TYPE(NODE_BITCORE_TXIN)
	HANDLE_TYPE(NODE_BITCORE_VINLIST)

	HANDLE_TYPE(NODE_BITCORE_TXOUT)
	HANDLE_TYPE(NODE_BITCORE_VOUTLIST)

	HANDLE_TYPE(NODE_BITCORE_SCRIPT_OPCODE)
	HANDLE_TYPE(NODE_BITCORE_SCRIPT)
	HANDLE_TYPE(NODE_BITCORE_LOCATOR)
	HANDLE_TYPE(NODE_BITCORE_ECDSA_SIG)
	HANDLE_TYPE(NODE_NET_IP)

	HANDLE_TYPE(NODE_RT_SCENE)
	HANDLE_TYPE(NODE_RT_VEC3)
	HANDLE_TYPE(NODE_RT_VEC3_ARRAY)
	HANDLE_TYPE(NODE_RT_MAT3x3)
	HANDLE_TYPE(NODE_RT_CUBEMAP)
	HANDLE_TYPE(NODE_RT_SHADER_UNIFORM_LIST)
	HANDLE_TYPE(NODE_RT_MATERIAL)
	HANDLE_TYPE(NODE_RT_MATERIAL_LIST)
	HANDLE_TYPE(NODE_RT_BBOX)
	HANDLE_TYPE(NODE_RT_SPHERE)
	HANDLE_TYPE(NODE_RT_CUBE)
	HANDLE_TYPE(NODE_RT_PLANE)
	HANDLE_TYPE(NODE_RT_CYLINDER)

		put_file("type_hash.h", def.str, def.size);
#endif
}


int main(int argc, const char **argv)
{
	
	struct string			node_name = { PTR_NULL },data_dir={PTR_NULL};
	mem_zone_ref			nodeDef = { PTR_NULL }, params = { PTR_NULL }, script_vars = { PTR_NULL }, init_node_proc = { PTR_NULL };
	const_mem_ptr			*params_ptr;
	tpo_mod_file			*nodix_mod;
	
	int						done = 0,n,withGC;




	init_mem_system			();
	init_default_mem_area	(24 * 1024 * 1024, 128*1024);
	set_exe_path			();
	network_init			();

	create_def				();

	log_output("Nodix starting ...\n\n");

	if (daemonize("nodix") <= 0)
	{
		console_print("daemonize failed \n");
		return 0;
	}

	tpo_mod_init			(&libbase_mod);
	load_module				("modz/libbase.tpo", "libbase", &libbase_mod);

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
			
#ifndef _DEBUG
	load_script				 = (load_script_func_ptr)get_tpo_mod_exp_addr_name(&libbase_mod, "load_script", 0);
	resolve_script_var		 = (resolve_script_var_func_ptr)get_tpo_mod_exp_addr_name(&libbase_mod, "resolve_script_var", 0);
	get_script_var_value_str = (get_script_var_value_str_func_ptr)get_tpo_mod_exp_addr_name(&libbase_mod, "get_script_var_value_str", 0);;
	get_script_var_value_ptr = (get_script_var_value_ptr_func_ptr)get_tpo_mod_exp_addr_name(&libbase_mod, "get_script_var_value_ptr", 0);;
	execute_script_proc		 = (execute_script_proc_func_ptr)get_tpo_mod_exp_addr_name(&libbase_mod, "execute_script_proc", 0);;
	tree_manager_init		 = (tree_manager_init_func_ptr)get_tpo_mod_exp_addr_name(&libbase_mod, "tree_manager_init", 0);;
#endif

#ifdef _DEBUG
	set_dbg_ptr(init_protocol, init_blocks, node_init_self, node_load_block_indexes, make_genesis_block, node_load_last_blks, connect_peer_node, node_log_version_infos, queue_verack_message, queue_getaddr_message, queue_version_message, queue_ping_message, queue_pong_message, queue_inv_message, queue_getdata_message, node_is_next_block, node_check_chain, node_store_last_pos_hash, node_set_last_block, set_block_hash, add_money_supply, truncate_chain_to, sub_money_supply, node_check_block_index/* remove_stored_block*/, block_has_pow, set_next_check, node_check_block_index, init_staking);
	set_dbg_ptr2(node_add_block_header, accept_block, node_compute_pow_diff_after, store_block, node_init_service, node_get_script_modules, get_pow_reward, node_get_script_msg_handlers, node_get_mem_pool, node_del_txs_from_mempool, node_add_tx_to_mempool, store_wallet_tx, store_wallet_txs, queue_mempool_message, node_list_accounts, node_list_addrs, get_sess_account, node_has_service_module, queue_getblock_hdrs_message, node_get_root_app, node_get_root_app_addr, node_get_mempool_hashes, node_get_root_app_fee, node_get_apps, node_get_app, node_get_app_types_def, node_get_types_def, node_get_app_objs, node_add_tmp_file, check_tx_files, get_scene_list, get_nscene, node_del_btree_from_mempool, blog_get_ncats, blog_get_cats, blog_get_cat_posts, blog_get_cat, blog_get_post, blog_get_last_posts, node_broadcast_addr, node_add_remote_peers, node_set_bookmark, node_load_bookmark, blog_get_accounts, blog_get_msgs);
	set_pos_dbg_ptr	(init_pos, store_blk_staking, load_last_pos_blk, find_last_pos_block, compute_last_pos_diff, store_blk_tx_staking,stake_get_reward);
#endif

	if(withGC == 1 )
		tree_manager_init		(32 * 1024 * 1024, 128*1024, 0x10);
	else
		tree_manager_init		(32 * 1024 * 1024, 128 * 1024, 0x0);


	
	if (!load_script("nodix.node", "nodix.node", &script_vars, 3))
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

#ifndef _DEBUG
	app_init = (app_func_ptr)get_tpo_mod_exp_addr_name(nodix_mod, "app_init", 0);
	app_start = (app_func_ptr)get_tpo_mod_exp_addr_name(nodix_mod, "app_start", 0);
	app_loop = (app_func_ptr)get_tpo_mod_exp_addr_name(nodix_mod, "app_loop", 0);
	app_stop = (app_func_ptr)get_tpo_mod_exp_addr_name(nodix_mod, "app_stop", 0);
#endif




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