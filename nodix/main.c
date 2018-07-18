//copyright antoine bentue-ferrer 2016
//#include <stdio.h>

#include <base/std_def.h>
#include <base/std_mem.h>
#include <base/mem_base.h>
#include <base/std_str.h>
#include <fsio.h>


#include <strs.h>
#include <tree.h>
#include <parser.h>
#include <connect.h>
#include <sha256.h>
#include <crypto.h>
#include <http.h>
#include <upnp.h>
#include <mem_stream.h>
#include <tpo_mod.h>
#include <strbuffer.h>

#include "../block_adx/block_api.h"
#include "../wallet/wallet_api.h"
#include "../node_adx/node_api.h"

//#define _DEBUG

mem_zone_ref		self_node  = { PTR_INVALID };
hash_t				null_hash = { 0xCD };

mem_zone_ref		pos_kernel_def = { PTR_INVALID };
tpo_mod_file		*pos_kernel = PTR_INVALID;
unsigned int		ping_nonce = 0x01;

C_EXPORT int _fltused = 0;
C_EXPORT mod_name_decoration_t	 mod_name_deco_type = MOD_NAME_DECO;


//POS kernel module
typedef int				C_API_FUNC init_pos_func(mem_zone_ref_ptr stake_conf);
typedef int				C_API_FUNC store_blk_staking_func(mem_zone_ref_ptr header);
typedef int				C_API_FUNC store_blk_tx_staking_func(mem_zone_ref_ptr tx_list);
typedef int				C_API_FUNC get_stake_reward_func(uint64_t height, uint64_t *reward);
typedef int				C_API_FUNC	store_tx_staking_func(mem_zone_ref_ptr tx, hash_t tx_hash, btc_addr_t stake_addr, uint64_t	stake_in);
typedef int				C_API_FUNC	check_tx_pos_func(mem_zone_ref_ptr blk, mem_zone_ref_ptr tx);
typedef int				C_API_FUNC	compute_last_pos_diff_func(mem_zone_ref_ptr lastPOS, mem_zone_ref_ptr nBits);
typedef int				C_API_FUNC	check_blk_sig_func(mem_zone_ref_ptr hdr, struct string *vpubK);
typedef int				C_API_FUNC	load_last_pos_blk_func(mem_zone_ref_ptr header);
typedef int				C_API_FUNC	store_last_pos_hash_func(hash_t hash);
typedef int				C_API_FUNC	find_last_pos_block_func(mem_zone_ref_ptr pindex);
typedef int				C_API_FUNC	find_last_stake_modifier_func(hash_t hash, hash_t nStakeModifier);

typedef int				C_API_FUNC	find_blk_staking_tx_func(mem_zone_ref_ptr tx_list, mem_zone_ref_ptr tx);

typedef init_pos_func						*init_pos_func_ptr;
typedef store_blk_staking_func				*store_blk_staking_func_ptr;
typedef store_blk_tx_staking_func			*store_blk_tx_staking_func_ptr;
typedef get_stake_reward_func				*get_stake_reward_func_ptr;
typedef store_tx_staking_func				*store_tx_staking_func_ptr;
typedef check_tx_pos_func					*check_tx_pos_func_ptr;
typedef check_blk_sig_func					*check_blk_sig_func_ptr;
typedef compute_last_pos_diff_func			*compute_last_pos_diff_func_ptr;
typedef load_last_pos_blk_func				*load_last_pos_blk_func_ptr;
typedef store_last_pos_hash_func			*store_last_pos_hash_func_ptr;
typedef find_last_pos_block_func			*find_last_pos_block_func_ptr;
typedef find_blk_staking_tx_func			*find_blk_staking_tx_func_ptr;
typedef find_last_stake_modifier_func		*find_last_stake_modifier_func_ptr;


#ifdef _DEBUG

C_IMPORT int			C_API_FUNC		init_pos(mem_zone_ref_ptr stake_conf);
C_IMPORT int			C_API_FUNC		store_blk_tx_staking(mem_zone_ref_ptr tx_list);
C_IMPORT int			C_API_FUNC		store_blk_staking(mem_zone_ref_ptr header);
C_IMPORT int			C_API_FUNC		get_stake_reward(uint64_t height, uint64_t *reward);
C_IMPORT int			C_API_FUNC		check_tx_pos(mem_zone_ref_ptr hdr, mem_zone_ref_ptr tx);
C_IMPORT int			C_API_FUNC		compute_last_pos_diff(mem_zone_ref_ptr lastPOS, mem_zone_ref_ptr nBits);
C_IMPORT int			C_API_FUNC		store_tx_staking(mem_zone_ref_ptr tx, hash_t tx_hash, btc_addr_t stake_addr, uint64_t	stake_in);
C_IMPORT unsigned int	C_API_FUNC		get_current_pos_difficulty();
C_IMPORT int			C_API_FUNC		load_last_pos_blk(mem_zone_ref_ptr header);
C_IMPORT int			C_API_FUNC		node_store_last_pos_hash(mem_zone_ref_ptr hdr);
C_IMPORT int			C_API_FUNC		find_last_pos_block(mem_zone_ref_ptr pindex);
C_IMPORT int			C_API_FUNC		find_blk_staking_tx(mem_zone_ref_ptr tx_list, mem_zone_ref_ptr tx);
C_IMPORT int			C_API_FUNC		check_blk_sig(mem_zone_ref_ptr hdr, struct string *vpubK);
C_IMPORT int			C_API_FUNC		find_blk_staking_tx(mem_zone_ref_ptr tx_list, mem_zone_ref_ptr tx);
C_IMPORT int			C_API_FUNC		find_last_stake_modifier(hash_t hash, hash_t nStakeModifier);
#else

init_pos_func_ptr							init_pos = PTR_INVALID;
store_blk_staking_func_ptr					store_blk_staking = PTR_INVALID;
store_blk_tx_staking_func_ptr				store_blk_tx_staking = PTR_INVALID;
store_tx_staking_func_ptr					store_tx_staking = PTR_INVALID;
check_tx_pos_func_ptr						check_tx_pos= PTR_INVALID;
compute_last_pos_diff_func_ptr				compute_last_pos_diff = PTR_INVALID;
get_stake_reward_func_ptr					get_stake_reward = PTR_INVALID;
load_last_pos_blk_func_ptr					load_last_pos_blk = PTR_INVALID;

find_last_pos_block_func_ptr				find_last_pos_block = PTR_INVALID;
find_blk_staking_tx_func_ptr				find_blk_staking_tx	= PTR_INVALID;
check_blk_sig_func_ptr						check_blk_sig = PTR_INVALID;
find_last_stake_modifier_func_ptr			find_last_stake_modifier = PTR_INVALID;
#endif

//protocol module
C_IMPORT int			C_API_FUNC node_process_event_handler(const char *msg_list_name, mem_zone_ref_ptr node, mem_zone_ref_ptr msg);


OS_API_C_FUNC(int) truncate_chain(uint64_t to_height)
{
	hash_t			hash, null_hash;
	uint64_t		block_reward, current_height;
	mem_zone_ref	log = { PTR_NULL };
	mem_zone_ref	last_hdr = { PTR_NULL }, blk = { PTR_NULL }, lb = { PTR_NULL }, nBits = { PTR_NULL };
	size_t			size;
	unsigned int	time;

	size			= file_size("blocks");
	if (size < 512)
		return 1;

	current_height	=	(size / 512) - 1;

	if (to_height >= current_height)
		return 1;
	
	tree_manager_create_node("log", NODE_LOG_PARAMS, &log);
	tree_manager_set_child_value_i32(&log, "cur", current_height);
	tree_manager_set_child_value_i32(&log, "new", to_height);
	log_message("truncate chain to height : %new% ( current %cur%)", &log);
	release_zone_ref(&log);

	while ((current_height>0)&&(current_height > to_height))
	{
		if (is_pow_block_at(current_height))
			get_blockreward(current_height, &block_reward);
		else
			get_stake_reward(current_height, &block_reward);

		node_remove_last_block();
		sub_moneysupply(block_reward);

		current_height = (file_size("blocks") / 512) - 1;
	}

	create_sorted_block_index(current_height);


	//copy new last block
	tree_manager_find_child_node	(&self_node, NODE_HASH("last_block"), NODE_BITCORE_BLK_HDR, &blk);
	load_blk_hdr_at					(&blk, current_height);
	tree_manager_get_child_value_i32(&blk, NODE_HASH("time"), &time);
	release_zone_ref				(&blk);

	tree_manager_set_child_value_i32(&self_node, "last_block_time", time);
	tree_manager_set_child_value_i32(&self_node, "block_height", current_height);

	
	
	if (load_blk_hdr_at(&blk, current_height))
	{
		if (blk_find_last_pow_block(&blk, PTR_NULL) >= 0)
		{
			tree_manager_find_child_node	(&self_node, NODE_HASH("lastPOWBlk"), NODE_BITCORE_BLK_HDR, &lb);
			tree_manager_copy_children_ref	(&lb, &blk);
			node_compute_pow_diff_after		(&lb);
			release_zone_ref				(&lb);
			tree_manager_get_child_value_hash(&blk, NODE_HASH("blkHash"), hash);
			store_last_pow_hash(hash);
		}
		release_zone_ref(&blk);
	}
	
	
	if (load_blk_hdr_at(&blk, current_height))
	{
		if (blk_find_last_pos_block(&blk, PTR_NULL) >= 0)
		{
			tree_manager_find_child_node	(&self_node, NODE_HASH("current_pos_diff"), NODE_GFX_INT, &nBits);
			tree_manager_find_child_node	(&self_node, NODE_HASH("lastPOSBlk"), NODE_BITCORE_BLK_HDR, &lb);
			tree_manager_copy_children_ref	(&lb, &blk);
			compute_last_pos_diff			(&lb, &nBits);
			release_zone_ref				(&lb);
			release_zone_ref				(&nBits);
			tree_manager_get_child_value_hash(&blk, NODE_HASH("blkHash"), hash);
			store_last_pos_hash(hash);
		}
		release_zone_ref(&blk);
	}
	
	

	get_blockreward(current_height + 1, &block_reward);
	tree_manager_set_child_value_i64(&self_node, "pow_reward", block_reward);

	get_stake_reward(current_height+1, &block_reward);
	tree_manager_set_child_value_i64(&self_node, "pos_reward", block_reward);
	return current_height;
}


OS_API_C_FUNC(int) truncate_chain_to(mem_zone_ref_ptr nHeight)
{
	uint64_t		height;
	tree_mamanger_get_node_qword(nHeight, 0, &height);
	return truncate_chain(height);
}

void rebuild_money_supply()
{
	mem_zone_ref blk_hdr = { PTR_NULL }, last_blk_hdr = { PTR_NULL };
	uint64_t start, cur, lb, last_log, total_supply;
	int		 ret;

	tree_manager_get_child_value_i64(&self_node, NODE_HASH("block_height"), &lb);
	 
	start = 0;
	total_supply = 0;
	cur = start;
	last_log = cur;
	reset_moneysupply();
	while (cur < lb)
	{
		hash_t			hash;
		struct string	blk_path = { PTR_NULL };
		uint64_t		reward;
		unsigned int	nBits;

		if ((cur - last_log) > 100)
		{
			mem_zone_ref log = { PTR_NULL };
			tree_manager_create_node("log", NODE_LOG_PARAMS, &log);
			tree_manager_set_child_value_i32(&log, "cur", cur);
			tree_manager_set_child_value_i32(&log, "lb", lb);
			tree_manager_set_child_value_i64(&log, "supply", total_supply);

			log_message("processing block %cur% / %lb%, %supply% ADX", &log);
			release_zone_ref(&log);
			last_log = cur;
		}
		
		
		
		if (!load_blk_hdr_at(&blk_hdr, cur))
			break;

		get_block_hash(cur, hash);

		if (last_blk_hdr.zone != PTR_NULL)
		{
			mem_zone_ref txs = { PTR_NULL };
			mem_zone_ref stake_tx = { PTR_NULL };

			struct string pubK = { PTR_NULL };


			tree_manager_create_node("txs", NODE_BITCORE_TX_LIST, &txs);
			if (!load_blk_txs(hash, &txs))
				break;

			if (find_blk_staking_tx(&txs, &stake_tx))
			{
				ret = check_tx_input_sig(&stake_tx, 0, &pubK);
				if (ret)
				{
					ret = check_blk_sig(&blk_hdr, &pubK);
					free_string(&pubK);
				}
				if (ret)ret = check_tx_pos(&blk_hdr, &stake_tx);
				if (ret)ret = get_stake_reward(cur, &reward);
				release_zone_ref(&stake_tx);

				if (ret != 0)
				{
					mem_zone_ref nBits = { PTR_NULL };

					store_blk_staking(&blk_hdr);
					store_blk_tx_staking(&txs);
					store_wallet_txs(&txs);

					if (!tree_manager_find_child_node(&self_node, NODE_HASH("current_pos_diff"), NODE_GFX_INT, &nBits))
						tree_manager_add_child_node(&self_node, "current_pos_diff", NODE_GFX_INT, &nBits);

					ret = compute_last_pos_diff		(&blk_hdr, &nBits);
					release_zone_ref				(&nBits);

					tree_manager_set_child_value_i64(&self_node, "pos_reward", reward);
				}
			}
			else
				reward = 0;

			release_zone_ref(&txs);

			if (!ret)
				break;
		}
		else
			reward = 0;

		if (reward == 0)
		{
			hash_t			out_diff;
			mem_zone_ref	pdif = { PTR_NULL };

			tree_manager_get_child_value_i32(&self_node, NODE_HASH("current_pow_diff"), &nBits);

			SetCompact(nBits, out_diff);
			if (!check_block_pow(&blk_hdr, out_diff))
				break;

			if (!node_compute_pow_diff_after(&blk_hdr))
				tree_manager_get_child_value_i32(&blk_hdr, NODE_HASH("bits"), &nBits);

			release_zone_ref(&pdif);

			if (pos_kernel != PTR_NULL)
				check_tx_pos(&blk_hdr, PTR_NULL);

			get_blockreward						(cur, &reward);
			tree_manager_set_child_value_i64	(&self_node, "pow_reward", reward);
		}
		add_moneysupply(reward);
		total_supply += reward;
		copy_zone_ref(&last_blk_hdr, &blk_hdr);
		release_zone_ref(&blk_hdr);
		cur++;
	}
	release_zone_ref(&blk_hdr);
	release_zone_ref(&last_blk_hdr);

	if (cur < lb)
	{
		truncate_chain(cur);
	}

	return;
}

int do_staking()
{
	struct string username = { 0 };
	mem_zone_ref mempool = { PTR_NULL }, newblk = { PTR_NULL };
	int newblkret;

	if (!node_aquire_mempool_lock(&mempool))
		return 0;

	make_string							(&username, "anonymous");
	newblkret = generate_staking_block	(&username, 4, &mempool, &newblk);
	
	release_zone_ref			(&mempool);
	node_release_mempool_lock	();

	if (newblkret)
	{
		node_fill_block_from_mempool(&newblk);
		sign_staking_block			(&username, &newblk);
	}

	free_string(&username);
	
	if (newblkret)
	{
		mem_zone_ref blk_list = { PTR_NULL };
		if (tree_manager_find_child_node(&self_node, NODE_HASH("submitted blocks"), NODE_BITCORE_BLOCK_LIST, &blk_list))
		{
			tree_manager_set_child_value_i32(&newblk, "ready", 1);

			tree_manager_node_add_child(&blk_list, &newblk);
			release_zone_ref(&blk_list);
		}
	
	}
	
	release_zone_ref(&newblk);
	return 1;
}


int bigger_diff(unsigned int *hash1, unsigned int *hash2)
{
	int n;
	for(n=0;n<8;n++)
	{
		if (hash1[n] < hash2[n])
			return 0;
		if (hash1[n] > hash2[n])
			return 1;
	}
	return 0;
}


OS_API_C_FUNC(int) accept_block(mem_zone_ref_ptr header, mem_zone_ref_ptr tx_list)
{
	hash_t				merkle;
	mem_zone_ref		lastPOWBlk = { PTR_NULL };
	mem_zone_ref		log = { PTR_NULL };
	struct string		sign = { PTR_NULL };
	uint64_t			block_reward = 0, staking_reward = 0, nblks;
	ctime_t				now;
	int					ret = 1;
	unsigned int		is_staking;
	unsigned int		checktx, block_time;


	if (!tree_manager_get_child_value_i32(header, NODE_HASH("time"), &block_time))
		return 0;
	
	now = get_time_c();
	if (!future_drift(now, block_time))
		return 0;


	tree_manager_get_child_value_hash(header, NODE_HASH("merkle_root"), merkle);

	tree_manager_get_child_value_i64(&self_node, NODE_HASH("block_height"), &nblks);

	is_staking = 0;
	if (pos_kernel != PTR_NULL)
	{
		mem_zone_ref stake_tx = { PTR_NULL };

		if (find_blk_staking_tx(tx_list, &stake_tx))
		{
			struct string pubK = { PTR_NULL };
			unsigned int  tx_time;
			
			tree_manager_get_child_value_i32(&stake_tx, NODE_HASH("time"), &tx_time);
			
			ret = future_drift(block_time, tx_time);
			if (ret)
			{
				ret = check_tx_input_sig(&stake_tx, 0, &pubK);
				if (!ret)log_output("bad staking input \n");
			}
			if (ret)
			{
				ret = check_blk_sig(header, &pubK);
				if (!ret)log_output("bad staking signature \n");
			}
			free_string(&pubK);

			//ret = 1;
			if (ret)
			{
				ret = check_tx_pos(header, &stake_tx);
				if (!ret)log_output("bad staking POS \n");
			}
			if (ret)
			{
				ret = get_stake_reward(nblks, &block_reward);
				if (!ret)log_output("stake reward error\n");
			}
			release_zone_ref(&stake_tx);
			if (!ret)return 0;

			is_staking = 1;
		}
	}


	if (!is_staking)
	{
		mem_zone_ref		POWSpace = { PTR_NULL }, nBits = { PTR_NULL }, lb = { PTR_NULL };
		unsigned int		powbits, blkbits,nbits, time, btime,powtime;
		hash_t				out_diff, my_hash;

		if (block_pow_limit())
			return 0;

		tree_manager_get_child_value_hash(header, NODE_HASH("merkle_root"), merkle);

		tree_manager_get_child_value_i32	(header, NODE_HASH("bits"), &blkbits);
		tree_manager_get_child_value_i32	(header, NODE_HASH("time"), &btime);
		tree_manager_get_child_value_hash	(header, NODE_HASH("prev"), my_hash);
		

		if (!tree_manager_get_child_value_i32(&self_node, NODE_HASH("current_pow_diff"), &powbits))
			powbits = blkbits;

		if (powbits != blkbits)
		{
			log_output("block bit differ \n");

			if (nblks != 110571)
				return 0;
		}

		SetCompact(blkbits, out_diff);
		ret = check_block_pow(header, out_diff);

		if (!ret)
		{
			if (nblks != 110571)
				return 0;
		}
			
		if (pos_kernel != PTR_NULL)
		{
			if (!check_tx_pos(header, PTR_NULL))return 0;
		}
		if (!get_blockreward(nblks, &block_reward))return 0;
	}
	
	if (!tree_manager_get_child_value_i32(&self_node, NODE_HASH("checktxsign"), &checktx))
		checktx = 0;

	tree_manager_get_child_value_hash(header, NODE_HASH("merkle_root"), merkle);
	return check_tx_list(tx_list, block_reward, merkle, block_time, checktx);
}

int handle_file(mem_zone_ref_ptr node, mem_zone_ref_ptr payload)
{
	mem_zone_ref apps = { PTR_NULL }, mempool = { PTR_NULL }, pending = { PTR_NULL };
	struct string appName = { 0 };
	int				ret;
	tree_manager_get_child_value_istr(payload, NODE_HASH("appName"), &appName, 0);

	get_apps		(&apps);

	if (tree_manager_find_child_node(&apps, NODE_HASH(appName.str), NODE_BITCORE_TX, PTR_NULL))
	{
		char			chash[65];
		hash_t			fileHash, txHash;
		struct string	fileData = { 0 }, filePath = { 0 };

		tree_manager_get_child_value_istr(payload, NODE_HASH("fileData"), &fileData, 0);

		mbedtls_sha256									(fileData.str, fileData.len, fileHash, 0);

		node_aquire_mempool_lock						(&mempool);
		ret = tree_find_child_node_by_member_name_hash	(&mempool, NODE_BITCORE_TX, "fileHash", fileHash, PTR_NULL);
		release_zone_ref								(&mempool);
		node_release_mempool_lock						();

		if(!ret)ret= get_appfile_tx(appName.str, fileHash, txHash);

		if (ret)
		{
			/*
			unsigned int n;
			n = 0;
			while (n<32)
			{
				chash[n * 2 + 0] = hex_chars[fileHash[n] >> 4];
				chash[n * 2 + 1] = hex_chars[fileHash[n] & 0x0F];
				n++;
			}
			chash[64] = 0;
			*/

			bin_2_hex(fileHash, 32, chash);

			make_string		(&filePath, "apps");
			cat_cstring_p	(&filePath, appName.str);
			cat_cstring_p	(&filePath, "datas");
			cat_cstring_p	(&filePath, chash);

			put_file		(filePath.str,fileData.str,fileData.len);
			free_string		(&filePath);

			tree_manager_find_child_node			(&self_node, NODE_HASH("pending files"), NODE_BITCORE_HASH_LIST, &pending);
			tree_remove_child_by_member_value_hash	(&pending, NODE_GFX_OBJECT, "hash", txHash);
			release_zone_ref						(&pending);
		}
		free_string(&fileData);
	}

	free_string(&appName);
	release_zone_ref(&apps);

	return 1;
}

int handle_getblocks(mem_zone_ref_ptr node, mem_zone_ref_ptr payload)
{
	mem_zone_ref locator = { PTR_NULL }, inv_pack = { PTR_NULL }, fb = { PTR_NULL };
	uint64_t block_height, height;

	if (!tree_manager_create_node("inv", NODE_BITCORE_HASH_LIST, &inv_pack))return 0;

	if (!tree_manager_find_child_node(payload, NODE_HASH("hashes"), NODE_BITCORE_HASH_LIST, &locator)){ release_zone_ref(&inv_pack); return 0; }

	get_locator_next_blocks			 (&locator, &inv_pack);
	if (tree_manager_get_child_at(&inv_pack, 0, &fb))
	{
		hash_t hash;
		if (tree_manager_get_node_hash(&fb, 0, hash))
		{
			tree_manager_get_child_value_i64(&self_node, NODE_HASH("block_height"), &block_height);

			if (find_block_hash(hash, block_height, &height))
			{
				tree_manager_set_child_value_i64(node, "block_height", height);
			}
		}
		release_zone_ref				(&fb);
	}

	queue_inv_message				 (node, &inv_pack);
	release_zone_ref				 (&locator);
	release_zone_ref				 (&inv_pack);


	return 1;
}

int handle_getheaders(mem_zone_ref_ptr node, mem_zone_ref_ptr payload)
{
	mem_zone_ref locator = { PTR_NULL }, inv_pack = { PTR_NULL }, node_qlist = { PTR_NULL };


	if (!tree_manager_find_child_node(payload, NODE_HASH("hashes"), NODE_BITCORE_HASH_LIST, &locator))return 0;

	get_locator_next_blocks	(&locator, &inv_pack);

	if (tree_manager_find_child_node(node, NODE_HASH("queried_headers"), NODE_BITCORE_HASH_LIST, &node_qlist))
	{ 
		tree_manager_cat_node_childs	(&node_qlist, &inv_pack, 0);
		release_zone_ref				(&node_qlist);
	}

	release_zone_ref		(&locator);
	release_zone_ref		(&inv_pack);

	return 1;
}


int handle_inv(mem_zone_ref_ptr node, mem_zone_ref_ptr payload)
{
	mem_zone_ref hash_list = { PTR_NULL };

	tree_manager_find_child_node(payload, NODE_HASH("hashes"), NODE_BITCORE_HASH_LIST, &hash_list);
	queue_getdata_message(node, &hash_list);
	release_zone_ref(&hash_list);


	return 1;
}	




int handle_getdata(mem_zone_ref_ptr node, mem_zone_ref_ptr payload)
{
	mem_zone_ref hash_list = { PTR_NULL }, node_qlist= { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr hash_node = PTR_NULL;
	unsigned int data_type = 0;


	if (!tree_manager_find_child_node(payload, NODE_HASH("hashes"), NODE_BITCORE_HASH_LIST, &hash_list))return 0;
	if (!tree_manager_find_child_node(node, NODE_HASH("queried_hashes"), NODE_BITCORE_HASH_LIST, &node_qlist)){ release_zone_ref(&hash_list); return 0; }

	for (tree_manager_get_first_child(&hash_list, &my_list, &hash_node); ((hash_node != PTR_NULL) && (hash_node->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &hash_node))
	{
		hash_t h;

		tree_manager_get_node_hash(hash_node, 0, h);

		if (!tree_find_child_node_by_member_name_hash(&node_qlist, 0xFFFFFFFF, "hash", h, PTR_NULL))
			tree_manager_node_add_child(&node_qlist, hash_node);
	}

	
	release_zone_ref(&hash_list);
	release_zone_ref(&node_qlist);
	return 1;
}




int handle_getaddr(mem_zone_ref_ptr src_node, mem_zone_ref_ptr payload)
{
	mem_zone_ref		my_list = { PTR_NULL }, peer_nodes = { PTR_NULL }, addrs = { PTR_NULL };
	mem_zone_ref_ptr	node = PTR_NULL;
	ctime_t				now;

	if (!tree_manager_find_child_node(&self_node, NODE_HASH("peer_nodes"), NODE_BITCORE_NODE_LIST, &peer_nodes))
		return 1;
	
	now = get_time_c();
	tree_manager_create_node("addrs", NODE_BITCORE_ADDR_LIST, &addrs);

	for (tree_manager_get_first_child(&peer_nodes, &my_list, &node); ((node != PTR_NULL) && (node->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &node))
	{
		ipv4_t		    ip = { 0 };
		uint64_t	    services;
		mem_zone_ref	addr = { PTR_NULL }, log = { PTR_NULL };
		unsigned int	status;
		unsigned short  port=0;

		if (!tree_manager_get_child_value_i32(node, NODE_HASH("p2p_status"), &status))continue;
		if (status == 0)continue;
		if (is_same_node(src_node, node))continue;
		if (!get_bitcore_addr(node, ip, &port, &services))continue;

		if (tree_manager_add_child_node(&addrs, "addr", NODE_BITCORE_ADDRT, &addr))
		{
			tree_manager_set_child_value_i32 (&addr, "time", now);
			tree_manager_set_child_value_i64 (&addr, "services", services);
			tree_manager_set_child_value_ipv4(&addr, "addr", ip);
			tree_manager_set_child_value_i16 (&addr, "port", port);
			release_zone_ref(&addr);
		}
	}

	queue_addr_message(src_node, &addrs);

	release_zone_ref(&addrs);
	release_zone_ref(&peer_nodes);

	return 1;
}


int handle_version(mem_zone_ref_ptr node, mem_zone_ref_ptr payload)
{
	char		 user_agent[32];
	mem_zone_ref log = { PTR_NULL };
	unsigned int proto_ver, last_blk, node_port,my_port;
	mem_zone_ref addrs[2] = { PTR_NULL };
	uint64_t	 time, services, timestamp;
	int			 index = 0;
	ipv4_t		 node_ip,my_ip;

	tree_manager_get_child_value_i32(payload, NODE_HASH("proto_ver"), &proto_ver);
	tree_manager_get_child_value_i64(payload, NODE_HASH("timestamp"), &time);
	tree_manager_get_child_value_i64(payload, NODE_HASH("services"), &services);
	tree_manager_get_child_value_i64(payload, NODE_HASH("timestamp"), &timestamp);
	tree_manager_get_child_value_str(payload, NODE_HASH("user_agent"),user_agent, 32, 16);
	tree_manager_get_child_value_i32(payload, NODE_HASH("last_blk"), &last_blk);

	index = 1;
	tree_node_list_child_by_type	(payload, NODE_BITCORE_ADDR, &addrs[0], 0);
	tree_node_list_child_by_type	(payload, NODE_BITCORE_ADDR, &addrs[1], 1);

	tree_manager_get_child_value_ipv4(&addrs[0], NODE_HASH("addr"), my_ip);
	tree_manager_get_child_value_i32(&addrs[0], NODE_HASH("port"), &my_port);
	
	tree_manager_get_child_value_ipv4(&addrs[1], NODE_HASH("addr"), node_ip);
	tree_manager_get_child_value_i32 (&addrs[1], NODE_HASH("port"), &node_port);


	tree_manager_create_node("log", NODE_LOG_PARAMS, &log);
	tree_manager_set_child_value_ipv4(&log, "nip", node_ip);
	tree_manager_set_child_value_i32(&log, "nport", node_port);
	tree_manager_set_child_value_ipv4(&log, "ip", my_ip);
	tree_manager_set_child_value_i32(&log, "port", my_port);
	tree_manager_set_child_value_str(&log, "services", services ? "network" : "no services");
	tree_manager_set_child_value_i32(&log, "version", proto_ver);
	tree_manager_set_child_value_str(&log, "user_agent", user_agent);
	tree_manager_set_child_value_i32(&log, "lastblk", last_blk);
	log_message("node %nip%:%nport% version %version% %services% last block: %lastblk%, me %ip%:%port% \n", &log);
	release_zone_ref(&log);
	
	//printf("node %u.%u.%u.%u:%d (%s) version %d '%s' last block : %d, me %u.%u.%u.%u:%d \n", node_ip[0], node_ip[1], node_ip[2], node_ip[3], node_port, services ? "network" : "no services", proto_ver, user_agent, last_blk, my_ip[0], my_ip[1], my_ip[2], my_ip[3], my_port);

	release_zone_ref(&addrs[0]);
	release_zone_ref(&addrs[1]);
	
	queue_verack_message	(node);
	queue_getaddr_message	(node);

	/*
	if (!tree_manager_get_child_value_i64(&self_node, NODE_HASH("block_height"), &nblks))
		nblks = 0;
	if (nblks > (last_blk + 1))
	{
		mem_zone_ref hash_list = { PTR_NULL };
		if (tree_manager_create_node("hashes", NODE_BITCORE_HASH_LIST, &hash_list))
		{
			int num = 1000;
			while (last_blk < nblks)
			{
				if ((num--) == 0)break;
				if (tree_manager_find_child_node(&self_node, NODE_HASH("block_index"), NODE_BITCORE_HASH, &block_index_node))
				{
					hash_t hash;
					if (tree_manager_get_node_hash(&block_index_node, last_blk*sizeof(hash_t), hash))
					{
						mem_zone_ref new_hash = { PTR_NULL };
						if (tree_manager_add_child_node(&hash_list, "hash", NODE_BITCORE_BLOCK_HASH, &new_hash))
						{
							tree_manager_write_node_hash(&new_hash, 0, hash);
							release_zone_ref(&new_hash);
						}
					}
					release_zone_ref(&block_index_node);
				}
				last_blk++;
			}
			queue_inv_message(node, &hash_list);
			release_zone_ref(&hash_list);
		}
	}
	*/
	return 1;
}

int handle_ping(mem_zone_ref_ptr node, mem_zone_ref_ptr payload)
{
	mem_zone_ref nonce_ref = { PTR_NULL };
	uint64_t	nonce;
	
	if (!tree_manager_get_child_value_i64(payload, NODE_HASH("nonce"), &nonce))return 0;

	if (tree_manager_create_node("nonce", NODE_GFX_BINT, &nonce_ref))
	{
		tree_manager_write_node_qword(&nonce_ref, 0, nonce);
		queue_pong_message(node, &nonce_ref);
		release_zone_ref(&nonce_ref);
	}
	return 1;
}

int handle_pong(mem_zone_ref_ptr node, mem_zone_ref_ptr payload)
{
	tree_manager_set_child_value_i32(node,"synching",  1);
	return 1;
}

int handle_verack(mem_zone_ref_ptr node, mem_zone_ref_ptr payload)
{
	queue_ping_message(node);
	return 1;
}

int handle_addr(mem_zone_ref_ptr node, mem_zone_ref_ptr payload)
{

	mem_zone_ref		addr_list	= { PTR_NULL };
	mem_zone_ref		my_list		= { PTR_NULL };
	mem_zone_ref_ptr	addr;

	tree_manager_find_child_node(payload, NODE_HASH("addrs"), NODE_BITCORE_ADDR_LIST, &addr_list);
	for (tree_manager_get_first_child(&addr_list, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
	{
		node_log_addr_infos(addr);
	}
	release_zone_ref(&addr_list);
	return 1;
}

int check_blk(mem_zone_ref_ptr blk, mem_zone_ref_ptr txs)
{
	hash_t				hash;
	hash_t				newMerk, oHash, tmp, nStakeModifierNew, merk, pMod;
	hash_t				lastStakeModifier, StakeModKernel;
	mem_zone_ref		ftx = { PTR_NULL };
	mbedtls_sha256_context	ctx;
	unsigned int			is_staking;

	compute_block_hash(blk, oHash);
	tree_manager_get_child_value_hash(blk, NODE_HASH("blkHash"), pMod);

	if (memcmp_c(oHash, pMod, sizeof(hash_t)))
		return 0;

	tree_manager_get_child_value_hash(blk, NODE_HASH("merkle_root"), merk);
	build_merkel_tree				 (txs, newMerk);

	if (memcmp_c(merk, newMerk, sizeof(hash_t)))
		return 0;

	if (!tree_manager_get_child_at(txs, 0, &ftx))return 0;
	
	is_staking = is_tx_null(&ftx);
	release_zone_ref(&ftx);

	tree_manager_get_child_value_hash(blk, NODE_HASH("prev"), hash);
	
	if (!find_last_stake_modifier(hash, lastStakeModifier))
	{
		memset_c(lastStakeModifier, 0, sizeof(hash_t));
	}
	if (!is_staking)
		tree_manager_get_child_value_hash(blk, NODE_HASH("blkHash"), StakeModKernel);
	else
	{
		tree_manager_get_child_at	(txs, 1, &ftx);
		get_tx_input_hash			(&ftx, 0, StakeModKernel);
	}
		
	release_zone_ref(&ftx);

	mbedtls_sha256_init(&ctx);
	mbedtls_sha256_starts(&ctx, 0);
	mbedtls_sha256_update(&ctx, StakeModKernel, sizeof(hash_t));
	mbedtls_sha256_update(&ctx, lastStakeModifier, sizeof(hash_t));
	mbedtls_sha256_finish(&ctx, tmp);
	mbedtls_sha256_free(&ctx);
	mbedtls_sha256(tmp, 32, nStakeModifierNew, 0);

	tree_manager_get_child_value_hash(blk, NODE_HASH("StakeMod2"), pMod);

	if (memcmp_c(pMod, nStakeModifierNew, sizeof(hash_t)))
		return 0;


	return 1;
}







int add_new_block(mem_zone_ref_ptr header, mem_zone_ref_ptr tx_list)
{
	hash_t			proof;
	mem_zone_ref	last_blk = { PTR_NULL };
	int				ret;
	uint64_t		block_reward = 0, cur_len=0;

	tree_manager_get_child_value_i64(&self_node, NODE_HASH("block_height"), &cur_len);
	
	if (!store_block(header, tx_list))
	{
		log_output("error storing block\n");
		return 0;
	}
	if (tree_manager_get_child_value_hash(header, NODE_HASH("blk pow"), proof))
	{
		ret = get_blockreward(cur_len, &block_reward);
		add_moneysupply(block_reward);
		node_inc_pow_blocks();
	}

	if (pos_kernel != PTR_NULL)
	{
		if (!store_blk_staking(header))
		{
			log_output("store_blk_staking error\n");
			return 0;
		}
		if (!store_blk_tx_staking(tx_list))
		{
			log_output("store_blk_tx_staking error\n");
			return 0;
		}
		if (tree_manager_get_child_value_hash(header, NODE_HASH("blk pos"), proof))
		{
			ret = get_stake_reward(cur_len, &block_reward);
			add_moneysupply(block_reward);
		}
	}


	
	ret = node_set_last_block	(header);
	if (!ret)log_output("node_set_last_block error\n");

	if (ret)
	{
		tree_manager_get_child_value_i64(&self_node, NODE_HASH("block_height"), &cur_len);

		if (tree_manager_get_child_value_hash(header, NODE_HASH("blk pos"), proof))
		{
			mem_zone_ref nBits = { PTR_NULL };

			if (!tree_manager_find_child_node(&self_node, NODE_HASH("current_pos_diff"), NODE_GFX_INT, &nBits))
				tree_manager_add_child_node(&self_node, "current_pos_diff", NODE_GFX_INT, &nBits);

			ret = compute_last_pos_diff(header, &nBits);
			release_zone_ref(&nBits);

			ret = get_stake_reward(cur_len, &block_reward);
			if (ret)tree_manager_set_child_value_i64(&self_node, "pos_reward", block_reward);
		}
	}
	

	if (ret){
		node_aquire_mempool_lock(PTR_NULL);
		ret = node_del_txs_from_mempool(tx_list);
		if (ret)ret = node_del_btree_from_mempool();
		node_release_mempool_lock();
	}
	if (ret)ret = store_wallet_txs				(tx_list);
	
	if(ret)
		log_message("blk[%blkHash%]", header);
	else
		log_message("error adding new block: %blkHash% , %time% - %version% %merkle_root%\n", header);

	return ret;
}


int handle_block(mem_zone_ref_ptr node, mem_zone_ref_ptr payload)
{
	hash_t				prevHash;
	hash_t				blk_hash;
	mem_zone_ref		header = { PTR_NULL }, tx_list = { PTR_NULL }, sig = { PTR_NULL }, pending_blk = { PTR_NULL };
	mem_zone_ref		log = { PTR_NULL };
	size_t				nz1 = 0, nz2 = 0;
	struct string		signature = { PTR_NULL }, msg_str = { PTR_NULL };
	int					ret = 1;
	uint64_t			block_height;
	int					testing=0;
	unsigned int		keep_block, cur_len;

	tree_manager_set_child_value_i32(&self_node, "next_check", get_time_c() + 5);
	
		
	if (!tree_manager_find_child_node(payload, NODE_HASH("header"), NODE_BITCORE_BLK_HDR, &header))
	{
		log_output("no blk hdr\n");
		return 1;
	}
	
	if (!tree_manager_get_child_value_hash(&header, NODE_HASH("prev"), prevHash))
	{
		log_output("no blk prev\n");
		release_zone_ref(&header);
		return 1;
	}
	
	if (!tree_manager_get_child_value_i32(&header, NODE_HASH("keep_block"), &keep_block))
		keep_block = 1;

	if (!keep_block)
	{
		log_output("no keep blk\n");
		release_zone_ref(&header);
		return 1;
	}
	tree_manager_get_child_value_i64(&self_node, NODE_HASH("block_height"), &block_height);

	cur_len = block_height;

	compute_block_hash					(&header, blk_hash);
	tree_manager_set_child_value_bhash	(&header, "blkHash", blk_hash);


	if (!tree_manager_find_child_node(payload, NODE_HASH("txs"), NODE_BITCORE_TX_LIST, &tx_list))
	{
		log_output("no blk tx\n");
		release_zone_ref(&header);
		node_release_mining_lock();
		return 1;
	}
	/*
	tree_manager_find_child_node(&self_node, NODE_HASH("pending blocks"), NODE_BITCORE_HASH_LIST, &pending_blk);

	if (!tree_find_child_node_by_hash(&pending_blk, NODE_BITCORE_BLOCK_HASH, blk_hash,PTR_NULL))
	{
		log_output("not pending block\n");
		release_zone_ref(&tx_list);
		release_zone_ref(&header);
		return 1;
	}
	release_zone_ref(&pending_blk);
	*/
	node_aquire_mining_lock();
	
	tree_manager_get_child_value_i32	(node, NODE_HASH("testing_chain"), &testing);
		
	if (testing>0)
	{
		hash_t			lh;
		unsigned int 	new_len, bestChainDepth;

		log_output("testing chain\n");

		if (!tree_manager_get_child_value_i32(node, NODE_HASH("bestChainDepth"), &bestChainDepth))
			bestChainDepth = 0;
		
		
		if (!tree_manager_get_child_value_hash(node, NODE_HASH("last_header_hash"), lh))
			node_get_hash_idx(testing-1, lh);

		tree_manager_create_node			("log", NODE_LOG_PARAMS, &log);
		tree_manager_set_child_value_hash	(&log, "ph", prevHash);
		tree_manager_set_child_value_hash	(&log, "lh", lh);
		log_message							(" testing block '%ph%' -- '%lh%'", &log);
		release_zone_ref					(&log);

		if (memcmp_c(prevHash, lh, sizeof(hash_t)))
		{
			release_zone_ref(&header);
			release_zone_ref(&tx_list);
			node_release_mining_lock();
			return 0;
		}

		new_len	=	node_add_block_header(node, &header);
		

		tree_manager_create_node("log", NODE_LOG_PARAMS, &log);
		tree_manager_set_child_value_i32(&log, "new_len", new_len);
		tree_manager_set_child_value_i32(&log, "cur_len", cur_len);
		log_message("testing chain at height : %new_len% - %cur_len%", &log);
		release_zone_ref(&log);

		if (new_len > (cur_len + bestChainDepth))
		{
			if(truncate_chain(testing))
			{ 
				log_output						("switching to new chain \n");
				tree_manager_set_child_value_i32(node, "testing_chain", 0);
			}

			
		}
		release_zone_ref(&tx_list);
		release_zone_ref(&header);
		node_release_mining_lock();
		return 0;
	}



	if (!tree_manager_find_child_node(payload, NODE_HASH("signature"), NODE_BITCORE_ECDSA_SIG, &sig))
	{ 
		log_output("no blk sig\n");
		release_zone_ref(&tx_list); 
		release_zone_ref(&header); 
		node_release_mining_lock();
		return 1; 
	}
	tree_manager_get_node_istr(&sig, 0, &signature, 0);
	release_zone_ref(&sig);

	ret = node_is_next_block	(&header);
	if (!ret)
	{
		log_output("not next block\n");

		ret = node_check_chain(node, &header);
	
		if (!tree_manager_get_child_value_i32(&header, NODE_HASH("keep_block"), &keep_block))
			keep_block = 1;

		if (!ret)
		{
			log_output("not check chain\n");

			free_string		(&signature);
			release_zone_ref(&header);
			release_zone_ref(&tx_list);
			node_release_mining_lock();
			return (keep_block==1)?0:1;
		}
		log_output("check chain\n");
	}

	if (pos_kernel != PTR_NULL)
	{
		mem_zone_ref sig = { PTR_NULL };

		if (!tree_manager_find_child_node(&header, NODE_HASH("signature"), NODE_BITCORE_ECDSA_SIG, &sig))
			tree_manager_add_child_node(&header, "signature", NODE_BITCORE_ECDSA_SIG, &sig);

		tree_manager_write_node_sig(&sig, 0, signature.str, signature.len);
		release_zone_ref(&sig);
	}


	

	if (find_block_hash(blk_hash, block_height,PTR_NULL))
	{
		return 1;
	}
	
	ret = accept_block(&header, &tx_list);
	if (!ret)
	{
		log_output("accept block error\n");

		tree_manager_dump_node_rec(&header, 0, 8);
		tree_manager_dump_node_rec(&tx_list, 0, 8);

		snooze_c(10000);

		tree_manager_set_child_value_i32(node, "need_recon", 1);
		free_string(&signature);
		release_zone_ref(&header);
		release_zone_ref(&tx_list);
		node_release_mining_lock();
		return 1;
	}

	ret=add_new_block(&header, &tx_list);

	node_release_mining_lock();
	if (!ret)
	{
		log_output("add_new_block error\n");
		tree_manager_dump_node_rec(&header, 0, 8);
		tree_manager_dump_node_rec(&tx_list, 0, 8);

		snooze_c(10000);

		tree_manager_set_child_value_i32(node, "need_recon", 1);
		free_string(&signature);
		release_zone_ref(&header);
		release_zone_ref(&tx_list);
		return 1;
	}

	
	
	if (ret)
	{
		mem_zone_ref	 my_list = { PTR_NULL }, files = { PTR_NULL };
		mem_zone_ref_ptr tx = PTR_NULL;
		unsigned int	 nFiles = 0;

		tree_manager_create_node("files", NODE_BITCORE_HASH_LIST, &files);
		for (tree_manager_get_first_child(&tx_list, &my_list, &tx); ((tx != NULL) && (tx->zone != NULL)); tree_manager_get_next_child(&my_list, &tx))
		{
			struct string app_name = { 0 };

			if (tree_manager_get_child_value_istr(tx, NODE_HASH("appFile"), &app_name, 0))
			{
				if(is_trusted_app(app_name.str))
					get_tx_file(tx, &files);

				free_string(&app_name);
			}
		}

		if (tree_manager_get_node_num_children(&files)>0)
		{
			queue_getdata_message(node, &files);
		}
		release_zone_ref(&files);
	}


	tree_manager_set_child_value_i32	(&self_node, "next_check", get_time_c() + 5);
	

	free_string							(&signature);
	release_zone_ref					(&header);
	release_zone_ref					(&tx_list);


	
	return ret;
}

int handle_headers(mem_zone_ref_ptr node, mem_zone_ref_ptr payload)
{
	mem_zone_ref hash_list = { PTR_NULL }, hdr_list = { PTR_NULL };

	tree_manager_find_child_node	(node, NODE_HASH("block_headers"), NODE_BITCORE_BLK_HDR_LIST, &hdr_list);
	get_hash_list					(&hdr_list, &hash_list);
	tree_remove_children			(&hdr_list);
	release_zone_ref				(&hdr_list);
	release_zone_ref				(&hash_list);

	return 1;
}




int handle_message(mem_zone_ref_ptr node,const char *cmd,mem_zone_ref_ptr payload)
{
	unsigned int testing=0;

	if (!strncmp_c(cmd, "headers", 7))return handle_headers(node, payload);
	if (!strncmp_c(cmd, "block", 5))return handle_block(node, payload);
	//if (!strncmp_c(cmd, "inv", 3))return handle_inv(node, payload);

	tree_manager_get_child_value_i32(node, NODE_HASH("testing_chain"), &testing);
	if (testing > 0)return 1;
	
	/*
	if (!strncmp_c(cmd, "verack", 6)){ return handle_verack(node, payload); }
	if (!strncmp_c(cmd, "version", 7))return handle_version(node, payload);
	if (!strncmp_c(cmd, "ping", 4))return handle_ping(node, payload);
	if (!strncmp_c(cmd, "pong", 4))return handle_pong(node, payload);
	if (!strncmp_c(cmd, "addr", 4))return handle_addr(node, payload);
	*/
	
	
	if (!strncmp_c(cmd, "getaddr", 7))return handle_getaddr(node, payload);
	if (!strncmp_c(cmd, "getdata", 7))return handle_getdata(node, payload);
	if (!strncmp_c(cmd, "getblocks", 9))return handle_getblocks(node, payload);
	if (!strncmp_c(cmd, "file", 4))return handle_file(node, payload);

	

	
	return -1;
}



int handle_element(mem_zone_ref_ptr node, mem_zone_ref_ptr element)
{
	switch (tree_mamanger_get_node_type(element))
	{
		case NODE_BITCORE_ADDRT:
			node_log_addr_infos(element);
			return 1;
		break;
		case NODE_BITCORE_BLK_HDR:
			node_add_block_header(node, element);
			return 1;
		break;
	}
	return 0;
}

OS_API_C_FUNC(int) scan_addresses()
{
	mem_zone_ref scan_list = { PTR_NULL };

	if (tree_manager_find_child_node(&self_node, NODE_HASH("addr scan list"), NODE_BITCORE_WALLET_ADDR_LIST, &scan_list))
	{
		mem_zone_ref_ptr	addr = PTR_NULL;
		mem_zone_ref		my_list = { PTR_NULL };
		for (tree_manager_get_first_child(&scan_list, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
		{

			mem_zone_ref new_addr = { PTR_NULL };
			if (tree_manager_find_child_node(addr, NODE_HASH("addr"), NODE_BITCORE_WALLET_ADDR, &new_addr))
			{
				btc_addr_t						addr;

				tree_manager_get_node_btcaddr	(&new_addr, 0, addr);
				rescan_addr						(addr);

				//background_func(scanaddr_func, &new_addr);
				release_zone_ref(&new_addr);
			}
			tree_manager_set_child_value_i32(addr, "done", 1);
		}
		tree_remove_child_by_member_value_dword(&scan_list, NODE_BITCORE_WALLET_ADDR, "done", 1);
		release_zone_ref(&scan_list);
	}

	return 1;
}



int process_node_messages(mem_zone_ref_ptr node)
{
	mem_zone_ref		msg_list = { PTR_NULL };
	mem_zone_ref_ptr	msg = PTR_NULL;
	mem_zone_ref		my_list = { PTR_NULL };
	unsigned int		now;


	if (!tree_manager_find_child_node(node, NODE_HASH("emitted_queue"), NODE_BITCORE_MSG_LIST, &msg_list))return 0;

	now = get_time_c();

	tree_manager_sort_childs(&msg_list, "priority", 1);

	for (tree_manager_get_first_child(&msg_list, &my_list, &msg); ((msg != NULL) && (msg->zone != NULL)); tree_manager_get_next_child(&my_list, &msg))
	{
		char			cmd[16];
		mem_zone_ref	payload_node = { PTR_NULL };
		int				ret, hndl;

		if (tree_mamanger_get_node_type(msg) != NODE_BITCORE_MSG) continue;
		if (!tree_manager_get_child_value_str	(msg, NODE_HASH("cmd"), cmd, 12, 16))continue;
		if (!tree_manager_get_child_value_si32	(msg, NODE_HASH("handled"), &hndl))hndl = -1;
		if (hndl != -1)continue;

		ret=node_process_event_handler		("emitted_queue", node, msg);
		if (ret<0)
		{
			tree_manager_find_child_node(msg, NODE_HASH("payload"), NODE_BITCORE_PAYLOAD, &payload_node);
			ret = handle_message		(node, cmd, &payload_node);
			release_zone_ref			(&payload_node);
		}

		if (ret < 0)ret = 1;
		tree_manager_set_child_value_si32(msg , "handled"		, ret);
		tree_manager_set_child_value_si32(node, "last_msg_time"	, now);
	}
	tree_remove_child_by_member_value_dword			(&msg_list, NODE_BITCORE_MSG, "handled" , 1);
	tree_remove_child_by_member_value_lt_dword		(&msg_list, NODE_BITCORE_MSG, "recvtime", now-300);

	release_zone_ref(&msg_list);
	return 1;
}

int process_node_elements(mem_zone_ref_ptr node)
{
	mem_zone_ref		list = { PTR_NULL };
	mem_zone_ref_ptr	el = PTR_NULL;
	mem_zone_ref		my_list = { PTR_NULL };
	int					ret;

	if (!tree_manager_find_child_node(node, NODE_HASH("emitted elements"), NODE_BITCORE_MSG_LIST, &list))return 0;

	for (tree_manager_get_first_child(&list, &my_list, &el); ((el != NULL) && (el->zone != NULL)); tree_manager_get_next_child(&my_list, &el))
	{
		ret=handle_element				(node, el);
	}
	tree_remove_children(&list);
	release_zone_ref(&list);
	return 1;
}





int process_nodes()
{
	mem_zone_ref_ptr	node = PTR_NULL;
	mem_zone_ref		my_list = { PTR_NULL }, peer_nodes = { PTR_NULL }, my_node = { PTR_NULL }, apps = { PTR_NULL }, missing = { PTR_NULL }, pending = { PTR_NULL },my_file_node = { PTR_NULL };
	mem_zone_ref_ptr	app=PTR_NULL;
	ctime_t				min_delay, cctime;
	uint64_t			self_height;
	unsigned int		next_check, curtime, n_missing_files;

	if (!tree_manager_find_child_node(&self_node, NODE_HASH("peer_nodes"), NODE_BITCORE_NODE_LIST, &peer_nodes))return 0;
	
	if (!tree_manager_get_child_value_i64(&self_node, NODE_HASH("block_height"), &self_height))
		self_height = 0;


	tree_manager_find_child_node(&self_node, NODE_HASH("pending files"), NODE_BITCORE_HASH_LIST, &pending);
	tree_manager_create_node	("missing", NODE_BITCORE_HASH_LIST, &missing);

	get_apps(&apps);

	for (tree_manager_get_first_child(&apps, &my_list, &app); ((app != NULL) && (app->zone != NULL)); tree_manager_get_next_child(&my_list, &app))
	{
		struct string appName = { PTR_NULL };

		tree_manager_get_child_value_istr(app, NODE_HASH("appName"), &appName, 0);
		get_app_missing_files(&appName, &pending, &missing);
		free_string(&appName);
	}
	release_zone_ref(&apps);
	release_zone_ref(&pending);

	n_missing_files = tree_manager_get_node_num_children(&missing);

	if (n_missing_files == 0)
	{
		release_zone_ref(&missing);
	}


	curtime		= get_time_c();
	get_system_time_c		(&cctime);
	min_delay	= 10000000;

	for (tree_manager_get_first_child(&peer_nodes, &my_list, &node); ((node != NULL) && (node->zone != NULL)); tree_manager_get_next_child(&my_list, &node))
	{
		unsigned int		test, synching;
		unsigned int		last_msg_time,status;
		uint64_t			block_height;
		ctime_t				ping_delay, last_ping, next_pong;

		if (!tree_manager_get_child_value_i32(node, NODE_HASH("p2p_status"), &status))status = 0;

		if (status == 0)continue;

		if (!tree_manager_get_child_value_i64(node, NODE_HASH("block_height"), &block_height))block_height = 0;

		process_node_messages(node);
		process_node_elements(node);

		
		if (!tree_manager_get_child_value_i32 (node, NODE_HASH("synching"), &synching))synching = 0;
		if (!tree_manager_get_child_value_i32 (node, NODE_HASH("testing_chain"), &test))test = 0;
		if (!tree_manager_get_child_value_si64(node, NODE_HASH("last_ping"), &last_ping))last_ping = 0;
		if (!tree_manager_get_child_value_i32 (node, NODE_HASH("last_msg_time"), &last_msg_time))last_msg_time = 0;
		if (!tree_manager_get_child_value_si64(node, NODE_HASH("next_pong"), &next_pong))next_pong = 0;

		
		if ((last_msg_time!=0)&&(curtime - last_msg_time) > 120)
		{
			log_output						("node ping timeout \n");
			tree_manager_set_child_value_i32(node, "need_recon", 1);
			continue;
		}

		if ( ((last_msg_time != 0) && ((curtime - last_msg_time) > 60)) && ((cctime-last_ping)>60000))
			queue_ping_message(node);

		if (!tree_manager_get_child_value_si64(node, NODE_HASH("ping_delay"), &ping_delay))
			ping_delay = 0;

		if (ping_delay < 0)
		{
			ping_delay = -ping_delay;
			tree_manager_set_child_value_si64(node, "ping_delay", ping_delay);
		}

		if (n_missing_files > 0)
		{
			if ((test == 0) && (ping_delay > 0) && (block_height >= self_height))
			{
				if (ping_delay <= min_delay)
				{
					min_delay = ping_delay;
					copy_zone_ref(&my_file_node, node);
				}
			}
		}

		if ((test == 0) && (synching == 1) && (ping_delay>0) && (block_height>self_height))
		{
			if (ping_delay <= min_delay)
			{
				min_delay = ping_delay;
				copy_zone_ref(&my_node, node);
			}
		}
	}

	if (my_file_node.zone!=PTR_NULL)
	{
		queue_getdata_message	(&my_file_node, &missing);
		release_zone_ref		(&my_file_node);
	}
	release_zone_ref(&missing);
	scan_addresses();

	if (my_node.zone != PTR_NULL)
	{
		if (tree_manager_get_child_value_i32(&self_node, NODE_HASH("next_check"), &next_check))
		{
			if (curtime >= next_check)
			{
				queue_getblocks_message				(&my_node);
				tree_manager_set_child_value_i32	(&self_node, "next_check", get_time_c() + 30);
			}
		}
		release_zone_ref(&my_node);
	}

	release_zone_ref(&peer_nodes);
	return 1;
}



int load_pos_module(const char *mod_name,const char *mod_file,tpo_mod_file *tpomod)
{
	if (!load_module(mod_file, mod_name, tpomod))return 0;

#ifndef _DEBUG
	init_pos					= (init_pos_func_ptr)get_tpo_mod_exp_addr_name(tpomod, "init_pos", 0);
	store_blk_staking			= (store_blk_staking_func_ptr)get_tpo_mod_exp_addr_name(tpomod, "store_blk_staking", 0);
	store_blk_tx_staking		= (store_blk_staking_func_ptr)get_tpo_mod_exp_addr_name(tpomod, "store_blk_tx_staking", 0);
	compute_last_pos_diff		= (compute_last_pos_diff_func_ptr)get_tpo_mod_exp_addr_name(tpomod, "compute_last_pos_diff", 0);
	store_tx_staking			= (store_tx_staking_func_ptr)get_tpo_mod_exp_addr_name(tpomod, "store_tx_staking", 0);
	get_stake_reward            = (get_stake_reward_func_ptr)get_tpo_mod_exp_addr_name(tpomod, "get_stake_reward", 0);
	check_tx_pos				= (check_tx_pos_func_ptr)get_tpo_mod_exp_addr_name(tpomod, "check_tx_pos", 0);
	check_blk_sig				= (check_blk_sig_func_ptr)get_tpo_mod_exp_addr_name(tpomod, "check_blk_sig", 0);
	load_last_pos_blk			= (load_last_pos_blk_func_ptr)get_tpo_mod_exp_addr_name(tpomod, "load_last_pos_blk", 0);
	find_last_pos_block			= (find_last_pos_block_func_ptr)get_tpo_mod_exp_addr_name(tpomod, "find_last_pos_block", 0);
	find_blk_staking_tx			=	(find_blk_staking_tx_func_ptr) get_tpo_mod_exp_addr_name(tpomod, "find_blk_staking_tx", 0);
	check_blk_sig				=	 (check_blk_sig_func_ptr)get_tpo_mod_exp_addr_name(tpomod, "check_blk_sig", 0);
	find_last_stake_modifier	= (find_last_stake_modifier_func_ptr)get_tpo_mod_exp_addr_name(tpomod, "find_last_stake_modifier", 0);
							 
#endif
	return 1;
}



OS_API_C_FUNC(int) app_init(mem_zone_ref_ptr params)
{
	int					ret=0;

	
	pos_kernel = PTR_NULL;
	self_node.zone = PTR_NULL;
	pos_kernel_def.zone = PTR_NULL;

	memset_c(null_hash, 0, 32);

	init_upnp();
	

	create_dir("txs");
	if (stat_file("txs") != 0)
	{
		log_message("unable to create tx dir \n", PTR_NULL);
		return 0;
	}
	
	create_dir("utxos");
	if (stat_file("utxos") != 0)
	{
		log_message("unable to create utxo dir \n", PTR_NULL);
		return 0;
	}

	create_dir("adrs");
	if (stat_file("adrs") != 0)
	{
		log_message("unable to create adrs dir \n", PTR_NULL);
		return 0;
	}
	
	if (params != PTR_NULL)
	{
		mem_zone_ref	stake_mod_def = { PTR_NULL };
		ret = resolve_script_var		(params, PTR_NULL,"SelfNode"							, NODE_BITCORE_NODE, &self_node,PTR_NULL);
		if (ret)ret = resolve_script_var(params, PTR_NULL, "configuration.staking.pos_kernel", NODE_MODULE_DEF, &pos_kernel_def, PTR_NULL);
		if(ret)node_set_script			(params);
	}

	if (get_zone_area_type(&self_node) & 0x10)
		tree_manager_set_child_value_i32(&self_node, "useGC", 1);
	else
		tree_manager_set_child_value_i32(&self_node, "useGC", 0);
	
	if (!ret)
	{
		release_zone_ref(&self_node);
		release_zone_ref(&pos_kernel_def);
	}

	return ret;
}

OS_API_C_FUNC(int) init_staking()
{
	int ret;

	ret = tree_manager_get_child_value_ptr(&pos_kernel_def, NODE_HASH("mod_ptr"), 0, &pos_kernel);
	if (ret)
	{
#ifndef _DEBUG
		init_pos = (init_pos_func_ptr)get_tpo_mod_exp_addr_name(pos_kernel, "init_pos", 0);
		store_blk_staking = (store_blk_staking_func_ptr)get_tpo_mod_exp_addr_name(pos_kernel, "store_blk_staking", 0);
		store_blk_tx_staking = (store_blk_staking_func_ptr)get_tpo_mod_exp_addr_name(pos_kernel, "store_blk_tx_staking", 0);
		compute_last_pos_diff = (compute_last_pos_diff_func_ptr)get_tpo_mod_exp_addr_name(pos_kernel, "compute_last_pos_diff", 0);
		store_tx_staking = (store_tx_staking_func_ptr)get_tpo_mod_exp_addr_name(pos_kernel, "store_tx_staking", 0);
		get_stake_reward = (get_stake_reward_func_ptr)get_tpo_mod_exp_addr_name(pos_kernel, "get_stake_reward", 0);
		check_tx_pos = (check_tx_pos_func_ptr)get_tpo_mod_exp_addr_name(pos_kernel, "check_tx_pos", 0);
		load_last_pos_blk = (load_last_pos_blk_func_ptr)get_tpo_mod_exp_addr_name(pos_kernel, "load_last_pos_blk", 0);
		find_last_pos_block = (find_last_pos_block_func_ptr)get_tpo_mod_exp_addr_name(pos_kernel, "find_last_pos_block", 0);
		find_blk_staking_tx = (find_blk_staking_tx_func_ptr)get_tpo_mod_exp_addr_name(pos_kernel, "find_blk_staking_tx", 0);
		check_blk_sig = (check_blk_sig_func_ptr)get_tpo_mod_exp_addr_name(pos_kernel, "check_blk_sig", 0);
		find_last_stake_modifier = (find_last_stake_modifier_func_ptr)get_tpo_mod_exp_addr_name(pos_kernel, "find_last_stake_modifier", 0);
#endif
	}
	init_wallet(&self_node, pos_kernel);
}

OS_API_C_FUNC(int) app_start(mem_zone_ref_ptr params)
{
	mem_zone_ref		service = { PTR_NULL };
	char				**params_ptr;
	int					rebuild_tx, rebuild_supply;
	int					ret;
	uint64_t			height;



	if (tree_node_find_child_by_type(&self_node, NODE_SERVICE, &service, 0))
	{
		node_init_service(&service, &pos_kernel_def);
		release_zone_ref(&service);
	}
		
	rebuild_tx = 0;
	rebuild_supply = 0;

	if ((params != PTR_NULL) && (params->zone != PTR_NULL))
	{
		int n=0;
		while ((params_ptr = (char **)get_zone_ptr(params, (n++)*sizeof(mem_ptr))) != PTR_NULL)
		{
			char *cmd = (*params_ptr);
			if (cmd == PTR_NULL)break;

			if (!stricmp_c(cmd, "rebuildtxs"))
			{
				rebuild_tx = 1;
			}
			if (!stricmp_c(cmd, "rebuildsupply"))
			{
				rebuild_supply = 1;
			}
		}
	}
	

	log_output("app start\n");

	tree_manager_get_child_value_i64(&self_node, NODE_HASH("block_height"), &height);
	

	//truncate_chain(110380);


	if (rebuild_tx)
		node_rewrite_txs(100);
	
	if (rebuild_supply)
		rebuild_money_supply();

	log_output("app started\n");
	return 1;
}



OS_API_C_FUNC(int) app_loop(mem_zone_ref_ptr params)
{
	mem_zone_ref		blk_list = { PTR_NULL }, tx_list = { PTR_NULL };
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref		hash_list = { PTR_NULL };
	unsigned int		new_block = 0, new_tx = 0;;
	


	
	empty_trash();
	do_mark_sweep(get_tree_mem_area_id(),5000);
	

	node_check_services();
	
	
	node_check_new_connections	();
	update_peernodes			();
	process_nodes				();
	do_staking					();

	if (tree_manager_find_child_node(&self_node, NODE_HASH("submitted txs"), NODE_BITCORE_TX_LIST, &tx_list))
	{
		mem_zone_ref		my_list = { PTR_NULL };
		mem_zone_ref_ptr	tx = PTR_NULL,my_list_ptr;


		if ((get_zone_area_type(&tx_list) & 0x10) == 0)
			my_list_ptr = &my_list;

	
		tree_manager_create_node("hashes", NODE_BITCORE_HASH_LIST, &hash_list);
		for (tree_manager_get_first_child_shared(&tx_list,&my_list_ptr, &tx); ((tx != PTR_NULL) && (tx->zone != PTR_NULL)); tree_manager_get_next_child_shared(&my_list_ptr, &tx))
		{
			mem_zone_ref		new_hash = { PTR_NULL };
			unsigned int		sbm;
			
			if (!tree_manager_get_child_value_i32(tx, NODE_HASH("submitted"), &sbm))
				sbm = 0;

			if (sbm == 1)continue;

			if (tree_manager_add_child_node(&hash_list, "hash", NODE_BITCORE_TX_HASH, &new_hash))
			{
				mem_zone_ref		nnew_tx = { PTR_NULL };
				hash_t				txh;

				tree_manager_set_child_value_bool	(tx, "submitted", 1);

				tree_manager_node_dup				(PTR_NULL, tx, &nnew_tx, 8);

				tree_manager_get_child_value_hash	(&nnew_tx, NODE_HASH("txid"), txh);
				tree_manager_write_node_hash		(&new_hash, 0, txh);
				release_zone_ref					(&new_hash);
				if(node_add_tx_to_mempool			(&nnew_tx))
					new_tx = 1;

				release_zone_ref(&nnew_tx);
			}
		}
		if (new_tx)
		{
			mem_zone_ref_ptr	node = PTR_NULL;
			mem_zone_ref		peer_nodes = { PTR_NULL };

			if (tree_manager_find_child_node(&self_node, NODE_HASH("peer_nodes"), NODE_BITCORE_NODE_LIST, &peer_nodes))
			{
				for (tree_manager_get_first_child(&peer_nodes, &my_list, &node); ((node != NULL) && (node->zone != NULL)); tree_manager_get_next_child(&my_list, &node))
				{
					queue_inv_message(node, &hash_list);
				}
				release_zone_ref(&peer_nodes);
			}
			tree_remove_child_by_member_value_dword(&tx_list, NODE_BITCORE_TX, "submitted", 1);
		}
		release_zone_ref(&tx_list);
		release_zone_ref(&hash_list);
	}
	
	



	if (tree_manager_find_child_node(&self_node, NODE_HASH("submitted blocks"), NODE_BITCORE_BLOCK_LIST, &blk_list))
	{
		mem_zone_ref_ptr	blk = PTR_NULL;
		tree_manager_create_node("hashes", NODE_BITCORE_HASH_LIST, &hash_list);

		for (tree_manager_get_first_child(&blk_list, &my_list, &blk); ((blk != PTR_NULL) && (blk->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &blk))
		{
			hash_t			blk_hash = { 0 };
			mem_zone_ref	new_blk = { PTR_NULL };
			unsigned int	ready;
			int				ret;
			if (!tree_manager_get_child_value_i32(blk, NODE_HASH("ready"), &ready))
				ready = 0;
			
			if(ready==0)
				continue;
			
			if (!tree_manager_find_child_node(blk, NODE_HASH("txs"), NODE_BITCORE_TX_LIST, &tx_list))
			{
				tree_manager_set_child_value_bool(blk, "done", 1);
				continue;
			}


			tree_manager_node_dup(PTR_NULL, blk, &new_blk, 8);
			
			ret=accept_block					(&new_blk, &tx_list);
			if (ret)
			{
				mem_zone_ref		new_hash = { PTR_NULL };
				ret= add_new_block(&new_blk, &tx_list);
				if (ret)
				{
					tree_manager_get_child_value_hash(&new_blk, NODE_HASH("blkHash"), blk_hash);
					if (tree_manager_add_child_node(&hash_list, "hash", NODE_BITCORE_BLOCK_HASH, &new_hash))
					{
						tree_manager_write_node_hash(&new_hash, 0, blk_hash);
						release_zone_ref(&new_hash);
					}
					new_block = 1;
				}
			}

			release_zone_ref					(&new_blk);
			release_zone_ref					(&tx_list);
			tree_manager_set_child_value_bool	(blk, "done", 1);
		}
		tree_remove_child_by_member_value_dword(&blk_list, NODE_BITCORE_BLK_HDR, "done", 1);
		release_zone_ref(&blk_list);

		if (new_block)
		{
			mem_zone_ref_ptr	node = PTR_NULL;
			mem_zone_ref		peer_nodes = { PTR_NULL };

			if (tree_manager_find_child_node(&self_node, NODE_HASH("peer_nodes"), NODE_BITCORE_NODE_LIST, &peer_nodes))
			{
				for (tree_manager_get_first_child(&peer_nodes, &my_list, &node); ((node != NULL) && (node->zone != NULL)); tree_manager_get_next_child(&my_list, &node))
				{
					queue_inv_message(node, &hash_list);
				}
				release_zone_ref(&peer_nodes);
			}
		}
		release_zone_ref(&hash_list);
	}




	return 1;
}

OS_API_C_FUNC(int) app_stop(mem_zone_ref_ptr params)
{
	return 1;
}

#ifdef _WIN32
	unsigned int C_API_FUNC _DllMainCRTStartup(unsigned int *prev, unsigned int *cur, unsigned int *xx){return 1;}
#endif
