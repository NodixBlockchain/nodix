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

//protocol module

C_IMPORT size_t			C_API_FUNC	compute_payload_size(mem_zone_ref_ptr payload_node);
C_IMPORT char*			C_API_FUNC	write_node(mem_zone_ref_const_ptr key, unsigned char *payload);
C_IMPORT size_t			C_API_FUNC	get_node_size(mem_zone_ref_ptr key);
C_IMPORT void			C_API_FUNC	serialize_children(mem_zone_ref_ptr node, unsigned char *payload);
C_IMPORT const unsigned char*	C_API_FUNC read_node(mem_zone_ref_ptr key, const unsigned char *payload,size_t len);
C_IMPORT size_t			C_API_FUNC init_node(mem_zone_ref_ptr key);


extern int tx_is_app_item(hash_t txh, unsigned int oidx, mem_zone_ref_ptr app_tx, unsigned char *val);
extern int tx_is_app_child(hash_t txh, unsigned int oidx,struct string *app_name);
extern int add_app_tx(mem_zone_ref_ptr new_app, const char *app_name);
extern int get_script_data(const struct string *script, size_t *offset, struct string *data);
extern int get_script_file(struct string *script, mem_zone_ref_ptr file);
extern int obj_new(mem_zone_ref_ptr type, const char *objName, struct string *script, mem_zone_ref_ptr obj);
extern int add_app_tx_type(mem_zone_ref_ptr app, mem_zone_ref_ptr typetx);
extern int add_app_script(mem_zone_ref_ptr app, mem_zone_ref_ptr script);

//local module
extern hash_t		null_hash;
extern unsigned int	has_root_app;
extern btc_addr_t	root_app_addr;
extern hash_t		app_root_hash;
extern mem_zone_ref	apps;

btc_addr_t			src_addr_list[1024] = { 0xABCDEF };

static __inline void make_blk_path(const char *chash, struct string *blk_path)
{

	make_string		(blk_path, "blks");
	cat_ncstring_p	(blk_path, chash + 0, 2);
	cat_ncstring_p	(blk_path, chash + 2, 2);
	cat_cstring_p	(blk_path, chash);
}

static __inline void get_utxo_path(const char *txh,unsigned int oidx,struct string *tx_path)
{
	make_string		(tx_path, "utxos");
	cat_ncstring_p	(tx_path, txh + 0, 2);
	create_dir		(tx_path->str);
	cat_ncstring_p	(tx_path, txh + 2, 2);
	create_dir		(tx_path->str);
	cat_ncstring_p	(tx_path, txh,64);
	cat_cstring		(tx_path, "_out_");
	strcat_int		(tx_path, oidx);
}

OS_API_C_FUNC(int) get_last_block_height()
{
	return file_size("blk_indexes") / 32;
}

OS_API_C_FUNC(int) find_index_hash(hash_t h)
{
	unsigned char *buffer;
	size_t		  len;
	int				ret = 0;

	if (get_file("blk_indexes", &buffer, &len) > 0)
	{
		size_t cur = 0;
		while (cur < len)
		{
			if (!memcmp_c(buffer + cur, h, sizeof(hash_t)))
			{
				ret = 1;
				break;
			}

			cur += 32;
		}

		free_c(buffer);
	}


	return ret;
}


OS_API_C_FUNC(int) get_moneysupply(uint64_t *amount)
{
	unsigned char *data;
	size_t len;
	int ret = 0;
	if (get_file("supply", &data, &len)>0)
	{
		if (len >= sizeof(uint64_t))
		{
			ret = 1;
			*amount = *((uint64_t *)data);
		}
		free_c(data);
	}
	return ret;

}



OS_API_C_FUNC(int) find_hash(hash_t hash)
{
	char				file_name[65];
	struct string		blk_path = { PTR_NULL };
	int					ret;

	bin_2_hex(hash, 32, file_name);

	/*
	unsigned int		n;
	n = 32;
	while (n--)
	{
		file_name[n * 2 + 0] = hex_chars[hash[n] >> 4];
		file_name[n * 2 + 1] = hex_chars[hash[n] & 0x0F];
	}
	file_name[64] = 0;
	*/

	make_blk_path	(file_name,&blk_path);
	cat_cstring		(&blk_path, "_blk");

	ret = (stat_file(blk_path.str) == 0) ? 1 : 0;

	free_string(&blk_path);
	return ret;
}





OS_API_C_FUNC(int) blk_load_tx_ofset(const char *blk_hash, unsigned int ofset, mem_zone_ref_ptr tx)
{
	struct string		tx_path = { 0 };
	unsigned char		*tx_data;
	size_t				tx_data_len;
	int					ret;

	make_string		(&tx_path, "blks");
	cat_ncstring_p	(&tx_path, blk_hash + 0, 2);
	cat_ncstring_p	(&tx_path, blk_hash + 2, 2);
	cat_cstring_p	(&tx_path, blk_hash);
	cat_cstring		(&tx_path, "_txs");

	ret = 0;
	if (get_file_chunk(tx_path.str,ofset, &tx_data, &tx_data_len) > 0)
	{
		if ((tx->zone != PTR_NULL) || (tree_manager_create_node("tx", NODE_BITCORE_TX, tx)))
		{
			hash_t tmph,txh;

			init_node		(tx);
			read_node		(tx, tx_data,tx_data_len);

			mbedtls_sha256	(tx_data, tx_data_len, tmph,0);
			mbedtls_sha256	(tmph, 32, txh,0);

			tree_manager_set_child_value_i32 (tx,"size",tx_data_len);
			tree_manager_set_child_value_hash(tx,"txid",txh);
			ret = 1;
		}
		free_c(tx_data);
	}
	free_string(&tx_path);

	return ret;
}

OS_API_C_FUNC(int) load_tx(mem_zone_ref_ptr tx, hash_t blk_hash, const hash_t tx_hash)
{
	hash_t				th;
	char				chash[65], cthash[65];
	struct string		tx_path = { 0 };
	unsigned char		*buffer;
	mem_size			size;
	int					ret = 0;

	unsigned int		n = 32,ofset;

	while (n--)
	{
		cthash[n * 2 + 0] = hex_chars[tx_hash[n] >> 4];
		cthash[n * 2 + 1] = hex_chars[tx_hash[n] & 0x0F];

		chash[n * 2 + 0] = hex_chars[tx_hash[31-n] >> 4];
		chash[n * 2 + 1] = hex_chars[tx_hash[31-n] & 0x0F];
	}

	cthash[64] = 0;
	chash[64] = 0;

	make_string(&tx_path, "txs");
	cat_ncstring_p(&tx_path, cthash, 2);
	cat_ncstring_p(&tx_path, cthash + 2, 2);
	ret = get_file(tx_path.str, &buffer, &size);
	free_string(&tx_path);
	if (ret <= 0)return 0;

	ret = 0;
	n = 0;
	while ((n+80)<=size)
	{
		if (!memcmp_c(&buffer[n], tx_hash, sizeof(hash_t)))
		{

			int nn= 0;
			while (nn<32)
			{
				blk_hash[nn] = buffer[n + 32 + nn];
				chash[nn * 2 + 0] = hex_chars[blk_hash[nn] >> 4];
				chash[nn * 2 + 1] = hex_chars[blk_hash[nn] & 0x0F];
				nn++;
			}
			chash[64]	= 0;
			ofset		= *((unsigned int *)(buffer+n+72));
			ret = 1;
			break;
		}
		n += 80;
	}
	free_c(buffer);
	if (!ret)return 0;
	ret = blk_load_tx_ofset(chash, ofset, tx);

	tree_manager_get_child_value_hash(tx,NODE_HASH("txid"),th);

	if(memcmp_c(th,tx_hash,sizeof(hash_t)!=0))
	{
		ret=0;
		log_message("error chcking tx hash %txid% ",tx);
	}


	return ret;
}

OS_API_C_FUNC(int) load_tx_addresses(btc_addr_t addr, mem_zone_ref_ptr tx_hashes)
{
	btc_addr_t null_addr = { 0 };
	unsigned char *data;
	size_t len;
	struct string tx_file = { 0 };

	memset_c		(null_addr, '0', sizeof(btc_addr_t));

	make_string		(&tx_file, "adrs");
	cat_ncstring_p	(&tx_file, &addr[31], 2);

	if (get_file(tx_file.str, &data, &len) > 0)
	{
		size_t idx_sz, n = 0, idx = 0;
		uint64_t ftx, ttx, ntx = 0, aidx;
		unsigned char *first_tx;

		ttx = 0;
		while (n < len)
		{
			if (!memcmp_c(&data[n], null_addr, sizeof(btc_addr_t)))
				break;


			if (!memcmp_c(&data[n], addr, sizeof(btc_addr_t)))
			{
				ftx = ttx;
				ntx = *((uint64_t *)(data + n + sizeof(btc_addr_t)));
				aidx = idx;
			}

			ttx += *((uint64_t *)(data + n + sizeof(btc_addr_t)));
			n += sizeof(btc_addr_t) + sizeof(uint64_t);
			idx++;
		}

		if (ntx>0)
		{
			int nn;
			idx_sz = idx*(sizeof(btc_addr_t) + sizeof(uint64_t)) + sizeof(btc_addr_t);
			first_tx = data + idx_sz + ftx*sizeof(hash_t);
			nn = 0;
			while (nn < ntx)
			{
				mem_zone_ref new_hash = { PTR_NULL };
				uint64_t  height,time;
				unsigned int tx_time;

				if (get_tx_blk_height(first_tx + nn*sizeof(hash_t), &height, &time, &tx_time))
				{
					if (tree_manager_add_child_node(tx_hashes, "tx", NODE_BITCORE_HASH, &new_hash))
					{
						tree_manager_write_node_hash(&new_hash, 0, first_tx + nn*sizeof(hash_t));
						release_zone_ref(&new_hash);
					}
				}
				nn++;
			}
		}
		free_c(data);
	}

	free_string(&tx_file);
	return 0;
}

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

int del_utxo   (const char *txh,unsigned int oidx)
{
	char			dir[16];
	struct string	tx_path = { 0 } ;
	int				ret;

	get_utxo_path	(txh,oidx,&tx_path);
	ret=del_file	(tx_path.str);
	free_string		(&tx_path);

	strcpy_cs		(dir,32, "utxos");
	strcat_cs		(dir,32, "/");
	strncat_c		(dir, txh + 0, 2);
	strcat_cs		(dir,32, "/");
	strncat_c		(dir, txh + 2, 2);
	del_dir			(dir);


	return ret;
}


OS_API_C_FUNC(int) check_utxo  (const char *txh,unsigned int oidx)
{
	struct string	tx_path = { 0 } ;
	int				ret;
	
	get_utxo_path	(txh,oidx,&tx_path);

	ret=(stat_file	(tx_path.str)==0)? 1 : 0;

	free_string(&tx_path);

	return ret;
}

int load_utxo(const char *txh,unsigned int oidx,uint64_t *amount,btc_addr_t addr)
{
	struct	string tx_path={0};
	int		sret,ret;

	get_utxo_path	(txh,oidx,&tx_path);

	ret  = 0;
	sret = stat_file(tx_path.str);
	if (sret == 0)
	{
		unsigned char *buffer;
		size_t		  len;
		if (get_file(tx_path.str, &buffer, &len)>0)
		{
			if (len >= (sizeof(uint64_t) + sizeof(btc_addr_t)))
			{
				*amount		=	*((uint64_t *)(buffer));
				memcpy_c	(addr, buffer + sizeof(uint64_t),sizeof(btc_addr_t));
				ret=1;
			}
			free_c(buffer);
		}
	}
	free_string(&tx_path);
	return ret;
}


int store_tx_vout(const char *txh,mem_zone_ref_ptr txout_list,unsigned int oidx, btc_addr_t out_addr)
{
	struct string		script = { 0 }, tx_path = { 0 } ;
	mem_zone_ref		vout = {PTR_NULL};

	uint64_t			amount;
	int					ret;

	if (!tree_manager_get_child_at(txout_list, oidx, &vout))
	{
		log_output("store vout bad utxo\n");
		return 0;
	}
	
	ret=tree_manager_get_child_value_i64	(&vout, NODE_HASH("value"), &amount);
	if (!ret)
	{
		log_output("store vout no value\n");
		release_zone_ref(&vout);
		return 0;
	}
	ret=tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &script, 16);
	release_zone_ref(&vout);
	if (!ret)
	{
		log_output("store vout no script\n");
		return 0;
	}
	if ((amount == 0) && (script.len == 0))
	{ 
		free_string(&script); 
		return 1;
	}
	

	ret = get_out_script_address(&script, PTR_NULL, out_addr);
	if(ret)
	{
		unsigned char		bbuffer[64];

		*((uint64_t *)(bbuffer)) = amount;
		memcpy_c	 (bbuffer + sizeof(uint64_t), out_addr, sizeof(btc_addr_t));

		get_utxo_path(txh,oidx,&tx_path);
		put_file	 (tx_path.str, bbuffer, sizeof(uint64_t) + sizeof(btc_addr_t));
		free_string	 (&tx_path);
	}
	else
	{
		log_output("store vout no addr\n");
	}
	
	
	free_string		(&script);

	return ret;
}

OS_API_C_FUNC(int) remove_tx_addresses(const btc_addr_t addr, const hash_t tx_hash)
{
	btc_addr_t		null_addr;
	struct string   tx_file = { 0 };
	size_t			len;
	unsigned char  *data;

	memset_c		(null_addr, '0', sizeof(btc_addr_t));

	/*open the address index file*/
	make_string		(&tx_file, "adrs");
	cat_ncstring_p	(&tx_file, &addr[31], 2);

	if (get_file(tx_file.str, &data, &len)>0)
	{
		size_t		idx_sz, tx_list_ofs, ftidx;
		size_t		n = 0, idx = 0;
		uint64_t	ftx, ttx, cntx = 0, ntx = 0, aidx;
		unsigned char *first_tx;

		ttx = 0;
		ftx = 0;
		while ((n + sizeof(btc_addr_t)) <= len)
		{
			/*address is not in the index*/
			if (!memcmp_c(&data[n], null_addr, sizeof(btc_addr_t)))
				break;

			cntx = *((uint64_t *)(data + n + sizeof(btc_addr_t)));

			/*address is in the index at current position*/
			if (!memcmp_c(&data[n], addr, sizeof(btc_addr_t)))
			{
				/*position of the first transaction for this address*/
				ftx = ttx;

				/*number of transactions for this address*/
				ntx = cntx;

				/*index of the address*/
				aidx = idx;
			}

			//index of the first transaction of the next address
			ttx += cntx;

			//next address in the index
			n += sizeof(btc_addr_t) + sizeof(uint64_t);
			idx++;
		}

		//check transaction from the address
		if (ntx > 0)
		{
			//position of the end of address list
			idx_sz		= idx*(sizeof(btc_addr_t) + sizeof(uint64_t));


			//position of the first_transaction
			tx_list_ofs = idx_sz + sizeof(btc_addr_t);

			//position of the first tx for the address
			first_tx	= data + tx_list_ofs + ftx * sizeof(hash_t);

			//find the transaction in the address index
			for (n = 0; n < ntx;n++)
			{
				if (!memcmp_c(first_tx + n*sizeof(hash_t), tx_hash, sizeof(hash_t)))
				{
					uint64_t	*addr_ntx_ptr;
					size_t		next_tx_pos;

					addr_ntx_ptr	= (uint64_t	*)(data + aidx*(sizeof(btc_addr_t) + sizeof(uint64_t)) + sizeof(btc_addr_t));
					*addr_ntx_ptr	= ntx - 1;

					//position of the transaction to remove in the index
					ftidx			= (ftx + n)*sizeof(hash_t);
					next_tx_pos		= tx_list_ofs + ftidx + sizeof(hash_t);

					//write the new address index and transaction up to the one to remove
					put_file("newfile", data, tx_list_ofs + ftidx);

					//write transactions in the index after the one to remove
					append_file("newfile", data + next_tx_pos, len - next_tx_pos);

					//write the new index in the file
					del_file(tx_file.str);
					move_file("newfile", tx_file.str);
					break;
				}
			}
		}
		free_c(data);
	}
	free_string(&tx_file);
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
			if (!memcmp_c(&buffer[cur+4], hash, sizeof(hash_t)))
			{
				len -= 36;
				if (len > 0)
				{
					if (len>cur)
						memmove_c(&buffer[cur], &buffer[cur + 36], len - cur);

					put_file(file_name, buffer, len);
				}
				else
					del_file (file_name);
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
			unsigned char	sz	= *((unsigned char *)(buffer + cur));
			size_t entry_len	= (sz + 1 + sizeof(hash_t));

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
	return ;
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

	make_string		(&obj_path, "apps");
	cat_cstring_p	(&obj_path, app_name);
	cat_cstring_p	(&obj_path, "objs");
	cat_cstring_p	(&obj_path, tchash);
	cat_cstring		(&obj_path, "_");
	cat_cstring		(&obj_path, key);
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
	unsigned int n;

	bin_2_hex(ohash, 32, objHash);
		
	/*
	n = 0;
	while (n<32)
	{
		objHash[n * 2 + 0] = hex_chars[ohash[n] >> 4];
		objHash[n * 2 + 1] = hex_chars[ohash[n] & 0x0F];
		n++;
	}
	objHash[64] = 0;
	*/

	uitoa_s(type_id, buff, 16, 16);

	if (load_obj(app_name, objHash, "obj", 0, &obj, PTR_NULL))
	{
		
		mem_zone_ref_ptr	key=PTR_NULL;
		for (tree_manager_get_first_child(&obj, &my_list, &key); ((key != NULL) && (key->zone != NULL)); tree_manager_get_next_child(&my_list, &key))
		{
			if ((tree_mamanger_get_node_type(key) == NODE_JSON_ARRAY) || (tree_mamanger_get_node_type(key) == NODE_PUBCHILDS_ARRAY))
			{
				const char *name = tree_mamanger_get_node_name(key);

				make_string		(&obj_path, "apps");
				cat_cstring_p	(&obj_path, app_name);
				cat_cstring_p	(&obj_path, "objs");
				cat_cstring_p	(&obj_path, objHash);
				cat_cstring		(&obj_path, "_");
				cat_cstring		(&obj_path, name);
				del_file		(obj_path.str);
				free_string		(&obj_path);
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
		}
		free_string(&idx_path);
	}

	release_zone_ref(&idxs);
	

	make_string				(&obj_path, "apps");
	cat_cstring_p			(&obj_path, app_name);
	cat_cstring_p			(&obj_path, "objs");
	cat_cstring_p			(&obj_path, buff);
	rm_hash_from_file_obj	(obj_path.str, ohash);
	free_string				(&obj_path);

	make_string				(&obj_path, "apps");
	cat_cstring_p			(&obj_path, app_name);
	cat_cstring_p			(&obj_path, "objs");
	cat_cstring_p			(&obj_path, buff);
	cat_cstring				(&obj_path, "_time.idx");
	rm_hash_from_file_obj	(obj_path.str, ohash);
	free_string				(&obj_path);

	make_string				(&obj_path, "apps");
	cat_cstring_p			(&obj_path, app_name);
	cat_cstring_p			(&obj_path, "objs");
	cat_cstring_p			(&obj_path, objHash);
	del_file				(obj_path.str);
	free_string				(&obj_path);

	return 1;
}


int rm_type(const char *app_name, unsigned int type_id, const char *typeHash)
{
	struct string obj_path = { 0 };
	char buff[16];

	uitoa_s(type_id, buff, 16, 16);

	make_string		(&obj_path, "apps");
	cat_cstring_p	(&obj_path, app_name);
	cat_cstring_p	(&obj_path, "types");
	cat_cstring_p	(&obj_path, typeHash);
	del_file		(obj_path.str);
	free_string		(&obj_path);

	make_string		(&obj_path, "apps");
	cat_cstring_p	(&obj_path, app_name);
	cat_cstring_p	(&obj_path, "objs");
	cat_cstring_p	(&obj_path, buff);
	del_file		(obj_path.str);
	free_string		(&obj_path);

	make_string		(&obj_path, "apps");
	cat_cstring_p	(&obj_path, app_name);
	cat_cstring_p	(&obj_path, "objs");
	cat_cstring_p	(&obj_path, buff);
	cat_cstring		(&obj_path, "_time.idx");
	del_file		(obj_path.str);
	free_string		(&obj_path);

	return 1;
}


int rm_app_file(const char *app_name, mem_zone_ref_ptr file)
{
	hash_t hash;
	char fileHash[65];
	struct string file_path = { 0 };
	unsigned int n;

	if (!tree_manager_get_child_value_hash(file, NODE_HASH("dataHash"), hash))return 0;

	bin_2_hex(hash, 32, fileHash);
	/*
	n = 0;
	while (n<32)
	{
		fileHash[n * 2 + 0] = hex_chars[hash[n] >> 4];
		fileHash[n * 2 + 1] = hex_chars[hash[n] & 0x0F];
		n++;
	}
	fileHash[64] = 0;
	*/

	make_string		(&file_path, "apps");
	cat_cstring_p	(&file_path, app_name);
	cat_cstring_p	(&file_path, "datas");
	cat_cstring_p	(&file_path, fileHash);
	del_file		(file_path.str);
	free_string		(&file_path);


	make_string		(&file_path, "apps");
	cat_cstring_p	(&file_path, app_name);
	cat_cstring_p	(&file_path, "datas");
	cat_cstring_p	(&file_path, "index");

	rm_file_from_index(file_path.str, hash);

	free_string	(&file_path);

	return 1;
}

int rm_app(const char *app_name)
{
	struct string app_path = { 0 };


	make_string		(&app_path, "apps");
	cat_cstring_p	(&app_path, app_name);
	cat_cstring_p	(&app_path, "types");
	rm_dir			(app_path.str);
	free_string		(&app_path);

	make_string		(&app_path, "apps");
	cat_cstring_p	(&app_path, app_name);
	cat_cstring_p	(&app_path, "objs");
	rm_dir			(app_path.str);
	free_string		(&app_path);

	make_string		(&app_path, "apps");
	cat_cstring_p	(&app_path, app_name);
	cat_cstring_p	(&app_path, "layouts");
	rm_dir			(app_path.str);
	free_string		(&app_path);

	make_string		(&app_path, "apps");
	cat_cstring_p	(&app_path, app_name);
	cat_cstring_p	(&app_path, "datas");
	rm_dir			(app_path.str);
	free_string		(&app_path);

	make_string		(&app_path, "apps");
	cat_cstring_p	(&app_path, app_name);
	cat_cstring_p	(&app_path, "modz");
	rm_dir			(app_path.str);
	free_string		(&app_path);

	make_string		(&app_path, "apps");
	cat_cstring_p	(&app_path, app_name);
	rm_dir			(app_path.str);
	free_string		(&app_path);

	

	return 1;
}


OS_API_C_FUNC(int) remove_tx_index(hash_t tx_hash)
{
	char tchash[65];
	struct string tx_path = { 0 };
	unsigned char *buffer;
	size_t size;
	unsigned int ret, n;

	bin_2_hex(tx_hash, 32, tchash);

	/*
	n = 0;
	while (n<32)
	{
		tchash[n * 2 + 0] = hex_chars[tx_hash[n] >> 4];
		tchash[n * 2 + 1] = hex_chars[tx_hash[n] & 0x0F];
		n++;
	}
	tchash[64] = 0;
	*/


	//open index file for the hash
	make_string(&tx_path, "txs");
	cat_ncstring_p(&tx_path, tchash + 0, 2);
	cat_ncstring_p(&tx_path, tchash + 2, 2);

	if (get_file(tx_path.str, &buffer, &size) <= 0)
	{
		//not in the index
		free_string(&tx_path);
		return 0;
	}

	ret = 0;
	n = 0;
	while ((n + 80) <= size)
	{
		if (!memcmp_c(&buffer[n], tx_hash, sizeof(hash_t)))
		{
			if ((n + 80)<size)
				truncate_file(tx_path.str, n, &buffer[n + 80], size - (n + 80));
			else if (n>0)
				truncate_file(tx_path.str, n, PTR_NULL, 0);
			else
				del_file(tx_path.str);

			ret = 1;
			break;
		}
		n += 80;
	}
	if (size>0)
		free_c(buffer);

	free_string(&tx_path);

	return ret;
}



int cancel_tx_outputs(mem_zone_ref_ptr tx)
{
	char				tchash[65];
	mem_zone_ref	    txout_list = { PTR_NULL };
	unsigned int		oidx,n_utxo;

	if (!tree_manager_find_child_node(tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))return 0;

	if(!tree_manager_get_child_value_str(tx,NODE_HASH("txid"),tchash,65,16))
	{
		hash_t h;
		compute_tx_hash						(tx,h);
		tree_manager_set_child_value_hash	(tx,"txid",h);
		tree_manager_get_child_value_str	(tx,NODE_HASH("txid"),tchash,65,16);
	}

	n_utxo=tree_manager_get_node_num_children(&txout_list);
	for (oidx = 0; oidx<n_utxo; oidx++)
	{
		uint64_t amount;
		unsigned int app_item;
		get_tx_output_amount(tx, oidx, &amount);
		
		if (oidx == 0)
		{
			char			app_name[64];
			if (tree_manager_get_child_value_i32(tx, NODE_HASH("is_app_item"), &app_item))
			{
				tree_manager_get_child_value_str(tx, NODE_HASH("appName"), app_name, 64, 0);

				if (((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000) && (app_item == 2))
				{
					hash_t			h;
					unsigned int	type_id;
					type_id = amount & 0xFFFFFFFF;

					tree_manager_get_child_value_hash(tx, NODE_HASH("txid"), h);

					rm_obj(app_name, type_id, h);
					continue;
				}
				else if (app_item == 1)
				{
					char			typeName[32];
					mem_zone_ref	vout = { PTR_NULL };
					struct string	oscript = { 0 };
					unsigned int	type_id,flags;

					get_tx_output(tx, 0, &vout);
					tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &oscript, 0);

					if (get_type_infos(&oscript, typeName, &type_id,&flags))
					{
						rm_type(app_name, type_id, tchash);
					}

					free_string(&oscript);
					release_zone_ref(&vout);
					continue;
				}
				else if (app_item == 3)
				{
					mem_zone_ref	vout = { PTR_NULL }, file = { PTR_NULL };
					struct string	oscript = { 0 };
					

					get_tx_output(tx, 0, &vout);
					tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &oscript, 0);

					tree_manager_create_node("file", NODE_GFX_OBJECT, &file);
					
					if (get_script_file(&oscript, &file))
						rm_app_file(app_name, &file);

					release_zone_ref(&file);

					free_string(&oscript);
					release_zone_ref(&vout);
					continue;
				}
			}
			else if (tree_manager_get_child_value_str(tx, NODE_HASH("objChild"), app_name, 64,0))
			{
				char			objHash[65];
				mem_zone_ref	vout = { PTR_NULL };
				struct string	oscript = { 0 }, key = { 0 }, cHash = { 0 };
				size_t			offset = 0;
				int				ret;

				tree_manager_get_child_value_str(tx, NODE_HASH("appChildOf"), objHash, 65, 0);
				

				get_tx_output(tx, 0, &vout);
				tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &oscript, 0);

				ret = get_script_data(&oscript, &offset, &key);
				if (ret)ret = get_script_data(&oscript, &offset, &cHash);
				ret = (cHash.len == 32) ? 1 : 0;
				if (ret)
				{
					rm_child_obj(app_name, objHash, key.str, cHash.str);
				}

				free_string(&key);
				free_string(&cHash);
				free_string(&oscript);

				continue;
			}
		}
		if (amount > 0)
		{
			del_utxo(tchash, oidx);
		}
	}
	release_zone_ref(&txout_list);
	return 1;
}

int cancel_tx_inputs(mem_zone_ref_ptr tx)
{
	mem_zone_ref	 txin_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr input = PTR_NULL;
	int				 ret;


	if (!tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))return 0;

	//process tx inputs
	for (tree_manager_get_first_child(&txin_list, &my_list, &input); ((input != NULL) && (input->zone != NULL)); tree_manager_get_next_child(&my_list, &input))
	{
		mem_zone_ref	 ptx = { PTR_NULL };
		hash_t			 prev_hash, pblk_hash;
		unsigned int	 oidx;
		unsigned char	 app_item;

		tree_manager_get_child_value_hash(input, NODE_HASH("txid"), prev_hash);
		tree_manager_get_child_value_i32 (input, NODE_HASH("idx"), &oidx);


		if (tx_is_app_item(prev_hash, oidx, &ptx, &app_item))
		{
			release_zone_ref(&ptx);
			continue;
		}
		else if (!memcmp_c(prev_hash, app_root_hash, sizeof(hash_t)))
		{
			continue;
		}
		/* load the transaction with the spent output */
		else if (load_tx(&ptx, pblk_hash, prev_hash))
		{
			char			 txh[65];
			btc_addr_t		 out_addr;
			mem_zone_ref	 txout_list = { PTR_NULL };
			int			  	 n;

			/*rewrite the original tx out from the parent transaction*/

			bin_2_hex(prev_hash, 32, txh);
			/*
			n = 0;
			while (n<32)
			{
				txh[n * 2 + 0] = hex_chars[prev_hash[n] >> 4];
				txh[n * 2 + 1] = hex_chars[prev_hash[n] & 0x0F];
				n++;
			}
			txh[64] = 0;
			*/

			ret=tree_manager_find_child_node (&ptx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list);

			if(ret)ret=store_tx_vout		 (txh,&txout_list,oidx,out_addr);

			release_zone_ref(&txout_list);
			release_zone_ref(&ptx);
				
			/*
			cancel the spent in the wallet
			cancel_spend_tx_addr(out_addr, pchash, oidx);

			remove tx from the address index
			remove_tx_addresses(out_addr, tx_hash);
			*/
		}
	}
	release_zone_ref(&txin_list);
	return 1;
}



OS_API_C_FUNC(int) remove_tx(hash_t tx_hash)
{
	hash_t			blk_hash;
	mem_zone_ref	tx = { PTR_NULL };

	/*load transaction data from the block*/
	if (load_tx(&tx, blk_hash, tx_hash))
	{
		if (is_app_root(&tx))
		{
			blk_del_app_root();
		}
		else
		{
			hash_t			hash;
			mem_zone_ref	vin = { PTR_NULL }, ptx = { PTR_NULL };
			unsigned char	app_item;
			unsigned int	oidx;

			if (get_tx_input(&tx, 0, &vin))
			{
				struct string app_name = { 0 };
				tree_manager_get_child_value_hash(&vin, NODE_HASH("txid"), hash);
				tree_manager_get_child_value_i32(&vin, NODE_HASH("idx"), &oidx);

				if ((has_root_app) && (!memcmp_c(hash, app_root_hash, sizeof(hash_t))))
				{
					struct string script = { 0 };

					tree_manager_get_child_value_istr	(&vin, NODE_HASH("script"), &script,0);

					if(get_app_name	(&script, &app_name))
					{
						rm_app		(app_name.str);
						free_string	(&app_name);
					}

					tree_remove_child_by_member_value_hash(&apps, NODE_BITCORE_TX, "txid", tx_hash);
					free_string	(&script);
				}
				else if (tx_is_app_item(hash, oidx, &ptx, &app_item))
				{
					const char *app_name = tree_mamanger_get_node_name(&ptx);

					tree_manager_set_child_value_str(&tx, "appName", app_name);
					tree_manager_set_child_value_i32(&tx, "is_app_item", app_item);
					release_zone_ref(&ptx);
				}
				else if (tx_is_app_child(hash, oidx, &app_name))
				{
					tree_manager_set_child_value_vstr(&tx, "objChild", &app_name);
					tree_manager_set_child_value_hash(&tx, "appChildOf", hash);
					free_string(&app_name);
				}
				release_zone_ref(&vin);
			}
			/*cancel transaction on wallet*/
			cancel_tx_outputs	(&tx);
			cancel_tx_inputs	(&tx);
		}

		release_zone_ref	(&tx);
	}
	/*remove transaction from global index*/
	remove_tx_index(tx_hash);

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
		tree_manager_get_child_value_hash	(&input, NODE_HASH("txid"), prev_hash);
		tree_manager_get_child_value_i32	(&input, NODE_HASH("idx"), &oidx);

		if (tx_is_app_item(prev_hash, oidx, &app_tx, &app_item))
		{
			if (app_item == 3)
			{
				mem_zone_ref output = { PTR_NULL };
				
				if (get_tx_output(file_tx, 0, &output))
				{
					struct string script = { 0 };

					tree_manager_get_child_value_istr(&output, NODE_HASH("script"), &script,0);

					if (get_script_file(&script,file))
					{
						ret = 1;
						tree_manager_get_child_value_istr(&app_tx, NODE_HASH("appName"), app_name,0);
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
	
	/*
	n = 0;
	while (n<32)
	{
		chash[n * 2 + 0] = hex_chars[fileHash[n] >> 4];
		chash[n * 2 + 1] = hex_chars[fileHash[n] & 0x0F];
		n++;
	}
	chash[64] = 0;
	*/

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
			if (!memcmp_c(buffer+cur, fileHash, sizeof(hash_t)))
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
	size_t len,nh,total;
	int ret = 0;

	make_string(&app_path, "apps");
	cat_cstring_p(&app_path, app_name->str);
	cat_cstring_p(&app_path, "datas");
	cat_cstring_p(&app_path, "index");

	total = 0;
	nh = 0;

	if (get_file(app_path.str, &buffer, &len) > 0)
	{
		size_t cur	= first*64;

		total		= len / 64;

		while ((cur < len) && (nh  <num))
		{
			mem_zone_ref newh = { PTR_NULL };

			if (tree_manager_add_child_node(files, "file", NODE_BITCORE_HASH, &newh))
			{
				tree_manager_write_node_hash(&newh, 0, buffer + cur + 32);
				release_zone_ref(&newh);
				nh ++;
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
			

			if (!tree_find_child_node_by_member_name_hash(pending, NODE_GFX_OBJECT, "hash", buffer + cur+32, PTR_NULL))
			{
				bin_2_hex(&buffer[cur], 32, fHAsh);

				/*
				unsigned int n;
				n = 0;
				while (n < 32)
				{
					fHAsh[n * 2 + 0] = hex_chars[buffer[n + cur] >> 4];
					fHAsh[n * 2 + 1] = hex_chars[buffer[n + cur] & 0x0F];
					n++;
				}
				fHAsh[64] = 0;
				*/

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

OS_API_C_FUNC(int) get_blk_txs(const char* blk_hash, mem_zone_ref_ptr txs, size_t max)
{
	struct string	blk_path = { 0 };
	unsigned char	*ptxs;
	size_t			len, ntx;

	make_string(&blk_path, "blks");
	cat_ncstring_p(&blk_path, blk_hash + 0, 2);
	cat_ncstring_p(&blk_path, blk_hash + 2, 2);
	cat_cstring_p(&blk_path, blk_hash);
	cat_cstring(&blk_path, "_blk");


	if (get_file(blk_path.str, &ptxs, &len) >0)
	{
		ntx = 238;
		while (ntx < len)
		{
			mem_zone_ref tx = { PTR_NULL };
			if (tree_manager_add_child_node(txs, "tx", NODE_BITCORE_HASH, &tx))
			{
				tree_manager_write_node_hash(&tx, 0, &ptxs[ntx]);
				release_zone_ref(&tx);
			}
			ntx += 32;
		}
		free_c(ptxs);
	}
	free_string(&blk_path);

	return 1;
}


OS_API_C_FUNC(int) load_blk_txs(const char* blk_hash, mem_zone_ref_ptr txs)
{
	struct string	blk_path = { 0 };
	unsigned char	*ptxs;
	size_t			len;

	make_string(&blk_path, "blks");
	cat_ncstring_p(&blk_path, blk_hash + 0, 2);
	cat_ncstring_p(&blk_path, blk_hash + 2, 2);
	cat_cstring_p(&blk_path, blk_hash);
	cat_cstring(&blk_path, "_txs");
	if (get_file(blk_path.str, &ptxs, &len) >0)
	{
		size_t ofset = 0;

		while (ofset < len)
		{
			mem_zone_ref	tx = { PTR_NULL };
			unsigned int	tx_size;

			tx_size = *((unsigned int *)(ptxs + ofset));

			if (tree_manager_add_child_node(txs, "tx", NODE_BITCORE_TX, &tx))
			{
				hash_t		tmph, txh;

				init_node(&tx);
				read_node(&tx, ptxs + ofset + 4, tx_size);

				mbedtls_sha256(ptxs + ofset + 4, tx_size, tmph, 0);
				mbedtls_sha256(tmph, 32, txh, 0);

				tree_manager_set_child_value_i32(&tx, "size", tx_size);
				tree_manager_set_child_value_hash(&tx, "txid", txh);

				release_zone_ref(&tx);
			}
			ofset += (tx_size + 4);
		}
		free_c(ptxs);
	}
	free_string(&blk_path);

	return 1;
}



OS_API_C_FUNC(int) clear_tx_index()
{
	struct string	dir_list = { PTR_NULL }, tx_path = { PTR_NULL };
	size_t			cur, nfiles;

	nfiles = get_sub_dirs("txs", &dir_list);
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

			make_string(&txp, "txs");
			cat_ncstring_p(&txp, optr, sz);
			rm_dir(txp.str);
			free_string(&txp);

			cur++;
			optr = ptr + 1;
			dir_list_len -= sz;
		}
	}
	free_string(&dir_list);

	nfiles = get_sub_files("adrs", &dir_list);
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

			make_string(&txp, "adrs");
			cat_ncstring_p(&txp, optr, sz);
			del_file(txp.str);
			free_string(&txp);

			cur++;
			optr = ptr + 1;
			dir_list_len -= sz;
		}
	}
	free_string(&dir_list);
	return 1;
}

OS_API_C_FUNC(int) store_tx_inputs(mem_zone_ref_ptr tx)
{
	hash_t			 thash = { 0 }, nhash={ 0 };
	char			 tx_hash[65];
	struct string	 tx_path = { 0 };
	mem_zone_ref	 txin_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr input = PTR_NULL;
	unsigned int	 vin;
	int				 n,ret;

	if (!tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))
	{
		log_message("store_tx_inputs no txsin",PTR_NULL);
		return 0;
	}
	

	compute_tx_hash(tx, nhash);

	if (!tree_manager_get_child_value_hash(tx, NODE_HASH("txid"), thash))
		tree_manager_set_child_value_hash	(tx, "txid", nhash);

	if (memcmp_c(nhash, thash, sizeof(hash_t)))
	{
		mem_zone_ref log = { PTR_NULL };

		tree_manager_create_node			("log", NODE_LOG_PARAMS, &log);
		tree_manager_set_child_value_hash	(&log, "h1", thash);
		tree_manager_set_child_value_hash	(&log, "h2", nhash);
		log_message							("store_tx_inputs bad tx hash %h1% != %h2%", &log);
		release_zone_ref					(&log);
		return 0;
	}

	bin_2_hex(thash, 32, tx_hash);

	/*
	n = 0;
	while (n<32)
	{
		tx_hash[n * 2 + 0] = hex_chars[thash[n] >> 4];
		tx_hash[n * 2 + 1] = hex_chars[thash[n] & 0x0F];
		n++;
	}
	tx_hash[64] = 0;
	*/
	
	for (vin = 0, tree_manager_get_first_child(&txin_list, &my_list, &input); ((input != NULL) && (input->zone != NULL)); tree_manager_get_next_child(&my_list, &input), vin++)
	{
		char			phash[65];
		hash_t			prev_hash = { 0 };
		uint64_t		amount;
		btc_addr_t		addr;
		int				n;
		unsigned int	oidx, objChild;

		if (!tree_manager_get_child_value_hash(input, NODE_HASH("txid"), prev_hash))
		{
			log_output		("store_tx_inputs no txid\n");
			dec_zone_ref	(input);
			release_zone_ref(&my_list);
			release_zone_ref(&txin_list);
			return 0;
		}

		if (!tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &oidx))
		{
			log_output("store_tx_inputs no oidx\n");
			dec_zone_ref	(input);
			release_zone_ref(&my_list);
			release_zone_ref(&txin_list);
			return 0;
		}

		if (!memcmp_c(prev_hash, null_hash, sizeof(hash_t)))
		{
			btc_addr_t coinbase;
			memset_c(coinbase, '0', sizeof(btc_addr_t));
			tree_manager_set_child_value_btcaddr(input, "srcaddr", coinbase);
			continue;
		}
		else if (!memcmp_c(prev_hash, app_root_hash, sizeof(btc_addr_t)))
		{
			continue;
		}
		else if (tree_find_child_node_by_member_name_hash(&apps, NODE_BITCORE_TX, "txid", prev_hash, PTR_NULL))
		{
			continue;
		}
		else if (tree_manager_get_child_value_i32(input, NODE_HASH("isObjChild"),&objChild))
		{
			continue;
		}
			
		bin_2_hex(prev_hash, 32, phash);

		/*
		n = 0;
		while (n<32)
		{
			phash[n * 2 + 0] = hex_chars[prev_hash[n] >> 4];
			phash[n * 2 + 1] = hex_chars[prev_hash[n] & 0x0F];
			n++;
		}
		phash[64] = 0;
		*/
	
		ret=load_utxo	(phash,oidx,&amount,addr);
		if(ret)
		{
			del_utxo								(phash, oidx);

			tree_manager_set_child_value_i64		(input, "amount" , amount);
			tree_manager_set_child_value_btcaddr	(input, "srcaddr", addr);
			store_tx_addresses						(addr, thash);
		}
	}
	release_zone_ref(&txin_list);
	return 1;
}


OS_API_C_FUNC(int) store_tx_addresses(btc_addr_t addr, hash_t tx_hash)
{
	btc_addr_t		null_addr = { 0 };
	unsigned char	*data;
	size_t			len;
	struct string	tx_file = { 0 };

	if (addr[0] == 0)return 1;

	memset_c		(null_addr, '0', sizeof(btc_addr_t));
	make_string		(&tx_file, "adrs");
	cat_ncstring_p	(&tx_file, &addr[31], 2);
	if (get_file(tx_file.str, &data, &len)>0)
	{
		size_t			idx_sz, ftidx;
		size_t			n = 0, idx = 0;
		uint64_t		ftx, ttx, ntx, aidx;
		unsigned char	*first_tx;


		/*  
			scan file for the address in the index 
			the index contain 34 bytes address and 64 bits count of transaction for that address
		*/
		ttx = 0;
		ntx = 0;
		for ( n = 0, idx = 0; n + sizeof(btc_addr_t) < len; n += (sizeof(btc_addr_t) + sizeof(uint64_t)), idx++)
		{
			uint64_t cntx;

			/* null address mark end of the index, address not in the index */
			if (!memcmp_c(&data[n], null_addr, sizeof(btc_addr_t)))
				break;

			/* number of transaction indexed for this address */
			cntx = *((uint64_t *)(data + n + sizeof(btc_addr_t)));

			/* address is found in the index */
			if (!memcmp_c(&data[n], addr, sizeof(btc_addr_t)))
			{
				/* ofset of the first transaction for this address in the file */
				ftx		= ttx;

				ntx		= cntx;

				/* index of the address in the address index */
				aidx	= idx;
			}
			
			/* increment the offet of the transactions list for the current address in the file */
			ttx += cntx;
		}

		/* offset of the last address in the index */
		idx_sz = idx*(sizeof(btc_addr_t) + sizeof(uint64_t));

		/* address is already indexed */
		if (ntx > 0)
		{
			int fnd		= 0;

			/* offset of the first transaction of the selected address */
			first_tx	= data + idx_sz + sizeof(btc_addr_t) + ftx*sizeof(hash_t);
			
			/* search the transaction hash in the transaction list */
			for(n=0; n < ntx; n++)
			{
				if (!memcmp_c(first_tx + n * sizeof(hash_t), tx_hash, sizeof(hash_t)))
				{
					fnd = 1;
					break;
				}
			}

			/* if the transaction is not already indexed for this address */
			if (!fnd)
			{
				/* increment transaction count in the address index */
				*((uint64_t *)(data + aidx*(sizeof(btc_addr_t) + sizeof(uint64_t)) + sizeof(btc_addr_t))) = ntx + 1;
				
				/* offset of the end of the last transaction for this address */
				ftidx			= (ftx + ntx)*sizeof(hash_t);

				/* write the address index in the file and all the transactions before the last one for the address */
				put_file		("newfile", data, idx_sz + sizeof(btc_addr_t) + ftidx);

				/* append the new tx hash in the file */
				append_file		("newfile", tx_hash, sizeof(hash_t));

				/* append remaining tx hashes in the file */
				append_file		("newfile", data + idx_sz + sizeof(btc_addr_t) + ftidx, len - (idx_sz + sizeof(btc_addr_t) + ftidx));

				/* replace original file */
				del_file		(tx_file.str);
				move_file		("newfile", tx_file.str);
			}
		}
		else
		{
			uint64_t one = 1;

			/* write data before last address in the index */
			put_file		("newfile", data			, idx_sz);

			/* append new address and number of transactions in the file */
			append_file		("newfile", addr			, sizeof(btc_addr_t));
			append_file		("newfile", &one			, sizeof(uint64_t));

			/* append the rest of the original file */
			append_file		("newfile", data + idx_sz	, len - (idx_sz));

			/* append the new tx hash */
			append_file		("newfile", tx_hash			, sizeof(hash_t));

			/* replace original file */
			del_file		(tx_file.str);
			move_file		("newfile", tx_file.str);
		}

		free_c(data);
	}
	else
	{
		/* initialize new address entry plus null addr */
		size_t s	= sizeof(btc_addr_t) * 2 + sizeof(uint64_t);
		data		= malloc_c(s);

		memcpy_c	(data, addr, sizeof(btc_addr_t));
		*((uint64_t *)(data + sizeof(btc_addr_t))) = 1;

		memset_c	(data + sizeof(btc_addr_t) + sizeof(uint64_t), '0', sizeof(btc_addr_t));

		/* write the address index in the file */
		put_file	(tx_file.str, data, s);
		free_c		(data);

		/* append tx hash */
		append_file(tx_file.str, tx_hash, sizeof(hash_t));
	}

	
	free_string(&tx_file);
	return 1;

}

OS_API_C_FUNC(int) store_tx_outputs(mem_zone_ref_ptr tx)
{
	hash_t				thash, nhash;
	char				tx_hash[65];
	mem_zone_ref		txout_list = { PTR_NULL };
	unsigned int		oidx, n_utxo, app_item, childOf;
	int					n,ret=0;

	compute_tx_hash(tx, nhash);

	if (!tree_manager_get_child_value_hash(tx, NODE_HASH("txid"), thash))
		tree_manager_set_child_value_hash(tx, "txid", nhash);

	if (memcmp_c(nhash, thash, sizeof(hash_t)))
	{
		mem_zone_ref log = { PTR_NULL };

		tree_manager_dump_node_rec			(tx,0,4);

		tree_manager_create_node			("log", NODE_LOG_PARAMS, &log);
		tree_manager_set_child_value_hash	(&log, "h1", thash);
		tree_manager_set_child_value_hash	(&log, "h2", nhash);
		log_message							("store_tx_outputs bad tx hash %h1% != %h2%", &log);
		release_zone_ref					(&log);
		return 0;
	}

	if (!tree_manager_find_child_node(tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))
	{
		log_output("store tx no utxos\n");
		return 0;
	}

	bin_2_hex(thash, 32, tx_hash);

	/*
	n = 0;
	while (n<32)
	{
		tx_hash[n * 2 + 0] = hex_chars[thash[n] >> 4];
		tx_hash[n * 2 + 1] = hex_chars[thash[n] & 0x0F];
		n++;
	}
	tx_hash[64] = 0;
	*/

	n_utxo=tree_manager_get_node_num_children(&txout_list);

	if (n_utxo <= 0)
	{
		log_output		("utxo count 0\n");
		release_zone_ref(&txout_list);
		return 0;
	}

	for (oidx = 0; oidx<n_utxo; oidx++)
	{
		btc_addr_t		out_addr = { 0 };
		
		if (tree_manager_find_child_node(tx, NODE_HASH("AppName"), NODE_GFX_STR, PTR_NULL))
		{
			struct string	script = { PTR_NULL };
			mem_zone_ref	vout = { 0 };

			tree_manager_get_child_at				(&txout_list, oidx, &vout);
			if (tree_manager_get_child_value_istr	(&vout, NODE_HASH("script"), &script, 16))
			{
				struct string my_val = { PTR_NULL };
				ret = get_out_script_return_val(&script, &my_val);
				if (ret)
				{
					free_string(&my_val);
					continue;
				}
				free_string(&script);
			}
			release_zone_ref(&vout);
		}
		else if (tree_manager_get_child_value_i32(tx, NODE_HASH("app_item"), &app_item))
		{
			uint64_t		amount;
			ret=get_tx_output_amount(tx, oidx, &amount);
			if (amount == 0)continue;
			if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)continue;
		}
		else if (tree_manager_get_child_value_i32(tx, NODE_HASH("childOf"), &childOf))
		{
			uint64_t		amount;
			ret = get_tx_output_amount(tx, oidx, &amount);
			if (amount == 0)continue;
		}

		if (store_tx_vout(tx_hash, &txout_list, oidx, out_addr))
		{
			ret = store_tx_addresses(out_addr, thash);
			if (!ret)log_output("store_tx_addresses error\n");
		}
		else
			ret = 1;
		if (!ret)
			break;
	}
	release_zone_ref(&txout_list);
	
	
	
	return ret;
}

/* -------------------------------------------------------- */

OS_API_C_FUNC(int) find_blk_hash(const hash_t tx_hash, hash_t blk_hash,uint64_t *height,unsigned int *ofset,unsigned int *tx_time)
{
	char				cthash[65];
	unsigned int		n = 32;
	struct string		tx_path = { 0 };
	unsigned char		*buffer;
	mem_size			size;
	int					ret;


	bin_2_hex(tx_hash, 32, cthash);

	/*
	while (n--)
	{
		cthash[n * 2 + 0] = hex_chars[tx_hash[n] >> 4];
		cthash[n * 2 + 1] = hex_chars[tx_hash[n] & 0x0F];
	}
	cthash[64] = 0;
	*/

	make_string		(&tx_path, "txs");
	cat_ncstring_p	(&tx_path, cthash, 2);
	cat_ncstring_p	(&tx_path, cthash + 2, 2);
	ret = get_file	(tx_path.str, &buffer, &size);
	free_string(&tx_path);
	if (ret <= 0)return 0;

	ret = 0;
	n = 0;
	while (n<size)
	{
		if (!memcmp_c(&buffer[n], tx_hash, sizeof(hash_t)))
		{
			memcpy_c(blk_hash, &buffer[n + 32], sizeof(hash_t));
			if(height!=PTR_NULL)(*height)=*((uint64_t *)(buffer+n+64));
			if(ofset!=PTR_NULL)(*ofset)=*((unsigned int *)(buffer+n+72));
			if(tx_time!=PTR_NULL)(*tx_time)=*((unsigned int *)(buffer+n+76));
			ret = 1;
			break;
		}
		n += 80;
	}
	free_c(buffer);
	return ret;
}



OS_API_C_FUNC(int) get_block_time(const char *blkHash, ctime_t *time)
{
	struct string blk_path = { 0 };
	int ret;

	make_string		(&blk_path, "blks");
	cat_ncstring_p	(&blk_path, blkHash + 0, 2);
	cat_ncstring_p	(&blk_path, blkHash + 2, 2);
	cat_cstring_p	(&blk_path, blkHash);
	cat_cstring		(&blk_path, "_blk");

	ret = get_ftime	(blk_path.str, time);

	free_string		(&blk_path);

	return ret;
}

OS_API_C_FUNC(int) is_pow_block(const char *blk_hash)
{
	struct string	file_path = { 0 };
	unsigned char	*blk_data;
	size_t			len;
	int				ret=0;

	make_string		(&file_path, "blks");
	cat_ncstring_p	(&file_path, blk_hash + 0, 2);
	cat_ncstring_p	(&file_path, blk_hash + 2, 2);
	cat_cstring_p	(&file_path, blk_hash);
	cat_cstring		(&file_path, "_blk");

	if(get_file_len(file_path.str,205,&blk_data,&len)>0)
	{
		if(len>=205)
		{
			if((*((unsigned char *)(blk_data+172)))==1)
				ret=1;
		}
		free_c(blk_data);
	}
	else
		ret = 0;

	free_string(&file_path);

	return ret;
}


OS_API_C_FUNC(unsigned int) get_blk_ntxs(const char* blk_hash)
{
	struct string	blk_path = { 0 };
	size_t			ntx;
	int				size;

	make_string		(&blk_path, "blks");
	cat_ncstring_p	(&blk_path, blk_hash + 0, 2);
	cat_ncstring_p	(&blk_path, blk_hash + 2, 2);
	cat_cstring_p	(&blk_path, blk_hash);
	cat_cstring		(&blk_path, "_blk");

	size		=	file_size(blk_path.str);
	if(size>238)
		ntx = (size-238) / sizeof(hash_t);
	else
		ntx =  0;

	free_string(&blk_path);
	return ntx;
}


OS_API_C_FUNC(int) get_blk_height(const char *blk_hash, uint64_t *height)
{
	struct string	blk_path = { PTR_NULL }, tx_path = { PTR_NULL };
	unsigned char	*data;
	int				ret = 0;
	size_t			len;


	make_string		(&blk_path, "blks");
	cat_ncstring_p	(&blk_path, blk_hash + 0, 2);
	cat_ncstring_p	(&blk_path, blk_hash + 2, 2);
	cat_cstring_p	(&blk_path, blk_hash);
	cat_cstring		(&blk_path, "_blk");

	if(get_file_len(blk_path.str,88,&data,&len)>0)
	{
		if(len>=88)
		{
			*height	=*((uint64_t *)(data+80));
			ret		=1;
		}
		free_c(data);
	}
	else
		ret = 0;

	free_string(&blk_path);
	return ret;
}

OS_API_C_FUNC(int) get_tx_blk_height(const hash_t tx_hash, uint64_t *height, uint64_t *block_time, unsigned int *tx_time)
{
	char chash[65];
	hash_t blk_hash;
	struct string blk_path = { PTR_NULL };
	ctime_t ctime;
	unsigned int n;

	if (!find_blk_hash(tx_hash, blk_hash,height, PTR_NULL,tx_time))
		return 0;


	bin_2_hex(blk_hash, 32, chash);
	/*
	n = 32;
	while (n--)
	{
		chash[n * 2 + 0] = hex_chars[blk_hash[n] >> 4];
		chash[n * 2 + 1] = hex_chars[blk_hash[n] & 0x0F];
	}
	chash[64] = 0;
	*/

	if (block_time != PTR_NULL)
	{
		get_block_time(chash, &ctime);
		*block_time = ctime;
	}
	return 1;
}


OS_API_C_FUNC(int) get_block_size(const char *blk_hash, size_t *size)
{
	struct string	blk_path = { PTR_NULL }, tx_path = { PTR_NULL };
	unsigned char	*data;
	unsigned int	ntx,signlen;
	int				ret = 0;
	size_t			len;


	make_string		(&blk_path, "blks");
	cat_ncstring_p	(&blk_path, blk_hash + 0, 2);
	cat_ncstring_p	(&blk_path, blk_hash + 2, 2);
	cat_cstring_p	(&blk_path, blk_hash);
	cat_cstring		(&blk_path, "_blk");

	if(get_file_len(blk_path.str,96,&data,&len)>0)
	{
		if(len>=96)
		{
			ntx		=	*((unsigned int *)(data+88));
			signlen =	*((unsigned char *)(data+92));
			ret		=	1;
		}
		free_c			(data);
		free_string		(&blk_path);

		if(ret)
		{
			if(ntx<0xFD)
				*size = 1;
			else
				*size = 3;

			make_string		(&blk_path, "blks");
			cat_ncstring_p	(&blk_path, blk_hash + 0, 2);
			cat_ncstring_p	(&blk_path, blk_hash + 2, 2);
			cat_cstring_p	(&blk_path, blk_hash);
			cat_cstring		(&blk_path, "_txs");

			*size +=		(80 + (file_size(blk_path.str) - ntx*4));
		}
	}
	else
		ret = -1;

	free_string(&blk_path);
	return ret;
}

OS_API_C_FUNC(int) store_tx_blk_index(const hash_t tx_hash, const hash_t blk_hash,uint64_t height,size_t tx_ofset,unsigned int tx_time)
{
	char			tchash[65];
	unsigned char   buffer[80];
	struct string	tx_path = { 0 };
	int				n= 0;

	bin_2_hex(tx_hash, 32, tchash);

	/*
	while (n<32)
	{
		tchash[n * 2 + 0] = hex_chars[tx_hash[n] >> 4];
		tchash[n * 2 + 1] = hex_chars[tx_hash[n] & 0x0F];
		n++;
	}
	tchash[64] = 0;
	*/


	make_string		(&tx_path, "txs");
	cat_ncstring_p	(&tx_path, tchash + 0, 2);
	create_dir		(tx_path.str);
	cat_ncstring_p	(&tx_path, tchash + 2, 2);

	memcpy_c	(buffer		, tx_hash	, sizeof(hash_t));
	memcpy_c	(buffer+32	, blk_hash	, sizeof(hash_t));

	*((uint64_t *)(buffer+64))		=height;
	*((unsigned int *)(buffer+72))	=tx_ofset;
	*((unsigned int *)(buffer+76))	=tx_time;

	append_file (tx_path.str, buffer	, 80);
	free_string (&tx_path);

	return 1;
}

OS_API_C_FUNC(int) load_blk_hdr(mem_zone_ref_ptr hdr, const char *blk_hash)
{
	unsigned char		*hdr_data;
	size_t				hdr_data_len;
	struct string		blk_path = { PTR_NULL };
	int    ret = 0;

	make_string(&blk_path, "blks");
	cat_ncstring_p(&blk_path, blk_hash, 2);
	cat_ncstring_p(&blk_path, blk_hash + 2, 2);
	cat_cstring_p(&blk_path, blk_hash);
	cat_cstring  (&blk_path, "_blk");

	/*cat_cstring_p(&blk_path, "header");*/

	if (get_file_len(blk_path.str, 205, &hdr_data, &hdr_data_len) > 0)
	{
		hash_t h1, h2,hash;
		int		n = 32;

		while (n--)
		{
			char    hex[3];
			hex[0] = blk_hash[n * 2 + 0];
			hex[1] = blk_hash[n * 2 + 1];
			hex[2] = 0;
			hash[n] = strtoul_c(hex, PTR_NULL, 16);
		}

		mbedtls_sha256(hdr_data, 80, h1, 0);
		mbedtls_sha256(h1, 32, h2, 0);

		if (memcmp_c(h2, hash, sizeof(hash_t)))
		{
			log_output("bad block hash \n");
			free_string(&blk_path);
			free_c(hdr_data);

			return 0;
		}

		if ((hdr->zone != PTR_NULL) || (tree_manager_create_node("blk", NODE_BITCORE_BLK_HDR, hdr)))
		{
			
			struct string sign;
			unsigned char vntx[16];
			unsigned int ntx;
			

			init_node(hdr);
			read_node(hdr, hdr_data, hdr_data_len);


			tree_manager_set_child_value_bhash(hdr, "blkHash", hash);


			
			/*length = 80+4+8+33+80+32; /* hdr size + ntx + hght + pos/pow + sig */
			
			tree_manager_set_child_value_i64(hdr,"height",*((uint64_t *)(hdr_data+80)));

			ntx	=*((unsigned int *)(hdr_data+88));

			if (ntx < 0xFD)
				vntx[0] = ntx;
			else
			{
				vntx[0] = 0xFD;
				*((unsigned short *)(&vntx[1])) = (unsigned short)ntx;
			}
			tree_manager_set_child_value_vint(hdr, "ntx", vntx);

			
			sign.len=*((unsigned char *)(hdr_data+92));

			if(sign.len>0)
			{
				mem_zone_ref sig={PTR_NULL};
				sign.str=(char *)(hdr_data+93);

				if (!tree_manager_find_child_node(hdr, NODE_HASH("signature"), NODE_BITCORE_ECDSA_SIG, &sig))
					tree_manager_add_child_node(hdr, "signature", NODE_BITCORE_ECDSA_SIG, &sig);

				
				tree_manager_write_node_sig(&sig, 0, (unsigned char *)sign.str, sign.len);
				release_zone_ref(&sig);
				
			}
			
			switch(hdr_data[172])
			{
				case 0:break;
				case 1:
					tree_manager_set_child_value_hash(hdr,"blk pow",hdr_data+173);
				break;
				default:
					tree_manager_set_child_value_hash(hdr,"blk pos",hdr_data+173);
				break;
			}
			ret = 1;
		}
		free_c(hdr_data);
	}
	free_string(&blk_path);

	return ret;
}

OS_API_C_FUNC(int) store_block_txs(mem_zone_ref_ptr header, mem_zone_ref_ptr tx_list)
{
	char				blk_hash[65];
	mem_zone_ref_ptr	tx = PTR_NULL;
	mem_zone_ref		my_list = { PTR_NULL };
	struct string		file_path={0};
	size_t				nc,n_tx;
	unsigned char		*blk_txs;

	tree_manager_get_child_value_str(header,NODE_HASH("blkHash"),blk_hash,65,16);

	nc	   = tree_manager_get_node_num_children(tx_list);

	blk_txs = (unsigned char *)calloc_c(sizeof(hash_t), nc);

	for (n_tx=0,tree_manager_get_first_child(tx_list, &my_list, &tx); ((tx != NULL) && (tx->zone != NULL)); tree_manager_get_next_child(&my_list, &tx), n_tx++)
	{
		tree_manager_get_child_value_hash	(tx,NODE_HASH("txid"),&blk_txs[n_tx * 32]);
	}

	make_string		(&file_path, "blks");
	cat_ncstring_p	(&file_path, blk_hash + 0, 2);
	cat_ncstring_p	(&file_path, blk_hash + 2, 2);
	cat_cstring_p	(&file_path, blk_hash);
	cat_cstring		(&file_path, "_blk");
	append_file		(file_path.str,blk_txs,nc*32);
	free_string		(&file_path);
	free_c			(blk_txs);

	return 1;
}


OS_API_C_FUNC(int) blk_load_app_root()
{
	struct string	app_root_path={0};
	unsigned char	*root_app_tx;
	size_t			len;

	make_string		(&app_root_path,"apps");
	cat_cstring_p	(&app_root_path,"root_app");

	if(get_file(app_root_path.str,&root_app_tx,&len)>0)
	{
		mem_zone_ref			apptx={PTR_NULL};

		if(tree_manager_create_node("rootapp",NODE_BITCORE_TX,&apptx))
		{
			tree_manager_add_child_node(&apptx, "txsin", NODE_BITCORE_VINLIST, PTR_NULL);
			tree_manager_add_child_node(&apptx, "txsout", NODE_BITCORE_VOUTLIST, PTR_NULL);
			read_node					(&apptx,root_app_tx,len);
			set_root_app				(&apptx);



			release_zone_ref			(&apptx);
		}
		free_c(root_app_tx);
	}
	free_string(&app_root_path);

	return 1;
}

int blk_load_app_types(mem_zone_ref_ptr app)
{
	struct string types_path = { 0 }, dir_list = { 0 };
	size_t  cur, nfiles;
	const char	*name;

	name = tree_mamanger_get_node_name(app);

	make_string		(&types_path, "apps");
	cat_cstring_p	(&types_path, name);
	cat_cstring_p	(&types_path, "types");


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
			sz  = mem_sub(optr, ptr);

			clone_string	(&path, &types_path);
			cat_ncstring_p	(&path, optr,sz);

			if (get_file(path.str, &buffer, &len)>0)
			{
				hash_t				txh;
				mem_zone_ref		new_type = { PTR_NULL };

				tree_manager_create_node			("types", NODE_BITCORE_TX, &new_type);

				init_node							(&new_type);
				read_node							(&new_type, buffer,len);
				compute_tx_hash						(&new_type, txh);
				tree_manager_set_child_value_hash	(&new_type, "txid", txh);
				add_app_tx_type						(app, &new_type);
				release_zone_ref					(&new_type);
				free_c								(buffer);
			}
			free_string(&path);

			cur++;
			optr = ptr + 1;
			dir_list_len -= sz;
		}
	}
	free_string		(&dir_list);
	free_string		(&types_path);
	return 1;
}

int blk_load_app_scripts(mem_zone_ref_ptr app)
{
	struct string script_path = { 0 }, dir_list = { 0 }, appName = { 0 };
	size_t  cur, nfiles;
	

	tree_manager_get_child_value_istr(app, NODE_HASH("appName"), &appName,0);

	make_string		(&script_path, "apps");
	cat_cstring_p	(&script_path, appName.str);
	cat_cstring_p	(&script_path, "modz");


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
			sz  = mem_sub(optr, ptr);

			if (sz < 5){
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

				clone_string	(&path, &script_path);
				cat_ncstring_p	(&path, optr, sz);
				if (load_script(path.str, script_name, &script_var, 1))
				{
					ctime_t ftime;
					get_ftime						(path.str, &ftime);
					tree_manager_write_node_dword	(&script_var, 0, ftime);
					add_app_script					(app, &script_var);
				}
				free_string		(&path);
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

			strncpy_c		(app_name, optr, sz);
			make_string		(&path, "apps");
			cat_cstring_p	(&path, app_name);
			cat_cstring_p	(&path, "app");

			if (get_file(path.str, &buffer, &len)>0)
			{
				mem_zone_ref new_app = { PTR_NULL };

				if (tree_manager_create_node(app_name, NODE_BITCORE_TX, &new_app))
				{
					hash_t txh;
					mem_zone_ref txout_list = { PTR_NULL };

					init_node			(&new_app);
					read_node			(&new_app, buffer,len);
					compute_tx_hash		(&new_app, txh);
					tree_manager_set_child_value_hash(&new_app, "txid", txh);

					add_app_tx			(&new_app, app_name);
					blk_load_app_types	(&new_app);

					if (is_trusted_app(app_name))
						blk_load_app_scripts(&new_app);

					release_zone_ref	(&new_app);
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
	struct string	app_root_path={0};
	unsigned char	*root_app_tx;
	size_t			len;

	make_string		(&app_root_path,"apps");
	create_dir		(app_root_path.str);
	cat_cstring_p	(&app_root_path,"root_app");

	len			=	get_node_size(tx);
	root_app_tx =	(unsigned char	*)malloc_c(len);

	write_node		(tx,root_app_tx);
	put_file		(app_root_path.str,root_app_tx,len);

	free_string		(&app_root_path);

	return 1;
}

OS_API_C_FUNC(int) get_app_obj_hashes(const char *app_name,mem_zone_ref_ptr hash_list)
{
	struct string obj_path = { PTR_NULL }, dir_list = { PTR_NULL };
	size_t nfiles, cur;

	tree_remove_children(hash_list);

	make_string  (&obj_path, "apps");
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
			
				if(tree_manager_add_child_node		(hash_list,"hash", NODE_BITCORE_HASH, &hashn))
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

	make_string		(&obj_path, "apps");
	cat_cstring_p	(&obj_path, app_name);
	cat_cstring_p	(&obj_path, "objs");
	cat_cstring_p	(&obj_path, typeStr);

	size = file_size(obj_path.str)/32;

	free_string		(&obj_path);

	return size;
}

OS_API_C_FUNC(int) get_app_type_obj_hashes(const char *app_name, unsigned int type_id,size_t first, size_t max, mem_zone_ref_ptr hash_list)
{
	char			typeStr[32];
	struct string	obj_path = { PTR_NULL };
	unsigned char	*buffer;
	size_t			cur,len,nHashes;
	int				ret;

	uitoa_s(type_id, typeStr, 32, 16);

	tree_remove_children(hash_list);

	make_string			(&obj_path, "apps");
	cat_cstring_p		(&obj_path, app_name);
	cat_cstring_p		(&obj_path, "objs");
	cat_cstring_p		(&obj_path, typeStr);

	ret = (get_file(obj_path.str, &buffer, &len) > 0) ? 1 : 0;

	if (ret)
	{
		cur =  first * 32;
		nHashes = 0;

		while ((cur < len) && (nHashes<max))
		{
			mem_zone_ref	hashn = { PTR_NULL };

			if (tree_manager_add_child_node(hash_list, "hash", NODE_BITCORE_HASH, &hashn))
			{
				tree_manager_write_node_hash(&hashn, 0, &buffer[cur]);
				release_zone_ref			(&hashn);
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
	cat_cstring  (&obj_path, "_time.idx");

	ret = (get_file(obj_path.str, &buffer, &len) > 0) ? 1 : 0;

	if (ret)
	{
		cur			= first * 36;
		nHashes		= 0;
		*total		= len/36;

		while ((cur < len) && (nHashes<max))
		{
			mem_zone_ref	hashn = { PTR_NULL };

			if (tree_manager_add_child_node(hash_list, "hash", NODE_BITCORE_HASH, &hashn))
			{
				tree_manager_write_node_hash(&hashn, 0, &buffer[cur+4]);
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

OS_API_C_FUNC(int) find_app_type_obj(const char *app_name, unsigned int type_id, const char *objHash, mem_zone_ref_ptr hash_list)
{
	char			typeStr[32];
	struct string	obj_path = { PTR_NULL };
	unsigned char	*buffer;
	size_t			cur, len;
	size_t			ilen;
	int				ret;

	uitoa_s(type_id, typeStr, 32, 16);

	make_string(&obj_path, "apps");
	cat_cstring_p(&obj_path, app_name);
	cat_cstring_p(&obj_path, "objs");
	cat_cstring_p(&obj_path, typeStr);

	tree_remove_children(hash_list);

	ret  = (get_file(obj_path.str, &buffer, &len) > 0) ? 1 : 0;
	ilen = strlen_c(objHash);

	if (ret)
	{
		cur	= 0;
		ret = 0;
		while (cur < len) 
		{
			char			chash[65];

			bin_2_hex(objHash, 32, chash);

			/*
			unsigned int	n = 0;
			
			while (n<32)
			{
				chash[n * 2 + 0] = hex_chars[objHash[n] >> 4];
				chash[n * 2 + 1] = hex_chars[objHash[n] & 0x0F];
				n++;
			}
			chash[64] = 0;
			*/

			if (!strncmp_c(chash, objHash, ilen))
			{
				mem_zone_ref	hashn = { PTR_NULL };

				ret = 1;

				if (hash_list != PTR_NULL)
				{
					if (tree_manager_add_child_node(hash_list, "hash", NODE_BITCORE_HASH, &hashn))
					{
						tree_manager_write_node_hash(&hashn, 0, &buffer[cur]);
						release_zone_ref(&hashn);
					}
				}
				else
					break;
			}
			cur += 32;
		}
		free_c(buffer);
	}
	free_string(&obj_path);

	return ret;
}

OS_API_C_FUNC(int) load_obj_childs(const char *app_name, const char *objHash, const char *KeyName, size_t first, size_t max, unsigned int opts,size_t *count,mem_zone_ref_ptr objs)
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
		size_t cur = first*32;
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
				/*
				unsigned int n;
				n = 0;
				while (n < 32)
				{
					oh[n * 2 + 0] = hex_chars[buffer[cur + n] >> 4];
					oh[n * 2 + 1] = hex_chars[buffer[cur + n] & 0x0F];
					n++;
				}
				oh[64] = 0;
				*/

				load_obj							(app_name, oh, "obj", 0, &newobj, addr);
				tree_manager_set_child_value_btcaddr(&newobj, "objAddr", addr);


				tree_manager_node_add_child (objs, &newobj);
				release_zone_ref			(&newobj);
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
		read_node(&obj_tx, tx_data,tx_len);
		free_c(tx_data);

		if (get_tx_output(&obj_tx, 0, &vout))
		{
			uint64_t	value;
			tree_manager_get_child_value_i64(&vout, NODE_HASH("value"), &value);
			if (objAddr != PTR_NULL)
			{
				struct string		objStr = { PTR_NULL };
				tree_manager_get_child_value_istr	(&vout, NODE_HASH("script"), &objStr, 0);
				get_out_script_address				(&objStr, PTR_NULL, objAddr);
				free_string							(&objStr);
			}
				

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

OS_API_C_FUNC(int) get_app_obj_addr (const char *app_name, unsigned int type_id, btc_addr_t objAddr,mem_zone_ref_ptr obj_list)
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
	free_string	(&obj_path);
	if (!ret)return 0;

	if (!tree_manager_find_child_node(&apps, NODE_HASH(app_name), NODE_BITCORE_TX, &app))return 0;

	get_app_types							(&app, &types);
	ret = tree_find_child_node_by_id_name	(&types, NODE_BITCORE_TX, "typeId", type_id, &type);

	release_zone_ref						(&app);
	release_zone_ref						(&types);

	if (!ret)return 0;


	cur = 0;
	ret = 0;
	while (cur < len)
	{
		char			chash[65];
		unsigned char	*tx_data;
		size_t			tx_len;

		bin_2_hex(&buffer[cur], 32, chash);

		/*
		unsigned int	n = 0;

		while (n<32)
		{
			chash[n * 2 + 0] = hex_chars[buffer[cur+n] >> 4];
			chash[n * 2 + 1] = hex_chars[buffer[cur+n] & 0x0F];
			n++;
		}
		chash[64] = 0;
		*/

		make_string		(&obj_path, "apps");
		cat_cstring_p	(&obj_path, app_name);
		cat_cstring_p	(&obj_path, "objs");
		cat_cstring_p	(&obj_path, chash);

		if (get_file(obj_path.str, &tx_data, &tx_len) > 0)
		{
			mem_zone_ref	obj_tx = { PTR_NULL }, vout = { PTR_NULL };

			tree_manager_create_node("tx", NODE_BITCORE_TX, &obj_tx);
			init_node				(&obj_tx);
			read_node				(&obj_tx, tx_data, tx_len);
			free_c					(tx_data);

			if (get_tx_output(&obj_tx, 0, &vout))
			{
				btc_addr_t		myAddr;
				struct string	objStr = { PTR_NULL };

				tree_manager_get_child_value_istr	(&vout, NODE_HASH("script"), &objStr, 0);
				get_out_script_address				(&objStr, PTR_NULL, myAddr);
				release_zone_ref					(&vout);

				if (!memcmp_c(objAddr, myAddr, sizeof(btc_addr_t)))
				{
					mem_zone_ref obj = { PTR_NULL };

					if (obj_new(&type, "myobj", &objStr, &obj))
					{
						tree_manager_node_add_child	(obj_list, &obj);
						release_zone_ref			(&obj);

						ret = 1;
					}
				}
				free_string(&objStr);
			}
			
			release_zone_ref(&obj_tx);
		}

		free_string	(&obj_path);

		cur += 32;
	}
	free_c			(buffer);
	release_zone_ref(&type);

	return ret;
}

OS_API_C_FUNC(int) load_obj(const char *app_name, const char *objHash, const char *objName, unsigned int opts, mem_zone_ref_ptr obj,btc_addr_t objAddr)
{
	mem_zone_ref	app = { PTR_NULL };
	struct string	obj_path = { 0 };
	unsigned char	*tx_data;
	size_t			tx_len;
	int				ret=0;



	if (!tree_manager_find_child_node(&apps, NODE_HASH(app_name), NODE_BITCORE_TX, &app))return 0;

	make_string		(&obj_path, "apps");
	cat_cstring_p	(&obj_path, app_name);
	cat_cstring_p	(&obj_path, "objs");
	cat_cstring_p	(&obj_path, objHash);

	if (get_file(obj_path.str, &tx_data, &tx_len)>0)
	{
		hash_t oh;
		mem_zone_ref obj_tx = { PTR_NULL }, vout = { PTR_NULL };
		unsigned int  time;
		tree_manager_create_node("tx", NODE_BITCORE_TX, &obj_tx);
		init_node				(&obj_tx);
		read_node				(&obj_tx,tx_data,tx_len);
		free_c					(tx_data);

		if (get_tx_output(&obj_tx, 0, &vout))
		{
			uint64_t	value;
			unsigned int type_id;

			tree_manager_get_child_value_i32(&obj_tx, NODE_HASH("time")	, &time);
			tree_manager_get_child_value_i64(&vout	, NODE_HASH("value"), &value);

			if ((value & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
			{
				struct string		pkey = { PTR_NULL };
				mem_zone_ref types = { PTR_NULL }, type = { PTR_NULL };
				
				type_id			= value & 0xFFFFFFFF;
				
				get_app_types	(&app, &types);
				
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

									/*
									unsigned int	n;
									n = 0;
									while (n < 32)
									{
										chash[n * 2 + 0] = hex_chars[hdata[n] >> 4];
										chash[n * 2 + 1] = hex_chars[hdata[n] & 0x0F];
										n++;
									}
									chash[64] = 0;
									*/

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
									release_zone_ref			(&mkey);
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
									
									load_obj_childs(app_name, objHash, keyname, 0, 10, 1, &count,key);
								}
							}
						}
					}
				}
				release_zone_ref(&types);
			}
			release_zone_ref(&vout);
			
			compute_tx_hash					 (&obj_tx, oh);
			tree_manager_set_child_value_hash(obj, "objHash", oh);
			tree_manager_set_child_value_i32 (obj, "time", time);
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

	memcpy_c		(buffer, &val, 4);
	memcpy_c		(&buffer[4], hash, 32);


	make_string		(&idx_path, "apps");
	cat_cstring_p	(&idx_path, app_name);
	cat_cstring_p	(&idx_path, "objs");
	cat_cstring_p	(&idx_path, typeStr);
	cat_cstring		(&idx_path, "_");
	cat_cstring		(&idx_path, keyname);
	cat_cstring		(&idx_path, ".idx");
	
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

	
	free_string	(&idx_path);
}

void add_index_str(const char *app_name, const char *typeStr, const char *keyname, const struct string *val, const hash_t hash)
{
	char			newval[512];
	struct string	idx_path = { 0 };
	unsigned char	*idx_buff;
	size_t			idx_len, cur;

	newval[0] = val->len;
	strcpy_cs		(&newval[1], 511, val->str);
	memcpy_c		(&newval[1 + val->len], hash, sizeof(hash_t));


	make_string		(&idx_path, "apps");
	cat_cstring_p	(&idx_path, app_name);
	cat_cstring_p	(&idx_path, "objs");
	cat_cstring_p	(&idx_path, typeStr);
	cat_cstring		(&idx_path, "_");
	cat_cstring		(&idx_path, keyname);
	cat_cstring		(&idx_path, ".idx");

	if (get_file(idx_path.str, &idx_buff, &idx_len) > 0)
	{
		cur = 0;
		while (cur < idx_len)
		{
			char			sval[256];
			unsigned char	sz = *((unsigned char *)(idx_buff + cur));

			strncpy_cs	(sval, 256, (idx_buff + cur + 1), sz);

			if (strcmp_c(val->str, sval)<0)
				break;

			cur += sizeof(hash_t) + sz + 1;
		}

		truncate_file	(idx_path.str, cur, newval, sizeof(hash_t) + val->len + 1);

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

	
	memcpy_c	(newval		, val , sizeof(btc_addr_t));
	memcpy_c	(&newval[34], hash, sizeof(hash_t));


	make_string		(&idx_path, "apps");
	cat_cstring_p	(&idx_path, app_name);
	cat_cstring_p	(&idx_path, "objs");
	cat_cstring_p	(&idx_path, typeStr);
	cat_cstring		(&idx_path, "_");
	cat_cstring		(&idx_path, keyname);
	cat_cstring		(&idx_path, ".idx");

	if (get_file(idx_path.str, &idx_buff, &idx_len) > 0)
	{
		cur = 0;
		while ((cur+ sizeof(hash_t) + sizeof(btc_addr_t)) <= idx_len)
		{
			if (memcmp_c(idx_buff + cur, val,sizeof(btc_addr_t))<0)
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

int find_index_str(const char *app_name, const char *typeStr, const char *keyname, const struct string *val, hash_t hash)
{
	struct string	idx_path = { 0 };
	unsigned char	*idx_buff;
	size_t			idx_len, cur;
	int ret=0;

	make_string		(&idx_path, "apps");
	cat_cstring_p	(&idx_path, app_name);
	cat_cstring_p	(&idx_path, "objs");
	cat_cstring_p	(&idx_path, typeStr);
	cat_cstring		(&idx_path, "_");
	cat_cstring		(&idx_path, keyname);
	cat_cstring		(&idx_path, ".idx");

	if (get_file(idx_path.str, &idx_buff, &idx_len) > 0)
	{
		cur = 0;
		while ( (cur + 1) < idx_len)
		{
			char			sval[256];
			unsigned char	sz = *((unsigned char *)(idx_buff + cur));
			
			if ((cur + 1 + sz + sizeof(hash_t)) > idx_len) break;

			strncpy_cs		(sval, 256, (idx_buff + cur + 1), sz);

			if (strcmp_c(val->str, sval) == 0){
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

	make_string		(&idx_path, "apps");
	cat_cstring_p	(&idx_path, app_name);
	cat_cstring_p	(&idx_path, "objs");
	cat_cstring_p	(&idx_path, typeStr);
	cat_cstring		(&idx_path, "_");
	cat_cstring		(&idx_path, keyname);
	cat_cstring		(&idx_path, ".idx");

	if (get_file(idx_path.str, &idx_buff, &idx_len) > 0)
	{
		cur = 0;
		while ((cur + sizeof(btc_addr_t) + sizeof(hash_t)) <= idx_len)
		{
			if (!memcmp_c(idx_buff + cur, val, sizeof(btc_addr_t))) {

				mem_zone_ref myhash = { PTR_NULL };

				tree_manager_add_child_node	 (hash_list, "hash", NODE_BITCORE_HASH, &myhash);
				tree_manager_write_node_hash(&myhash, 0, idx_buff + cur + sizeof(btc_addr_t));
				release_zone_ref			(&myhash);
			}
			cur += sizeof(hash_t) + sizeof(btc_addr_t);
		}
		free_c(idx_buff);
	}
	free_string(&idx_path);
	return 1;
}


OS_API_C_FUNC(int) store_block(mem_zone_ref_ptr header, mem_zone_ref_ptr tx_list)
{
	unsigned char		blkbuffer[512];
	char				chash[65];
	mem_zone_ref_ptr	tx = PTR_NULL;
	mem_zone_ref		my_list = { PTR_NULL };
	size_t				length, tx_ofset;
	uint64_t			height;
	hash_t				blk_hash;
	int					ret;
	struct string		signature = { 0 }, blk_path = { 0 }, blk_data_path = { 0 };
	unsigned int		n_tx, nc, block_time;
	

	if (!tree_manager_get_child_value_hash(header, NODE_HASH("blkHash"), blk_hash))return 0;


	bin_2_hex	(blk_hash, 32, chash);

	/*
	unsigned int n;
	n = 0;
	while (n<32)
	{
		chash[n * 2 + 0] = hex_chars[blk_hash[n] >> 4];
		chash[n * 2 + 1] = hex_chars[blk_hash[n] & 0x0F];
		n++;
	}
	chash[64] = 0;
	*/

	make_string		(&blk_path, "blks");
	cat_ncstring_p	(&blk_path, chash + 0, 2);
	create_dir		(blk_path.str);

	cat_ncstring_p	(&blk_path, chash + 2, 2);
	create_dir		(blk_path.str);

	cat_cstring_p	(&blk_path, chash);
	
	nc	   = tree_manager_get_node_num_children(tx_list);
	height = get_last_block_height() + 1;
	length = 80+8+4+33+80+33; /* hdr size  + hght + ntx + pos/pow + sig + stakem*/


	write_node	(header, (unsigned char *)blkbuffer);

	*((uint64_t *)(blkbuffer+80))		=height;
	*((unsigned int *)(blkbuffer+88))	=nc;

	if (tree_manager_get_child_value_istr(header, NODE_HASH("signature"), &signature, 0))
	{
		*((unsigned char *)(blkbuffer+92))=signature.len;
		memcpy_c(blkbuffer+93,signature.str,signature.len);
		free_string(&signature);
	}

	if (tree_manager_get_child_value_hash(header, NODE_HASH("blk pow"), blkbuffer+173)) {
		*((unsigned char *)(blkbuffer+172))	= 1;
	}
	else {
		memset_c	(blkbuffer + 172, 0, 33);
	}

	memset_c		(blkbuffer + 205, 0, 33);

	clone_string	(&blk_data_path		, &blk_path);
	cat_cstring		(&blk_data_path		, "_blk");
	ret=put_file	(blk_data_path.str	, blkbuffer, length);

	if (ret!=length)
	{
		log_output("bad write block\n");
		free_string(&blk_path);
		return 0;
	}

	if (stat_file(blk_data_path.str) != 0)
	{
		log_output("bad write block\n");
		free_string(&blk_path);
		return 0;
	}

	if (tree_manager_get_child_value_i32(header, NODE_HASH("time"), &block_time))
		set_ftime(blk_data_path.str, block_time);

	free_string(&blk_data_path);

	if (nc <= 0)
	{
		free_string(&blk_path);
		return 0;
	}
	
	clone_string(&blk_data_path, &blk_path);
	cat_cstring (&blk_data_path, "_txs");

	tx_ofset = 0;

	for (n_tx = 0, tree_manager_get_first_child(tx_list, &my_list, &tx); ((tx != NULL) && (tx->zone != NULL)); tree_manager_get_next_child(&my_list, &tx), n_tx++)
	{
		hash_t				tx_hash, pObjHash;
		struct string		app_name = { 0 };
		unsigned int		tx_time, app_item;
		unsigned char		*buffer;

		length		= get_node_size(tx);
		buffer		= (unsigned char *)malloc_c(length);
		write_node	(tx, (unsigned char *)buffer);

		if (!tree_manager_get_child_value_hash(tx, NODE_HASH("txid"), tx_hash))
		{
			hash_t		tmp_hash;

			mbedtls_sha256						((unsigned char *)buffer, length, tmp_hash, 0);
			mbedtls_sha256						(tmp_hash, 32, tx_hash, 0);

			tree_manager_set_child_value_hash	(tx, "txid", tx_hash);
		}

		if (!tree_manager_get_child_value_i32(tx, NODE_HASH("time"), &tx_time))
			tx_time = block_time;
		
		if (tree_manager_get_child_value_istr(tx, NODE_HASH("AppName"), &app_name, 0))
		{
			mem_zone_ref	app_tx = { PTR_NULL };
			struct string	app_path = { 0 };

			make_string		(&app_path, "apps");
			cat_cstring_p	(&app_path, app_name.str);
			create_dir		(app_path.str);

			cat_cstring_p	(&app_path, "app");
			put_file		(app_path.str, buffer, length);
			free_string		(&app_path);

			make_string		(&app_path, "apps");
			cat_cstring_p	(&app_path, app_name.str);
			cat_cstring_p	(&app_path, "types");
			create_dir		(app_path.str);
			free_string		(&app_path);

			make_string		(&app_path, "apps");
			cat_cstring_p	(&app_path, app_name.str);
			cat_cstring_p	(&app_path, "objs");
			create_dir		(app_path.str);
			free_string		(&app_path);

			make_string		(&app_path, "apps");
			cat_cstring_p	(&app_path, app_name.str);
			cat_cstring_p	(&app_path, "layouts");
			create_dir		(app_path.str);
			free_string		(&app_path);

			make_string		(&app_path, "apps");
			cat_cstring_p	(&app_path, app_name.str);
			cat_cstring_p	(&app_path, "datas");
			create_dir		(app_path.str);
			free_string		(&app_path);

			make_string		(&app_path, "apps");
			cat_cstring_p	(&app_path, app_name.str);
			cat_cstring_p	(&app_path, "modz");
			create_dir		(app_path.str);
			free_string		(&app_path);

			tree_manager_create_node		(app_name.str, NODE_BITCORE_TX, &app_tx);
			tree_manager_copy_children_ref	(&app_tx, tx);
			add_app_tx						(&app_tx, app_name.str);
			release_zone_ref				(&app_tx);

			free_string						(&app_name);
		}
		else if (tree_manager_get_child_value_i32(tx, NODE_HASH("app_item"), &app_item))
		{
			struct string	app_name = { 0 };
			mem_zone_ref	app = { PTR_NULL };
			switch (app_item)
			{
				case 1:
					if(tree_manager_get_child_value_istr(tx, NODE_HASH("appType"), &app_name, 0))
					{
						char			tchash[65];
						struct string	app_path = { 0 };


						bin_2_hex	(tx_hash, 32, tchash);
						/*
						n = 32;
						while (n--)
						{
							tchash[n * 2 + 0] = hex_chars[tx_hash[n] >> 4];
							tchash[n * 2 + 1] = hex_chars[tx_hash[n] & 0x0F];
						}
						tchash[64] = 0;
						*/

	
						make_string							(&app_path, "apps");
						cat_cstring_p						(&app_path, app_name.str);
						cat_cstring_p						(&app_path, "types");
						cat_cstring_p						(&app_path, tchash);
						put_file							(app_path.str, buffer, length);
						free_string							(&app_path);

						if (tree_node_find_child_by_name(&apps, app_name.str, &app))
						{
							add_app_tx_type	(&app, tx);
							release_zone_ref(&app);
						}
					}
					free_string(&app_name);
				break;
				case 2:
					if (tree_manager_get_child_value_istr(tx, NODE_HASH("appObj"), &app_name, 0))
					{
						char			tchash[65];
						char			typeStr[16];
						mem_zone_ref	idxs = { 0 };
						struct string	app_path = { 0 };
						unsigned int	typeID;

						bin_2_hex(tx_hash, 32, tchash);

						/*
						n = 32;
						while (n--)
						{
							tchash[n * 2 + 0] = hex_chars[tx_hash[n] >> 4];
							tchash[n * 2 + 1] = hex_chars[tx_hash[n] & 0x0F];
						}
						tchash[64] = 0;
						*/
						
						tree_manager_get_child_value_i32(tx, NODE_HASH("objType"), &typeID);

						uitoa_s				(typeID, typeStr, 16, 16);


						make_string			(&app_path		, "apps");
						cat_cstring_p		(&app_path		, app_name.str);
						cat_cstring_p		(&app_path		, "objs");
						cat_cstring_p		(&app_path		, tchash);
						put_file			(app_path.str	, buffer, length);
						free_string			(&app_path);

						make_string			(&app_path		, "apps");
						cat_cstring_p		(&app_path		, app_name.str);
						cat_cstring_p		(&app_path		, "objs");
						cat_cstring_p		(&app_path		, typeStr);
						append_file			(app_path.str	, tx_hash, 32);
						free_string			(&app_path);

						add_index			(app_name.str, typeStr, "time", tx_time, tx_hash);


						if (tree_manager_create_node("idxs", NODE_JSON_ARRAY, &idxs))
						{
							mem_zone_ref		m_idlist = { 0 }, obj = { 0 };
							mem_zone_ref_ptr	idx = PTR_NULL;
					
							tree_manager_find_child_node	(tx, NODE_HASH("objDef"), typeID, &obj);

							get_app_type_idxs				(app_name.str, typeID, &idxs);

							for (tree_manager_get_first_child(&idxs, &m_idlist, &idx); ((idx != NULL) && (idx->zone != NULL)); tree_manager_get_next_child(&m_idlist, &idx))
							{
								const char *id_name;
								id_name = tree_mamanger_get_node_name(idx);
								switch (tree_mamanger_get_node_type(idx))
								{
									case NODE_GFX_INT:
									{
										unsigned int val;
										tree_manager_get_child_value_i32	(&obj, NODE_HASH(id_name), &val);
										add_index							(app_name.str, typeStr, id_name, val, tx_hash);
									}
									break;
									case NODE_BITCORE_VSTR:
									{
										struct string val = { 0 };
										tree_manager_get_child_value_istr	(&obj, NODE_HASH(id_name), &val,0);
										add_index_str						(app_name.str, typeStr, id_name, &val, tx_hash);
									}
									break;
									case NODE_BITCORE_WALLET_ADDR:
									{
										btc_addr_t addr;
										tree_manager_get_child_value_btcaddr(&obj, NODE_HASH(id_name), addr);
										add_index_addr						(app_name.str, typeStr, id_name, addr, tx_hash);
									}
									break;
									case NODE_BITCORE_PUBKEY:
									{
										btc_addr_t		addr;
										unsigned char	*pubkey;
										unsigned int	sz;

										sz = tree_manager_get_child_data_ptr(&obj, NODE_HASH(id_name), &pubkey);
										key_to_addr							(pubkey, addr);
										add_index_addr						(app_name.str, typeStr, id_name, addr, tx_hash);
									}
									break;
								}
							}
							release_zone_ref	(&idxs);
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

								memcpy_c		(&buffer[32], tx_hash, sizeof(hash_t));
								make_string		(&app_path, "apps");
								cat_cstring_p	(&app_path, app_name.str);
								cat_cstring_p	(&app_path, "datas");
								cat_cstring_p	(&app_path, "index");

								if (stat_file(app_path.str)==0)
									append_file		(app_path.str, buffer, 64);
								else
									put_file		(app_path.str, buffer, 64);

								free_string		(&app_path);
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

							make_string		(&app_path, "apps");
							cat_cstring_p	(&app_path, app_name.str);
							cat_cstring_p	(&app_path, "layouts");
							cat_cstring_p	(&app_path, filename.str);
							put_file		(app_path.str, fileData.str, fileData.len);
							free_string		(&app_path);
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

							make_string		(&app_path, "apps");
							cat_cstring_p	(&app_path, app_name.str);
							cat_cstring_p	(&app_path, "modz");
							cat_cstring_p	(&app_path, filename.str);
							put_file		(app_path.str, fileData.str, fileData.len);
							

							tree_manager_find_child_node	(&apps, NODE_HASH(app_name.str), NODE_BITCORE_TX, &app);

							if ((filename.len >= 5) && (!strncmp_c(&filename.str[filename.len-5],".site",5)))
							{
								mem_zone_ref scripts = { PTR_NULL }, script = { PTR_NULL };

								get_app_scripts				(&app, &scripts);
								tree_remove_child_by_name	(&scripts, NODE_HASH(filename.str));
								release_zone_ref			(&scripts);

								if (load_script(app_path.str, filename.str, &script, 1))
								{
									ctime_t ftime;

									get_ftime						(app_path.str, &ftime);
									tree_manager_write_node_dword	(&script, 0, ftime);
									add_app_script					(&app, &script);
									release_zone_ref				(&script);
								}
							}
							release_zone_ref			(&app);
							free_string					(&app_path);
						}
						release_zone_ref(&file);
					}
				break;
			}
		}
		else if (tree_manager_get_child_value_hash(tx, NODE_HASH("appChildOf"), pObjHash))
		{
			char			key[32];
			hash_t			child_obj;
			mem_zone_ref	obj			= { PTR_NULL };
			struct string   child_path	= { PTR_NULL };
			char			pObj[65];
			

			tree_manager_get_child_value_str (tx,  NODE_HASH("appChildKey")	, key, 32, 16);
			tree_manager_get_child_value_hash(tx,  NODE_HASH("newChild")	, child_obj);
			tree_manager_get_child_value_istr(tx,  NODE_HASH("appChild")	, &app_name,	0);


			bin_2_hex(pObjHash, 32, pObj);

			/*
			n = 0;
			while (n<32)
			{
				pObj[n * 2 + 0] = hex_chars[pObjHash[n] >> 4];
				pObj[n * 2 + 1] = hex_chars[pObjHash[n] & 0x0F];
				n++;
			}
			pObj[64] = 0;
			*/

			make_string  (&child_path, "apps");
			cat_cstring_p(&child_path, app_name.str);
			cat_cstring_p(&child_path, "objs");
			cat_cstring_p(&child_path, pObj);
			cat_cstring  (&child_path, "_");
			cat_cstring	 (&child_path, key);

			append_file	 (child_path.str, child_obj, 32);

			free_string	 (&child_path);
			free_string	 (&app_name);
		}

				
		store_tx_blk_index	(tx_hash			, blk_hash, height, tx_ofset, tx_time);
		append_file			(blk_data_path.str	, &length, 4);
		append_file			(blk_data_path.str	, buffer, length);
		free_c				(buffer);
		
		tx_ofset		+= (length + 4);

		if (is_tx_null(tx) == 1)
			continue;

		if(is_app_root(tx))
		{
			blk_store_app_root	(tx);
			set_root_app		(tx);
		}
		else
		{
			ret = store_tx_inputs(tx);
			if (ret)ret = store_tx_outputs(tx);
		}

		if (!ret)
		{
			dec_zone_ref	(tx);
			release_zone_ref(&my_list);
			break;
		}
	}

	free_string		(&blk_data_path);
	free_string		(&blk_path);

	return ret;
}


