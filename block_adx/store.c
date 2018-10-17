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



extern int tx_is_app_child(hash_t txh, unsigned int oidx,struct string *app_name);
extern int add_app_tx(mem_zone_ref_ptr new_app, const char *app_name);
extern int get_script_data(const struct string *script, size_t *offset, struct string *data);
extern int get_script_file(struct string *script, mem_zone_ref_ptr file);
extern int obj_new(mem_zone_ref_ptr type, const char *objName, struct string *script, mem_zone_ref_ptr obj);
extern int add_app_tx_type(mem_zone_ref_ptr app, mem_zone_ref_ptr typetx);
extern int add_app_script(mem_zone_ref_ptr app, mem_zone_ref_ptr script);
extern int store_app_obj_child(mem_zone_ref_const_ptr tx, const hash_t pObjHash);
extern int store_new_app(const struct string *app_name, mem_zone_ref_const_ptr tx, mem_ptr buffer, size_t length);
extern int store_app_item(mem_zone_ref_const_ptr tx, unsigned int app_item, unsigned int tx_time, mem_ptr buffer, size_t length);
extern unsigned int compute_merkle_round(mem_zone_ref_ptr hashes, int cnt);
extern int add_sorted_block(uint64_t src_h, uint64_t block_height);
extern int blk_store_app_root(mem_zone_ref_ptr tx);
extern void add_obj_txfr(const char *app_name, const char *typeStr, const hash_t objHash, const hash_t txHash);
//local module
extern hash_t		null_hash;

extern void rm_hash_from_obj_txfr(const char *app_name, unsigned int type_id, hash_t hash);

btc_addr_t			src_addr_list[1024] = { 0xABCDEF };


mem_zone_ref block_index_node = { PTR_INVALID };
mem_zone_ref time_index_node = { PTR_INVALID };
mem_zone_ref block_rindex_node = { PTR_INVALID };

extern unsigned int	has_root_app;
extern btc_addr_t	root_app_addr;
extern hash_t		app_root_hash;
extern mem_zone_ref	apps;

#define BLOCK_STORE_SIZE 238



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
	size_t my_size;

	my_size = file_size("blk_indexes");
	if (my_size < 32)
		return 0;

	return (my_size / 32);
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


/*

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
OS_API_C_FUNC(int) find_hash(hash_t hash)
{
	char				file_name[65];
	struct string		blk_path = { PTR_NULL };
	int					ret;

	bin_2_hex(hash, 32, file_name);

	make_blk_path	(file_name,&blk_path);
	cat_cstring		(&blk_path, "_blk");

	ret = (stat_file(blk_path.str) == 0) ? 1 : 0;

	free_string(&blk_path);
	return ret;
}
*/

OS_API_C_FUNC(int) blk_load_tx_ofset(unsigned int ofset, mem_zone_ref_ptr tx)
{
	unsigned char		*tx_data;
	size_t				tx_data_len;
	int					ret;

	ret = 0;
	if (get_file_chunk("transactions",ofset, &tx_data, &tx_data_len) > 0)
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

	ret = blk_load_tx_ofset(ofset, tx);

	tree_manager_get_child_value_hash(tx,NODE_HASH("txid"),th);

	if(memcmp_c(th,tx_hash,sizeof(hash_t)!=0))
	{
		ret=0;
		log_message("error chcking tx hash %txid% ",tx);
	}


	return ret;
}

OS_API_C_FUNC(int) load_tx_addresses(btc_addr_t addr, mem_zone_ref_ptr tx_hashes,size_t first, size_t num)
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
			while ((num>0)&&(nn < ntx))
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
						num--;
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
	size_t			sZ;
	
	get_utxo_path	(txh,oidx,&tx_path);

	//ret=(stat_file	(tx_path.str)==0)? 1 : 0;

	sZ = file_size	(tx_path.str);
	
	if (sZ == (sizeof(btc_addr_t) + sizeof(uint64_t)))
		ret = 1;
	else if (sZ == (sizeof(btc_addr_t) + sizeof(hash_t) + sizeof(uint64_t)))
		ret = 2;
	else
		ret = 0;

	free_string(&tx_path);

	return ret;
}

int load_utxo(const char *txh,unsigned int oidx,uint64_t *amount,btc_addr_t addr,hash_t objh)
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
			if ((objh!=PTR_NULL)&&(len >= (sizeof(uint64_t) + sizeof(hash_t) + sizeof(btc_addr_t))))
			{
				memcpy_c(objh, buffer + sizeof(uint64_t) + sizeof(btc_addr_t), sizeof(hash_t));
				ret = 2;
			}
			free_c(buffer);
		}
	}
	free_string(&tx_path);
	return ret;
}


int store_tx_vout(const char *txh,mem_zone_ref_ptr txout_list,unsigned int oidx, btc_addr_t out_addr)
{
	hash_t				objHash;
	struct string		script = { 0 }, tx_path = { 0 } ;
	mem_zone_ref		vout = {PTR_NULL};
	uint64_t			amount;
	int					ret,isobj;

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
	ret   = tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &script, 16);

	if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
		isobj = tree_manager_get_child_value_hash(&vout, NODE_HASH("objHash"), objHash);
	else
		isobj = 0;

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
	if (ret)
	{
		if (((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)&&(!isobj))
		{
			struct string strHash = { 0 };
			if (get_out_script_return_val(&script, &strHash))
			{
				if (strHash.len == sizeof(hash_t))
				{
					isobj = 1;
					memcpy_c(objHash, strHash.str, sizeof(hash_t));
				}
				else
				{
					isobj = 1;

					hex_2_bin(txh,objHash,sizeof(hash_t));
				}
				free_string(&strHash);
			}
		}
	}


	if(ret)
	{
		get_utxo_path(txh, oidx, &tx_path);
		if (!isobj)
		{
			unsigned char		bbuffer[64];

			*((uint64_t *)(bbuffer)) = amount;
			memcpy_c(bbuffer + sizeof(uint64_t), out_addr, sizeof(btc_addr_t));
			
			put_file(tx_path.str, bbuffer, sizeof(uint64_t) + sizeof(btc_addr_t));

			ret = 1;
		}
		else
		{
			unsigned char		bbuffer[128];

			*((uint64_t *)(bbuffer)) = amount;
			memcpy_c(bbuffer + sizeof(uint64_t), out_addr, sizeof(btc_addr_t));
			memcpy_c(bbuffer + sizeof(uint64_t) + sizeof(btc_addr_t), objHash, sizeof(hash_t));
			
			put_file(tx_path.str, bbuffer, sizeof(uint64_t) + sizeof(hash_t) + sizeof(btc_addr_t));

			ret = 2;
		}
		free_string(&tx_path);

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

OS_API_C_FUNC(int) remove_tx_index(hash_t tx_hash)
{
	char tchash[65];
	struct string tx_path = { 0 };
	unsigned char *buffer;
	size_t size;
	unsigned int ret, n;

	bin_2_hex(tx_hash, 32, tchash);

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
		bin_2_hex							(h, 32, tchash);

		//tree_manager_get_child_value_str	(tx,NODE_HASH("txid"),tchash,65,16);
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
					del_utxo(tchash, oidx);
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
				struct string	oscript = { 0 }, key = { 0 }, cHash = { 0 }, DataStr;
				size_t			offset = 0;
				int				ret;

				tree_manager_get_child_value_str(tx, NODE_HASH("appChildOf"), objHash, 65, 0);
				get_tx_output(tx, 0, &vout);
				tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &oscript, 0);


				if (get_out_script_return_val(&oscript, &DataStr))
				{
					uint64_t amount;
					
					if (!tree_manager_get_child_value_i64(&vout, NODE_HASH("value"), &amount))
						amount = 0;

					if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
					{
						char buff[16];
						struct string obj_path = { 0 };
						unsigned int type_id = amount & 0xFFFFFFFF;

						uitoa_s(type_id, buff, 16, 16);

						rm_hash_from_obj_txfr(app_name, type_id, DataStr.str);

						make_string(&obj_path, "apps");
						cat_cstring_p(&obj_path, app_name);
						cat_cstring_p(&obj_path, "objs");
						cat_cstring_p(&obj_path, buff);
						cat_cstring(&obj_path, "_addr.idx");
						rm_hash_from_index_addr(obj_path.str, DataStr.str);
						free_string(&obj_path);

						del_utxo(tchash, oidx);
					}
					free_string(&DataStr);

				}
				else
				{
					ret = get_script_data(&oscript, &offset, &key);
					if (ret)ret = get_script_data(&oscript, &offset, &cHash);
					ret = (cHash.len == 32) ? 1 : 0;
					if (ret)
					{
						rm_child_obj(app_name, objHash, key.str, cHash.str);
					}
				}
				
				free_string(&key);
				free_string(&cHash);
				free_string(&oscript);

				continue;
			}
		}
		
		if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
		{
			struct string	DataStr = { 0 };
			mem_zone_ref	vout = { PTR_NULL };
			if (get_tx_output(tx, oidx, &vout))
			{
				struct string	oscript = { 0 };
				
				tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &oscript, 0);

				if (get_out_script_return_val(&oscript, &DataStr))
				{
					char appName[32];

					if ((DataStr.len == 32)&& (get_obj_app(DataStr.str, appName)))
					{
						char buff[16];
						struct string obj_path = { 0 };
						unsigned int type_id = amount & 0xFFFFFFFF;

						uitoa_s(type_id, buff, 16, 16);

						rm_hash_from_obj_txfr(appName, type_id, DataStr.str);

						make_string(&obj_path, "apps");
						cat_cstring_p(&obj_path, appName);
						cat_cstring_p(&obj_path, "objs");
						cat_cstring_p(&obj_path, buff);
						cat_cstring(&obj_path, "_addr.idx");
						rm_hash_from_index_addr(obj_path.str, DataStr.str);
						free_string(&obj_path);
					}
					free_string(&DataStr);
				}
				free_string(&oscript);
				release_zone_ref(&vout);
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
	hash_t txh;
	mem_zone_ref	 txin_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr input = PTR_NULL;
	int				 ret;


	if (!tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))return 0;

	tree_manager_get_child_value_hash(tx, NODE_HASH("txid"), txh);

	//process tx inputs
	for (tree_manager_get_first_child(&txin_list, &my_list, &input); ((input != NULL) && (input->zone != NULL)); tree_manager_get_next_child(&my_list, &input))
	{
		mem_zone_ref	 ptx = { PTR_NULL };
		hash_t			 prev_hash, pblk_hash;
		unsigned int	 oidx;
		unsigned char	 app_item;

		tree_manager_get_child_value_hash(input, NODE_HASH("txid"), prev_hash);
		tree_manager_get_child_value_i32 (input, NODE_HASH("idx"), &oidx);
		if (!memcmp_c(prev_hash, app_root_hash, sizeof(hash_t)))
		{
			continue;
		}

		if (tx_is_app_item(prev_hash, oidx, &ptx, &app_item))
		{
			if (app_item == 2)
			{
				mem_zone_ref vout = { PTR_NULL };
				uint64_t	amount;

				get_tx_output(&ptx, oidx, &vout);

				tree_manager_get_child_value_i64(&vout, NODE_HASH("value"), &amount);

				if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
				{
					char		appName[32];
					if (get_obj_app(prev_hash, appName))
					{
						btc_addr_t	out_addr;
						struct string oscript = { 0 };
						mem_zone_const_ref	ptxos = { 0 };

						ret = tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &oscript, 0);
						if(ret)ret=get_out_script_address(&oscript, PTR_NULL, out_addr);
						free_string(&oscript);

						if (ret)ret = add_index_addr(appName, amount & 0xFFFFFFFF, "addr", out_addr, prev_hash);
						if (ret)ret = tree_manager_find_child_node(&ptx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &ptxos);
						if (ret)ret = store_tx_vout(prev_hash, &ptxos, oidx, out_addr);
						release_zone_ref(&ptxos);
					}
				}
				release_zone_ref(&vout);
			}

			release_zone_ref(&ptx);
			continue;
		}
			/* load the transaction with the spent output */
		
		
		if (load_tx(&ptx, pblk_hash, prev_hash))
		{
			char			 ptxh[65];
			btc_addr_t		 out_addr;
			mem_zone_ref	 txout_list = { PTR_NULL };
			int			  	 n;

			/*rewrite the original tx out from the parent transaction*/

			bin_2_hex(prev_hash, 32, ptxh);

			ret=tree_manager_find_child_node (&ptx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list);

			if (ret)
			{
				ret = store_tx_vout(ptxh, &txout_list, oidx, out_addr);
				if (ret == 2)
				{
					mem_zone_ref vout = { PTR_NULL };
					uint64_t	amount;

					get_tx_output(&ptx, oidx, &vout);
					
					if (!tree_manager_get_child_value_i64(&vout, NODE_HASH("value"), &amount))
						amount = 0;

					if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
					{
						struct string oscript = { 0 }, DataStr = { PTR_NULL };

						tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &oscript, 0);
						if (get_out_script_return_val(&oscript, &DataStr))
						{
							char appName[32];
							char typeStr[16];

							uitoa_s(amount & 0xFFFFFFFF, typeStr, 16, 16);

							if (DataStr.len != 32)
							{
								if (get_obj_app(prev_hash, appName))
									add_index_addr(appName, typeStr, "addr", out_addr, prev_hash);
							}
							else if (get_obj_app(DataStr.str, appName))
							{
								add_index_addr(appName, typeStr, "addr", out_addr, DataStr.str);
								add_obj_txfr  (appName, typeStr, DataStr.str, ptxh);
							}

	


							free_string(&DataStr);
						}
						free_string(&oscript);
					}
					release_zone_ref(&vout);
				}
			}

			release_zone_ref(&txout_list);
			release_zone_ref(&ptx);
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



OS_API_C_FUNC(int) get_blk_txs(const hash_t blk_hash,uint64_t block_height, mem_zone_ref_ptr txs, size_t max)
{
	uint64_t		height;
	unsigned char	*data;
	size_t			len, ntx,cnt;
	uint64_t		tx_offset;

	if (!find_block_hash(blk_hash, block_height, &height))
		return 0;
	
	if (get_file_chunk("blocks", mul64(height, 512), &data, &len) <= 0)
		return 0;

	tx_offset = *((uint64_t *)(data + 80));

	free_c(data);

	if (get_file_chunk("transactions", tx_offset, &data, &len) <= 0)
		return 0;

	ntx = 0;
	cnt = 0;
	while ((ntx+32) <= len)
	{
		mem_zone_ref tx = { PTR_NULL };
		if (tree_manager_add_child_node(txs, "tx", NODE_BITCORE_HASH, &tx))
		{
			tree_manager_write_node_hash(&tx, 0, &data[ntx]);
			release_zone_ref(&tx);
			cnt++;
		}
		if (cnt >= max)break;

		ntx += 32;
	}
	free_c(data);
	

	return 1;
}


OS_API_C_FUNC(int) load_blk_txs(const hash_t blk_hash, mem_zone_ref_ptr txs)
{
	uint64_t		block_height, height;
	uint64_t		tx_offset;
	unsigned char	*data;
	size_t			n,len, ntx;
	int				ret=1;

	block_height = get_last_block_height();

	if (!find_block_hash(blk_hash, block_height, &height))
		return 0;

	if (get_file_chunk("blocks", mul64(height, 512), &data, &len) <= 0)
		return 0;

	tx_offset	= *((uint64_t *)(data + 80));
	ntx			= *((unsigned int *)(data + 88));
	free_c(data);

	tx_offset += (ntx * 32) + 4;

	for (n = 0;n < ntx; n++)
	{
		mem_zone_ref	tx = { PTR_NULL };
		unsigned int	tx_size;

		if (!blk_load_tx_ofset(tx_offset, &tx))
		{
			ret = 0;
			break;
		}
		tree_manager_get_child_value_i32(&tx, NODE_HASH("size"), &tx_size);
		tree_manager_node_add_child(txs, &tx);
		release_zone_ref(&tx);

		tx_offset += (tx_size + 4);

		/*
		if (get_file_chunk("transactions", tx_offset, &data, &tx_size) <= 0)
		{
			release_zone_ref(&tx);
			ret = 0;
			break;
		}

		init_node(&tx);
		read_node(&tx, data, tx_size);

		mbedtls_sha256(data, tx_size, tmph, 0);
		mbedtls_sha256(tmph, 32, txh, 0);

		tree_manager_set_child_value_i32(&tx, "size", tx_size);
		tree_manager_set_child_value_hash(&tx, "txid", txh);

		tx_offset += (tx_size + 4);

		free_c(data);
		*/
	}
	

	return ret;
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

	ret = 1;

	bin_2_hex(thash, 32, tx_hash);

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
		else if (!memcmp_c(prev_hash, app_root_hash, sizeof(hash_t)))
		{
			continue;
		}
		else if (tree_find_child_node_by_member_name_hash(&apps, NODE_BITCORE_TX, "txid", prev_hash, PTR_NULL))
		{
			continue;
		}
		else if (tree_manager_get_child_value_i32(input, NODE_HASH("isObjChild"),&objChild))
		{
			hash_t ch;
			if(tree_manager_get_child_value_hash(tx, NODE_HASH("newChild"), ch))
				continue;
		}
		
			
		bin_2_hex(prev_hash, 32, phash);

		ret=load_utxo	(phash,oidx,&amount,addr,PTR_NULL);

		if(ret)
		{
			del_utxo								(phash, oidx);
			tree_manager_set_child_value_i64		(input, "amount" , amount);
			tree_manager_set_child_value_btcaddr	(input, "srcaddr", addr);
			store_tx_addresses						(addr, thash);
		}

		if (!ret)
		{
			dec_zone_ref(input);
			release_zone_ref(&my_list);
			break;
		}
	}
	release_zone_ref(&txin_list);
	return ret;
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
		uint64_t		amount;
		if (tree_manager_find_child_node(tx, NODE_HASH("AppName"), NODE_GFX_STR, PTR_NULL))
		{
			struct string	script = { PTR_NULL };
			mem_zone_ref	vout = { 0 };

			tree_manager_get_child_at				(&txout_list, oidx, &vout);
			ret = tree_manager_get_child_value_istr (&vout, NODE_HASH("script"), &script, 16);
			release_zone_ref						(&vout);

			if(ret)
			{
				struct string my_val = { PTR_NULL };
				int myret;
				myret = get_out_script_return_val(&script, &my_val);
				free_string(&script);
				if (myret)
				{
					free_string(&my_val);
					continue;
				}
			}
		}
		else if (tree_manager_get_child_value_i32(tx, NODE_HASH("app_item"), &app_item))
		{
			ret=get_tx_output_amount(tx, oidx, &amount);
			if (amount == 0)continue;
			if (amount == 0xffffffffffffffff)continue;

			
			if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
			{
				struct string	script = { PTR_NULL };
				mem_zone_ref	vout = { 0 };

				tree_manager_get_child_at(&txout_list, oidx, &vout);
				ret = tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &script, 16);
				release_zone_ref(&vout);
				if(ret)ret=get_out_script_address(&script, PTR_NULL, out_addr);
			}
				
		}
		else if (tree_manager_get_child_value_i32(tx, NODE_HASH("childOf"), &childOf))
		{
			ret = get_tx_output_amount(tx, oidx, &amount);
			if (amount == 0)continue;
		}
		else
			ret = 1;

		if(ret)
		{
			if (store_tx_vout(tx_hash, &txout_list, oidx, out_addr))
				ret = store_tx_addresses(out_addr, thash);

			if (!ret)log_output("store_tx_addresses error\n");
		}

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



OS_API_C_FUNC(int) load_block_time(const hash_t blkHash, ctime_t *time)
{
	mem_zone_ref hdr = { PTR_NULL };
	unsigned int mytime = 0;
	int ret=0;

	if (!load_blk_hdr(&hdr,blkHash))
		return 0;

	if (tree_manager_get_child_value_i32(&hdr, NODE_HASH("time"), &mytime))
	{
		*time = mytime;
		ret = 1;
	}

	release_zone_ref(&hdr);
	
	return ret;
}

OS_API_C_FUNC(int) is_pow_block_at(uint64_t height)
{
	unsigned char	*blk_data;
	size_t			len;
	int				ret = 0;

	if (get_file_chunk("blocks", mul64(height, 512), &blk_data, &len) <= 0)
		return 0;

	if (len >= 205)
	{
		if ((*((unsigned char *)(blk_data + 172))) == 1)
			ret = 1;
	}
	free_c(blk_data);

	return ret;
}


OS_API_C_FUNC(int) is_pow_block(const hash_t blk_hash)
{
	uint64_t		height,block_height;
	unsigned char	*blk_data;
	size_t			len;
	int				ret=0;

	block_height = get_last_block_height();

	if (!find_block_hash(blk_hash, block_height, &height))
		return 0;

	return is_pow_block_at(height);
}


OS_API_C_FUNC(unsigned int) get_blk_ntxs(const hash_t blk_hash,uint64_t block_height)
{
	uint64_t		height;
	unsigned char	*blk_data;
	unsigned int	ntx;
	size_t			len;
	int				ret = 0;

	if (!find_block_hash(blk_hash, block_height, &height))
		return 0;

	if (get_file_chunk("blocks", mul64(height, 512), &blk_data, &len) <= 0)
		return 0;

	if (len >= 128)
		ntx = *((unsigned int *)(blk_data + 88));
	else
		ntx = 0;

	free_c(blk_data);

	return ntx;
}

OS_API_C_FUNC(int) get_tx_blk_height(const hash_t tx_hash, uint64_t *height, uint64_t *block_time, unsigned int *tx_time)
{
	hash_t blk_hash;
	struct string blk_path = { PTR_NULL };
	ctime_t ctime;
	unsigned int n;

	if (!find_blk_hash(tx_hash, blk_hash,height, PTR_NULL,tx_time))
		return 0;

	
	if (block_time != PTR_NULL)
	{
		load_block_time(blk_hash, &ctime);
		*block_time = ctime;
	}
	return 1;
}


OS_API_C_FUNC(int) get_block_size(const hash_t blk_hash, size_t *size)
{
	uint64_t		block_height, height, tx_offset;
	size_t			len;
	unsigned char	*data;
	unsigned int	ntx,signlen;
	unsigned int	n;
	int				ret = 0;

	block_height = get_last_block_height();

	if (!find_block_hash(blk_hash, block_height+1, &height))
		return 0;

	if (get_file_chunk("blocks", mul64(height, 512), &data, &len)<=0)
		return 0;
	
	if(len>=96)
	{
		tx_offset = *((uint64_t *)(data + 80));
		ntx		=	*((unsigned int *)(data+88));
		signlen =	*((unsigned char *)(data+92));
		ret		=	1;
	}
	free_c			(data);

	if (!ret)
		return 0;

	tx_offset += ntx * 32 + 4;
	
	if (ntx<0xFD)
		*size = 1;
	else
		*size = 3;

	*size += 80;
	*size += signlen;

	for (n = 0; n < ntx; n++)
	{
		unsigned char	*tx_data;
		size_t			tx_len;

		if (get_file_chunk("transactions", tx_offset, &tx_data, &tx_len)<=0)
		{
			ret = 0;
			break;
		}

		*size		+= tx_len;
		tx_offset	+= tx_len + 4;

		free_c(tx_data);
	}
	return ret;
}

OS_API_C_FUNC(int) store_tx_blk_index(const hash_t tx_hash, const hash_t blk_hash,uint64_t height,size_t tx_ofset,unsigned int tx_time)
{
	char			tchash[65];
	unsigned char   buffer[80];
	struct string	tx_path = { 0 };
	int				n= 0;

	bin_2_hex		(tx_hash, 32, tchash);


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

OS_API_C_FUNC(int) load_blk_hdr_at(mem_zone_ref_ptr hdr, int64_t height)
{
	hash_t				h1, h2, hash, blk_hash;
	unsigned char		*hdr_data;
	size_t				hdr_data_len;
	int					ret = 0;
	int					n;

	if (get_file_chunk("blocks", mul64(height, 512), &hdr_data, &hdr_data_len) <= 0)
		return 0;

	if (hdr_data_len < 80)
	{
		free_c(hdr_data);
		return 0;
	}

	mbedtls_sha256(hdr_data, 80, h1, 0);
	mbedtls_sha256(h1, 32, h2, 0);


	if (height > 0)
	{
		get_block_hash(height, blk_hash);

		if (memcmp_c(h2, blk_hash, sizeof(hash_t)))
		{
			log_output("bad block hash \n");
			free_c(hdr_data);
			return 0;
		}
	}

	if ((hdr->zone != PTR_NULL) || (tree_manager_create_node("blk", NODE_BITCORE_BLK_HDR, hdr)))
	{
		struct string sign;
		unsigned char vntx[16];
		unsigned int ntx;


		init_node(hdr);
		read_node(hdr, hdr_data, hdr_data_len);
		tree_manager_set_child_value_bhash(hdr, "blkHash", blk_hash);

		/*length = 80+4+8+33+80+32; /* hdr size + ntx + hght + sig + pos/pow */

		tree_manager_set_child_value_i64(hdr, "height", height);
		tree_manager_set_child_value_i64(hdr, "txoffset", *((uint64_t *)(hdr_data + 80)));

		ntx = *((unsigned int *)(hdr_data + 88));

		if (ntx < 0xFD)
			vntx[0] = ntx;
		else
		{
			vntx[0] = 0xFD;
			*((unsigned short *)(&vntx[1])) = (unsigned short)ntx;
		}
		tree_manager_set_child_value_vint(hdr, "ntx", vntx);


		sign.len = *((unsigned char *)(hdr_data + 92));

		if (sign.len>0)
		{
			mem_zone_ref sig = { PTR_NULL };
			sign.str = (char *)(hdr_data + 93);

			if (!tree_manager_find_child_node(hdr, NODE_HASH("signature"), NODE_BITCORE_ECDSA_SIG, &sig))
				tree_manager_add_child_node(hdr, "signature", NODE_BITCORE_ECDSA_SIG, &sig);


			tree_manager_write_node_sig(&sig, 0, (unsigned char *)sign.str, sign.len);
			release_zone_ref(&sig);

		}

		switch (hdr_data[172])
		{
		case 0:break;
		case 1:
			tree_manager_set_child_value_hash(hdr, "blk pow", hdr_data + 173);
			break;
		default:
			tree_manager_set_child_value_hash(hdr, "blk pos", hdr_data + 173);
			break;
		}
		ret = 1;
	}
	free_c(hdr_data);

	return ret;
}

OS_API_C_FUNC(int) load_blk_hdr(mem_zone_ref_ptr hdr, const hash_t blk_hash)
{
	hash_t				h1, h2, hash;
	uint64_t			block_height,height;
	size_t				hdr_data_len;
	int					ret = 0;
	int					n;

	block_height		= get_last_block_height();

	if (!find_block_hash(blk_hash, block_height, &height))
		return 0;

	return load_blk_hdr_at(hdr, height);
}




int get_tx_offset(uint64_t *size)
{
	*size  =  file_size("transactions");

	return 1;
}

OS_API_C_FUNC(int) get_obj_app(hash_t objHash,char *appName)
{
	hash_t			bh, appH;
	btc_addr_t		out_addr = { 0 };
	mem_zone_ref	objTx = { PTR_NULL }, app = { PTR_NULL };
	struct string	objHashStr = { 0 };
	struct string	out_path = { 0 }, script = { 0 };
	int ret;

	if (!load_tx(&objTx, bh, objHash))
		return 0;
	
	ret = get_tx_input_hash(&objTx, 0, appH);
	release_zone_ref(&objTx);

	if (ret)ret = tree_find_child_node_by_member_name_hash(&apps, NODE_BITCORE_TX, "txid", appH, &app);
	if (ret)ret = tree_manager_get_child_value_str(&app, NODE_HASH("appName"), appName, 32, 0);

	release_zone_ref(&app);
	return ret;
}

OS_API_C_FUNC(int) store_block(mem_zone_ref_ptr header, mem_zone_ref_ptr tx_list)
{
	unsigned char		buffer[512];
	unsigned char		*blkbuffer;
	mem_zone_ref_ptr	tx = PTR_NULL;
	unsigned char		*blk_txs;
	mem_zone_ref		my_list = { PTR_NULL };
	uint64_t			height, tx_offset;
	hash_t				blk_hash;
	int					ret;
	struct string		signature = { 0 };// , blk_path = { 0 }, blk_data_path = { 0 };
	unsigned int		n_tx, nc, block_time;


	if (!tree_manager_get_child_value_hash(header, NODE_HASH("blkHash"), blk_hash))return 0;
	
	get_tx_offset(&tx_offset);

	nc = tree_manager_get_node_num_children(tx_list);
	height = get_last_block_height();

	*((unsigned int *)(buffer)) = 508;
	blkbuffer = buffer + 4;

	write_node(header, (unsigned char *)blkbuffer);

	*((uint64_t *)(blkbuffer + 80)) = tx_offset;
	*((unsigned int *)(blkbuffer + 88)) = nc;

	if (tree_manager_get_child_value_istr(header, NODE_HASH("signature"), &signature, 0))
	{
		*((unsigned char *)(blkbuffer + 92)) = signature.len;
		memcpy_c(blkbuffer + 93, signature.str, signature.len);
		free_string(&signature);
	}

	if (tree_manager_get_child_value_hash(header, NODE_HASH("blk pow"), blkbuffer + 173)) {
		*((unsigned char *)(blkbuffer + 172)) = 1;

	}
	else {
		memset_c(blkbuffer + 172, 0, 33);
	}

	memset_c(blkbuffer + 205, 0, 508 - 205);

	ret = append_file("blocks", buffer, 512);

	if (ret != 512)
	{
		log_output("error writing new block\n");
		return 0;
	}
	


	if (nc <= 0)
		return 0;


	blk_txs = (unsigned char *)malloc_c(4 + (sizeof(hash_t) * nc));
	*((unsigned int *)(blk_txs)) = nc * sizeof(hash_t);
	for (n_tx = 0, tree_manager_get_first_child(tx_list, &my_list, &tx); ((tx != NULL) && (tx->zone != NULL)); tree_manager_get_next_child(&my_list, &tx), n_tx++)
	{
		tree_manager_get_child_value_hash(tx, NODE_HASH("txid"), &blk_txs[4 + n_tx * 32]);
	}
	ret=append_file("transactions", blk_txs, 4 + (sizeof(hash_t) * nc));
	free_c(blk_txs);

	if (!ret)
		return 0;

	tree_manager_get_child_value_i32(header, NODE_HASH("time"), &block_time);

	tx_offset += 4 + (sizeof(hash_t) * nc);

	for (n_tx = 0, tree_manager_get_first_child(tx_list, &my_list, &tx); ((tx != NULL) && (tx->zone != NULL)); tree_manager_get_next_child(&my_list, &tx), n_tx++)
	{
		hash_t				tmp_hash,tx_hash, pObjHash;
		struct string		app_name = { 0 };
		unsigned int		tx_time, app_item;
		unsigned char		*chunk_buffer;
		size_t				chunk_len;

		unsigned char		*buffer;
		size_t				length;

		length = get_node_size(tx);
		chunk_len = length + 4;
		chunk_buffer = (unsigned char *)malloc_c(chunk_len);

		*((unsigned int *)(chunk_buffer)) = length;
		buffer = chunk_buffer+4;

		write_node(tx, (unsigned char *)buffer);

		mbedtls_sha256((unsigned char *)buffer, length, tmp_hash, 0);
		mbedtls_sha256(tmp_hash, 32, tx_hash, 0);
		tree_manager_set_child_value_hash(tx, "txid", tx_hash);

		if (!tree_manager_get_child_value_i32(tx, NODE_HASH("time"), &tx_time))
			tx_time = block_time;

		if (ret)
		{
			ret = append_file("transactions", chunk_buffer, chunk_len);
			if (!ret)log_message("could not store transaction data ", PTR_NULL);
		}
		if (ret)
		{
			ret = store_tx_blk_index(tx_hash, blk_hash, height, tx_offset, tx_time);
			if (!ret)log_message("could not store tx index ", PTR_NULL);
		}

		if (!ret)
		{
			dec_zone_ref(tx);
			release_zone_ref(&my_list);
			break;
		}

		tx_offset += (length + 4);

		if (is_tx_null(tx) == 1)
		{
			free_c(chunk_buffer);
			continue;
		}

		tree_manager_get_child_value_hash(tx, NODE_HASH("txid"), tx_hash);

		if (is_app_root(tx))
		{
			ret = blk_store_app_root(tx);
			if (ret)set_root_app(tx);
			if (!ret)log_message("could not store app root", PTR_NULL);
		}
		else
		{
			ret = store_tx_inputs(tx);
			if (!ret)log_message("could not store tx inputs", PTR_NULL);
			if (ret)
			{
				ret = store_tx_outputs(tx);
				if (!ret)log_message("could not store tx outputs", PTR_NULL);
			}
		}

		if (ret)
		{
			if (tree_manager_get_child_value_istr(tx, NODE_HASH("AppName"), &app_name, 0))
			{
				ret = store_new_app(&app_name, tx, buffer, length);
				free_string(&app_name);
			}
			else if (tree_manager_get_child_value_i32(tx, NODE_HASH("app_item"), &app_item))
			{
				ret = store_app_item(tx, app_item, tx_time, buffer, length);
			}
			else if (tree_manager_find_child_node(tx, NODE_HASH("newChild"), NODE_BITCORE_HASH,PTR_NULL))
			{
				ret = tree_manager_get_child_value_hash(tx, NODE_HASH("appChildOf"), pObjHash);
				if(ret)ret = store_app_obj_child(tx, pObjHash);
			}
			else if (tree_manager_find_child_node(tx, NODE_HASH("objTxfr"), 0xFFFFFFFF, PTR_NULL))
			{
				if (ret)ret = store_app_obj_txfr(tx);
			}
		}

		free_c(chunk_buffer);

		if (!ret)
		{
			dec_zone_ref(tx);
			release_zone_ref(&my_list);
			break;
		}
	}

	return ret;
}




OS_API_C_FUNC(int) check_block_txs(mem_zone_ref_ptr block)
{
	hash_t			blockHash;
	mem_zone_ref    hashes = { PTR_NULL };
	uint64_t		tx_offset,height;
	size_t			tx_hashes_len;
	unsigned int	ntx;
	int				n,ret;
	unsigned char	*tx_hashes;

	if (!tree_manager_get_child_value_i64(block, NODE_HASH("txoffset"), &tx_offset))
		return 0;

	if (!tree_manager_get_child_value_i32(block, NODE_HASH("ntx"), &ntx))
		return 0;

	if (get_file_chunk("transactions", tx_offset, &tx_hashes, &tx_hashes_len) <= 0)
		return 0;

	if (tx_hashes_len != (ntx * 32))
	{
		free_c(tx_hashes);
		return 0;
	}

	tree_manager_get_child_value_i64	(block, NODE_HASH("height"), &height);
	tree_manager_get_child_value_hash	(block, NODE_HASH("blkHash"), blockHash);

	ret= tree_manager_create_node("hashes", NODE_BITCORE_TX_HASH, &hashes);
	if (ret)ret = tree_manager_write_node_data(&hashes, tx_hashes, 0, tx_hashes_len);
	

	if (ret)
	{
		hash_t merkleRoot;

		tree_manager_get_child_value_hash(block, NODE_HASH("merkle_root"), merkleRoot);

		if (ntx > 1)
		{
			size_t newLen;
			size_t n;
			n = ntx;


			while ((newLen = compute_merkle_round(&hashes, n)) > 1)
			{
				n = newLen;
			}
		}
	
		if (memcmp_c(merkleRoot, tree_mamanger_get_node_data_ptr(&hashes, 0), sizeof(hash_t)))
		{
			log_message("bad merkle \n", block);
			ret = 0;
		}
	}
	release_zone_ref(&hashes);
	if(!ret)
	{
		free_c(tx_hashes);
		return 0;
	}
	
	tx_offset += tx_hashes_len + 4;
	
	for (n = 0; n < tx_hashes_len; n += 32)
	{
		hash_t			tmph, txh, txblockHash;
		mem_zone_ref	tx = { PTR_NULL };
		unsigned char	*data;
		unsigned int	tx_size;

		uint64_t		txBlockHeight;
		unsigned int	txOffset;
		unsigned int	txTime;

		if (get_file_chunk("transactions", tx_offset, &data, &tx_size) <= 0)
		{
			ret = 0;
			break;
		}

		mbedtls_sha256(data, tx_size, tmph, 0);
		mbedtls_sha256(tmph, 32, txh, 0);
		free_c(data);

		if (memcmp_c(txh, tx_hashes + n, sizeof(hash_t)))
		{
			log_message("bad tx hash \n", block);
			ret = 0;
			break;
		}

		if (!find_blk_hash(tx_hashes + n, txblockHash, &txBlockHeight, &txOffset, &txTime))
		{
			ret = 0;
			break;
		}

		if (memcmp_c(txblockHash, blockHash, sizeof(hash_t)))
		{
			ret = 0;
			break;
		}

		if (txBlockHeight!= height)
		{
			ret = 0;
			break;
		}

		if (txOffset != (tx_offset))
		{
			ret = 0;
			break;
		}

		tx_offset += (tx_size + 4);
	}
	
	free_c(tx_hashes);


	return ret;
}



OS_API_C_FUNC(int) load_block_range(uint64_t start, uint64_t end,mem_zone_ref_ptr blocks)
{
	unsigned char	* blocks_data;
	size_t			blocks_data_len;
	size_t			current_block,num;
	int				ret = 1;

	if (get_file_range("blocks", start, end, &blocks_data, &blocks_data_len) <= 0)
		return 0;

	
	for (current_block = 0, num=0; current_block < blocks_data_len; current_block += 512, num++)
	{
		hash_t			h1, h2;
		mem_zone_ref	block = { PTR_NULL };
		size_t			hdr_data_len;
		unsigned char	*hdr_data;

		hdr_data_len = *((unsigned int *)(blocks_data + current_block));

		if (hdr_data_len < 80)
		{
			ret = 0;
			break;
		}

		hdr_data	 = mem_add(blocks_data, current_block + 4);
		mbedtls_sha256(hdr_data, 80, h1, 0);
		mbedtls_sha256(h1, 32, h2, 0);


		if (!tree_manager_add_child_node(blocks, "blk", NODE_BITCORE_BLK_HDR, &block))
		{
			ret = 0;
			break;
		}

		init_node(&block);
		ret= (read_node(&block, hdr_data, hdr_data_len)!=INVALID_SIZE)?1:0;
		if (ret)
		{
			tree_manager_set_child_value_i64(&block, "txoffset", *((uint64_t *)(hdr_data + 80)));
			tree_manager_set_child_value_i32(&block, "ntx", *((unsigned int *)(hdr_data + 88)));
			tree_manager_set_child_value_bhash(&block, "blkHash", h2);
			tree_manager_set_child_value_i64(&block, "height", start+num);

			switch (hdr_data[172])
			{
				case 0:break;
				case 1:
					tree_manager_set_child_value_hash(&block, "blk pow", hdr_data + 173);
					break;
				default:
					tree_manager_set_child_value_hash(&block, "blk pos", hdr_data + 173);
					break;
			}
		}
		release_zone_ref(&block);
		if (!ret)
			break;
	}

	free_c(blocks_data);

	return ret;

}

// A recursive binary search function. It returns 
// location of x in given array arr[l..r] is present, 
// otherwise -1
int binaryTimeSearch(mem_zone_ref_ptr time_node, int l, int r, unsigned int time)
{
	if (r >= l)
	{
		unsigned int mytime;
		int mid = l + (r - l) / 2;

		if (!tree_mamanger_get_node_dword(time_node, mul64(mid, sizeof(unsigned int)), &mytime))
			return -1;

		if (time == mytime)
			return mid;

		// If the element is present at the middle 
		// itself
		if (time< mytime)
			return binaryTimeSearch(time_node, l, mid - 1, time);
		else
			return binaryTimeSearch(time_node, mid + 1, r, time);
	}
	return r;
	// We reach here when element is not 
	// present in array
	return -1;
}


OS_API_C_FUNC(int) load_block_time_range(unsigned int start, unsigned int end, size_t first, size_t max, size_t *total, mem_zone_ref_ptr blocks)
{
	uint64_t		block_height;
	unsigned int	block_time;
	int				ret = 1;
	int				first_block, cnt,last_block, first_block_range;

	if (max == 0)return 1;

	block_height = get_last_block_height()-1;

	last_block = binaryTimeSearch(&time_index_node, 0, block_height, end);
	if (last_block < 0)
		last_block = block_height;

	if (last_block > block_height)
		last_block = block_height;

	if (last_block > first)
		last_block -= first;

	first_block_range = last_block;
	cnt = 0;

	for(first_block = first_block_range; first_block >= 0; first_block--)
	{
		if (!get_block_time(first_block, &block_time))
			break;

		if (block_time < start) break;

		if (cnt <= max)
		{
			first_block_range = first_block;
			cnt++;
		}
	}

	*total = (last_block - first_block)+first;

	return load_block_range(first_block_range, last_block, blocks);

}

OS_API_C_FUNC(int) block_check_last(uint64_t num, uint64_t *last_good)
{
	uint64_t		first,last,height;
	unsigned char	*block_index;
	unsigned int	*block_time;
	size_t			n,len, block_time_len;
	mem_zone_ref blocks = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr block = PTR_NULL ;
	int ret;

	if (get_file("blk_indexes", &block_index, &len) <= 0)
	{
		*last_good = 0;
		return 1;
	}

	if (get_file("blk_times", &block_time, &block_time_len) <= 0)
	{
		free_c(block_index);
		*last_good = 0;
		return 1;
	}
		

	if((len/8)<block_time_len)
		height = muldiv64(len, 1, sizeof(hash_t));
	else
		height = muldiv64(block_time_len, 1, sizeof(unsigned int));

	if (num < height)
		first = height - num;
	else
		first = 1;

	last = first + num;

	if (last > height)
		last = height;

	ret = tree_manager_create_node("blocks", NODE_BITCORE_BLK_HDR_LIST, &blocks);
	if (ret)ret = load_block_range(first, last, &blocks);
	if (!ret)
	{
		log_message("could not read block index data \n", PTR_NULL);
		release_zone_ref(&blocks);
		free_c(block_index);
		free_c(block_time);
		return 0;
	}

	ret = 0;

	for (n= first,tree_manager_get_first_child(&blocks, &my_list, &block); ((block != NULL) && (block->zone != NULL)); tree_manager_get_next_child(&my_list, &block), n ++ )
	{
		hash_t h, ph;
		int	   ok;
		unsigned int blktime;

		ok=tree_manager_get_child_value_hash(block, NODE_HASH("blkHash"), h);
		if (ok)ok = (memcmp_c(h, block_index + n * 32, sizeof(hash_t)) == 0) ? 1 : 0;
		if (ok)ok = tree_manager_get_child_value_hash(block, NODE_HASH("prev"), ph);
		if (ok)ok = (memcmp_c(ph, block_index + (n - 1) * 32, sizeof(hash_t)) == 0) ? 1 : 0;
		if (ok)ok = tree_manager_get_child_value_i32(block, NODE_HASH("time"), &blktime);
		if (ok)ok = (blktime == block_time[n])?1:0;
		if (ok)ok = (blktime >= block_time[n-1]) ? 1 : 0;


		if (ok)ok = check_block_txs(block);
		if(!ok)
		{
			dec_zone_ref(block);
			release_zone_ref(&my_list);
			break;
		}
		ret = 1;
		*last_good = n;
	}

	release_zone_ref(&blocks);
	free_c(block_index);
	free_c(block_time);
	return ret;
}



OS_API_C_FUNC(int) C_API_FUNC block_load_index(uint64_t *block_height)
{
	uint64_t	    last_good =0,height = 0;
	size_t			len;
	unsigned char *	block_index;
	int				ret;

	tree_manager_create_node("block revidx", NODE_GFX_BINT, &block_rindex_node);
	tree_manager_create_node("block time", NODE_GFX_INT, &time_index_node);
	tree_manager_create_node("block_index", NODE_BITCORE_HASH, &block_index_node);

	if (get_file("blk_indexes", &block_index, &len) <= 0)
		return 0;



	height = (len / sizeof(hash_t));
	ret = tree_manager_write_node_data(&block_index_node, block_index, 0, len);
	free_c(block_index);

	if (!ret)
	{
		log_message("could not write block index data \n", PTR_NULL);
		return 0;
	}

	if (stat_file("blk_times") != 0)
	{
		rebuild_time_index(height);
	}

	if (get_file("blk_times", &block_index, &len) <= 0)
		return 0;

	if (ret)
	{
		ret = (mul64(height, sizeof(unsigned int)) == len) ? 1 : 0;
		if (!ret)log_message("block index size mistmatch.", PTR_NULL);
	}

	if (ret)
	{
		ret = tree_manager_write_node_data(&time_index_node, block_index, 0, len);
		if (!ret)log_message("could not write time index data \n", PTR_NULL);
	}

	if (!ret)
	{
		free_c(block_index);
		return 0;
	}
	

	log_message("creating sorted block index.", PTR_NULL);
	if (!create_sorted_block_index(height))
	{
		log_message("could not create sorted index \n", PTR_NULL);
		return 0;
	}
	log_message("created sorted block index.", PTR_NULL);
	//dump_sorted_hashes(height + 1);

	*block_height = height;

	return 1;
}
OS_API_C_FUNC(int) block_add_index(hash_t hash, uint64_t nblks, unsigned int time)
{
	uint64_t		blkidx;
	unsigned int	ntries = 3;

	if (append_file("blk_times", &time, 4) <= 0)
	{
		log_output("could not write time index\n");
		return 0;
	}
	
	if(append_file("blk_indexes", hash, 32) <= 0)
	{
		log_output("could not write block index\n");
		return 0;
	}

	if (block_index_node.zone == PTR_NULL)
	{
		log_output("no blk idx \n");
		return 1;
	}


	blkidx = mul64(nblks, sizeof(hash_t));

	if (!tree_manager_expand_node_data_ptr(&block_index_node, blkidx, sizeof(hash_t)))
	{
		mem_zone_ref log = { PTR_NULL };
		if (tree_manager_create_node("log", NODE_LOG_PARAMS, &log))
		{
			tree_manager_set_child_value_i32(&log, "nblks", nblks);
			log_message("could not expand block index to %nblks%", &log);
			release_zone_ref(&log);
		}
		release_zone_ref(&block_index_node);
		return 0;
	}

	tree_manager_write_node_hash(&block_index_node, blkidx, hash);

	blkidx = mul64(nblks, sizeof(unsigned int));

	if (!tree_manager_expand_node_data_ptr(&time_index_node, blkidx, sizeof(unsigned int)))
	{
		mem_zone_ref log = { PTR_NULL };
		if (tree_manager_create_node("log", NODE_LOG_PARAMS, &log))
		{
			tree_manager_set_child_value_i32(&log, "nblks", nblks);
			log_message("could not expand block time index to %nblks%", &log);
			release_zone_ref(&log);
		}
		return 0;
	}

	tree_manager_write_node_dword(&time_index_node, blkidx, time);
	add_sorted_block(nblks, nblks);

	/*
	log_output("----------------\n");
	dump_sorted_hashes(nblks+1);
	log_output("----------------\n");

	*/
	return 1;
}

OS_API_C_FUNC(int) C_API_FUNC write_block_index(uint64_t nblks)
{
	unsigned int *time_ptr;
	unsigned char *bh_ptr;
	uint64_t cur_idx;

	time_ptr = (unsigned int *)tree_mamanger_get_node_data_ptr(&time_index_node, 0);
	bh_ptr = (unsigned char *)tree_mamanger_get_node_data_ptr(&block_index_node, 0);

	cur_idx = mul64(nblks, sizeof(unsigned int));
	put_file("blk_times", time_ptr, cur_idx);
	cur_idx = mul64(nblks, sizeof(hash_t));
	put_file("blk_indexes", bh_ptr, cur_idx);

	return 1;
}

