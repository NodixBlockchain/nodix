﻿//copyright antoine bentue-ferrer 2016
#include <stdio.h>

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

#include "../block_adx/block_api.h"
#include "../wallet/wallet_api.h"
#include "../node_adx/node_api.h"

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
typedef int				C_API_FUNC	find_last_pos_block_func(mem_zone_ref_ptr pindex, unsigned int *block_time);

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

#endif

//protocol module
C_IMPORT int			C_API_FUNC create_getheaders_message(mem_zone_ref_ptr node, mem_zone_ref_ptr blk_locator, hash_t hash_stop, mem_zone_ref_ptr blk_hdr_pack);
C_IMPORT int			C_API_FUNC node_process_event_handler(const char *msg_list_name, mem_zone_ref_ptr node, mem_zone_ref_ptr msg);




OS_API_C_FUNC(int) compute_pow_diff(mem_zone_ref_ptr newPOWBlock, mem_zone_ref_ptr nBits)
{
	mem_zone_ref		pprev = { PTR_NULL }, spacing = { PTR_NULL };
	int64_t				nActualSpacing;
	unsigned int		curTime,lastTime;
	int					ret;

	if (!tree_manager_get_child_value_i32(newPOWBlock, NODE_HASH("time"), &curTime))return 0;
	if (!tree_manager_find_child_node(&self_node, NODE_HASH("lastPOWBlk"), NODE_BITCORE_BLK_HDR, &pprev))return 0;

	if (!tree_manager_get_child_value_i32(&pprev, NODE_HASH("time"), &lastTime))
	{
		release_zone_ref(&pprev);
		return 0;
	}
	nActualSpacing = curTime - lastTime;
	
	tree_manager_create_node("spacing", NODE_GFX_INT, &spacing);
	tree_manager_write_node_dword(&spacing, 0, nActualSpacing);

	ret = block_compute_pow_target(&spacing, nBits);

	release_zone_ref(&spacing);
	release_zone_ref(&pprev);
	return ret;
}


void rebuild_money_supply()
{
	mem_zone_ref block_index_node = { PTR_NULL }, blk_hdr = { PTR_NULL }, last_blk_hdr = { PTR_NULL };
	uint64_t start, cur, lb, last_log, total_supply;
	int		 ret;
	if (!tree_manager_find_child_node(&self_node, NODE_HASH("block_index"), NODE_BITCORE_HASH, &block_index_node))return;

	lb = get_last_block_height();
	start = 0;
	total_supply = 0;
	cur = start;
	last_log = cur;
	reset_moneysupply();
	while (cur < lb)
	{
		char			chash[65];
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
		tree_manager_get_node_str(&block_index_node, cur * 32, chash, 65, 16);
		if (!load_blk_hdr(&blk_hdr, chash))
			break;

		if (last_blk_hdr.zone != PTR_NULL)
		{
			mem_zone_ref txs = { PTR_NULL };
			mem_zone_ref stake_tx = { PTR_NULL };

			struct string pubK = { PTR_NULL };


			tree_manager_create_node("txs", NODE_BITCORE_TX_LIST, &txs);
			if (!load_blk_txs(chash, &txs))
				break;

			if (find_blk_staking_tx(&txs, &stake_tx))
			{
				ret = check_tx_input_sig(&stake_tx, 0, &pubK);
				if (ret)
				{
					struct string signature = { PTR_NULL };
					mem_zone_ref sig = { PTR_NULL };

					get_blk_sign(chash, &signature);

					if (tree_manager_add_child_node(&blk_hdr, "signature", NODE_BITCORE_ECDSA_SIG, &sig))
					{
						tree_manager_write_node_sig(&sig, 0, signature.str, signature.len);
						release_zone_ref(&sig);
					}

					ret = check_blk_sig(&blk_hdr, &pubK);
					free_string(&pubK);
					free_string(&signature);
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
			hash_t			out_diff, blk_pow;
			mem_zone_ref	pdif = { PTR_NULL };

			if (!tree_manager_find_child_node(&self_node, NODE_HASH("current_pow_diff"), NODE_GFX_INT, &pdif))
				tree_manager_add_child_node(&self_node, "current_pow_diff", NODE_GFX_INT, &pdif);

			if (compute_pow_diff(&blk_hdr, &pdif))
				tree_mamanger_get_node_dword	(&pdif, 0, &nBits);
			else
				tree_manager_get_child_value_i32(&blk_hdr, NODE_HASH("bits"), &nBits);

			release_zone_ref					(&pdif);
			
			SetCompact(nBits, out_diff);
			if (!check_block_pow(&blk_hdr, out_diff))
				break;

			if (pos_kernel != PTR_NULL)
				check_tx_pos(&blk_hdr, PTR_NULL);

			get_blockreward						(cur, &reward);
			tree_manager_set_child_value_i64	(&self_node, "pow_reward", reward);

			if (tree_manager_get_child_value_hash(&blk_hdr, NODE_HASH("blk pow"), blk_pow))
			{
				make_string(&blk_path, "blks");
				cat_ncstring_p(&blk_path, chash + 0, 2);
				cat_ncstring_p(&blk_path, chash + 2, 2);
				cat_cstring_p(&blk_path, chash);
				cat_cstring_p(&blk_path, "pow");
				put_file(blk_path.str, blk_pow, sizeof(hash_t));
				free_string(&blk_path);
			}
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
		mem_zone_ref log = { PTR_NULL };

		tree_manager_create_node("log", NODE_LOG_PARAMS, &log);
		tree_manager_set_child_value_i32(&log, "cur", cur);
		log_message("truncate chain to height : %cur%", &log);
		release_zone_ref(&log);
		node_truncate_chain(cur);
	}

	return;
}




OS_API_C_FUNC(int) accept_block(mem_zone_ref_ptr header, mem_zone_ref_ptr tx_list)
{
	hash_t				merkle;
	mem_zone_ref		lastPOWBlk = { PTR_NULL };
	mem_zone_ref		log = { PTR_NULL };
	struct string		sign = { PTR_NULL };
	uint64_t			block_reward = 0, staking_reward = 0, nblks;
	int					ret = 1;
	unsigned int		is_staking;
	unsigned int		checktx;

	tree_manager_get_child_value_i64(&self_node, NODE_HASH("block_height"), &nblks);

	is_staking = 0;
	if (pos_kernel != PTR_NULL)
	{
		mem_zone_ref stake_tx = { PTR_NULL };
		struct string pubK = { PTR_NULL };

		if (find_blk_staking_tx(tx_list, &stake_tx))
		{
			is_staking = 1;
			ret = check_tx_input_sig(&stake_tx, 0, &pubK);
			if (ret)
			{
				ret = check_blk_sig(header, &pubK);
				free_string(&pubK);
			}
			if (ret)ret = check_tx_pos(header, &stake_tx);
			if (ret)ret = get_stake_reward(nblks, &block_reward);
			release_zone_ref(&stake_tx);

			if (!ret)return 0;
		}
	}


	if (!is_staking)
	{
		unsigned int		powbits, blkbits;
		hash_t				out_diff;

		tree_manager_get_child_value_i32(header, NODE_HASH("bits"), &blkbits);
		if (!tree_manager_get_child_value_i32(&self_node, NODE_HASH("current_pow_diff"), &powbits))
			powbits = blkbits;
		
		if (powbits != blkbits)
			return 0;

		SetCompact(powbits, out_diff);
		if (!check_block_pow(header, out_diff))return 0;

		if (pos_kernel != PTR_NULL)
		{
			if (!check_tx_pos(header, PTR_NULL))return 0;
		}
		if (!get_blockreward(nblks, &block_reward))return 0;
	}
	log_output("verify block txs\n");

	
	if (!tree_manager_get_child_value_i32(&self_node, NODE_HASH("checktxsign"), &checktx))
		checktx = 0;

	tree_manager_get_child_value_hash(header, NODE_HASH("merkle_root"), merkle);
	return check_tx_list(tx_list, block_reward, merkle, checktx);
}
int handle_getblocks(mem_zone_ref_ptr node, mem_zone_ref_ptr payload)
{
	mem_zone_ref locator = { PTR_NULL }, inv_pack = { PTR_NULL };


	if (!tree_manager_create_node("inv", NODE_BITCORE_HASH_LIST, &inv_pack))return 0;

	if (!tree_manager_find_child_node(payload, NODE_HASH("hashes"), NODE_BITCORE_HASH_LIST, &locator)){ release_zone_ref(&inv_pack); return 0; }

	get_locator_next_blocks			 (&locator, &inv_pack);
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

	//tree_manager_cat_node_childs(&node_qlist, &hash_list, 0);

	/*

	for (tree_manager_get_first_child(&hash_list, &my_list, &hash_node); ((hash_node != NULL) && (hash_node->zone != NULL)); tree_manager_get_next_child(&my_list, &hash_node))
	{
		char			chash[65];
		unsigned char	*hash_data;

		hash_data = tree_mamanger_get_node_data_ptr(hash_node, 0);
		if (tree_mamanger_get_node_type(hash_node) == NODE_BITCORE_BLOCK_HASH)
		{
			int				n = 0;
			mem_zone_ref	block = { PTR_NULL }, txs = { PTR_NULL };
			while (n<32)
			{
				chash[n * 2 + 0] = hex_chars[hash_data[n] >> 4];
				chash[n * 2 + 1] = hex_chars[hash_data[n] & 0x0F];
				n++;
			}
			chash[64] = 0;
			if (load_blk_hdr(&block, chash))
			{
				if (tree_manager_create_node("txs", NODE_BITCORE_TX_LIST, &txs))
				{
					struct string sign = { 0 };
					get_blk_sign(chash, &sign);

					if (load_blk_txs(chash, &txs))
						queue_block_message(node, &block, &txs, &sign);

					free_string(&sign);

					release_zone_ref(&txs);
				}
				release_zone_ref(&block);
			}
		}
		else  if (tree_mamanger_get_node_type(hash_node) == NODE_BITCORE_TX_HASH)
		{
			hash_t			blkhash,txh;
			mem_zone_ref	tx={ PTR_NULL };
			int				n = 32,ret=0;
			mem_zone_ref	block = { PTR_NULL }, txs = { PTR_NULL }, etx = { PTR_NULL };
			
			while (n--)txh[n] = hash_data[31 - n];

			
			ret = load_tx(&tx, blkhash, txh);
			if (!ret)
			{
				if (tree_manager_find_child_node(&self_node, NODE_HASH("mempool"), NODE_BITCORE_TX_LIST, &etx))
				{
					ret = tree_find_child_node_by_member_name_hash(&etx, NODE_BITCORE_TX, "txid", hash_data, &tx);
					release_zone_ref(&etx);
				}
			}
			if (ret)
			{
				queue_tx_message(node, &tx);
				release_zone_ref(&tx);
			}
		}
	}
	*/
	release_zone_ref(&hash_list);
	release_zone_ref(&node_qlist);
	return 1;
}


int handle_version(mem_zone_ref_ptr node, mem_zone_ref_ptr payload)
{
	char		 user_agent[32];
	mem_zone_ref log = { PTR_NULL }, block_index_node = { PTR_NULL };
	unsigned int proto_ver, last_blk, node_port,my_port;
	mem_zone_ref addrs[2] = { PTR_NULL };
	uint64_t	 time, services, timestamp;
	int			 index = 0;
	ipv4_t		 node_ip,my_ip;
	uint64_t		nblks;
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



int add_new_block(mem_zone_ref_ptr header, mem_zone_ref_ptr tx_list)
{
	hash_t			proof;
	mem_zone_ref	nBits = { PTR_NULL };
	uint64_t		block_reward = 0;
	int				ret, cur_len;
	
	if (!store_block(header, tx_list))return 0;

	if (pos_kernel != PTR_NULL)
	{
		if (!store_blk_staking(header))return 0;
		if (!store_blk_tx_staking(tx_list))return 0;
	}

	cur_len = get_last_block_height();
	
	if (tree_manager_get_child_value_hash(header, NODE_HASH("blk pos"), proof))
	{
		if (!tree_manager_find_child_node(&self_node, NODE_HASH("current_pos_diff"), NODE_GFX_INT, &nBits))
			tree_manager_add_child_node(&self_node, "current_pos_diff", NODE_GFX_INT, &nBits);

		ret = compute_last_pos_diff(header, &nBits);
		release_zone_ref(&nBits);

		if (ret)ret = get_stake_reward(cur_len, &block_reward);

		tree_manager_set_child_value_i64(&self_node, "pos_reward", block_reward);
	}
	else
	{
		if (!tree_manager_find_child_node(&self_node, NODE_HASH("current_pow_diff"), NODE_GFX_INT, &nBits))
			tree_manager_add_child_node(&self_node, "current_pow_diff", NODE_GFX_INT, &nBits);

		ret = compute_pow_diff(header, &nBits);
		release_zone_ref(&nBits);

		if (ret)ret = get_blockreward(cur_len, &block_reward);

		tree_manager_set_child_value_i64(&self_node, "pow_reward", block_reward);
	}
	
	if (ret)
	{
		add_moneysupply				(block_reward);
		node_set_last_block			(header);
		node_del_txs_from_mempool	(tx_list);
		store_wallet_txs			(tx_list);

		log_message("added new block: %blk hash% , %time% - %version% %merkle_root%\n", header);
	}
	else
		log_message("error adding new block: %blk hash% , %time% - %version% %merkle_root%\n", header);

	return ret;
}


int handle_block(mem_zone_ref_ptr node, mem_zone_ref_ptr payload)
{
	hash_t				prevHash;
	hash_t				blk_hash;
	mem_zone_ref		header = { PTR_NULL }, tx_list = { PTR_NULL }, sig = { PTR_NULL };
	mem_zone_ref		log = { PTR_NULL };
	size_t				nz1 = 0, nz2 = 0;
	struct string		signature = { PTR_NULL };
	int					ret = 1;
	uint64_t			nblks;
	int					testing=0;
	unsigned int		keep_block, cur_len;

	
	if (!tree_manager_find_child_node(payload, NODE_HASH("header"), NODE_BITCORE_BLK_HDR, &header))return 1;
	if (!tree_manager_get_child_value_hash(&header, NODE_HASH("prev"), prevHash))
	{
		release_zone_ref(&header);
		return 1;
	}
	
	if (!tree_manager_get_child_value_i32(&header, NODE_HASH("keep_block"), &keep_block))
		keep_block = 1;

	if (!keep_block)return 1;
	
	cur_len = get_last_block_height();

	compute_block_hash					(&header, blk_hash);
	tree_manager_set_child_value_bhash	(&header, "blkHash", blk_hash);
	
	tree_manager_get_child_value_i32	(node, NODE_HASH("testing_chain"), &testing);
		
	if (testing>0)
	{
		hash_t			lh;
		unsigned int 	new_len, bestChainDepth;

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
			mem_zone_ref	last_blk = { PTR_NULL };
			int				n;
			uint64_t		lost_reward;
			
			lost_reward = 0;
			n			= cur_len;
			while ((n--) > testing)
			{
				char		chash[65];
				hash_t		h;
				uint64_t	reward;

				if (!node_get_hash_idx(n, h))continue;

				n = 0; 
				while (n<32)
				{
					chash[n * 2 + 0] = hex_chars[h[n] >> 4];
					chash[n * 2 + 1] = hex_chars[h[n] & 0x0F];
					n++;
				}
				chash[64] = 0;
				
				if (is_pow_block(chash))
					get_blockreward (n, &reward);
				else
					get_stake_reward(n, &reward);

				lost_reward += reward;
			}
			sub_moneysupply(lost_reward);
			
			tree_manager_create_node("log", NODE_LOG_PARAMS, &log);
			tree_manager_set_child_value_i32(&log, "testing", testing);
			log_message("truncate chain to height : %testing%", &log);
			release_zone_ref(&log);

			if(node_truncate_chain(testing))
			{ 
				if (tree_manager_find_child_node(&self_node, NODE_HASH("lastPOSBlk"), NODE_BITCORE_BLK_HDR, &last_blk))
				{
					char chash[65];
					uint64_t reward;
					mem_zone_ref nBits = { PTR_NULL };

					if (!tree_manager_find_child_node(&self_node, NODE_HASH("current_pos_diff"), NODE_GFX_INT, &nBits))
						tree_manager_add_child_node(&self_node, "current_pos_diff", NODE_GFX_INT, &nBits);

					compute_last_pos_diff(&last_blk, &nBits);

					release_zone_ref(&nBits);
					release_zone_ref(&last_blk);

					tree_manager_get_child_value_str		(&last_blk, NODE_HASH("blkHash"), chash, 65, 16);

					if (is_pow_block(chash))
					{
						get_blockreward						(testing, &reward);
						tree_manager_set_child_value_i64	(&self_node, "pow_reward", reward);
					}
					else
					{
						get_stake_reward					(testing, &reward);
						tree_manager_set_child_value_i64	(&self_node, "pos_reward", reward);
					}
				}
				log_output						("switching to new chain \n");
				tree_manager_set_child_value_i32(node, "testing_chain", 0);
			}
		}
		release_zone_ref(&header);
		return 0;
	}

	if (!tree_manager_find_child_node(payload, NODE_HASH("txs"), NODE_BITCORE_TX_LIST, &tx_list)){ release_zone_ref(&header); return 1; }
	if (!tree_manager_find_child_node(payload, NODE_HASH("signature"), NODE_BITCORE_ECDSA_SIG, &sig)){ release_zone_ref(&tx_list); release_zone_ref(&header); return 1; }
	tree_manager_get_node_istr(&sig, 0, &signature, 0);
	release_zone_ref(&sig);

	ret = node_is_next_block	(&header);
	if (!ret)
	{
		ret = node_check_chain(node, &header);

		if (!tree_manager_get_child_value_i32(&header, NODE_HASH("keep_block"), &keep_block))
			keep_block = 1;

		if (!ret)
		{
			free_string		(&signature);
			release_zone_ref(&header);
			release_zone_ref(&tx_list);
			return (keep_block==1)?0:1;
		}
	}

	if (pos_kernel != PTR_NULL)
	{
		mem_zone_ref sig = { PTR_NULL };

		if (!tree_manager_find_child_node(&header, NODE_HASH("signature"), NODE_BITCORE_ECDSA_SIG, &sig))
			tree_manager_add_child_node(&header, "signature", NODE_BITCORE_ECDSA_SIG, &sig);

		tree_manager_write_node_sig(&sig, 0, signature.str, signature.len);
		release_zone_ref(&sig);
	}

	if (find_hash(blk_hash))
		remove_block(blk_hash);

	ret = accept_block(&header, &tx_list);
	if (ret)ret=add_new_block(&header, &tx_list);
	
	tree_manager_set_child_value_i32	(&self_node, "next_check", get_time_c() + 10);
	tree_manager_get_child_value_i64	(&self_node, NODE_HASH("block_height"), &nblks);

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
	
	if (!strncmp_c(cmd, "getdata", 7))return handle_getdata(node, payload);
	if (!strncmp_c(cmd, "getblocks", 9))return handle_getblocks(node, payload);

	
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


	if (!tree_manager_find_child_node(node, NODE_HASH("emitted_queue"), NODE_BITCORE_MSG_LIST, &msg_list))return 0;

	for (tree_manager_get_first_child(&msg_list, &my_list, &msg); ((msg != NULL) && (msg->zone != NULL)); tree_manager_get_next_child(&my_list, &msg))
	{
		char			cmd[16];
		mem_zone_ref	payload_node = { PTR_NULL };
		int				ret;
		if (tree_mamanger_get_node_type(msg) != NODE_BITCORE_MSG) continue;
		if (!tree_manager_get_child_value_str	(msg, NODE_HASH("cmd"), cmd, 12, 16))continue;

		ret=node_process_event_handler		("emitted_queue", node, msg);
		if (ret<0)
		{
			tree_manager_find_child_node(msg, NODE_HASH("payload"), NODE_BITCORE_PAYLOAD, &payload_node);
			ret = handle_message		(node, cmd, &payload_node);
			release_zone_ref			(&payload_node);
		}
		tree_manager_set_child_value_i32(msg, "handled", ret);
		
		
	}
	tree_remove_child_by_member_value_dword		(&msg_list, NODE_BITCORE_MSG, "handled", 1);
	tree_remove_child_by_member_value_lt_dword	(&msg_list, NODE_BITCORE_MSG, "recvtime", get_time_c()-100);

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
	unsigned int		next_check, curtime, last_block_time;
	ctime_t				min_delay;
	mem_zone_ref_ptr	node = PTR_NULL;
	mem_zone_ref		my_list = { PTR_NULL }, peer_nodes = { PTR_NULL }, my_node = { PTR_NULL };

	if (!tree_manager_find_child_node(&self_node, NODE_HASH("peer_nodes"), NODE_BITCORE_NODE_LIST, &peer_nodes))return 0;

	curtime = get_time_c();
	min_delay = 100000;

	for (tree_manager_get_first_child(&peer_nodes, &my_list, &node); ((node != NULL) && (node->zone != NULL)); tree_manager_get_next_child(&my_list, &node))
	{
		unsigned int		test, synching;
		ctime_t				ping_delay;

		process_node_messages(node);
		process_node_elements(node);


		if (!tree_manager_get_child_value_i32(node, NODE_HASH("synching"), &synching))
			synching = 0;

		if (!tree_manager_get_child_value_i32(node, NODE_HASH("testing_chain"), &test))
			test = 0;

		if (!tree_manager_get_child_value_si64(node, NODE_HASH("ping_delay"), &ping_delay))
			ping_delay = 0;


		if ((test == 0) && (synching == 1) && (ping_delay>0))
		{
			if (ping_delay < min_delay)
			{
				min_delay = ping_delay;
				copy_zone_ref(&my_node, node);
			}
		}
	}
	

	scan_addresses();


	/*
	if (!node_get_last_block_time(&last_block_time))
	last_block_time = 0;

	if ((test == 0) && (synching == 0))
	{
		if ((last_block_time + 60) >= curtime)
		{
			tree_manager_set_child_value_i32(node, "synching", 1);
			tree_manager_set_child_value_i32(node, "next_check", curtime);
		}
	}
	*/

	if (my_node.zone != PTR_NULL)
	{
		if (tree_manager_get_child_value_i32(&self_node, NODE_HASH("next_check"), &next_check))
		{
			if (curtime >= next_check)
			{
				queue_getblocks_message				(&my_node);
				tree_manager_set_child_value_i32	(&self_node, "next_check", curtime + 3600);
			}
		}
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

	create_dir("txs");
	if (stat_file("txs") != 0)
	{
		log_message("unable to create tx dir \n", PTR_NULL);
		return 0;
	}

	create_dir("blks");
	if (stat_file("blks") != 0)
	{
		log_message("unable to create blks dir \n", PTR_NULL);
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
		ret = resolve_script_var		(params, PTR_NULL,"SelfNode"							, NODE_BITCORE_NODE, &self_node);
		if(ret)ret = resolve_script_var	(params, PTR_NULL,"configuration.staking.pos_kernel"	, NODE_MODULE_DEF, &pos_kernel_def);
		if(ret)node_set_script			(params);
	}

	if (!ret)
	{
		release_zone_ref(&self_node);
		release_zone_ref(&pos_kernel_def);
	}

	return ret;
}

OS_API_C_FUNC(int) app_start(mem_zone_ref_ptr params)
{
	mem_zone_ref		service = { PTR_NULL };
	char				**params_ptr;
	int					rebuild_tx, rebuild_supply;
	int					ret;


	ret	=	tree_manager_get_child_value_ptr(&pos_kernel_def, NODE_HASH("mod_ptr"), 0, &pos_kernel);
		
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
#endif
	}
	init_wallet(&self_node, pos_kernel);

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

	//node_truncate_chain(109995);
	//node_rewrite_txs(100);
	//node_remove_last_block();

	if (rebuild_tx)
		node_rewrite_txs(1000);

	if (rebuild_supply)
		rebuild_money_supply();

	return 1;
}

OS_API_C_FUNC(int) app_loop(mem_zone_ref_ptr params)
{
	mem_zone_ref		blk_list = { PTR_NULL }, tx_list = { PTR_NULL };
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref		hash_list = { PTR_NULL };
	unsigned int		new_block = 0, new_tx = 0;;

	node_check_new_connections	();
	node_check_services			();
	update_peernodes			();
	process_nodes				();

	if (tree_manager_find_child_node(&self_node, NODE_HASH("submitted txs"), NODE_BITCORE_TX_LIST, &tx_list))
	{
		mem_zone_ref_ptr	tx = PTR_NULL;
		tree_manager_create_node("hashes", NODE_BITCORE_HASH_LIST, &hash_list);
		for (tree_manager_get_first_child(&tx_list, &my_list, &tx); ((tx != PTR_NULL) && (tx->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &tx))
		{
			mem_zone_ref		new_hash = { PTR_NULL };

			if (tree_manager_find_child_node(tx, NODE_HASH("submitted"), 0xFFFFFFFF, PTR_NULL))continue;

			if (tree_manager_add_child_node(&hash_list, "hash", NODE_BITCORE_TX_HASH, &new_hash))
			{
				hash_t				txh;

				tree_manager_get_child_value_hash	(tx, NODE_HASH("txid"), txh);
				tree_manager_write_node_hash		(&new_hash, 0, txh);
				release_zone_ref					(&new_hash);
				tree_manager_set_child_value_bool	(tx, "submitted", 1);
				node_add_tx_to_mempool				(tx);
				new_tx = 1;
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
			hash_t blk_hash = { 0 };
			struct string signature = { PTR_NULL };
			int			 ret;
			if (!tree_manager_find_child_node(blk, NODE_HASH("txs"), NODE_BITCORE_TX_LIST, &tx_list))continue;
			if (!tree_manager_get_child_value_istr(blk, NODE_HASH("signature"), &signature, 0))continue;
			
			ret=accept_block					(blk, &tx_list);
			if (ret)
			{
				mem_zone_ref		new_hash = { PTR_NULL };
				ret= add_new_block(blk, &tx_list);
				if (ret)
				{
					tree_manager_get_child_value_hash(blk, NODE_HASH("blkHash"), blk_hash);
					if (tree_manager_add_child_node(&hash_list, "hash", NODE_BITCORE_BLOCK_HASH, &new_hash))
					{
						tree_manager_write_node_hash(&new_hash, 0, blk_hash);
						release_zone_ref(&new_hash);
					}
					new_block = 1;
				}
			}
			release_zone_ref					(&tx_list);
			free_string							(&signature);
			tree_manager_set_child_value_bool	(blk, "done", 1);
		}
		tree_remove_child_by_member_value_dword(&blk_list, NODE_BITCORE_BLOCK, "done", 1);
		release_zone_ref(&blk_list);
	}


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

	return 1;
}

OS_API_C_FUNC(int) app_stop(mem_zone_ref_ptr params)
{
	return 1;
}

#ifdef _WIN32
	unsigned int C_API_FUNC _DllMainCRTStartup(unsigned int *prev, unsigned int *cur, unsigned int *xx){return 1;}
#endif
