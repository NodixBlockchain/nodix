//copyright antoine bentue-ferrer 2016
#include <base/std_def.h>
#include <base/std_mem.h>
#include <base/mem_base.h>
#include <base/std_str.h>

#include <mem_stream.h>

#include <sha256.h>
#define FORWARD_CRYPTO
#include <crypto.h>
#include <strs.h>
#include <tree.h>
#include <fsio.h>
#include <parser.h>


#define BLOCK_API C_EXPORT
#include "block_api.h"

extern unsigned int	has_root_app;
extern btc_addr_t	root_app_addr;
extern hash_t		app_root_hash;
extern mem_zone_ref	apps;


C_IMPORT size_t			C_API_FUNC	compute_payload_size(mem_zone_ref_ptr payload_node);
C_IMPORT char*			C_API_FUNC	write_node(mem_zone_ref_const_ptr key, unsigned char *payload);
C_IMPORT size_t			C_API_FUNC	get_node_size(mem_zone_ref_ptr key);
C_IMPORT void			C_API_FUNC	serialize_children(mem_zone_ref_ptr node, unsigned char *payload);
C_IMPORT const unsigned char*	C_API_FUNC read_node(mem_zone_ref_ptr key, const unsigned char *payload, size_t len);
C_IMPORT size_t			C_API_FUNC init_node(mem_zone_ref_ptr key);



extern int obj_new(mem_zone_ref_ptr type, const char *objName, struct string *script, mem_zone_ref_ptr obj);
extern int add_app_tx(mem_zone_ref_ptr new_app, const char *app_name);

extern struct string get_next_script_var(const struct string *script, size_t *offset);
extern int get_script_file(struct string *script, mem_zone_ref_ptr file);
extern int	load_utxo(const char *txh, unsigned int oidx, uint64_t *amount, btc_addr_t addr, hash_t objh);

int blk_del_app_root()
{
	struct string	app_root_path = { 0 };

	make_string(&app_root_path, "apps");
	cat_cstring_p(&app_root_path, "root_app");

	del_file(app_root_path.str);

	set_root_app(PTR_NULL);

	free_string(&app_root_path);

	return 1;
}

int rm_hash_from_file_obj(const char *file_name, hash_t hash)
{
	unsigned char *buffer;
	size_t			len;
	if (get_file(file_name, &buffer, &len)>0)
	{
		size_t  cur = 0;
		while (cur < len)
		{
			if (!memcmp_c(&buffer[cur], hash, sizeof(hash_t)))
			{
				len -= 32;
				if (len > 0)
				{
					if (len > cur)
						memmove_c(&buffer[cur], &buffer[cur + 32], len - cur);

					put_file(file_name, buffer, len);
				}
				else
					del_file(file_name);
				break;
			}
			cur += 32;
		}
		free_c(buffer);
	}
	return 1;
}




int rm_hash_from_index(const char *file_name, hash_t hash)
{
	unsigned char *buffer;
	size_t			len;
	if (get_file(file_name, &buffer, &len)>0)
	{
		size_t  cur = 0;
		while (cur < len)
		{
			if (!memcmp_c(&buffer[cur + 4], hash, sizeof(hash_t)))
			{
				len -= 36;
				if (len > 0)
				{
					if (len>cur)
						memmove_c(&buffer[cur], &buffer[cur + 36], len - cur);

					put_file(file_name, buffer, len);
				}
				else
					del_file(file_name);
				break;
			}
			cur += 36;
		}
		free_c(buffer);
	}
	return 1;
}

void rm_hash_from_index_str(char *file_name, hash_t hash)
{

	unsigned char *buffer;
	size_t			len;
	if (get_file(file_name, &buffer, &len)>0)
	{
		size_t  cur = 0;
		while (cur < len)
		{
			unsigned char	sz = *((unsigned char *)(buffer + cur));
			size_t entry_len = (sz + 1 + sizeof(hash_t));

			if (!memcmp_c(&buffer[cur + sz + 1], hash, sizeof(hash_t)))
			{
				len -= entry_len;
				if (len > 0)
				{
					if (len>cur)
						memmove_c(&buffer[cur], &buffer[cur + entry_len], len - entry_len);

					put_file(file_name, buffer, len);
				}
				else
					del_file(file_name);

				break;
			}
			cur += entry_len;
		}
		free_c(buffer);
	}
	return;
}

void rm_hash_from_index_addr(char *file_name, hash_t hash)
{

	unsigned char *buffer;
	size_t			len;
	if (get_file(file_name, &buffer, &len)>0)
	{
		size_t  cur = 0;
		while (cur < len)
		{
			size_t entry_len = sizeof(btc_addr_t) + sizeof(hash_t);

			if (!memcmp_c(&buffer[cur + sizeof(btc_addr_t)], hash, sizeof(hash_t)))
			{
				len -= entry_len;
				if (len > 0)
				{
					if (len>cur)
						memmove_c(&buffer[cur], &buffer[cur + entry_len], len - entry_len);

					put_file(file_name, buffer, len);
				}
				else
					del_file(file_name);

				break;
			}
			cur += entry_len;
		}
		free_c(buffer);
	}
	return;
}

void rm_hash_from_obj_txfr(const char *app_name,unsigned int type_id, hash_t hash)
{
	char typeStr[16];
	struct string	idx_path = { PTR_NULL };
	unsigned char *buffer;
	size_t			len;

	uitoa_s(type_id, typeStr, 16, 16);

	make_string(&idx_path, "apps");
	cat_cstring_p(&idx_path, app_name);
	cat_cstring_p(&idx_path, "objs");
	cat_cstring_p(&idx_path, typeStr);
	cat_cstring(&idx_path, "_txfr.idx");

	if (get_file(idx_path.str, &buffer, &len)>0)
	{
		size_t  cur = 0;
		while (cur < len)
		{
			size_t entry_len = sizeof(hash_t) + sizeof(hash_t);

			if (!memcmp_c(&buffer[cur], hash, sizeof(hash_t)))
			{
				len -= entry_len;
				if (len > 0)
				{
					if (len>cur)
						memmove_c(&buffer[cur], &buffer[cur + entry_len], len - entry_len);

					put_file(idx_path.str, buffer, len);
				}
				else
					del_file(idx_path.str);

				break;
			}
			cur += entry_len;
		}
		free_c(buffer);
	}

	free_string(&idx_path);
	return;
}

int rm_file_from_index(const char *file_name, hash_t hash)
{
	unsigned char *buffer;
	size_t			len;
	if (get_file(file_name, &buffer, &len)>0)
	{
		size_t  cur = 0;
		while (cur < len)
		{
			if (!memcmp_c(buffer, hash, sizeof(hash_t)))
			{
				len -= 64;
				if (len > 0)
				{
					if (len > cur)
						memmove_c(&buffer[cur], &buffer[cur + 64], len - cur);

					put_file(file_name, buffer, len);
				}
				else
					del_file(file_name);
				break;
			}
			cur += 64;
		}
		free_c(buffer);
	}
	return 1;
}

int rm_child_obj(const char *app_name, const char *tchash, const char *key, hash_t ch)
{
	struct string obj_path = { 0 };

	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, app_name);
	cat_cstring_p(&obj_path, "objs");
	cat_cstring_p(&obj_path, tchash);
	cat_cstring(&obj_path, "_");
	cat_cstring(&obj_path, key);
	rm_hash_from_file_obj(obj_path.str, ch);
	free_string(&obj_path);

	return 1;
}


int rm_obj(const char *app_name, unsigned int type_id, hash_t ohash)
{
	char objHash[65];
	char buff[16];
	mem_zone_ref		obj = { PTR_NULL }, idxs = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr	idx = PTR_NULL;
	struct string		obj_path = { 0 };

	bin_2_hex(ohash, 32, objHash);


	uitoa_s(type_id, buff, 16, 16);

	if (load_obj(app_name, objHash, "obj", 0, &obj, PTR_NULL))
	{

		mem_zone_ref_ptr	key = PTR_NULL;
		for (tree_manager_get_first_child(&obj, &my_list, &key); ((key != NULL) && (key->zone != NULL)); tree_manager_get_next_child(&my_list, &key))
		{
			if ((tree_mamanger_get_node_type(key) == NODE_JSON_ARRAY) || (tree_mamanger_get_node_type(key) == NODE_PUBCHILDS_ARRAY))
			{
				const char *name = tree_mamanger_get_node_name(key);

				make_string(&obj_path, "apps");
				cat_cstring_p(&obj_path, app_name);
				cat_cstring_p(&obj_path, "objs");
				cat_cstring_p(&obj_path, objHash);
				cat_cstring(&obj_path, "_");
				cat_cstring(&obj_path, name);
				del_file(obj_path.str);
				free_string(&obj_path);
			}
		}
		release_zone_ref(&obj);
	}


	tree_manager_create_node("idxs", NODE_JSON_ARRAY, &idxs);
	get_app_type_idxs(app_name, type_id, &idxs);


	for (tree_manager_get_first_child(&idxs, &my_list, &idx); ((idx != NULL) && (idx->zone != NULL)); tree_manager_get_next_child(&my_list, &idx))
	{
		struct string idx_path = { 0 };
		const char *keyname;

		keyname = tree_mamanger_get_node_name(idx);

		make_string(&idx_path, "apps");
		cat_cstring_p(&idx_path, app_name);
		cat_cstring_p(&idx_path, "objs");
		cat_cstring_p(&idx_path, buff);
		cat_cstring(&idx_path, "_");
		cat_cstring(&idx_path, keyname);
		cat_cstring(&idx_path, ".idx");

		switch (tree_mamanger_get_node_type(idx))
		{
			case NODE_GFX_INT:
				rm_hash_from_index(idx_path.str, ohash);
			break;
			case NODE_BITCORE_VSTR:
				rm_hash_from_index_str(idx_path.str, ohash);
			break;
			case NODE_BITCORE_WALLET_ADDR:
				rm_hash_from_index_addr(idx_path.str, ohash);
			break;
			case NODE_BITCORE_PUBKEY:
				rm_hash_from_index_addr(idx_path.str, ohash);
			break;
		}
		free_string(&idx_path);
	}

	release_zone_ref(&idxs);


	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, app_name);
	cat_cstring_p(&obj_path, "objs");
	cat_cstring_p(&obj_path, buff);
	rm_hash_from_file_obj(obj_path.str, ohash);
	free_string(&obj_path);

	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, app_name);
	cat_cstring_p(&obj_path, "objs");
	cat_cstring_p(&obj_path, buff);
	cat_cstring(&obj_path, "_time.idx");
	rm_hash_from_file_obj(obj_path.str, ohash);
	free_string(&obj_path);

	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, app_name);
	cat_cstring_p(&obj_path, "objs");
	cat_cstring_p(&obj_path, buff);
	cat_cstring(&obj_path, "_addr.idx");
	rm_hash_from_index_addr(obj_path.str, ohash);
	free_string(&obj_path);

	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, app_name);
	cat_cstring_p(&obj_path, "objs");
	cat_cstring_p(&obj_path, objHash);
	del_file(obj_path.str);
	free_string(&obj_path);

	return 1;
}


int rm_type(const char *app_name, unsigned int type_id, const char *typeHash)
{
	struct string obj_path = { 0 };
	char buff[16];

	uitoa_s(type_id, buff, 16, 16);

	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, app_name);
	cat_cstring_p(&obj_path, "types");
	cat_cstring_p(&obj_path, typeHash);
	del_file(obj_path.str);
	free_string(&obj_path);

	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, app_name);
	cat_cstring_p(&obj_path, "objs");
	cat_cstring_p(&obj_path, buff);
	del_file(obj_path.str);
	free_string(&obj_path);

	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, app_name);
	cat_cstring_p(&obj_path, "objs");
	cat_cstring_p(&obj_path, buff);
	cat_cstring(&obj_path, "_time.idx");
	del_file(obj_path.str);
	free_string(&obj_path);

	return 1;
}


int rm_app_file(const char *app_name, mem_zone_ref_ptr file)
{
	hash_t hash;
	char fileHash[65];
	struct string file_path = { 0 };

	if (!tree_manager_get_child_value_hash(file, NODE_HASH("dataHash"), hash))return 0;

	bin_2_hex(hash, 32, fileHash);

	make_string(&file_path, "apps");
	cat_cstring_p(&file_path, app_name);
	cat_cstring_p(&file_path, "datas");
	cat_cstring_p(&file_path, fileHash);
	del_file(file_path.str);
	free_string(&file_path);


	make_string(&file_path, "apps");
	cat_cstring_p(&file_path, app_name);
	cat_cstring_p(&file_path, "datas");
	cat_cstring_p(&file_path, "index");

	rm_file_from_index(file_path.str, hash);

	free_string(&file_path);

	return 1;
}

int rm_app(const char *app_name)
{
	struct string app_path = { 0 };


	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, app_name);
	cat_cstring_p(&app_path, "types");
	rm_dir(app_path.str);
	free_string(&app_path);

	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, app_name);
	cat_cstring_p(&app_path, "objs");
	rm_dir(app_path.str);
	free_string(&app_path);

	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, app_name);
	cat_cstring_p(&app_path, "layouts");
	rm_dir(app_path.str);
	free_string(&app_path);

	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, app_name);
	cat_cstring_p(&app_path, "datas");
	rm_dir(app_path.str);
	free_string(&app_path);

	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, app_name);
	cat_cstring_p(&app_path, "modz");
	rm_dir(app_path.str);
	free_string(&app_path);

	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, app_name);
	rm_dir(app_path.str);
	free_string(&app_path);



	return 1;
}

OS_API_C_FUNC(int) blk_load_app_root()
{
	struct string	app_root_path = { 0 };
	unsigned char	*root_app_tx;
	size_t			len;

	make_string(&app_root_path, "apps");
	cat_cstring_p(&app_root_path, "root_app");

	if (get_file(app_root_path.str, &root_app_tx, &len)>0)
	{
		mem_zone_ref			apptx = { PTR_NULL };

		if (tree_manager_create_node("rootapp", NODE_BITCORE_TX, &apptx))
		{
			tree_manager_add_child_node(&apptx, "txsin", NODE_BITCORE_VINLIST, PTR_NULL);
			tree_manager_add_child_node(&apptx, "txsout", NODE_BITCORE_VOUTLIST, PTR_NULL);
			read_node(&apptx, root_app_tx, len);
			set_root_app(&apptx);



			release_zone_ref(&apptx);
		}
		free_c(root_app_tx);
	}
	free_string(&app_root_path);

	return 1;
}


int add_app_tx_type(mem_zone_ref_ptr app, mem_zone_ref_ptr typetx)
{
	mem_zone_ref txout_list = { PTR_NULL }, my_list = { PTR_NULL }, type_def = { PTR_NULL };
	struct string typeStr = { 0 }, typeName = { 0 }, typeId = { 0 }, Flags = { 0 };
	mem_zone_ref_ptr out = PTR_NULL;
	size_t offset = 0;
	unsigned int type_id;
	int ret = 0;

	if (!get_tx_output(typetx, 0, &type_def))return 0;

	if (!tree_manager_find_child_node(app, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list)) {
		release_zone_ref(&type_def);
		return 0;
	}

	tree_manager_get_child_value_istr(&type_def, NODE_HASH("script"), &typeStr, 0);

	typeName = get_next_script_var(&typeStr, &offset);
	typeId = get_next_script_var(&typeStr, &offset);


	free_string(&typeStr);

	if (typeId.len != 4)
	{
		free_string(&typeName);
		free_string(&typeId);
		release_zone_ref(&type_def);
		release_zone_ref(&txout_list);
		return 0;
	}


	type_id = *((unsigned int *)(typeId.str));

	tree_manager_set_child_value_vstr(typetx, "typeName", &typeName);
	tree_manager_set_child_value_i32(typetx, "typeId", type_id);



	for (tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); tree_manager_get_next_child(&my_list, &out))
	{
		unsigned int app_item;
		if (!tree_manager_get_child_value_i32(out, NODE_HASH("app_item"), &app_item))continue;
		if (app_item == 1)
		{
			mem_zone_ref types = { PTR_NULL };

			if (!tree_manager_find_child_node(out, NODE_HASH("types"), NODE_BITCORE_TX_LIST, &types))
				tree_manager_add_child_node(out, "types", NODE_BITCORE_TX_LIST, &types);

			tree_manager_node_add_child(&types, typetx);

			release_zone_ref(&types);

			dec_zone_ref(out);
			release_zone_ref(&my_list);
			ret = 1;
			break;
		}
	}

	free_string(&Flags);
	free_string(&typeName);
	free_string(&typeId);
	release_zone_ref(&txout_list);
	release_zone_ref(&type_def);
	return ret;
}


int blk_load_app_types(mem_zone_ref_ptr app)
{
	struct string types_path = { 0 }, dir_list = { 0 };
	size_t  cur, nfiles;
	const char	*name;

	name = tree_mamanger_get_node_name(app);

	make_string(&types_path, "apps");
	cat_cstring_p(&types_path, name);
	cat_cstring_p(&types_path, "types");


	nfiles = get_sub_files(types_path.str, &dir_list);
	if (nfiles > 0)
	{
		const char		*ptr, *optr;
		unsigned int	dir_list_len;

		dir_list_len = dir_list.len;
		optr = dir_list.str;
		cur = 0;
		while (cur < nfiles)
		{
			struct string	path = { 0 };
			size_t			sz, len;
			unsigned char	*buffer;

			ptr = memchr_c(optr, 10, dir_list_len);
			sz = mem_sub(optr, ptr);

			clone_string(&path, &types_path);
			cat_ncstring_p(&path, optr, sz);

			if (get_file(path.str, &buffer, &len)>0)
			{
				hash_t				txh;
				mem_zone_ref		new_type = { PTR_NULL };

				tree_manager_create_node("types", NODE_BITCORE_TX, &new_type);

				init_node(&new_type);
				read_node(&new_type, buffer, len);
				compute_tx_hash(&new_type, txh);
				tree_manager_set_child_value_hash(&new_type, "txid", txh);
				add_app_tx_type(app, &new_type);
				release_zone_ref(&new_type);
				free_c(buffer);
			}
			free_string(&path);

			cur++;
			optr = ptr + 1;
			dir_list_len -= sz;
		}
	}
	free_string(&dir_list);
	free_string(&types_path);
	return 1;
}


int add_app_script(mem_zone_ref_ptr app, mem_zone_ref_ptr script_vars)
{
	mem_zone_ref txout_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr out = PTR_NULL;

	int ret = 0;

	if (!tree_manager_find_child_node(app, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))return 0;


	for (tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); tree_manager_get_next_child(&my_list, &out))
	{
		unsigned int app_item;
		if (!tree_manager_get_child_value_i32(out, NODE_HASH("app_item"), &app_item))continue;
		if (app_item == 5)
		{
			mem_zone_ref scripts = { PTR_NULL };

			if (!tree_manager_find_child_node(out, NODE_HASH("scripts"), NODE_SCRIPT_LIST, &scripts))
				tree_manager_add_child_node(out, "scripts", NODE_SCRIPT_LIST, &scripts);

			tree_manager_node_add_child(&scripts, script_vars);

			release_zone_ref(&scripts);

			dec_zone_ref(out);
			release_zone_ref(&my_list);
			ret = 1;
			break;
		}
	}
	release_zone_ref(&txout_list);

	return ret;
}

int blk_load_app_scripts(mem_zone_ref_ptr app)
{
	struct string script_path = { 0 }, dir_list = { 0 }, appName = { 0 };
	size_t  cur, nfiles;


	tree_manager_get_child_value_istr(app, NODE_HASH("appName"), &appName, 0);

	make_string(&script_path, "apps");
	cat_cstring_p(&script_path, appName.str);
	cat_cstring_p(&script_path, "modz");


	nfiles = get_sub_files(script_path.str, &dir_list);
	if (nfiles > 0)
	{
		const char		*ptr, *optr;
		unsigned int	dir_list_len;

		dir_list_len = dir_list.len;
		optr = dir_list.str;
		cur = 0;
		while (cur < nfiles)
		{
			struct string	path = { 0 };
			size_t			sz;

			ptr = memchr_c(optr, 10, dir_list_len);
			sz = mem_sub(optr, ptr);

			if (sz < 5) {
				cur++;
				optr = ptr + 1;
				dir_list_len -= sz;
				continue;
			}

			if (!strncmp_c(&optr[sz - 5], ".site", 5))
			{
				mem_zone_ref script_var = { PTR_NULL };
				char		 script_name[32];

				strncpy_cs(script_name, 32, optr, sz);

				clone_string(&path, &script_path);
				cat_ncstring_p(&path, optr, sz);
				if (load_script(path.str, script_name, &script_var, 1))
				{
					ctime_t ftime;
					get_ftime(path.str, &ftime);
					tree_manager_write_node_dword(&script_var, 0, ftime);
					add_app_script(app, &script_var);
				}
				free_string(&path);
				release_zone_ref(&script_var);
			}
			cur++;
			optr = ptr + 1;
			dir_list_len -= sz;
		}
	}
	free_string(&dir_list);
	free_string(&script_path);
	free_string(&appName);
	return 1;
}

OS_API_C_FUNC(int) blk_load_apps(mem_zone_ref_ptr apps)
{
	struct string dir_list = { 0 };
	size_t cur, nfiles;

	nfiles = get_sub_dirs("apps", &dir_list);
	if (nfiles > 0)
	{
		const char		*ptr, *optr;
		unsigned int	dir_list_len;

		dir_list_len = dir_list.len;
		optr = dir_list.str;
		cur = 0;
		while (cur < nfiles)
		{
			char			app_name[128];
			struct string	path = { 0 };
			size_t			sz, len;
			unsigned char	*buffer;

			ptr = memchr_c(optr, 10, dir_list_len);
			sz = mem_sub(optr, ptr);

			strncpy_c(app_name, optr, sz);
			make_string(&path, "apps");
			cat_cstring_p(&path, app_name);
			cat_cstring_p(&path, "app");

			if (get_file(path.str, &buffer, &len)>0)
			{
				mem_zone_ref new_app = { PTR_NULL };

				if (tree_manager_create_node(app_name, NODE_BITCORE_TX, &new_app))
				{
					hash_t txh;
					mem_zone_ref txout_list = { PTR_NULL };

					init_node(&new_app);
					read_node(&new_app, buffer, len);
					compute_tx_hash(&new_app, txh);
					tree_manager_set_child_value_hash(&new_app, "txid", txh);


					add_app_tx(&new_app, app_name);
					blk_load_app_types(&new_app);

					if (is_trusted_app(app_name))
						blk_load_app_scripts(&new_app);

					release_zone_ref(&new_app);
				}
				free_c(buffer);
				free_string(&path);
			}
			cur++;
			optr = ptr + 1;
			dir_list_len -= sz;
		}
	}
	free_string(&dir_list);

	return 1;
}



int blk_store_app_root(mem_zone_ref_ptr tx)
{
	struct string	app_root_path = { 0 };
	unsigned char	*root_app_tx;
	size_t			len;

	make_string(&app_root_path, "apps");
	create_dir(app_root_path.str);
	cat_cstring_p(&app_root_path, "root_app");

	len = get_node_size(tx);
	root_app_tx = (unsigned char	*)malloc_c(len);

	write_node(tx, root_app_tx);
	put_file(app_root_path.str, root_app_tx, len);

	free_string(&app_root_path);

	return 1;
}

OS_API_C_FUNC(int) get_app_obj_hashes(const char *app_name, mem_zone_ref_ptr hash_list)
{
	struct string obj_path = { PTR_NULL }, dir_list = { PTR_NULL };
	size_t nfiles, cur;

	tree_remove_children(hash_list);

	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, app_name);
	cat_cstring_p(&obj_path, "objs");

	nfiles = get_sub_files(obj_path.str, &dir_list);
	if (nfiles > 0)
	{
		const char		*ptr, *optr;
		unsigned int	dir_list_len;

		dir_list_len = dir_list.len;
		optr = dir_list.str;
		cur = 0;
		while (cur < nfiles)
		{
			struct string	txp = { PTR_NULL };
			size_t			sz;

			ptr = memchr_c(optr, 10, dir_list_len);
			sz = mem_sub(optr, ptr);

			if (sz == 64)
			{
				mem_zone_ref	hashn = { PTR_NULL };

				if (tree_manager_add_child_node(hash_list, "hash", NODE_BITCORE_HASH, &hashn))
				{
					hash_t			hash;
					unsigned int	n = 0;
					while (n<32)
					{
						char    hex[3];
						hex[0] = optr[n * 2 + 0];
						hex[1] = optr[n * 2 + 1];
						hex[2] = 0;
						hash[n] = strtoul_c(hex, PTR_NULL, 16);
						n++;
					}
					tree_manager_write_node_hash(&hashn, 0, hash);
					release_zone_ref(&hashn);
				}
			}

			cur++;
			optr = ptr + 1;
			dir_list_len -= sz;
		}
	}

	free_string(&obj_path);
	free_string(&dir_list);
	return 1;
}
OS_API_C_FUNC(int) get_app_type_nobjs(const char *app_name, unsigned int type_id)
{
	char			typeStr[32];
	struct string	obj_path = { PTR_NULL };
	size_t			size;
	uitoa_s(type_id, typeStr, 32, 16);

	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, app_name);
	cat_cstring_p(&obj_path, "objs");
	cat_cstring_p(&obj_path, typeStr);

	size = file_size(obj_path.str) / 32;

	free_string(&obj_path);

	return size;
}

OS_API_C_FUNC(int) get_app_type_obj_hashes(const char *app_name, unsigned int type_id, size_t first, size_t max, mem_zone_ref_ptr hash_list)
{
	char			typeStr[32];
	struct string	obj_path = { PTR_NULL };
	unsigned char	*buffer;
	size_t			cur, len, nHashes;
	int				ret;

	uitoa_s(type_id, typeStr, 32, 16);

	tree_remove_children(hash_list);

	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, app_name);
	cat_cstring_p(&obj_path, "objs");
	cat_cstring_p(&obj_path, typeStr);

	ret = (get_file(obj_path.str, &buffer, &len) > 0) ? 1 : 0;

	if (ret)
	{
		cur = first * 32;
		nHashes = 0;

		while ((cur < len) && (nHashes<max))
		{
			mem_zone_ref	hashn = { PTR_NULL };

			if (tree_manager_add_child_node(hash_list, "hash", NODE_BITCORE_HASH, &hashn))
			{
				tree_manager_write_node_hash(&hashn, 0, &buffer[cur]);
				release_zone_ref(&hashn);
				nHashes++;
			}
			cur += 32;
		}
		free_c(buffer);
	}
	free_string(&obj_path);

	return ret;
}

OS_API_C_FUNC(int) get_app_type_last_objs_hashes(const char *app_name, unsigned int type_id, size_t first, size_t max, size_t *total, mem_zone_ref_ptr hash_list)
{
	char			typeStr[32];
	struct string	obj_path = { PTR_NULL };
	unsigned char	*buffer;
	size_t			cur, len, nHashes;
	int				ret;

	uitoa_s(type_id, typeStr, 32, 16);

	tree_remove_children(hash_list);

	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, app_name);
	cat_cstring_p(&obj_path, "objs");
	cat_cstring_p(&obj_path, typeStr);
	cat_cstring(&obj_path, "_time.idx");

	ret = (get_file(obj_path.str, &buffer, &len) > 0) ? 1 : 0;

	if (ret)
	{
		cur = first * 36;
		nHashes = 0;
		*total = len / 36;

		while ((cur < len) && (nHashes<max))
		{
			mem_zone_ref	hashn = { PTR_NULL };

			if (tree_manager_add_child_node(hash_list, "hash", NODE_BITCORE_HASH, &hashn))
			{
				tree_manager_write_node_hash(&hashn, 0, &buffer[cur + 4]);
				release_zone_ref(&hashn);
				nHashes++;
			}
			cur += 36;
		}
		free_c(buffer);
	}
	free_string(&obj_path);

	return ret;
}


OS_API_C_FUNC(int) load_obj_childs(const char *app_name, const char *objHash, const char *KeyName, size_t first, size_t max, unsigned int opts, size_t *count, mem_zone_ref_ptr objs)
{

	mem_zone_ref	app = { PTR_NULL };
	struct string	obj_path = { 0 };
	unsigned char	*buffer;
	size_t			len;
	int				ret = 0;

	if (!tree_manager_find_child_node(&apps, NODE_HASH(app_name), NODE_BITCORE_TX, &app))return 0;


	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, app_name);
	cat_cstring_p(&obj_path, "objs");
	cat_cstring_p(&obj_path, objHash);
	cat_cstring(&obj_path, "_");
	cat_cstring(&obj_path, KeyName);

	if (get_file(obj_path.str, &buffer, &len) > 0)
	{
		size_t cur = first * 32;
		size_t nums = 0;

		*count = len / 32;

		while ((cur < len) && (nums<max))
		{
			if (opts & 1)
			{
				char oh[65];
				btc_addr_t addr;
				mem_zone_ref newobj = { PTR_NULL };

				bin_2_hex(&buffer[cur], 32, oh);

				load_obj(app_name, oh, "obj", 0, &newobj, addr);
				tree_manager_set_child_value_btcaddr(&newobj, "objAddr", addr);


				tree_manager_node_add_child(objs, &newobj);
				release_zone_ref(&newobj);
			}
			else
			{
				mem_zone_ref hashn = { PTR_NULL };
				if (tree_manager_add_child_node(objs, "hash", NODE_BITCORE_HASH, &hashn))
				{
					tree_manager_write_node_hash(&hashn, 0, &buffer[cur]);
					release_zone_ref(&hashn);
					nums++;
				}
			}
			cur += 32;
		}
		free_c(buffer);
	}
	free_string(&obj_path);

	return 1;
}

OS_API_C_FUNC(int) load_obj_type(const char *app_name, const char *objHash, unsigned int *type_id, btc_addr_t objAddr)
{
	mem_zone_ref	app = { PTR_NULL };
	struct string	obj_path = { 0 };
	unsigned char	*tx_data;
	size_t			tx_len;
	int				ret = 0;

	if (!tree_manager_find_child_node(&apps, NODE_HASH(app_name), NODE_BITCORE_TX, &app))return 0;

	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, app_name);
	cat_cstring_p(&obj_path, "objs");
	cat_cstring_p(&obj_path, objHash);

	if (get_file(obj_path.str, &tx_data, &tx_len) > 0)
	{
		mem_zone_ref obj_tx = { PTR_NULL }, vout = { PTR_NULL };

		tree_manager_create_node("tx", NODE_BITCORE_TX, &obj_tx);
		init_node(&obj_tx);
		read_node(&obj_tx, tx_data, tx_len);
		free_c(tx_data);

		if (get_tx_output(&obj_tx, 0, &vout))
		{
			uint64_t	value;
			
			if (objAddr != PTR_NULL)
			{
				struct string		objStr = { PTR_NULL };
				tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &objStr, 0);
				get_out_script_address(&objStr, PTR_NULL, objAddr);
				free_string(&objStr);
			}

			tree_manager_get_child_value_i64(&vout, NODE_HASH("value"), &value);
			if ((value & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
			{
				*type_id = value & 0xFFFFFFFF;
				ret = 1;
			}
			release_zone_ref(&vout);
		}

		release_zone_ref(&obj_tx);
	}


	free_string(&obj_path);
	release_zone_ref(&app);

	return ret;
}

OS_API_C_FUNC(int) get_app_obj_addr(const char *app_name, unsigned int type_id, btc_addr_t objAddr, mem_zone_ref_ptr obj_list)
{
	char			typeStr[32];
	struct string	obj_path = { PTR_NULL };
	mem_zone_ref	app = { PTR_NULL }, types = { PTR_NULL }, type = { PTR_NULL };
	unsigned char	*buffer;
	size_t			cur, len;
	int				ret;

	tree_remove_children(obj_list);

	uitoa_s(type_id, typeStr, 32, 16);

	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, app_name);
	cat_cstring_p(&obj_path, "objs");
	cat_cstring_p(&obj_path, typeStr);

	ret = (get_file(obj_path.str, &buffer, &len) > 0) ? 1 : 0;
	free_string(&obj_path);
	if (!ret)return 0;

	if (!tree_manager_find_child_node(&apps, NODE_HASH(app_name), NODE_BITCORE_TX, &app))return 0;

	get_app_types(&app, &types);
	ret = tree_find_child_node_by_id_name(&types, NODE_BITCORE_TX, "typeId", type_id, &type);

	release_zone_ref(&app);
	release_zone_ref(&types);

	if (!ret)return 0;


	cur = 0;
	ret = 0;
	while (cur < len)
	{
		char			chash[65];
		unsigned char	*tx_data;
		size_t			tx_len;

		bin_2_hex(&buffer[cur], 32, chash);

		make_string(&obj_path, "apps");
		cat_cstring_p(&obj_path, app_name);
		cat_cstring_p(&obj_path, "objs");
		cat_cstring_p(&obj_path, chash);

		if (get_file(obj_path.str, &tx_data, &tx_len) > 0)
		{
			mem_zone_ref	obj_tx = { PTR_NULL }, vout = { PTR_NULL };

			tree_manager_create_node("tx", NODE_BITCORE_TX, &obj_tx);
			init_node(&obj_tx);
			read_node(&obj_tx, tx_data, tx_len);
			free_c(tx_data);

			if (get_tx_output(&obj_tx, 0, &vout))
			{
				btc_addr_t		myAddr;
				struct string	objStr = { PTR_NULL };

				tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &objStr, 0);
				get_out_script_address(&objStr, PTR_NULL, myAddr);
				release_zone_ref(&vout);

				if (!memcmp_c(objAddr, myAddr, sizeof(btc_addr_t)))
				{
					mem_zone_ref obj = { PTR_NULL };

					if (obj_new(&type, "myobj", &objStr, &obj))
					{
						tree_manager_node_add_child(obj_list, &obj);
						release_zone_ref(&obj);

						ret = 1;
					}
				}
				free_string(&objStr);
			}

			release_zone_ref(&obj_tx);
		}

		free_string(&obj_path);

		cur += 32;
	}
	free_c(buffer);
	release_zone_ref(&type);

	return ret;
}

OS_API_C_FUNC(int) load_obj(const char *app_name, const char *objHash, const char *objName, unsigned int opts, mem_zone_ref_ptr obj, btc_addr_t objAddr)
{
	mem_zone_ref	app = { PTR_NULL };
	struct string	obj_path = { 0 };
	unsigned char	*tx_data;
	size_t			tx_len;
	int				ret = 0;



	if (!tree_manager_find_child_node(&apps, NODE_HASH(app_name), NODE_BITCORE_TX, &app))return 0;

	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, app_name);
	cat_cstring_p(&obj_path, "objs");
	cat_cstring_p(&obj_path, objHash);

	if (get_file(obj_path.str, &tx_data, &tx_len)>0)
	{
		hash_t oh;
		mem_zone_ref obj_tx = { PTR_NULL }, vout = { PTR_NULL };
		unsigned int  time;
		tree_manager_create_node("tx", NODE_BITCORE_TX, &obj_tx);
		init_node(&obj_tx);
		read_node(&obj_tx, tx_data, tx_len);
		free_c(tx_data);

		if (get_tx_output(&obj_tx, 0, &vout))
		{
			uint64_t	value;
			unsigned int type_id;

			tree_manager_get_child_value_i32(&obj_tx, NODE_HASH("time"), &time);
			tree_manager_get_child_value_i64(&vout, NODE_HASH("value"), &value);

			if ((value & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
			{
				btc_addr_t			oaddr;
				struct string		pkey = { PTR_NULL };
				mem_zone_ref types = { PTR_NULL }, type = { PTR_NULL };

				type_id = value & 0xFFFFFFFF;

				get_app_types(&app, &types);

				if (tree_find_child_node_by_id_name(&types, NODE_BITCORE_TX, "typeId", type_id, &type))
				{
					struct string		objStr = { PTR_NULL }, objData = { 0 };
					mem_zone_ref		type_outs = { PTR_NULL }, my_list = { PTR_NULL };
					mem_zone_ref_ptr	key = PTR_NULL;

					tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &objStr, 0);

					ret = obj_new(&type, objName, &objStr, obj);
					release_zone_ref(&type);

					if (objAddr != PTR_NULL)
						get_out_script_address(&objStr, &pkey, objAddr);


					if (opts & 8)
					{
						hash_t txfr,oh;
						char   typeStr[16];

						uitoa_s(type_id, typeStr, 16, 16);
						hex_2_bin(objHash, oh, 32);



						if (find_obj_txfr(app_name, typeStr, oh, txfr))
						{
							hash_t bh;
							mem_zone_ref tx = { PTR_NULL };

							if (load_tx(&tx, bh, txfr))
							{
								mem_zone_ref outputs = { PTR_NULL }, addr = { PTR_NULL };


								tree_manager_create_node("outputs", NODE_BITCORE_HASH_LIST, &outputs);
								find_obj_tx(&tx, oh, PTR_NULL, &outputs);

								if (tree_manager_get_child_at(&outputs, 0, &addr))
								{
									
									if (tree_manager_get_node_btcaddr(&addr, 0, oaddr))
										tree_manager_set_child_value_btcaddr(obj, "curAddr", oaddr);

									release_zone_ref(&addr);
								}

								release_zone_ref(&outputs);
								release_zone_ref(&tx);
							}
						}
						else
						{
							get_out_script_address(&objStr, PTR_NULL, oaddr);
							tree_manager_set_child_value_btcaddr(obj, "curAddr", oaddr);
						}
					}

					free_string(&objStr);

					if (ret)
					{
						if (opts & 1)
						{
							for (tree_manager_get_first_child(obj, &my_list, &key); ((key != NULL) && (key->zone != NULL)); tree_manager_get_next_child(&my_list, &key))
							{
								unsigned int type;
								type = tree_mamanger_get_node_type(key);

								if ((type >> 24) == 0x1E)
								{
									char			chash[256];
									mem_zone_ref	subObj = { PTR_NULL };
									unsigned char	*hdata;

									hdata = tree_mamanger_get_node_data_ptr(key, 0);

									bin_2_hex(hdata, 32, chash);

									ret = load_obj(app_name, chash, "item", opts, &subObj, PTR_NULL);
									tree_manager_copy_children_ref(key, &subObj);
									release_zone_ref(&subObj);

								}
							}
						}

			
						if (opts & 4)
						{
							if (pkey.len == 33)
							{
								mem_zone_ref mkey = { PTR_NULL };

								if (tree_manager_add_child_node(obj, "objKey", NODE_BITCORE_PUBKEY, &mkey))
								{
									tree_manager_write_node_data(&mkey, pkey.str, 0, 33);
									release_zone_ref(&mkey);
								}
							}
						}

						free_string(&pkey);

						if (opts & 2)
						{
							for (tree_manager_get_first_child(obj, &my_list, &key); ((key != NULL) && (key->zone != NULL)); tree_manager_get_next_child(&my_list, &key))
							{
								unsigned int type;
								type = tree_mamanger_get_node_type(key);

								if ((type == NODE_JSON_ARRAY) || (type == NODE_PUBCHILDS_ARRAY))
								{
									const char *keyname = tree_mamanger_get_node_name(key);
									size_t count;

									load_obj_childs(app_name, objHash, keyname, 0, 10, 1, &count, key);
								}
							}
						}
					}
				}
				release_zone_ref(&types);
			}
			release_zone_ref(&vout);

			compute_tx_hash(&obj_tx, oh);
			tree_manager_set_child_value_hash(obj, "objHash", oh);
			tree_manager_set_child_value_i32(obj, "time", time);
		}

		release_zone_ref(&obj_tx);
	}

	free_string(&obj_path);
	release_zone_ref(&app);

	return ret;

}

void add_index(const char *app_name, const char *typeStr, const char *keyname, unsigned int val, const hash_t hash)
{
	char			buffer[36];
	struct string	idx_path = { 0 };
	unsigned char	*idx_buff;
	size_t			idx_len, cur;

	memcpy_c(buffer, &val, 4);
	memcpy_c(&buffer[4], hash, 32);


	make_string(&idx_path, "apps");
	cat_cstring_p(&idx_path, app_name);
	cat_cstring_p(&idx_path, "objs");
	cat_cstring_p(&idx_path, typeStr);
	cat_cstring(&idx_path, "_");
	cat_cstring(&idx_path, keyname);
	cat_cstring(&idx_path, ".idx");

	if (get_file(idx_path.str, &idx_buff, &idx_len) > 0)
	{
		cur = 0;
		while (cur < idx_len)
		{
			if (val >  *((unsigned int *)(idx_buff + cur)))break;
			cur += 36;
		}
		truncate_file(idx_path.str, cur, buffer, 36);

		if (idx_len > cur)
			append_file(idx_path.str, &idx_buff[cur], idx_len - cur);

		free_c(idx_buff);
	}
	else
		put_file(idx_path.str, buffer, 36);


	free_string(&idx_path);
}

void add_index_str(const char *app_name, const char *typeStr, const char *keyname, const struct string *val, const hash_t hash)
{
	char			newval[512];
	struct string	idx_path = { 0 };
	unsigned char	*idx_buff;
	size_t			idx_len, cur;

	newval[0] = val->len;
	strcpy_cs(&newval[1], 511, val->str);
	memcpy_c(&newval[1 + val->len], hash, sizeof(hash_t));


	make_string(&idx_path, "apps");
	cat_cstring_p(&idx_path, app_name);
	cat_cstring_p(&idx_path, "objs");
	cat_cstring_p(&idx_path, typeStr);
	cat_cstring(&idx_path, "_");
	cat_cstring(&idx_path, keyname);
	cat_cstring(&idx_path, ".idx");

	if (get_file(idx_path.str, &idx_buff, &idx_len) > 0)
	{
		cur = 0;
		while (cur < idx_len)
		{
			char			sval[256];
			unsigned char	sz = *((unsigned char *)(idx_buff + cur));

			strncpy_cs(sval, 256, (idx_buff + cur + 1), sz);

			if (strcmp_c(val->str, sval)<0)
				break;

			cur += sizeof(hash_t) + sz + 1;
		}

		truncate_file(idx_path.str, cur, newval, sizeof(hash_t) + val->len + 1);

		if (idx_len > cur)
			append_file(idx_path.str, &idx_buff[cur], idx_len - cur);

		free_c(idx_buff);
	}
	else
		put_file(idx_path.str, newval, sizeof(hash_t) + val->len + 1);


	free_string(&idx_path);
}

void add_index_addr(const char *app_name, const char *typeStr, const char *keyname, const btc_addr_t val, const hash_t hash)
{
	unsigned char	newval[128];
	struct string	idx_path = { 0 };
	unsigned char	*idx_buff;
	size_t			idx_len, cur;


	memcpy_c(newval, val, sizeof(btc_addr_t));
	memcpy_c(&newval[34], hash, sizeof(hash_t));


	make_string(&idx_path, "apps");
	cat_cstring_p(&idx_path, app_name);
	cat_cstring_p(&idx_path, "objs");
	cat_cstring_p(&idx_path, typeStr);
	cat_cstring(&idx_path, "_");
	cat_cstring(&idx_path, keyname);
	cat_cstring(&idx_path, ".idx");

	if (get_file(idx_path.str, &idx_buff, &idx_len) > 0)
	{
		cur = 0;
		while ((cur + sizeof(hash_t) + sizeof(btc_addr_t)) <= idx_len)
		{
			if (memcmp_c(idx_buff + cur, val, sizeof(btc_addr_t))<0)
				break;

			cur += sizeof(hash_t) + sizeof(btc_addr_t);
		}

		truncate_file(idx_path.str, cur, newval, sizeof(hash_t) + sizeof(btc_addr_t));

		if (idx_len > cur)
			append_file(idx_path.str, &idx_buff[cur], idx_len - cur);

		free_c(idx_buff);
	}
	else
		put_file(idx_path.str, newval, sizeof(hash_t) + sizeof(btc_addr_t));


	free_string(&idx_path);
}

void add_obj_txfr(const char *app_name, const char *typeStr, const hash_t objHash, const hash_t txHash)
{
	unsigned char	newval[128];
	struct string	idx_path = { 0 };
	unsigned char	*idx_buff;
	size_t			idx_len, cur;


	memcpy_c(newval, objHash, sizeof(hash_t));
	memcpy_c(&newval[32], txHash, sizeof(hash_t));


	make_string(&idx_path, "apps");
	cat_cstring_p(&idx_path, app_name);
	cat_cstring_p(&idx_path, "objs");
	cat_cstring_p(&idx_path, typeStr);
	cat_cstring(&idx_path, "_txfr.idx");

	if (get_file(idx_path.str, &idx_buff, &idx_len) > 0)
	{
		cur = 0;
		while ((cur + sizeof(hash_t) + sizeof(hash_t)) <= idx_len)
		{
			if (memcmp_c(idx_buff + cur, objHash, sizeof(hash_t))<=0)
				break;

			cur += sizeof(hash_t) + sizeof(hash_t);
		}

		truncate_file(idx_path.str, cur, newval, sizeof(hash_t) + sizeof(hash_t));

		if (idx_len > cur)
			append_file(idx_path.str, &idx_buff[cur], idx_len - cur);

		free_c(idx_buff);
	}
	else
		put_file(idx_path.str, newval, sizeof(hash_t) + sizeof(hash_t));


	free_string(&idx_path);
}


int find_index_str(const char *app_name, const char *typeStr, const char *keyname, const struct string *val, hash_t hash)
{
	struct string	idx_path = { 0 };
	unsigned char	*idx_buff;
	size_t			idx_len, cur;
	int ret = 0;

	make_string(&idx_path, "apps");
	cat_cstring_p(&idx_path, app_name);
	cat_cstring_p(&idx_path, "objs");
	cat_cstring_p(&idx_path, typeStr);
	cat_cstring(&idx_path, "_");
	cat_cstring(&idx_path, keyname);
	cat_cstring(&idx_path, ".idx");

	if (get_file(idx_path.str, &idx_buff, &idx_len) > 0)
	{
		cur = 0;
		while ((cur + 1) < idx_len)
		{
			char			sval[256];
			unsigned char	sz = *((unsigned char *)(idx_buff + cur));

			if ((cur + 1 + sz + sizeof(hash_t)) > idx_len) break;

			strncpy_cs(sval, 256, (idx_buff + cur + 1), sz);

			if (strcmp_c(val->str, sval) == 0) {
				memcpy_c(hash, (idx_buff + cur + 1 + sz), sizeof(hash_t));
				ret = 1;
				break;
			}
			cur += sizeof(hash_t) + sz + 1;
		}
		free_c(idx_buff);
	}
	free_string(&idx_path);
	return ret;
}


OS_API_C_FUNC(int) find_objs_by_addr(const char *app_name, const char *typeStr, const char *keyname, const btc_addr_t val, mem_zone_ref_ptr hash_list)
{
	struct string	idx_path = { 0 };
	unsigned char	*idx_buff;
	size_t			idx_len, cur;
	int ret = 0;

	make_string(&idx_path, "apps");
	cat_cstring_p(&idx_path, app_name);
	cat_cstring_p(&idx_path, "objs");
	cat_cstring_p(&idx_path, typeStr);
	cat_cstring(&idx_path, "_");
	cat_cstring(&idx_path, keyname);
	cat_cstring(&idx_path, ".idx");

	if (get_file(idx_path.str, &idx_buff, &idx_len) > 0)
	{
		cur = 0;
		while ((cur + sizeof(btc_addr_t) + sizeof(hash_t)) <= idx_len)
		{
			if (!memcmp_c(idx_buff + cur, val, sizeof(btc_addr_t))) {

				mem_zone_ref myhash = { PTR_NULL };

				tree_manager_add_child_node(hash_list, "hash", NODE_BITCORE_HASH, &myhash);
				tree_manager_write_node_hash(&myhash, 0, idx_buff + cur + sizeof(btc_addr_t));
				release_zone_ref(&myhash);
			}
			cur += sizeof(hash_t) + sizeof(btc_addr_t);
		}
		free_c(idx_buff);
	}
	free_string(&idx_path);
	return 1;
}

OS_API_C_FUNC(int) find_obj_txfr(const char *app_name, const char *typeStr,const hash_t objHash, hash_t txh)
{
	struct string	idx_path = { 0 };
	unsigned char	*idx_buff;
	size_t			idx_len, cur;
	int ret = 0;


	make_string(&idx_path, "apps");
	cat_cstring_p(&idx_path, app_name);
	cat_cstring_p(&idx_path, "objs");
	cat_cstring_p(&idx_path, typeStr);
	cat_cstring(&idx_path, "_txfr.idx");

	if (get_file(idx_path.str, &idx_buff, &idx_len) > 0)
	{
		cur = 0;
		while ((cur + sizeof(hash_t) + sizeof(hash_t)) <= idx_len)
		{
			if (!memcmp_c(idx_buff+cur, objHash, sizeof(hash_t))) {
				memcpy_c(txh, idx_buff + cur + sizeof(hash_t), sizeof(hash_t));
				ret = 1;
				break;
			}
			cur += sizeof(hash_t) + sizeof(hash_t);
		}
		free_c(idx_buff);
	}
	free_string(&idx_path);
	return ret;
}
OS_API_C_FUNC(int) list_obj_txfr(const char *app_name, const char *typeStr, const hash_t objHash,  mem_zone_ref_ptr txfrs)
{
	hash_t txh,bh;
	btc_addr_t dstAddr, srcAddr;
	mem_zone_ref inputs = { PTR_NULL }, outputs = { PTR_NULL }, oAddr = { PTR_NULL }, dAddr = { PTR_NULL };
	mem_zone_ref tx = { PTR_NULL }, txfr = { PTR_NULL };
	unsigned int tx_time;
	int ret;

	if (find_obj_txfr(app_name, typeStr, objHash, txh))
	{
		ret = load_tx(&tx, bh, txh);
		
		while (ret)
		{
		
			mem_zone_ref prev_tx = { PTR_NULL };

			if (ret)ret = tree_manager_get_child_value_i32(&tx, NODE_HASH("time"), &tx_time);

			if (ret)ret = tree_manager_create_node("outputs", NODE_BITCORE_WALLET_ADDR_LIST, &outputs);
			if (ret)ret = tree_manager_create_node("inputs", NODE_BITCORE_WALLET_ADDR_LIST, &inputs);
			if (ret)ret = find_obj_tx(&tx, objHash, &inputs, &outputs);

			if (tree_manager_get_child_at(&inputs, 0, &oAddr))
			{
				ret = tree_manager_get_node_btcaddr(&oAddr, 0, srcAddr);
				release_zone_ref(&oAddr);
			}
			release_zone_ref(&inputs);

			if (ret)
			{
				if (tree_manager_get_child_at(&outputs, 0, &dAddr))
				{
					ret = tree_manager_get_node_btcaddr(&dAddr, 0, dstAddr);
					release_zone_ref(&dAddr);
				}
			}
			
			release_zone_ref(&outputs);

			if (ret)ret = tree_manager_add_child_node(txfrs, "origin", NODE_GFX_OBJECT, &txfr);
			if (ret)
			{
				tree_manager_set_child_value_i32(&txfr, "time", tx_time);
				tree_manager_set_child_value_hash(&txfr, "txid", txh);
				tree_manager_set_child_value_btcaddr(&txfr, "srcaddr", srcAddr);
				tree_manager_set_child_value_btcaddr(&txfr, "dstaddr", dstAddr);
				release_zone_ref(&txfr);
			}

			ret = find_obj_ptxfr	(&tx, objHash, &prev_tx);
			copy_zone_ref			(&tx, &prev_tx);
			release_zone_ref		(&prev_tx);
		}

	}


	ret = load_tx(&tx, bh, objHash);
	if (ret)ret = tree_manager_get_child_value_i32(&tx, NODE_HASH("time"), &tx_time);
	if (ret)ret = tree_manager_create_node("outputs", NODE_BITCORE_WALLET_ADDR_LIST, &outputs);
	if (ret)ret = find_obj_tx(&tx, objHash, PTR_NULL, &outputs);
	if (ret)ret = tree_manager_get_child_at(&outputs, 0, &dAddr);
	if (ret)ret = tree_manager_get_node_btcaddr(&dAddr, 0, dstAddr);
	release_zone_ref(&dAddr);
	release_zone_ref(&outputs);

	if (ret)ret = tree_manager_add_child_node(txfrs, "origin", NODE_GFX_OBJECT, &txfr);
	if (ret)
	{
		tree_manager_set_child_value_i32(&txfr, "time", tx_time);
		tree_manager_set_child_value_hash(&txfr, "txid", objHash);
		tree_manager_set_child_value_btcaddr(&txfr, "dstaddr", dstAddr);
		
		release_zone_ref(&txfr);
	}

	


	return 1;

}

OS_API_C_FUNC(int) get_app_file(mem_zone_ref_ptr file_tx, struct string *app_name, mem_zone_ref_ptr file)
{
	hash_t			prev_hash;
	mem_zone_ref	input = { PTR_NULL }, app_tx = { PTR_NULL };
	unsigned int	oidx, ret = 0;
	unsigned char	app_item;


	if (get_tx_input(file_tx, 0, &input))
	{
		tree_manager_get_child_value_hash(&input, NODE_HASH("txid"), prev_hash);
		tree_manager_get_child_value_i32(&input, NODE_HASH("idx"), &oidx);

		if (tx_is_app_item(prev_hash, oidx, &app_tx, &app_item))
		{
			if (app_item == 3)
			{
				mem_zone_ref output = { PTR_NULL };

				if (get_tx_output(file_tx, 0, &output))
				{
					struct string script = { 0 };

					tree_manager_get_child_value_istr(&output, NODE_HASH("script"), &script, 0);

					if (get_script_file(&script, file))
					{
						ret = 1;
						tree_manager_get_child_value_istr(&app_tx, NODE_HASH("appName"), app_name, 0);
					}
					free_string(&script);
					release_zone_ref(&output);
				}
			}
			release_zone_ref(&app_tx);
		}
		release_zone_ref(&input);
	}
	return ret;
}


OS_API_C_FUNC(int) has_app_file(struct string *app_name, hash_t fileHash)
{
	char chash[65];
	struct string app_path = { 0 };
	unsigned int n = 0;
	int ret;


	bin_2_hex(fileHash, 32, chash);

	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, app_name->str);
	cat_cstring_p(&app_path, "datas");
	cat_cstring_p(&app_path, chash);

	ret = (stat_file(app_path.str) == 0) ? 1 : 0;

	free_string(&app_path);

	return ret;
}
OS_API_C_FUNC(int) get_appfile_tx(const char *app_name, hash_t fileHash, hash_t txHash)
{
	struct string app_path = { 0 };
	unsigned char *buffer;
	size_t len;
	int ret = 0;

	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, app_name);
	cat_cstring_p(&app_path, "datas");
	cat_cstring_p(&app_path, "index");

	if (get_file(app_path.str, &buffer, &len)>0)
	{
		size_t cur = 0;

		while (cur < len)
		{
			if (!memcmp_c(buffer + cur, fileHash, sizeof(hash_t)))
			{
				memcpy_c(txHash, buffer + cur + 32, sizeof(hash_t));
				ret = 1;
				break;
			}
			cur += 64;
		}
		free_c(buffer);
	}
	free_string(&app_path);
	return ret;
}

OS_API_C_FUNC(int) get_app_files(struct string *app_name, size_t first, size_t num, mem_zone_ref_ptr files)
{
	struct string app_path = { 0 };
	unsigned char *buffer;
	size_t len, nh, total;
	int ret = 0;

	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, app_name->str);
	cat_cstring_p(&app_path, "datas");
	cat_cstring_p(&app_path, "index");

	total = 0;
	nh = 0;

	if (get_file(app_path.str, &buffer, &len) > 0)
	{
		size_t cur = first * 64;

		total = len / 64;

		while ((cur < len) && (nh  <num))
		{
			mem_zone_ref newh = { PTR_NULL };

			if (tree_manager_add_child_node(files, "file", NODE_BITCORE_HASH, &newh))
			{
				tree_manager_write_node_hash(&newh, 0, buffer + cur + 32);
				release_zone_ref(&newh);
				nh++;
			}
			cur += 64;
		}
		free_c(buffer);
	}
	free_string(&app_path);

	return total;
}

OS_API_C_FUNC(int) get_app_missing_files(struct string *app_name, mem_zone_ref_ptr pending, mem_zone_ref_ptr files)
{
	struct string app_path = { 0 };
	unsigned char *buffer;
	size_t len;
	int ret = 0;

	if (!is_trusted_app(app_name->str))return 0;

	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, app_name->str);
	cat_cstring_p(&app_path, "datas");
	cat_cstring_p(&app_path, "index");

	if (get_file(app_path.str, &buffer, &len) > 0)
	{
		size_t cur = 0;

		while (cur < len)
		{
			char fHAsh[65];
			struct string filePath = { 0 };


			if (!tree_find_child_node_by_member_name_hash(pending, NODE_GFX_OBJECT, "hash", buffer + cur + 32, PTR_NULL))
			{
				bin_2_hex(&buffer[cur], 32, fHAsh);

				make_string(&filePath, "apps");
				cat_cstring_p(&filePath, app_name->str);
				cat_cstring_p(&filePath, "datas");
				cat_cstring_p(&filePath, fHAsh);

				if (stat_file(filePath.str) != 0)
				{
					mem_zone_ref newh = { PTR_NULL };
					if (tree_manager_add_child_node(files, "file", NODE_FILE_HASH, &newh))
					{
						tree_manager_write_node_hash(&newh, 0, buffer + cur + 32);
						release_zone_ref(&newh);
					}
				}
				free_string(&filePath);
			}
			cur += 64;
		}
		free_c(buffer);
	}
	free_string(&app_path);

	return 1;
}

int clear_app_index(const char *appName)
{
	struct string	app_path = { 0 };
	mem_zone_ref	app = { PTR_NULL }, appTypes = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr type;

	if (!tree_manager_find_child_node(&apps, NODE_HASH(appName), NODE_BITCORE_TX, &app))
		return 0;

	get_app_types(&app, &appTypes);

	for (tree_manager_get_first_child(&appTypes, &my_list, &type); ((type != NULL) && (type->zone != NULL)); tree_manager_get_next_child(&my_list, &type))
	{
		char			KeyName[32];
		char			typeStr[16];
		struct string	KeyStr = { 0 };
		unsigned int	KeyId, flags;
		mem_zone_ref typeOut = { PTR_NULL };

		if (!get_tx_output(type, 1, &typeOut))
			continue;

		tree_manager_get_child_value_istr(&typeOut, NODE_HASH("script"), &KeyStr, 0);

		if (get_type_infos(&KeyStr, KeyName, &KeyId, &flags))
		{
			mem_zone_ref		m_idlist = { 0 };
			mem_zone_ref_ptr	idx = PTR_NULL;
			mem_zone_ref idxs = { PTR_NULL };

			uitoa_s(KeyId, typeStr, 16, 16);

			make_string(&app_path, "apps");
			cat_cstring_p(&app_path, appName);
			cat_cstring_p(&app_path, "objs");
			cat_cstring_p(&app_path, typeStr);
			cat_cstring_p(&app_path, "_time.idx");
			del_file(app_path.str);
			free_string(&app_path);

			make_string(&app_path, "apps");
			cat_cstring_p(&app_path, appName);
			cat_cstring_p(&app_path, "objs");
			cat_cstring_p(&app_path, typeStr);
			cat_cstring_p(&app_path, "_addr.idx");
			del_file(app_path.str);
			free_string(&app_path);

			tree_manager_create_node("idxs", NODE_JSON_ARRAY, &idxs);
			
			
			get_app_type_idxs(appName, KeyId, &idxs);

			for (tree_manager_get_first_child(&idxs, &m_idlist, &idx); ((idx != NULL) && (idx->zone != NULL)); tree_manager_get_next_child(&m_idlist, &idx))
			{
				char keyStr[16];
				struct string idx_path = { 0 };
				const char *keyname = tree_mamanger_get_node_name(idx);
				unsigned int ktype = tree_mamanger_get_node_type(idx);

				uitoa_s(ktype, keyStr, 16, 16);

				make_string(&idx_path, "apps");
				cat_cstring_p(&idx_path, appName);
				cat_cstring_p(&idx_path, "objs");
				cat_cstring_p(&idx_path, keyStr);
				cat_cstring(&idx_path, "_");
				cat_cstring(&idx_path, keyname);
				cat_cstring(&idx_path, ".idx");
				del_file(idx_path.str);
				free_string(&idx_path);
			}
			release_zone_ref(&idxs);
			
			make_string(&app_path, "apps");
			cat_cstring_p(&app_path, appName);
			cat_cstring_p(&app_path, "objs");
			cat_cstring_p(&app_path, typeStr);
			del_file(app_path.str);
			free_string(&app_path);

		}

		free_string(&KeyStr);
		release_zone_ref(&typeOut);
		

	}

	release_zone_ref(&appTypes);

	return 0;

}
int add_obj_index(const char *appName,unsigned int typeID, mem_zone_ref_const_ptr obj,unsigned int tx_time,const hash_t tx_hash)
{
	char			typeStr[16];
	char			tchash[65];
	btc_addr_t		objAddr;
	mem_zone_ref	idxs = { 0 };
	struct string	app_path = { 0 };

	uitoa_s(typeID, typeStr, 16, 16);

	bin_2_hex(tx_hash, 32, tchash);


	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, appName);
	cat_cstring_p(&app_path, "objs");
	cat_cstring_p(&app_path, typeStr);
	append_file(app_path.str, tx_hash, 32);
	free_string(&app_path);

	tree_manager_get_child_value_btcaddr(obj, NODE_HASH("objAddr"), objAddr);

	add_index(appName, typeStr, "time", tx_time, tx_hash);
	add_index_addr(appName, typeStr, "addr", objAddr, tx_hash);


	if (tree_manager_create_node("idxs", NODE_JSON_ARRAY, &idxs))
	{
		mem_zone_ref		m_idlist = { 0 };
		mem_zone_ref_ptr	idx = PTR_NULL;

		get_app_type_idxs(appName, typeID, &idxs);

		for (tree_manager_get_first_child(&idxs, &m_idlist, &idx); ((idx != NULL) && (idx->zone != NULL)); tree_manager_get_next_child(&m_idlist, &idx))
		{
			const char *id_name;
			id_name = tree_mamanger_get_node_name(idx);
			switch (tree_mamanger_get_node_type(idx))
			{
			case NODE_GFX_INT:
			{
				unsigned int val;
				tree_manager_get_child_value_i32(obj, NODE_HASH(id_name), &val);
				add_index(appName, typeStr, id_name, val, tx_hash);
			}
			break;
			case NODE_BITCORE_VSTR:
			{
				struct string val = { 0 };
				tree_manager_get_child_value_istr(obj, NODE_HASH(id_name), &val, 0);
				add_index_str(appName, typeStr, id_name, &val, tx_hash);
			}
			break;
			case NODE_BITCORE_WALLET_ADDR:
			{
				btc_addr_t addr;
				tree_manager_get_child_value_btcaddr(obj, NODE_HASH(id_name), addr);
				add_index_addr(appName, typeStr, id_name, addr, tx_hash);
			}
			break;
			case NODE_BITCORE_PUBKEY:
			{
				btc_addr_t		addr;
				unsigned char	*pubkey;
				unsigned int	sz;

				sz = tree_manager_get_child_data_ptr(obj, NODE_HASH(id_name), &pubkey);
				key_to_addr(pubkey, addr);
				add_index_addr(appName, typeStr, id_name, addr, tx_hash);
			}
			break;
			}
		}
		release_zone_ref(&idxs);
	}

	return 1;
}


int rebuild_app_index(const char *appName)
{
	struct string obj_path = { 0 }, objList = { 0 };
	mem_zone_ref	app = { PTR_NULL }, appTypes = { PTR_NULL }, my_list = { PTR_NULL };
	size_t  cur, nfiles;

	if (!clear_app_index(appName))
		return 0;

	if (!tree_manager_find_child_node(&apps, NODE_HASH(appName), NODE_BITCORE_TX, &app))return 0;

	get_app_types(&app, &appTypes);

	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, appName);
	cat_cstring_p(&obj_path, "objs");

	nfiles = get_sub_files(obj_path.str, &objList);
	if (nfiles > 0)
	{
		const char		*ptr, *optr;
		unsigned int	dir_list_len;

		dir_list_len = objList.len;
		optr = objList.str;
		cur = 0;
		while (cur < nfiles)
		{
			struct string	path = { 0 };
			size_t			sz, tx_len;
			unsigned char	*tx_data;

			ptr = memchr_c(optr, 10, dir_list_len);
			sz = mem_sub(optr, ptr);



			if (get_file(obj_path.str, &tx_data, &tx_len) > 0)
			{

				mem_zone_ref obj_tx = { PTR_NULL }, myobj = { PTR_NULL }, vout = { PTR_NULL };
				unsigned int  time;

				tree_manager_create_node("tx", NODE_BITCORE_TX, &obj_tx);
				init_node(&obj_tx);
				read_node(&obj_tx, tx_data, tx_len);
				free_c(tx_data);



				if (get_tx_output(&obj_tx, 0, &vout))
				{
					hash_t txh;
					uint64_t	value;

					compute_tx_hash(&obj_tx, txh);
					tree_manager_set_child_value_hash(&obj_tx, "txid", txh);

					tree_manager_get_child_value_i32(&obj_tx, NODE_HASH("time"), &time);
					tree_manager_get_child_value_i64(&vout, NODE_HASH("value"), &value);

					if ((value & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
					{
						struct string script = { PTR_NULL };
						mem_zone_ref type = { PTR_NULL };
						unsigned int type_id;

						type_id = value & 0xFFFFFFFF;

						if (tree_find_child_node_by_id_name(&appTypes, NODE_BITCORE_TX, "typeId", type_id, &type))
						{
							tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &script, 0);
							obj_new(&type, "objDef", &script, &myobj);
							add_obj_index(appName, type_id, &myobj, time, txh);

							free_string(&script);
							release_zone_ref(&type);
							release_zone_ref(&myobj);
						}
					}
					release_zone_ref(&vout);
				}
				release_zone_ref(&obj_tx);
				free_c(tx_data);
			}

			cur++;
			optr = ptr + 1;
			dir_list_len -= sz;
		}
	}
}


int store_app_item(mem_zone_ref_const_ptr tx, unsigned int app_item, unsigned int tx_time, mem_ptr buffer, size_t length)
{
	hash_t tx_hash;
	struct string	app_name = { 0 };
	mem_zone_ref	app = { PTR_NULL };
	int ret = 1;

	tree_manager_get_child_value_hash(tx, NODE_HASH("txid"), tx_hash);

	switch (app_item)
	{
	case 1:
		if (tree_manager_get_child_value_istr(tx, NODE_HASH("appType"), &app_name, 0))
		{
			char			tchash[65];
			struct string	app_path = { 0 };


			bin_2_hex(tx_hash, 32, tchash);

			make_string(&app_path, "apps");
			cat_cstring_p(&app_path, app_name.str);
			cat_cstring_p(&app_path, "types");
			cat_cstring_p(&app_path, tchash);
			put_file(app_path.str, buffer, length);
			free_string(&app_path);

			if (tree_node_find_child_by_name(&apps, app_name.str, &app))
			{
				add_app_tx_type(&app, tx);
				release_zone_ref(&app);
			}
		}
		free_string(&app_name);
		break;
		case 2:
			if (tree_manager_get_child_value_istr(tx, NODE_HASH("appObj"), &app_name, 0))
			{
				mem_zone_ref	obj = { 0 };
				unsigned int	typeID;

				if (!tree_manager_get_child_value_i32(tx, NODE_HASH("objType"), &typeID))
					typeID = 0;

				if (tree_manager_find_child_node(tx, NODE_HASH("objDef"), typeID, &obj))
				{
					char			tchash[65];
					struct string	app_path = { 0 };

					bin_2_hex(tx_hash, 32, tchash);

					make_string(&app_path, "apps");
					cat_cstring_p(&app_path, app_name.str);
					cat_cstring_p(&app_path, "objs");
					cat_cstring_p(&app_path, tchash);
					put_file(app_path.str, buffer, length);
					free_string(&app_path);

					add_obj_index(app_name.str, typeID, &obj, tx_time, tx_hash);
					release_zone_ref(&obj);
				}
			}
			free_string(&app_name);
		break;
		case 3:
			if (tree_manager_get_child_value_istr(tx, NODE_HASH("appFile"), &app_name, 0))
			{
				mem_zone_ref file = { PTR_NULL };

				if (tree_manager_find_child_node(tx, NODE_HASH("fileDef"), NODE_GFX_OBJECT, &file))
				{
					unsigned char	buffer[64];
					if (tree_manager_get_child_value_hash(&file, NODE_HASH("dataHash"), buffer))
					{
						struct string	app_path = { 0 };

						memcpy_c(&buffer[32], tx_hash, sizeof(hash_t));
						make_string(&app_path, "apps");
						cat_cstring_p(&app_path, app_name.str);
						cat_cstring_p(&app_path, "datas");
						cat_cstring_p(&app_path, "index");

						if (stat_file(app_path.str) == 0)
							append_file(app_path.str, buffer, 64);
						else
							put_file(app_path.str, buffer, 64);

						free_string(&app_path);
					}
					release_zone_ref(&file);
				}
			}
		break;
	case 4:
		if (tree_manager_get_child_value_istr(tx, NODE_HASH("appLayout"), &app_name, 0))
		{
			struct string	fileData = { 0 }, filename = { 0 };
			mem_zone_ref	file = { PTR_NULL };

			ret = tree_manager_find_child_node(tx, NODE_HASH("layoutDef"), NODE_GFX_OBJECT, &file);
			if (ret)ret = tree_manager_get_child_value_istr(&file, NODE_HASH("filedata"), &fileData, 0);
			if (ret)ret = tree_manager_get_child_value_istr(&file, NODE_HASH("filename"), &filename, 0);
			if (ret)
			{
				struct string	app_path = { 0 };

				make_string(&app_path, "apps");
				cat_cstring_p(&app_path, app_name.str);
				cat_cstring_p(&app_path, "layouts");
				cat_cstring_p(&app_path, filename.str);
				put_file(app_path.str, fileData.str, fileData.len);
				free_string(&app_path);
			}
			release_zone_ref(&file);

		}
		break;
	case 5:
		if (tree_manager_get_child_value_istr(tx, NODE_HASH("appModule"), &app_name, 0))
		{
			struct string	fileData = { 0 }, filename = { 0 };
			mem_zone_ref	file = { PTR_NULL };

			ret = tree_manager_find_child_node(tx, NODE_HASH("moduleDef"), NODE_GFX_OBJECT, &file);
			if (ret)ret = tree_manager_get_child_value_istr(&file, NODE_HASH("filedata"), &fileData, 0);
			if (ret)ret = tree_manager_get_child_value_istr(&file, NODE_HASH("filename"), &filename, 0);
			if (ret)
			{
				struct string	app_path = { 0 };

				make_string(&app_path, "apps");
				cat_cstring_p(&app_path, app_name.str);
				cat_cstring_p(&app_path, "modz");
				cat_cstring_p(&app_path, filename.str);
				put_file(app_path.str, fileData.str, fileData.len);


				tree_manager_find_child_node(&apps, NODE_HASH(app_name.str), NODE_BITCORE_TX, &app);

				if ((filename.len >= 5) && (!strncmp_c(&filename.str[filename.len - 5], ".site", 5)))
				{
					mem_zone_ref scripts = { PTR_NULL }, script = { PTR_NULL };

					get_app_scripts(&app, &scripts);
					tree_remove_child_by_name(&scripts, NODE_HASH(filename.str));
					release_zone_ref(&scripts);

					if (load_script(app_path.str, filename.str, &script, 1))
					{
						ctime_t ftime;

						get_ftime(app_path.str, &ftime);
						tree_manager_write_node_dword(&script, 0, ftime);
						add_app_script(&app, &script);
						release_zone_ref(&script);
					}
				}
				release_zone_ref(&app);
				free_string(&app_path);
			}
			release_zone_ref(&file);
		}
		break;
	}

	return ret;
}

int store_new_app(const struct string *app_name, mem_zone_ref_const_ptr tx, mem_ptr buffer, size_t length)
{
	mem_zone_ref	app_tx = { PTR_NULL };
	struct string	app_path = { 0 };

	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, app_name->str);
	create_dir(app_path.str);

	cat_cstring_p(&app_path, "app");
	put_file(app_path.str, buffer, length);
	free_string(&app_path);

	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, app_name->str);
	cat_cstring_p(&app_path, "types");
	create_dir(app_path.str);
	free_string(&app_path);

	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, app_name->str);
	cat_cstring_p(&app_path, "objs");
	create_dir(app_path.str);
	free_string(&app_path);

	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, app_name->str);
	cat_cstring_p(&app_path, "layouts");
	create_dir(app_path.str);
	free_string(&app_path);

	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, app_name->str);
	cat_cstring_p(&app_path, "datas");
	create_dir(app_path.str);
	free_string(&app_path);

	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, app_name->str);
	cat_cstring_p(&app_path, "modz");
	create_dir(app_path.str);
	free_string(&app_path);

	tree_manager_create_node(app_name->str, NODE_BITCORE_TX, &app_tx);
	tree_manager_copy_children_ref(&app_tx, tx);
	add_app_tx(&app_tx, app_name->str);
	release_zone_ref(&app_tx);

	return 1;
}
int store_app_obj_txfr(mem_zone_ref_const_ptr tx)
{
	hash_t				txh;
	
	mem_zone_ref		txin_list = { PTR_NULL }, my_ilist = { PTR_NULL };
	mem_zone_ref_ptr	output=PTR_NULL,input = PTR_NULL;

	
	tree_manager_get_child_value_hash(tx, NODE_HASH("txid"), txh);

	if (!tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))
		return 0;

	for (tree_manager_get_first_child(&txin_list, &my_ilist, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_ilist, &input))
	{
		char			appName[32];
		hash_t			th, objHash;
		char			buff[32];
		btc_addr_t		srcAddr;
		struct string	idx_path = { 0 };
		uint64_t		amount;
		unsigned int	idx,oc;

		if (!tree_manager_get_child_value_i32(input, NODE_HASH("isObjChild"), &oc))
			continue;

		if (!tree_manager_get_child_value_str(input, NODE_HASH("srcapp"), appName, 32, 0))
			continue;

		tree_manager_get_child_value_hash(input, NODE_HASH("txid"), th);
		tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &idx);

		load_tx_output_amount(th, idx, &amount);

		if ((amount & 0xFFFFFFFF00000000) != 0xFFFFFFFF00000000)
			continue;

		if (!tree_manager_get_child_value_hash(input, NODE_HASH("objHash"), objHash))
			continue;

		uitoa_s					(amount & 0xFFFFFFFF, buff, 32, 16);

		make_string(&idx_path, "apps");
		cat_cstring_p(&idx_path, appName);
		cat_cstring_p(&idx_path, "objs");
		cat_cstring_p(&idx_path, buff);
		cat_cstring(&idx_path, "_addr.idx");

		rm_hash_from_index_addr(idx_path.str, objHash);

		free_string(&idx_path);
	}
	release_zone_ref(&txin_list);


	if (!tree_manager_find_child_node(tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txin_list))
		return 0;

	for (tree_manager_get_first_child(&txin_list, &my_ilist, &output); ((output != PTR_NULL) && (output->zone != PTR_NULL)); tree_manager_get_next_child(&my_ilist, &output))
	{
		char			appName[32];
		hash_t			objHash;
		char			buff[32];
		btc_addr_t		dstAddr;
		uint64_t		amount;

		if (!tree_manager_get_child_value_i64(output, NODE_HASH("value"), &amount))
			continue;

		if ((amount & 0xFFFFFFFF00000000) != 0xFFFFFFFF00000000)
			continue;

		if (!tree_manager_get_child_value_hash(output, NODE_HASH("objHash"), objHash))
			continue;

		if (!tree_manager_get_child_value_btcaddr(output, NODE_HASH("dstAddr"), dstAddr))
			continue;

		if (!get_obj_app(objHash, appName))
			continue;

		uitoa_s(amount & 0xFFFFFFFF, buff, 32, 16);

		add_index_addr(appName, buff, "addr", dstAddr, objHash);
		add_obj_txfr  (appName, buff, objHash, txh);

	}
	release_zone_ref(&txin_list);

	return 1;

}

int store_app_obj_child(mem_zone_ref_const_ptr tx, const hash_t pObjHash)
{
	char			key[32];
	hash_t			child_obj;
	mem_zone_ref	obj = { PTR_NULL };
	struct string   app_name = { PTR_NULL }, child_path = { PTR_NULL };
	char			pObj[65];


	tree_manager_get_child_value_str(tx, NODE_HASH("appChildKey"), key, 32, 16);
	tree_manager_get_child_value_hash(tx, NODE_HASH("newChild"), child_obj);
	tree_manager_get_child_value_istr(tx, NODE_HASH("appChild"), &app_name, 0);

	bin_2_hex(pObjHash, 32, pObj);

	make_string(&child_path, "apps");
	cat_cstring_p(&child_path, app_name.str);
	cat_cstring_p(&child_path, "objs");
	cat_cstring_p(&child_path, pObj);
	cat_cstring(&child_path, "_");
	cat_cstring(&child_path, key);

	append_file(child_path.str, child_obj, 32);

	free_string(&child_path);
	free_string(&app_name);

	return 1;
}
