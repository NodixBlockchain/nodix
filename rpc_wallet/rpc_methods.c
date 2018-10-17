//copyright antoine bentue-ferrer 2016
#include <base/std_def.h>
#include <base/std_mem.h>
#include <base/mem_base.h>
#include <base/std_str.h>

#include <strs.h>
#include <tree.h>
#include <fsio.h>
#include <sha256.h>
#include <crypto.h>
#include <mem_stream.h>
#include <tpo_mod.h>

#include "../node_adx/node_api.h"
#include "../block_adx/block_api.h"
#include "../wallet/wallet_api.h"

#ifdef _DEBUG
C_IMPORT int			C_API_FUNC		get_last_stake_modifier(mem_zone_ref_ptr pindex, hash_t nStakeModifier, unsigned int *nModifierTime);
C_IMPORT int			C_API_FUNC		get_tx_pos_hash_data(mem_zone_ref_ptr hdr, const hash_t txHash, unsigned int OutIdx, struct string *hash_data,uint64_t *amount, hash_t out_diff);
C_IMPORT int			C_API_FUNC		get_blk_staking_infos(mem_zone_ref_ptr blk, mem_zone_ref_ptr infos);
C_IMPORT int			C_API_FUNC		store_tx_staking(mem_zone_ref_ptr tx, hash_t tx_hash, btc_addr_t stake_addr, uint64_t	stake_in);
C_IMPORT int			C_API_FUNC		get_target_spacing(unsigned int *target);
C_IMPORT unsigned int	C_API_FUNC		get_current_pos_difficulty();
C_IMPORT int			C_API_FUNC		get_stake_reward(uint64_t height,uint64_t *reward);
C_IMPORT int			C_API_FUNC		compute_tx_pos(mem_zone_ref_ptr tx, hash_t StakeModifier, unsigned int txTime, hash_t pos_hash, uint64_t *weight);
C_IMPORT int			C_API_FUNC		create_pos_block(uint64_t height, mem_zone_ref_ptr tx, mem_zone_ref_ptr newBlock);
C_IMPORT int			C_API_FUNC		check_tx_pos(mem_zone_ref_ptr hdr, mem_zone_ref_ptr tx);
C_IMPORT int			C_API_FUNC		get_min_stake_depth(unsigned int *depth);

#else
get_blk_staking_infos_func_ptr		 get_blk_staking_infos = PTR_INVALID;
store_tx_staking_func_ptr			 store_tx_staking = PTR_INVALID;
get_tx_pos_hash_data_func_ptr		 get_tx_pos_hash_data = PTR_INVALID;
get_target_spacing_func_ptr			 get_target_spacing = PTR_INVALID;
get_stake_reward_func_ptr			 get_stake_reward = PTR_INVALID;
get_last_stake_modifier_func_ptr	 get_last_stake_modifier = PTR_INVALID;
get_current_pos_difficulty_func_ptr	 get_current_pos_difficulty = PTR_INVALID;
compute_tx_pos_func_ptr				 compute_tx_pos = PTR_INVALID;
create_pos_block_func_ptr			 create_pos_block = PTR_INVALID;
check_tx_pos_func_ptr				 check_tx_pos = PTR_INVALID;
get_min_stake_depth_func_ptr		 get_min_stake_depth = PTR_INVALID;
#endif




C_IMPORT size_t			C_API_FUNC	compute_payload_size(mem_zone_ref_ptr key);
C_IMPORT char*			C_API_FUNC	write_node	 (mem_zone_ref_const_ptr key, unsigned char *payload);
C_IMPORT size_t			C_API_FUNC	read_node(mem_zone_ref_ptr key, const char *payload, size_t len);
C_IMPORT size_t			C_API_FUNC	get_node_size(mem_zone_ref_ptr key);

unsigned int			WALLET_VERSION = 60000;

unsigned int			min_staking_depth = 2;
unsigned int			cur_mining_height = 0xFFFFFFFF;
mem_zone_ref			my_node = { PTR_INVALID }, next_block = { PTR_INVALID }, tmp_txs = { PTR_INVALID };
btc_addr_t				src_addr_list[1024] = { 0xCDFF }, mining_addr = { 0xFF };

OS_API_C_FUNC(int) set_node_rpc_wallet(mem_zone_ref_ptr node,tpo_mod_file *pos_mod)
{
	my_node.zone = PTR_NULL;
	copy_zone_ref(&my_node, node);

#ifndef _DEBUG
	get_blk_staking_infos		= (get_blk_staking_infos_func_ptr)		get_tpo_mod_exp_addr_name(pos_mod, "get_blk_staking_infos", 0);
	store_tx_staking			= (store_tx_staking_func_ptr)			get_tpo_mod_exp_addr_name(pos_mod, "store_tx_staking", 0);
	get_tx_pos_hash_data		= (get_tx_pos_hash_data_func_ptr)		get_tpo_mod_exp_addr_name(pos_mod, "get_tx_pos_hash_data", 0);
	get_target_spacing			= (get_target_spacing_func_ptr)			get_tpo_mod_exp_addr_name(pos_mod, "get_target_spacing", 0);
	get_stake_reward			= (get_stake_reward_func_ptr)			get_tpo_mod_exp_addr_name(pos_mod, "get_stake_reward", 0);
	get_last_stake_modifier		= (get_last_stake_modifier_func_ptr)	get_tpo_mod_exp_addr_name(pos_mod, "get_last_stake_modifier", 0);
	get_current_pos_difficulty  = (get_current_pos_difficulty_func_ptr)	get_tpo_mod_exp_addr_name(pos_mod, "get_current_pos_difficulty", 0);
	compute_tx_pos				= (compute_tx_pos_func_ptr)				get_tpo_mod_exp_addr_name(pos_mod, "compute_tx_pos", 0);
	check_tx_pos				= (check_tx_pos_func_ptr)				get_tpo_mod_exp_addr_name(pos_mod, "check_tx_pos", 0);
	create_pos_block			= (create_pos_block_func_ptr)			get_tpo_mod_exp_addr_name(pos_mod, "create_pos_block", 0);
	get_min_stake_depth			= (get_min_stake_depth_func_ptr)		get_tpo_mod_exp_addr_name(pos_mod, "get_min_stake_depth", 0);
#endif
	if (get_min_stake_depth != PTR_NULL)
		get_min_stake_depth(&min_staking_depth);

	create_dir("acpw");

	next_block.zone = PTR_NULL;

	cur_mining_height = 0;
	memset_c(mining_addr, 0, sizeof(btc_addr_t));

	tmp_txs.zone = PTR_NULL;

	tree_manager_create_node("tmp tx pool", NODE_BITCORE_TX_LIST, &tmp_txs);
	return 1;
}


OS_API_C_FUNC(int) getaddressscanstatus(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	btc_addr_t			new_addr;
	mem_zone_ref		addr = { PTR_NULL };
	struct string		adr_path = { 0 };
	unsigned char		*data;
	size_t				len;
	unsigned int		scanning, block;

	if (!tree_manager_get_child_at(params, 0, &addr))return 0;
	if (!tree_manager_get_node_btcaddr(&addr, 0, new_addr))
	{
		release_zone_ref(&addr);
		return 0;
	}
	release_zone_ref(&addr);

	make_string   (&adr_path, "adrs");
	cat_ncstring_p(&adr_path, new_addr, 34);
	cat_cstring_p  (&adr_path, "scanning");
	scanning = (stat_file(adr_path.str) == 0) ? 1 : 0;
	tree_manager_set_child_value_i32(result, "scanning", scanning);

	if (get_file(adr_path.str, &data, &len) > 0)
	{
		block = *((unsigned int *)(data));
		free_c(data);
	}
	else
		block = 0;
	tree_manager_set_child_value_i32(result, "block", block);

	free_string(&adr_path);
	
	return 1;
}

double GetDifficulty(unsigned int nBits)
{
	int nShift   = (nBits >> 24) & 0xff;
	double dDiff = (double)0x0000ffff / (double)(nBits & 0x00ffffff);

	while (nShift < 29)
	{
		dDiff *= 256.0;
		nShift++;
	}
	while (nShift > 29)
	{
		dDiff /= 256.0;
		nShift--;
	}

	return dDiff;
}

OS_API_C_FUNC(int) getlastblock(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	mem_zone_ref last_blk = { PTR_NULL };


	if (tree_manager_find_child_node(&my_node, NODE_HASH("last_block"), NODE_BITCORE_BLK_HDR, &last_blk))
	{
		hash_t hash, merkle, proof, nullhash, rdiff,hdiff,prev, block_hash;
		mem_zone_ref txs = { PTR_NULL },proofNode= { PTR_NULL };
		size_t size;
		unsigned int version, time, bits, nonce;
		uint64_t height;

		memset_c(nullhash, 0, sizeof(hash_t));

		tree_manager_get_child_value_hash(&last_blk, NODE_HASH("blk_hash"), block_hash);
		
		tree_manager_get_child_value_hash(&last_blk, NODE_HASH("merkle_root"), merkle);
		tree_manager_get_child_value_hash(&last_blk, NODE_HASH("prev"), prev);
		tree_manager_get_child_value_i32 (&last_blk, NODE_HASH("version"), &version);
		tree_manager_get_child_value_i32 (&last_blk, NODE_HASH("time"), &time);
		tree_manager_get_child_value_i32 (&last_blk, NODE_HASH("bits"), &bits);
		tree_manager_get_child_value_i32 (&last_blk, NODE_HASH("nonce"), &nonce);
		
		if (!get_block_size(block_hash, &size))
			size = 0;

		tree_manager_get_child_value_i64(&last_blk,NODE_HASH("height"),&height);

		if(tree_manager_find_child_node(&last_blk,NODE_HASH("blk pow"),0XFFFFFFFF,&proofNode))
		{
			SetCompact							(bits, hdiff);

			tree_manager_get_node_hash			(&proofNode,0,proof);
			release_zone_ref					(&proofNode);

			tree_manager_set_child_value_hash	(result, "proofhash", proof);
			tree_manager_set_child_value_hash	(result, "hbits", rdiff);
		}
		else if (get_blk_staking_infos)
			get_blk_staking_infos(&last_blk, result);


		tree_manager_set_child_value_hash (result, "hash", hash);
		tree_manager_set_child_value_i32  (result , "confirmations", 0);
		tree_manager_set_child_value_i32  (result , "size", size);
		tree_manager_set_child_value_i64  (result , "height", height);
		tree_manager_set_child_value_i32  (result, "time", time);
		tree_manager_set_child_value_i32  (result, "version", version);
		tree_manager_set_child_value_i32  (result, "bits", bits);
		tree_manager_set_child_value_i32  (result, "nonce", nonce);
		tree_manager_set_child_value_hash (result, "merkleroot", merkle);
		tree_manager_set_child_value_hash (result, "previousblockhash", prev);
		tree_manager_set_child_value_hash (result, "nextblockhash", nullhash);
		tree_manager_set_child_value_float(result, "difficulty", GetDifficulty(bits));
		tree_manager_add_child_node		  (result, "txs", NODE_JSON_ARRAY,&txs);


		get_blk_txs(hash, height, &txs,10);
		
		release_zone_ref(&txs);
			/*
			"mint" : 0.00000000,
			"blocktrust" : "100001",
			"chaintrust" : "100001",
			"nextblockhash" : "af49672bafd39e39f8058967a2cce926a9b21db14c452a7883fba63a78a611a6",
			"flags" : "proof-of-work stake-modifier",
			"entropybit" : 0,
			*/
		return 1;
	}

	return 0;
}


OS_API_C_FUNC(int) getinfo(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	char	ip[32];
	mem_zone_ref addr_node = { PTR_NULL }, difficulty = { PTR_NULL };
	uint64_t balance = 0, paytxfee = 0, services = 0, last_blk = 0, supply = 0;
	hash_t posd, powd;
	unsigned int PROTOCOL_VERSION, p2p_status;
	unsigned int pow_diff=0, pos_diff=0;
	short int port;
	
	tree_manager_get_child_value_i64(&my_node, NODE_HASH("block_height"), &last_blk);
	

	if (tree_manager_find_child_node(&my_node, NODE_HASH("p2p_addr"), NODE_BITCORE_ADDR, &addr_node))
	{
		tree_manager_get_child_value_str(&addr_node, NODE_HASH("addr"), ip,32,0);
		tree_manager_get_child_value_i16(&addr_node, NODE_HASH("port"), &port);
		tree_manager_get_child_value_i64(&addr_node, NODE_HASH("services"), &services);
		release_zone_ref(&addr_node);
	}
	
	get_moneysupply(&supply);
	tree_manager_get_child_value_i32(&my_node, NODE_HASH("version"), &PROTOCOL_VERSION);
	tree_manager_get_child_value_i32(&my_node, NODE_HASH("p2p_status"), &p2p_status);
	tree_manager_get_child_value_i64(&my_node, NODE_HASH("paytxfee"), &paytxfee);

	if (tree_manager_get_child_value_i32(&my_node, NODE_HASH("current_pos_diff"), &pos_diff))
		SetCompact(pos_diff, posd);
	
	if(tree_manager_get_child_value_i32(&my_node, NODE_HASH("current_pow_diff"), &pow_diff))
		SetCompact(pow_diff, powd);


	tree_manager_add_child_node(result, "difficulty", NODE_GFX_OBJECT,&difficulty);
	tree_manager_set_child_value_float(&difficulty, "pow", GetDifficulty(pow_diff));
	tree_manager_set_child_value_hash(&difficulty, "hpow", powd);
	tree_manager_set_child_value_float(&difficulty, "pos", GetDifficulty(pos_diff));
	tree_manager_set_child_value_hash(&difficulty, "hpos", posd);

	release_zone_ref(&difficulty);

	tree_manager_set_child_value_str(result, "version", "purenode v0.1");
	tree_manager_set_child_value_i32(result, "protocolversion", PROTOCOL_VERSION);
	tree_manager_set_child_value_i32(result, "walletversion", WALLET_VERSION);
	tree_manager_set_child_value_i64(result, "paytxfee", paytxfee);
	tree_manager_set_child_value_i64(result, "mininput", 0);
	tree_manager_set_child_value_i64(result, "moneysupply", supply);
	tree_manager_set_child_value_i64(result, "testnet", 0);
	tree_manager_set_child_value_str(result, "error","");
	tree_manager_set_child_value_str(result, "ip", ip);
	tree_manager_set_child_value_i32(result, "p2pport", port);
	tree_manager_set_child_value_i32(result, "p2p_status", p2p_status);
	tree_manager_set_child_value_i64(result, "balance", balance);
	tree_manager_set_child_value_i64(result, "blocks", last_blk);
	tree_manager_set_child_value_i64(result, "timeoffset", 0);
	/*
	"newmint" : 0.00000000,
	"stake" : 0.00000000,
	"connections" : 2,
	"proxy" : "",
	"unlocked_until" : 0,
	*/
	return 1;

}

OS_API_C_FUNC(int) getblockcount(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	uint64_t	block_height;
	tree_manager_get_child_value_i64(&my_node, NODE_HASH("block_height"), &block_height);
	tree_manager_set_child_value_i64(result, "count", block_height);

	return 1;
}


OS_API_C_FUNC(int) importaddress(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	btc_addr_t			new_addr;
	mem_zone_ref		addr = { PTR_NULL }, rescan = { PTR_NULL };
	struct string		adr_path = { 0 };
	unsigned int		scan;
	
	tree_manager_get_child_at		(params, 0, &addr);
	if (!tree_manager_get_node_btcaddr(&addr, 0, new_addr))
	{
		release_zone_ref(&addr);
		return 0;
	}

	if (tree_manager_get_child_at(params, 1, &rescan))
		tree_mamanger_get_node_dword(&rescan, 0, &scan);
	else
		scan = 1;


	make_string						(&adr_path, "adrs");
	cat_ncstring_p					(&adr_path, new_addr, 34);
	create_dir						(adr_path.str);
	
	if (scan)
	{
		mem_zone_ref scan_list = { PTR_NULL };
		if (tree_manager_find_child_node(&my_node, NODE_HASH("addr scan list"), NODE_BITCORE_WALLET_ADDR_LIST, &scan_list))
		{
			mem_zone_ref addr_scan = { PTR_NULL };
			if (tree_manager_create_node("scan", NODE_BITCORE_WALLET_ADDR, &addr_scan))
			{
				tree_manager_set_child_value_btcaddr(&addr_scan, "addr", new_addr);
				tree_manager_set_child_value_i32	(&addr_scan, "done", 0);
				tree_manager_node_add_child			(&scan_list, &addr_scan);
				release_zone_ref					(&addr_scan);
			}
			release_zone_ref						(&scan_list);
		}
	}
		

	release_zone_ref				(&addr);
	release_zone_ref				(&rescan);
	free_string						(&adr_path);
		
	return 1;
}
OS_API_C_FUNC(int) listtransactions(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t		null_hash;
	mem_zone_ref  tx_out_list = { PTR_NULL }, addrs = { PTR_NULL };
	mem_zone_ref  my_list = { PTR_NULL };
	mem_zone_ref_ptr addr;

	if (!tree_manager_get_child_at(params, 0, &addrs))return 0;
	tree_manager_add_child_node(result, "txs", NODE_JSON_ARRAY, &tx_out_list);

	memset_c(null_hash, 0, sizeof(hash_t));

	for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
	{
		btc_addr_t			my_addr;
		mem_zone_ref		tx_list = { PTR_NULL };
		mem_zone_ref		my_tlist = { PTR_NULL }, my_tx = { PTR_NULL };
		mem_zone_ref_ptr	tx = PTR_NULL;

		if (!tree_manager_create_node("txs", NODE_BITCORE_HASH_LIST, &tx_list))
			break;
				
		tree_manager_get_node_btcaddr	(addr, 0, my_addr);
		load_tx_addresses				(my_addr, &tx_list, 0, 1000);

		for (tree_manager_get_first_child(&tx_list, &my_tlist, &tx); ((tx != NULL) && (tx->zone != NULL)); tree_manager_get_next_child(&my_tlist, &tx))
		{
			hash_t tx_hash,blk_hash;
			btc_addr_t maddr;
			tree_manager_get_node_hash	(tx, 0, tx_hash);
			if (load_tx(&my_tx, blk_hash,tx_hash))
			{
				mem_zone_ref txout_list = { PTR_NULL }, txin_list = { PTR_NULL };
				if (tree_manager_find_child_node(&my_tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))
				{
					unsigned int vin_idx, nin;
					nin = tree_manager_get_node_num_children(&txin_list);
					for (vin_idx = 0; vin_idx < nin; vin_idx++)
					{
						struct string script = { 0 };
						mem_zone_ref vin = { PTR_NULL }, pvout = { PTR_NULL };

						if (get_tx_input(&my_tx, vin_idx, &vin))
						{ 
							hash_t prevOutHash;
							unsigned int prevOutIdx;

							tree_manager_get_child_value_hash(&vin, NODE_HASH("txid"), prevOutHash);
							tree_manager_get_child_value_i32(&vin, NODE_HASH("idx"), &prevOutIdx);

							if (memcmp_c(prevOutHash, null_hash, sizeof(hash_t)))
							{
								mem_zone_ref prev_tx = { PTR_NULL };
								load_tx(&prev_tx, blk_hash, prevOutHash);

								if (get_tx_output(&prev_tx, prevOutIdx, &pvout))
								{
									if (tree_manager_get_child_value_istr(&pvout, NODE_HASH("script"), &script, 0))
									{
										if (get_out_script_address(&script, PTR_NULL,maddr))
										{
											if (!memcmp_c(my_addr, maddr, sizeof(btc_addr_t)))
											{
												mem_zone_ref out = { PTR_NULL };
												if (tree_manager_create_node("tx", NODE_GFX_OBJECT, &out))
												{
													uint64_t		amount=0;
													unsigned int	time=0;

													tree_manager_get_child_value_i32(&my_tx, NODE_HASH("time"), &time);
													tree_manager_get_child_value_i64(&pvout, NODE_HASH("value"), &amount);

													tree_manager_set_child_value_hash(&out, "txid", tx_hash);
													tree_manager_set_child_value_hash(&out, "blockhash", blk_hash);
													tree_manager_set_child_value_i32(&out, "time", time);
													tree_manager_set_child_value_i64(&out, "amount", amount);
													tree_manager_set_child_value_btcaddr(&out, "address", maddr);
													tree_manager_set_child_value_str(&out, "category", "send");
													tree_manager_node_add_child(&tx_out_list, &out);
													release_zone_ref(&out);
												}
											}
										}
										free_string(&script);
									}
									release_zone_ref(&pvout);
								}
								release_zone_ref(&prev_tx);
							}
							release_zone_ref(&vin);
						}
					}
					release_zone_ref(&txin_list);
				}

				if (tree_manager_find_child_node(&my_tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))
				{
					mem_zone_ref my_outlist = { PTR_NULL };
					mem_zone_ref_ptr out = PTR_NULL;

					for (tree_manager_get_first_child(&txout_list, &my_outlist, &out); ((out != NULL) && (out->zone != NULL)); tree_manager_get_next_child(&my_outlist, &out))
					{
						struct string script = { 0 };
						if (tree_manager_get_child_value_istr(out, NODE_HASH("script"), &script, 0))
						{
							if (get_out_script_address(&script, PTR_NULL,maddr))
							{
								if (!memcmp_c(my_addr, maddr, sizeof(btc_addr_t)))
								{
									mem_zone_ref tout = { PTR_NULL };
									if (tree_manager_create_node("tx", NODE_GFX_OBJECT, &tout))
									{
										uint64_t		amount;
										unsigned int	time;

										tree_manager_get_child_value_i32(&my_tx, NODE_HASH("time"), &time);
										tree_manager_get_child_value_i64(out, NODE_HASH("value"), &amount);

										tree_manager_set_child_value_hash	(&tout, "txid", tx_hash);
										tree_manager_set_child_value_hash	(&tout, "blockhash", blk_hash);
										tree_manager_set_child_value_i32	(&tout, "time", time);
										tree_manager_set_child_value_btcaddr (&tout, "address", maddr);
										tree_manager_set_child_value_i64	(&tout, "amount", amount);
										tree_manager_set_child_value_str	(&tout, "category", "receive");
										tree_manager_node_add_child			(&tx_out_list, &tout);
										release_zone_ref(&tout);
									}
								}
							}
							free_string(&script);
						}
					}
					release_zone_ref(&txout_list);
				}
				release_zone_ref(&my_tx);
			}
		}
		release_zone_ref(&tx_list);
	}
	
	release_zone_ref(&tx_out_list);
	release_zone_ref(&addrs);

	return 1;
}

OS_API_C_FUNC(int) listreceived(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	mem_zone_ref		param = { PTR_NULL }, received = { PTR_NULL }, addrs = { PTR_NULL };
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref_ptr	addr;
	uint64_t			amount;
	size_t				min_conf=0, max_conf=9999;
	size_t				max = 200, ntx = 0,first = 0;

	if (tree_manager_get_child_at(params, 0, &param))
	{
		tree_mamanger_get_node_dword(&param, 0, &min_conf);
		release_zone_ref(&param);
	}
	if (tree_manager_get_child_at(params, 1, &param))
	{
		tree_mamanger_get_node_dword(&param, 0, &max_conf);
		release_zone_ref(&param);
	}
	if (!tree_manager_get_child_at(params, 2, &addrs))return 0;

	if (tree_manager_get_child_at(params, 3, &param))
	{
		tree_mamanger_get_node_dword(&param, 0, &first);
		release_zone_ref(&param);
	}
	
	if (!tree_manager_add_child_node(result, "received", NODE_JSON_ARRAY, &received))
		return 0;

	amount = 0;
	
	for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
	{
		btc_addr_t my_addr;
		tree_manager_get_node_btcaddr	(addr, 0, my_addr);
		list_received					(my_addr, &received,min_conf,max_conf, &amount,&ntx,&max, first);
	}

	tree_manager_set_child_value_i64(result, "ntx", ntx);
	tree_manager_set_child_value_i64(result, "total", amount);

	release_zone_ref(&received);
	release_zone_ref(&addrs);
	

	return 1;
}




OS_API_C_FUNC(int) listsentobjs(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	char				appName[32];
	mem_zone_ref pn = { PTR_NULL },minconf = { PTR_NULL }, maxconf = { PTR_NULL }, spents = { PTR_NULL }, addrs = { PTR_NULL };
	mem_zone_ref  my_list = { PTR_NULL };
	mem_zone_ref_ptr addr;
	size_t				min_conf = 0, max_conf = 9999;
	size_t				max = 200, ntx = 0;
	unsigned int		appType; 

	if (!tree_manager_get_child_at(params, 0, &pn))return 0;
	tree_manager_get_node_str(&pn, 0, appName, 32, 0);
	release_zone_ref(&pn);

	if (!tree_manager_get_child_at(params, 1, &pn))return 0;
	tree_mamanger_get_node_dword(&pn, 0, &appType);
	release_zone_ref(&pn);


	if (tree_manager_get_child_at(params, 2, &minconf))
	{
		tree_mamanger_get_node_dword(&minconf, 0, &min_conf);
		release_zone_ref(&minconf);
	}
	if (tree_manager_get_child_at(params, 3, &maxconf))
	{
		tree_mamanger_get_node_dword(&maxconf, 0, &max_conf);
		release_zone_ref(&maxconf);
	}
	if (!tree_manager_get_child_at(params, 4, &addrs))return 0;

	if (!tree_manager_add_child_node(result, "objects", NODE_JSON_ARRAY, &spents))
		return 0;

	for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
	{
		btc_addr_t my_addr;

		tree_manager_get_node_btcaddr(addr, 0, my_addr);
		list_sentobjs(my_addr, appName,appType,&spents, min_conf, max_conf, &ntx, &max, 0);
	}

	tree_manager_set_child_value_i64(result, "ntx", ntx);

	release_zone_ref(&spents);
	release_zone_ref(&addrs);


	return 1;
}

OS_API_C_FUNC(int) listspent(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	mem_zone_ref minconf = { PTR_NULL }, maxconf = { PTR_NULL }, spents = { PTR_NULL }, addrs = { PTR_NULL };
	mem_zone_ref  my_list = { PTR_NULL };
	mem_zone_ref_ptr addr;
	uint64_t			total = 0;
	size_t				min_conf = 0, max_conf = 9999;
	size_t				max = 200, ntx = 0;



	if (tree_manager_get_child_at(params, 0, &minconf))
	{
		tree_mamanger_get_node_dword(&minconf, 0, &min_conf);
		release_zone_ref(&minconf);
	}
	if (tree_manager_get_child_at(params, 1, &maxconf))
	{
		tree_mamanger_get_node_dword(&maxconf, 0, &max_conf);
		release_zone_ref(&maxconf);
	}
	if (!tree_manager_get_child_at(params, 2, &addrs))return 0;

	if (!tree_manager_add_child_node(result, "spents", NODE_JSON_ARRAY, &spents))
		return 0;
	
	for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
	{
		btc_addr_t my_addr;

		tree_manager_get_node_btcaddr(addr, 0, my_addr);
		list_spent(my_addr, &spents, min_conf, max_conf, &total, &ntx, &max,0);
	}

	tree_manager_set_child_value_i64(result, "ntx", ntx);
	tree_manager_set_child_value_i64(result, "total", total);

	release_zone_ref(&spents);
	release_zone_ref(&addrs);


	return 1;
}

OS_API_C_FUNC(int) gettypes(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	mem_zone_ref types = { PTR_NULL };

	if (tree_manager_add_child_node(result, "types", NODE_JSON_ARRAY, &types))
	{
		node_get_types_def(&types);
		release_zone_ref(&types);
	}

	return 1;
}

OS_API_C_FUNC(int) create_root_app(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t				h;
	btc_addr_t			new_addr;
	mem_zone_ref		etx = { PTR_NULL }, appRoot = { PTR_NULL }, addr = { PTR_NULL }, approottx = { PTR_NULL }, AppFees = { PTR_NULL };
	uint64_t			appfees;
	int					ret;

	if(get_root_app       (&appRoot))
	{
		tree_manager_get_node_hash			(&appRoot,0,h);
		release_zone_ref					(&appRoot);
		tree_manager_set_child_value_hash	(result,"appRootTxHash",h);
		return 1;
	}

	if(!tree_manager_get_child_at		(params, 0, &addr))return 0;

	if (tree_manager_get_child_at(params, 0, &AppFees))
	{
		if(!tree_mamanger_get_node_qword(&AppFees, 0, &appfees))
			appfees = ONE_CENT;
	}
	else
		appfees = ONE_CENT;
	
	ret=tree_manager_get_node_btcaddr	(&addr, 0, new_addr);
	release_zone_ref					(&addr);
	
	if(!ret)return 0;
	
	make_approot_tx		(&approottx,get_time_c(), appfees,new_addr);

	ret = tree_manager_find_child_node(&my_node, NODE_HASH("submitted txs"), NODE_BITCORE_TX_LIST, &etx);
	if (ret)
	{
		hash_t txH;

		tree_manager_node_add_child				(&etx, &approottx);
		release_zone_ref						(&etx);

		tree_manager_get_child_value_hash		(&approottx, NODE_HASH("txid"), txH);
		tree_manager_set_child_value_hash		(result, "appRootTxHash", txH);
		tree_manager_set_child_value_btcaddr	(result, "appRootAddr", new_addr);
	}

	release_zone_ref(&approottx);

	return 1;
	
}
OS_API_C_FUNC(int) submittx(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t			txHash;
	unsigned char	chash[65];
	mem_zone_ref	pn = { PTR_NULL }, tx = { PTR_NULL }, etx = { PTR_NULL };
	int				n,ret;

	if (!tree_manager_get_child_at(params, 0, &pn))return 0;
	tree_manager_get_node_str(&pn, 0, chash, 65, 0);
	release_zone_ref(&pn);

	n = 0;
	while (n < 32)
	{
		char	hex[3];
		hex[0] = chash[n * 2 + 0];
		hex[1] = chash[n * 2 + 1];
		hex[2] = 0;
		txHash[31 - n] = strtoul_c(hex, PTR_NULL, 16);
		n++;
	}
	
	ret = tree_find_child_node_by_member_name_hash(&tmp_txs, NODE_BITCORE_TX, "txid", txHash, &tx);
	if (ret)
	{
		mem_zone_ref    mempool = { PTR_NULL };
		uint64_t		ins = 0, outs = 0;
		unsigned int	cb;

		node_aquire_mempool_lock(&mempool);
		ret=check_tx_inputs				(&tx, &ins, PTR_NULL, &cb, 1, &mempool);
		if(ret)ret	= check_tx_outputs	(&tx, &outs, PTR_NULL, &cb);
		node_release_mempool_lock();
		release_zone_ref(&mempool);

		if (ret)
		{
			unsigned int item_id;

			if (tree_manager_get_child_value_i32(&tx, NODE_HASH("app_item"), &item_id))
			{
				struct string appName = { 0 };
				if (item_id == 3)
				{
					mem_zone_ref file = { 0 };
					if (tree_manager_get_child_value_istr(&tx, NODE_HASH("appFile"), &appName, 0))
					{
						if (tree_manager_find_child_node(&tx, NODE_HASH("fileDef"), NODE_GFX_OBJECT, &file))
						{
							node_store_tmp_file(&appName, &file);
							release_zone_ref	(&file);
						}
					}
					free_string(&appName);
				}
				if (item_id == 4)
				{
					mem_zone_ref file = { 0 };
					if (tree_manager_get_child_value_istr(&tx, NODE_HASH("appLayout"), &appName, 0))
					{
						if (tree_manager_find_child_node(&tx, NODE_HASH("layoutDef"), NODE_GFX_OBJECT, &file))
						{
							node_rm_tmp_file(&appName, &file);
							release_zone_ref(&file);
						}
					}
					free_string(&appName);
				}
			}
		}

		if(ret)ret  = tree_manager_find_child_node	(&my_node, NODE_HASH("submitted txs"), NODE_BITCORE_TX_LIST, &etx);
		if(ret)
		{
			mem_zone_ref newtmp = { PTR_NULL };
			tree_manager_node_add_child				(&etx, &tx);
			release_zone_ref						(&etx);

			/*
			tree_manager_node_dup					(PTR_NULL, &tmp_txs, &newtmp,16);
			tree_remove_child_by_member_value_hash	(&newtmp, NODE_BITCORE_TX, "txid", txHash);
			copy_zone_ref							(&newtmp,&tmp_txs);
			*/

			tree_remove_child_by_member_value_hash	(&tmp_txs, NODE_BITCORE_TX, "txid", txHash);

			tree_manager_set_child_value_hash		(result, "txid", txHash);
		}
	}
	

	return ret;

}

OS_API_C_FUNC(int) signtxinput(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t			txHash;
	unsigned char	chash[65];
	struct	string  bsign = { PTR_NULL }, bpubkey = { PTR_NULL }, sign = { PTR_NULL }, inPubKey = { PTR_NULL };
	mem_zone_ref	pn = { PTR_NULL }, tx = { PTR_NULL };
	int				ret = 0;
	unsigned int	n, inIdx;
	unsigned char   hash_type = 1;

	if (!tree_manager_get_child_at(params, 0, &pn))return 0;
	tree_manager_get_node_str(&pn, 0, chash, 65, 0);
	release_zone_ref(&pn);

	n = 0;
	while (n < 32)
	{
		char	hex[3];
		hex[0] = chash[n * 2 + 0];
		hex[1] = chash[n * 2 + 1];
		hex[2] = 0;
		txHash[31-n] = strtoul_c(hex, PTR_NULL, 16);
		n++;
	}


	if (!tree_manager_get_child_at(params, 1, &pn))return 0;
	tree_mamanger_get_node_dword(&pn, 0, &inIdx);
	release_zone_ref(&pn);

	if (!tree_manager_get_child_at(params, 2, &pn))return 0;
	tree_manager_get_node_istr(&pn, 0, &sign, 0);
	release_zone_ref(&pn);

	bsign.len = (sign.len / 2) + 1;
	bsign.size = bsign.len + 1;
	bsign.str = malloc_c(bsign.size);
	n = 0;
	while (n < bsign.len)
	{
		char	hex[3];
		hex[0] = sign.str[n * 2 + 0];
		hex[1] = sign.str[n * 2 + 1];
		hex[2] = 0;
		bsign.str[n] = strtoul_c(hex, PTR_NULL, 16);
		n++;
	}
	free_string(&sign);

	bsign.str[bsign.len - 1] = hash_type;

	if (tree_manager_get_child_at(params, 3, &pn))
	{
		struct string inPubKey = { PTR_NULL };

		tree_manager_get_node_istr(&pn, 0, &inPubKey, 16);
		release_zone_ref(&pn);
		if (inPubKey.len == 66)
		{
			bpubkey.len  = 33;
			bpubkey.size = bpubkey.len + 1;
			bpubkey.str  = malloc_c(bpubkey.size);

			n = 0;
			while (n < bpubkey.len)
			{
				char	hex[3];
				hex[0] = inPubKey.str[n * 2 + 0];
				hex[1] = inPubKey.str[n * 2 + 1];
				hex[2] = 0;
				bpubkey.str[n] = strtoul_c(hex, PTR_NULL, 16);
				n++;
			}
		}
		free_string(&inPubKey);
	}

	if (tree_find_child_node_by_member_name_hash(&tmp_txs, NODE_BITCORE_TX, "txid", txHash, &tx))
	{
		mem_zone_ref mempool = { PTR_NULL };

		node_aquire_mempool_lock	(&mempool);
		ret	= tx_sign				(&tx, inIdx, hash_type, &bsign, &bpubkey, &mempool);
		release_zone_ref			(&mempool);
		node_release_mempool_lock	();


		if (ret)
		{
			hash_t		 txh;

			compute_tx_hash						(&tx	, txh);
			tree_manager_set_child_value_bhash	(&tx	, "txid", txh);
			tree_manager_set_child_value_hash	(result	, "txid", txh);
		}
		else
			log_output("check tx sign fail\n");

		release_zone_ref(&tx);
	}
	else
		log_output("tx not found\n");

	
	
	free_string(&bpubkey);
	free_string(&bsign);
	return ret;

}


OS_API_C_FUNC(int) maketxfrom(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t				txh;
	btc_addr_t			dstAddr,changeAddr;
	mem_zone_ref		pn = { PTR_NULL }, addrs = { PTR_NULL };
	mem_zone_ref		script_node = { PTR_NULL }, inList = { PTR_NULL }, my_list = { PTR_NULL }, new_tx = { PTR_NULL }, mempool = { PTR_NULL }, newtmp = { PTR_NULL };
	struct	string		oScript = { PTR_NULL };
	mem_zone_ref_ptr	addr;
	double				dAmount;
	uint64_t			nAmount, total_unspent, paytxfee, inFees;
	size_t				min_conf = 0, max_conf = 9999;
	size_t				max = 200, ntx = 0;
	size_t				nin;

	if (!tree_manager_get_child_at(params, 1, &pn))
		return 0;

	tree_mamanger_get_node_double(&pn, 0, &dAmount);
	release_zone_ref(&pn);


	dtoll_c(dAmount, &nAmount);


	if (!tree_manager_get_child_at(params, 2, &pn))
		return 0;
	
	tree_manager_get_node_btcaddr(&pn, 0, dstAddr);
	release_zone_ref(&pn);
	

	if (!tree_manager_get_child_at(params, 0, &addrs))
		return 0;

	if (tree_manager_get_child_at(params, 3, &pn))
	{
		tree_mamanger_get_node_qword(&pn, 0, &inFees);
		release_zone_ref(&pn);
	}
	else
		inFees = 0;

	
	if (tree_manager_get_child_at(params, 4, &pn))
	{
		tree_mamanger_get_node_dword(&pn, 0, &min_conf);
		release_zone_ref(&pn);
	}
	else
		min_conf = 10;

	if (tree_manager_get_child_at(params, 5, &pn))
	{
		tree_mamanger_get_node_dword(&pn, 0, &max_conf);
		release_zone_ref(&pn);
	}
	else
		max_conf = 9999;

	if (tree_manager_get_child_at(params, 6, &pn))
	{
		tree_manager_get_node_btcaddr	(&pn, 0, dstAddr);
		release_zone_ref				(&pn);
	}
	else
		memset_c	(changeAddr, 0, sizeof(btc_addr_t));

	if (tree_manager_get_child_value_i64(&my_node, NODE_HASH("paytxfee"), &paytxfee))
	{
		if (inFees > paytxfee)
			inFees = paytxfee;
	}
	else if (inFees > 0)
		paytxfee = inFees;
	else
		paytxfee = 0;
	

	
	node_aquire_mempool_lock(&mempool);
	new_transaction			(&new_tx, get_time_c());
	
	total_unspent = 0;
	for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
	{
		btc_addr_t						my_addr;
		tree_manager_get_node_btcaddr	(addr, 0, my_addr);
		get_tx_inputs_from_addr			(my_addr,&mempool, &total_unspent, nAmount+paytxfee, min_conf, max_conf, &new_tx);

		if (changeAddr[0] == 0)memcpy_c	(changeAddr, my_addr,sizeof(btc_addr_t));
	}
	tree_manager_set_child_value_i64(result, "total", total_unspent);

	release_zone_ref					(&mempool);

	node_release_mempool_lock();
	

	if (tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node))
	{
		create_p2sh_script(dstAddr, &script_node);
		serialize_script(&script_node, &oScript);
		release_zone_ref(&script_node);
	}

	tx_add_output		(&new_tx, nAmount, &oScript);
	free_string			(&oScript);

	if (total_unspent > nAmount)
	{
		if (tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node))
		{
			create_p2sh_script(changeAddr, &script_node);
			serialize_script(&script_node, &oScript);
			release_zone_ref(&script_node);
		}

		tx_add_output(&new_tx, total_unspent - nAmount - paytxfee, &oScript);
		free_string	 (&oScript);
	}

	tree_manager_set_child_value_i64(&new_tx, "fee", paytxfee);

	if (tree_manager_find_child_node(&new_tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &inList))
	{
		mem_zone_ref_ptr	input = PTR_NULL;

		for (nin = 0, tree_manager_get_first_child(&inList, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input), nin++)
		{
			hash_t				txh,hh;
			hash_t				h;
			btc_addr_t			src_addr;
			struct string		script = { PTR_NULL };
			uint64_t			inAmount;
			unsigned int		oIdx,cnt;

			tree_manager_get_child_value_hash	(input, NODE_HASH("txid"), h);
			tree_manager_get_child_value_i32	(input, NODE_HASH("idx"), &oIdx);

			get_tx_output_script				(h, oIdx, &script,&inAmount);
			get_out_script_address				(&script, PTR_NULL, src_addr);


			tree_manager_set_child_value_btcaddr(input, "srcaddr", src_addr);
			tree_manager_set_child_value_i32	(input, "index", nin);
			tree_manager_set_child_value_i64	(input, "value", inAmount);
			

			compute_tx_sign_hash				(&new_tx, nin, &script, 1, txh);
			free_string							(&script);

			cnt = 32;
			while (cnt--)
				hh[31 - cnt] = txh[cnt];

			tree_manager_set_child_value_hash	(input, "signHash", hh);
		}
		release_zone_ref(&inList);
	}

	compute_tx_hash						(&new_tx, txh);
	tree_manager_set_child_value_bhash	(&new_tx, "txid", txh);

	tree_manager_node_add_child			(&tmp_txs, &new_tx);
	tree_manager_node_add_child			(result, &new_tx);
	release_zone_ref					(&addrs);
	release_zone_ref					(&new_tx);
	
	return 1;
}

OS_API_C_FUNC(int) makeobjtxfr(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t				txh, objHash;
	btc_addr_t			dstAddr;
	mem_zone_ref		pn = { PTR_NULL }, new_tx = { PTR_NULL }, inList = { PTR_NULL }, my_list = { PTR_NULL };
	uint64_t			paytxfee, inFees;
	size_t				min_conf = 0, max_conf = 9999;
	unsigned int		oidx;
	int ret;

	if (!tree_manager_get_child_at(params, 0, &pn))return 0;
	ret = tree_manager_get_node_hash(&pn, 0, txh);
	release_zone_ref(&pn);
	if (!ret)return 0;

	if (!tree_manager_get_child_at(params, 1, &pn))return 0;
	ret = tree_mamanger_get_node_dword(&pn, 0, &oidx);
	release_zone_ref(&pn);
	if (!ret)return 0;

	if (!tree_manager_get_child_at(params, 2, &pn))return 0;
	ret = tree_manager_get_node_btcaddr(&pn, 0, dstAddr);
	release_zone_ref(&pn);
	if (!ret)return 0;

	if (tree_manager_get_child_at(params, 3, &pn))
	{
		tree_mamanger_get_node_qword(&pn, 0, &inFees);
		release_zone_ref(&pn);
	}
	else
		inFees = 0;

	if (tree_manager_get_child_at(params, 4, &pn))
	{
		tree_mamanger_get_node_dword(&pn, 0, &min_conf);
		release_zone_ref(&pn);
	}
	else
		min_conf = 10;

	if (tree_manager_get_child_at(params, 5, &pn))
	{
		tree_mamanger_get_node_dword(&pn, 0, &max_conf);
		release_zone_ref(&pn);
	}
	else
		max_conf = 9999;

	if (tree_manager_get_child_value_i64(&my_node, NODE_HASH("paytxfee"), &paytxfee))
	{
		if (inFees > paytxfee)
			inFees = paytxfee;
	}
	else if (inFees > 0)
		paytxfee = inFees;
	else
		paytxfee = 0;

	ret = make_obj_txfr_tx(&new_tx, txh, oidx, dstAddr,objHash);

	if (!ret)
		return 0;

	if (tree_manager_find_child_node(&new_tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &inList))
	{
		mem_zone_ref_ptr	input = PTR_NULL;
		unsigned int nin;

		for (nin = 0, tree_manager_get_first_child(&inList, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input), nin++)
		{
			hash_t				txh, hh;
			hash_t				h;
			btc_addr_t			src_addr;
			struct string		script = { PTR_NULL };
			uint64_t			inAmount;
			unsigned int		oIdx, cnt;

			tree_manager_get_child_value_hash(input, NODE_HASH("txid"), h);
			tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &oIdx);

			get_tx_output_script(h, oIdx, &script, &inAmount);
			if (get_out_script_address(&script, PTR_NULL, src_addr))
				tree_manager_set_child_value_btcaddr(input, "srcaddr", src_addr);

			tree_manager_set_child_value_i32(input, "index", nin);
			tree_manager_set_child_value_i64(input, "value", inAmount);
			tree_manager_set_child_value_hash(input, "objHash", objHash);

			compute_tx_sign_hash(&new_tx, nin, &script, 1, txh);
			free_string(&script);

			cnt = 32;
			while (cnt--)
				hh[31 - cnt] = txh[cnt];

			tree_manager_set_child_value_hash(input, "signHash", hh);
		}
		release_zone_ref(&inList);
	}


	compute_tx_hash(&new_tx, txh);
	tree_manager_set_child_value_bhash(&new_tx, "txid", txh);

	tree_manager_node_add_child(&tmp_txs, &new_tx);
	tree_manager_node_add_child(result, &new_tx);
	release_zone_ref(&new_tx);
	return ret;
}


OS_API_C_FUNC(int) makeapptx(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t				txh;
	btc_addr_t			dstAddr,changeAddr,appAddr;
	mem_zone_ref		pn = { PTR_NULL }, addrs = { PTR_NULL };
	mem_zone_ref		script_node = { PTR_NULL }, inList = { PTR_NULL }, my_list = { PTR_NULL }, new_tx = { PTR_NULL }, mempool = { PTR_NULL };
	struct	string		oScript = { 0 },appName={0};
	mem_zone_ref_ptr	addr = PTR_NULL ;
	uint64_t			nAmount, total_unspent, paytxfee, inFees;
	size_t				min_conf = 0, max_conf = 9999;
	size_t				max = 200, ntx = 0, locked = 0;
	size_t				nin;
	int					ret;

	if (!tree_manager_create_node("addr", NODE_BITCORE_WALLET_ADDR, &pn))return 0;
	ret = get_root_app_addr(&pn);
	if (ret)ret = tree_manager_get_node_btcaddr(&pn, 0, dstAddr);
	release_zone_ref(&pn);
	if(!ret)return 0;

	if (!tree_manager_get_child_at(params, 0, &pn))return 0;
	ret=tree_manager_get_node_btcaddr	(&pn, 0, appAddr);
	release_zone_ref					(&pn);
	if(!ret)return 0;

	if (!tree_manager_get_child_at(params, 1, &pn))return 0;
	ret=tree_manager_get_node_istr		(&pn, 0, &appName,16);
	release_zone_ref					(&pn);
	if(!ret)return 0;

	if (!tree_manager_get_child_at(params, 2, &pn))return 0;
	ret = tree_mamanger_get_node_dword(&pn, 0, &locked);
	release_zone_ref(&pn);
	if (!ret)return 0;

	if (!tree_manager_get_child_at(params, 3, &addrs)){
		free_string(&appName);
		return 0;
	}
	if (tree_manager_get_child_at(params, 4, &pn))
	{
		tree_mamanger_get_node_qword(&pn, 0, &inFees);
		release_zone_ref			(&pn);
	}
	else
		inFees = 0;

	if (tree_manager_get_child_at(params, 5, &pn))
	{
		tree_mamanger_get_node_dword(&pn, 0, &min_conf);
		release_zone_ref			(&pn);
	}
	else
		min_conf = 10;

	if (tree_manager_get_child_at(params, 6, &pn))
	{
		tree_mamanger_get_node_dword(&pn, 0, &max_conf);
		release_zone_ref(&pn);
	}
	else
		max_conf = 9999;

	if (tree_manager_get_child_value_i64(&my_node, NODE_HASH("paytxfee"), &paytxfee))
	{
		if (inFees > paytxfee)
			inFees = paytxfee;
	}
	else if (inFees > 0)
		paytxfee = inFees;
	else
		paytxfee = 0;
	
	
	make_app_tx		(&new_tx, appName.str, locked, appAddr);
	free_string		(&appName);


	ret=tree_manager_create_node("appfee", NODE_GFX_BINT, &pn);
	if (ret)
	{
		ret = get_root_app_fee(&pn);
		if(ret)tree_mamanger_get_node_qword(&pn, 0, &nAmount);
		release_zone_ref(&pn);
	}

	//tree_manager_find_child_node(&my_node, NODE_HASH("mempool"), NODE_BITCORE_TX_LIST, &mempool);
	node_aquire_mempool_lock(&mempool);

	memset_c	(changeAddr, 0, sizeof(btc_addr_t));
	total_unspent = 0;
	for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
	{
		btc_addr_t						my_addr;
		tree_manager_get_node_btcaddr	(addr, 0, my_addr);
		get_tx_inputs_from_addr			(my_addr,&mempool, &total_unspent, nAmount+paytxfee, min_conf, max_conf, &new_tx);

		if (changeAddr[0] == 0)memcpy_c	(changeAddr, my_addr,sizeof(btc_addr_t));
	}

	release_zone_ref				(&mempool);
	node_release_mempool_lock		();

	tree_manager_set_child_value_i64(result, "total", total_unspent);
	
	if (tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node))
	{
		create_p2sh_script	(dstAddr, &script_node);
		serialize_script	(&script_node, &oScript);
		release_zone_ref	(&script_node);
	}

	if (total_unspent<(nAmount + paytxfee))
	{
		release_zone_ref						(&addrs);
		release_zone_ref						(&new_tx);
		return 0;
	}

	tx_add_output		(&new_tx, nAmount, &oScript);
	free_string			(&oScript);


	if (tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node))
	{
		create_p2sh_script	(changeAddr, &script_node);
		serialize_script	(&script_node, &oScript);
		release_zone_ref	(&script_node);
	}

	tx_add_output(&new_tx, total_unspent - (nAmount + paytxfee), &oScript);
	free_string	 (&oScript);
	

	if (tree_manager_find_child_node(&new_tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &inList))
	{
		mem_zone_ref_ptr	input = PTR_NULL;

		for (nin = 0, tree_manager_get_first_child(&inList, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input), nin++)
		{
			hash_t				txh,hh;
			hash_t				h;
			btc_addr_t			src_addr;
			struct string		script = { PTR_NULL };
			uint64_t			inAmount;
			unsigned int		oIdx,cnt;

			tree_manager_get_child_value_hash	(input, NODE_HASH("txid"), h);
			tree_manager_get_child_value_i32	(input, NODE_HASH("idx"), &oIdx);

			get_tx_output_script				(h, oIdx, &script,&inAmount);
			if(get_out_script_address				(&script, PTR_NULL, src_addr))
				tree_manager_set_child_value_btcaddr(input, "srcaddr"	, src_addr);

			tree_manager_set_child_value_i32	(input, "index"		, nin);
			tree_manager_set_child_value_i64	(input, "value"		, inAmount);

			compute_tx_sign_hash	(&new_tx, nin, &script, 1, txh);
			free_string				(&script);

			cnt = 32;
			while (cnt--)
				hh[31 - cnt] = txh[cnt];

			tree_manager_set_child_value_hash(input, "signHash", hh);
		}
		release_zone_ref(&inList);
	}

	
	compute_tx_hash							(&new_tx, txh);
	tree_manager_set_child_value_bhash		(&new_tx, "txid", txh);

	tree_manager_node_add_child				(&tmp_txs	, &new_tx);
	tree_manager_node_add_child				(result		, &new_tx);
	release_zone_ref						(&addrs);
	release_zone_ref						(&new_tx);
	
	return 1;
}


OS_API_C_FUNC(int) makeapptypetx(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t				txh;
	btc_addr_t			changeAddr;
	mem_zone_ref		pn = { PTR_NULL }, addrs = { PTR_NULL }, type_keys= { PTR_NULL };
	mem_zone_ref		script_node = { PTR_NULL }, inList = { PTR_NULL }, my_list = { PTR_NULL }, new_tx = { PTR_NULL }, mempool = { PTR_NULL };
	struct	string		oScript = { 0 }, appName = { 0 }, typeName = { 0 };
	mem_zone_ref_ptr	type = PTR_NULL, addr = PTR_NULL;
	unsigned int		typeId = 0;
	uint64_t			total_unspent, paytxfee, inFees;
	size_t				min_conf = 0, max_conf = 9999;
	size_t				max = 200, ntx = 0;
	size_t				nin;
	int					ret;

	if (!tree_manager_get_child_at(params, 0, &pn))return 0;
	ret = tree_manager_get_node_istr(&pn, 0, &appName,0);
	release_zone_ref(&pn);
	if (!ret)return 0;

	if (!tree_manager_get_child_at(params, 1, &pn))return 0;
	ret = tree_manager_get_node_istr(&pn, 0, &typeName, 16);
	release_zone_ref(&pn);
	if (!ret)
	{
		free_string(&appName);
		return 0;
	}

	if (!tree_manager_get_child_at(params, 2, &pn))return 0;
	ret = tree_mamanger_get_node_dword(&pn, 0, &typeId);
	release_zone_ref(&pn);
	if ((!ret) || (typeId < 1))
	{
		free_string(&appName);
		free_string(&typeName);
	}

	typeId &= 0x00FFFFFF;
	typeId |= 0x1E000000;

	if (!tree_manager_get_child_at(params, 3, &type_keys))
	{
		free_string(&appName);
		free_string(&typeName);
		return 0;
	}
	
	if (!tree_manager_get_child_at(params, 4, &addrs)){
		free_string(&appName);
		free_string(&typeName);
		release_zone_ref(&type_keys);
		return 0;
	}
	if (tree_manager_get_child_at(params, 5, &pn))
	{
		tree_mamanger_get_node_qword(&pn, 0, &inFees);
		release_zone_ref(&pn);
	}
	else
		inFees = 0;

	if (tree_manager_get_child_at(params, 6, &pn))
	{
		tree_mamanger_get_node_dword(&pn, 0, &min_conf);
		release_zone_ref(&pn);
	}
	else
		min_conf = 10;

	if (tree_manager_get_child_at(params, 7, &pn))
	{
		tree_mamanger_get_node_dword(&pn, 0, &max_conf);
		release_zone_ref(&pn);
	}
	else
		max_conf = 9999;

	if (tree_manager_get_child_value_i64(&my_node, NODE_HASH("paytxfee"), &paytxfee))
	{
		if (inFees > paytxfee)
			paytxfee =inFees;
	}
	else if (inFees > 0)
		paytxfee = inFees;
	else
		paytxfee = 0;


	ret = make_app_item_tx(&new_tx, &appName, 1);
	free_string(&appName);
	if (!ret)
	{
		free_string		(&typeName);
		release_zone_ref(&type_keys);
		release_zone_ref(&addrs);
		return 0;
	}

	tree_manager_create_node		 ("script", NODE_BITCORE_SCRIPT, &script_node);
	tree_manager_set_child_value_vstr(&script_node, "name", &typeName);
	tree_manager_set_child_value_i32 (&script_node, "id", typeId);
	serialize_script				 (&script_node, &oScript);
	free_string						 (&typeName);

	release_zone_ref				 (&script_node);
	tx_add_output					 (&new_tx, 0, &oScript);
	free_string						 (&oScript);

	for (tree_manager_get_first_child(&type_keys, &my_list, &type); ((type != NULL) && (type->zone != NULL)); tree_manager_get_next_child(&my_list, &type))
	{
		unsigned int flags = 0;
		unsigned int unique, index;
		tree_manager_get_child_value_istr	(type, NODE_HASH("name"), &typeName,0);
		tree_manager_get_child_value_i32	(type, NODE_HASH("key") , &typeId);
		
		if (!tree_manager_get_child_value_i32(type, NODE_HASH("unique"), &unique))
			unique = 0;

		if (!tree_manager_get_child_value_i32(type, NODE_HASH("index"), &index))
			index = 0;

		if (unique) flags |= 1;
		if (index)  flags |= 2;

		tree_manager_create_node			("script", NODE_BITCORE_SCRIPT, &script_node);
		tree_manager_set_child_value_vstr	(&script_node, "name", &typeName);
		tree_manager_set_child_value_i32	(&script_node, "id", typeId);
		tree_manager_set_child_value_i32	(&script_node, "flags", flags);
		serialize_script					(&script_node, &oScript);
		free_string							(&typeName);
		release_zone_ref					(&script_node);

		tx_add_output						(&new_tx, 0, &oScript);
		free_string							(&oScript);
	}

	release_zone_ref(&type_keys);

	if (paytxfee > 0)
	{
		node_aquire_mempool_lock(&mempool);

		memset_c(changeAddr, 0, sizeof(btc_addr_t));
		total_unspent = 0;
		for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
		{
			btc_addr_t						my_addr;
			tree_manager_get_node_btcaddr(addr, 0, my_addr);
			get_tx_inputs_from_addr(my_addr, &mempool, &total_unspent, paytxfee, min_conf, max_conf, &new_tx);

			if (changeAddr[0] == 0)memcpy_c(changeAddr, my_addr, sizeof(btc_addr_t));
		}
		tree_manager_set_child_value_i64(result, "total", total_unspent);
		release_zone_ref(&mempool);

		node_release_mempool_lock();

		if (total_unspent < paytxfee)
		{
			release_zone_ref(&addrs);
			release_zone_ref(&new_tx);
			return 0;
		}

		if (tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node))
		{
			create_p2sh_script(changeAddr, &script_node);
			serialize_script(&script_node, &oScript);
			release_zone_ref(&script_node);
		}

		tx_add_output(&new_tx, total_unspent - paytxfee, &oScript);
		free_string(&oScript);
	}

	if (tree_manager_find_child_node(&new_tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &inList))
	{
		mem_zone_ref_ptr	input = PTR_NULL;

		for (nin = 0, tree_manager_get_first_child(&inList, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input), nin++)
		{
			hash_t				txh, hh;
			hash_t				h;
			btc_addr_t			src_addr;
			struct string		script = { PTR_NULL };
			uint64_t			inAmount;
			unsigned int		oIdx, cnt;

			tree_manager_get_child_value_hash(input, NODE_HASH("txid"), h);
			tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &oIdx);

			get_tx_output_script(h, oIdx, &script, &inAmount);
			get_out_script_address(&script, PTR_NULL, src_addr);

			tree_manager_set_child_value_btcaddr(input, "srcaddr", src_addr);
			tree_manager_set_child_value_i32(input, "index", nin);
			tree_manager_set_child_value_i64(input, "value", inAmount);

			compute_tx_sign_hash(&new_tx, nin, &script, 1, txh);
			free_string(&script);

			cnt = 32;
			while (cnt--)
				hh[31 - cnt] = txh[cnt];

			tree_manager_set_child_value_hash(input, "signHash", hh);
		}

		release_zone_ref(&inList);
	}

	compute_tx_hash						(&new_tx, txh);
	tree_manager_set_child_value_bhash	(&new_tx, "txid", txh);

	tree_manager_node_add_child			(&tmp_txs, &new_tx);
	tree_manager_node_add_child			(result, &new_tx);
	release_zone_ref(&addrs);
	release_zone_ref(&new_tx);

	return 1;
}


OS_API_C_FUNC(int) makeappobjtx(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t				txh;
	btc_addr_t			changeAddr, pubaddr;
	mem_zone_ref		vin = { PTR_NULL },tmp = { PTR_NULL }, pn = { PTR_NULL }, addrs = { PTR_NULL }, objData = { PTR_NULL }, newObj = { PTR_NULL }, type = { PTR_NULL };
	mem_zone_ref		script_node = { PTR_NULL }, inList = { PTR_NULL }, my_list = { PTR_NULL }, new_tx = { PTR_NULL }, mempool = { PTR_NULL };
	struct	string		oScript = { 0 }, appName = { 0 }, objPKey = { PTR_NULL }, bKey = { PTR_NULL };
	mem_zone_ref_ptr	 addr = PTR_NULL;
	uint64_t			total_unspent, paytxfee, inFees;
	unsigned int		typeId = 0;
	unsigned char		*buffer;
	size_t				min_conf = 0, max_conf = 9999;
	size_t				max = 200, ntx = 0,objLen;
	size_t				nin;
	int					ret;

	ret			= tree_manager_get_child_at		(params, 0, &pn);
	if(ret)ret	= tree_manager_get_node_istr	(&pn, 0, &appName, 0);
	release_zone_ref(&pn);
	

	if(ret)ret	= tree_manager_get_child_at		(params, 1, &pn);
	if(ret)ret  = tree_mamanger_get_node_dword	(&pn, 0, &typeId);
	release_zone_ref							(&pn);

	
	if(ret)ret  = tree_manager_get_child_at		(params, 2, &pn);
	if(ret)ret	= tree_manager_get_node_istr	(&pn, 0, &objPKey,16);
	release_zone_ref(&pn);

	ret = (objPKey.len == 66) ? 1 : 0;

	if (ret)
	{
		int n = 0;

		bKey.len	= 33;
		bKey.size	= 33;
		bKey.str	= malloc_c(bKey.len);

		while (n<33)
		{
			char    hex[3];
			if (!isxdigit_c(objPKey.str[n * 2 + 0]) || !isxdigit_c(objPKey.str[n * 2 + 1]))
			{
				ret = 0;
				break;
			}

			hex[0]		= objPKey.str[n  * 2 + 0];
			hex[1]		= objPKey.str[n  * 2 + 1];
			hex[2]		= 0;
			bKey.str[n] = strtoul_c(hex, PTR_NULL, 16);

			n++;
		}
		free_string(&objPKey);
	}

	if (ret)key_to_addr(bKey.str, pubaddr);
	if (ret)ret = tree_manager_get_child_at		(params, 3, &objData);
	if (ret)ret = tree_manager_get_child_at		(params, 4, &addrs);

	if ((!ret) || (typeId < 1))
	{
		release_zone_ref(&objData);
		release_zone_ref(&addrs);
		free_string(&appName);
		return 0;
	}

	typeId &= 0x00FFFFFF;
	typeId |= 0x1E000000;


	if (tree_manager_get_child_at(params, 5, &pn))
	{
		tree_mamanger_get_node_qword(&pn, 0, &inFees);
		release_zone_ref(&pn);
	}
	else
		inFees = 0;

	if (tree_manager_get_child_at(params, 6, &pn))
	{
		tree_mamanger_get_node_dword(&pn, 0, &min_conf);
		release_zone_ref(&pn);
	}
	else
		min_conf = 10;

	if (tree_manager_get_child_at(params, 7, &pn))
	{
		tree_mamanger_get_node_dword(&pn, 0, &max_conf);
		release_zone_ref(&pn);
	}
	else
		max_conf = 9999;

	if (tree_manager_get_child_value_i64(&my_node, NODE_HASH("paytxfee"), &paytxfee))
	{
		if (inFees > paytxfee)
			paytxfee = inFees;
	}
	else if (inFees > 0)
		paytxfee = inFees;
	else
		paytxfee = 0;


	if (get_apps(&tmp))
	{
		mem_zone_ref app = { PTR_NULL };
		ret = tree_manager_find_child_node(&tmp, NODE_HASH(appName.str), NODE_BITCORE_TX, &app);
		release_zone_ref(&tmp);
		if (ret)
		{
			mem_zone_ref app_types = { PTR_NULL };

			if (get_app_types(&app, &app_types))
			{
				ret = tree_find_child_node_by_id_name	(&app_types, NODE_BITCORE_TX, "typeId", typeId, &type);
				release_zone_ref						(&app_types);
				if (ret)
				{
					mem_zone_ref		type_outs = { PTR_NULL };
					mem_zone_ref_ptr	key = PTR_NULL;
					unsigned int		oidx;

					tree_manager_find_child_node		(&type, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &type_outs);

					tree_manager_create_node			("newObj", NODE_GFX_OBJECT, &newObj);

					for (oidx = 0, tree_manager_get_first_child(&type_outs, &my_list, &key); ((key != NULL) && (key->zone != NULL)); oidx++,tree_manager_get_next_child(&my_list, &key))
					{
						char			KeyName[32];
						struct string	KeyStr;
						unsigned int	KeyId,flags;
						uint64_t		amount;

						if (oidx == 0)continue;
						tree_manager_get_child_value_i64(key, NODE_HASH("value"), &amount);
						if (amount != 0)continue;

						tree_manager_get_child_value_istr(key, NODE_HASH("script"), &KeyStr, 0);

						if (get_type_infos(&KeyStr, KeyName, &KeyId,&flags))
						{
							if ((KeyId >> 24) == 0x1E)
							{
								char		chash[65];
								hash_t		binHash;
								mem_zone_ref newKey = { PTR_NULL };
								unsigned int n = 0;
								tree_manager_get_child_value_str			(&objData, NODE_HASH(KeyName), chash,65,0);

								while (n<32)
								{
									char    hex[3];
									hex[0] = chash[(31 - n) * 2 + 0];
									hex[1] = chash[(31 - n) * 2 + 1];
									hex[2] = 0;
									binHash[n] = strtoul_c(hex, PTR_NULL, 16);
									n++;
								}
								if (tree_manager_add_child_node(&newObj, KeyName, KeyId, &newKey))
								{
									ret = tree_manager_write_node_hash(&newKey, 0, binHash);
									release_zone_ref(&newKey);
								}
							}
							else
							{
								switch (KeyId)
								{
									case NODE_BITCORE_VSTR:
									case NODE_GFX_STR:
									{
										struct string mystr = { 0 };
										ret = tree_manager_get_child_value_istr(&objData, NODE_HASH(KeyName), &mystr, 0);
										if (ret)
										{
											tree_manager_set_child_value_vstr(&newObj, KeyName, &mystr);
											free_string(&mystr);
										}
									}
									break;
									case NODE_GFX_INT:
									{
										unsigned int myint;
										ret = tree_manager_get_child_value_i32(&objData, NODE_HASH(KeyName), &myint);
										if (ret)ret = tree_manager_set_child_value_i32(&newObj, KeyName, myint);
									}
									break;
									case NODE_GFX_SIGNED_INT:
									{
										int myint;
										ret = tree_manager_get_child_value_si32(&objData, NODE_HASH(KeyName), &myint);
										if (ret)ret = tree_manager_set_child_value_si32(&newObj, KeyName, myint);
									}
									break;
									case NODE_GFX_BINT:
									{
										uint64_t myint;
										ret = tree_manager_get_child_value_i64(&objData, NODE_HASH(KeyName), &myint);
										if (ret)ret = tree_manager_set_child_value_i64(&newObj, KeyName, myint);
									}
									break;
									case NODE_GFX_SIGNED_BINT:
									{
										int64_t myint;
										ret = tree_manager_get_child_value_si64(&objData, NODE_HASH(KeyName), &myint);
										if (ret)ret = tree_manager_set_child_value_si64(&newObj, KeyName, myint);
									}
									break;
									case NODE_GFX_FLOAT:
									{
										float myfloat;
										ret = tree_manager_get_child_value_float(&objData, NODE_HASH(KeyName), &myfloat);
										if (ret)ret = tree_manager_set_child_value_float(&newObj, KeyName, myfloat);
									}
									break;

									case NODE_GFX_DOUBLE:
									{
										double mydouble;
										ret = tree_manager_get_child_value_double(&objData, NODE_HASH(KeyName), &mydouble);
										if (ret)ret = tree_manager_set_child_value_double(&newObj, KeyName, mydouble);
									}
									break;

									case NODE_BITCORE_PUBKEY:
									{
										unsigned char	binHash[33];
										mem_zone_ref	newKey = { PTR_NULL };
										unsigned char	*chash;
										int				sz;
										unsigned int	n;

										sz = tree_manager_get_child_data_ptr(&objData, NODE_HASH(KeyName), &chash);
										if (sz >= 66)
										{
											for (n = 0; n < 33;n++)
											{
												char    hex[3];
												hex[0] = chash[n * 2 + 0];
												hex[1] = chash[n * 2 + 1];
												hex[2] = 0;
												binHash[n] = strtoul_c(hex, PTR_NULL, 16);
											}
										}
										else
											memset_c(binHash, 0, 33);

										tree_manager_add_child_node		(&newObj, KeyName, NODE_BITCORE_PUBKEY, &newKey);
										tree_manager_write_node_data	(&newKey,binHash,0,33);
										release_zone_ref				(&newKey);
									}
									break;
									case NODE_BIN_DATA:
									{
										char		*chash;
										hash_t		binHash;
										int			sz;
										unsigned int n = 0;
				
										mem_zone_ref newKey = { PTR_NULL };

										sz = tree_manager_get_child_data_ptr(&objData, NODE_HASH(KeyName), &chash);
										if (sz >= 64)
										{
											while (n < 32)
											{
												char    hex[3];
												hex[0] = chash[(31 - n) * 2 + 0];
												hex[1] = chash[(31 - n) * 2 + 1];
												hex[2] = 0;
												binHash[n] = strtoul_c(hex, PTR_NULL, 16);
												n++;
											}
										}
										else
											memset_c(binHash, 0, sizeof(hash_t));
										
										tree_manager_add_child_node(&newObj, KeyName, NODE_BIN_DATA, &newKey);
										tree_manager_write_node_hash(&newKey, 0, binHash);
										release_zone_ref(&newKey);
									}
									break;
									case NODE_GFX_4UC:
									{
										mem_zone_ref	myvec = { PTR_NULL }, vecval = { PTR_NULL };
										vec_4uc_t		vec4;
										ret = tree_manager_find_child_node(&objData, NODE_HASH(KeyName), NODE_JSON_ARRAY, &myvec);

										tree_manager_get_child_at(&myvec, 0, &vecval);
										tree_mamanger_get_node_byte(&vecval, 0, &vec4[0]);
										release_zone_ref(&vecval);

										tree_manager_get_child_at(&myvec, 1, &vecval);
										tree_mamanger_get_node_byte(&vecval, 0, &vec4[1]);
										release_zone_ref(&vecval);

										tree_manager_get_child_at(&myvec, 2, &vecval);
										tree_mamanger_get_node_byte(&vecval, 0, &vec4[2]);
										release_zone_ref(&vecval);

										tree_manager_get_child_at(&myvec, 3, &vecval);
										tree_mamanger_get_node_byte(&vecval, 0, &vec4[3]);
										release_zone_ref(&vecval);


										if (ret)ret = tree_manager_set_child_value_4uc(&newObj, KeyName, vec4);
									}
									break;
									case NODE_RT_VEC2:
									{
										mem_zone_ref myvec = { PTR_NULL }, vecval = { PTR_NULL };
										float vecf[2];
										ret = tree_manager_find_child_node(&objData, NODE_HASH(KeyName), NODE_JSON_ARRAY, &myvec);

										tree_manager_get_child_at(&myvec, 0, &vecval);
										tree_mamanger_get_node_float(&vecval, 0, &vecf[0]);
										release_zone_ref(&vecval);

										tree_manager_get_child_at(&myvec, 1, &vecval);
										tree_mamanger_get_node_float(&vecval, 0, &vecf[1]);
										release_zone_ref(&vecval);

										release_zone_ref(&myvec);

										if (ret)ret = tree_manager_set_child_value_vec2(&newObj, KeyName, vecf[0], vecf[1]);
									}
									break;

									case NODE_RT_VEC2_ARRAY:
									{
										mem_zone_ref	myvecs = { PTR_NULL }, objvecs = { PTR_NULL };

										unsigned int	n, nvecs;
										ret = tree_manager_find_child_node(&objData, NODE_HASH(KeyName), NODE_JSON_ARRAY, &myvecs);

										if (ret)ret = tree_manager_add_child_node(&newObj, KeyName, NODE_RT_VEC2_ARRAY, &objvecs);

										nvecs = tree_manager_get_node_num_children(&myvecs) / 2;

										tree_manager_allocate_node_data	(&objvecs, nvecs * sizeof(float) * 2);
										tree_manager_set_child_value_i64(&objvecs, "nVtx", nvecs);

										for (n = 0; n < nvecs; n++)
										{
											float			vecf[2];
											mem_zone_ref myvec = { PTR_NULL };

											tree_manager_get_child_at(&myvecs, n * 2 +0 , &myvec);
											tree_mamanger_get_node_float(&myvec, 0, &vecf[0]);
											release_zone_ref(&myvec);

											tree_manager_get_child_at(&myvecs, n * 2 + 1, &myvec);
											tree_mamanger_get_node_float(&myvec, 0, &vecf[1]);
											release_zone_ref(&myvec);

											if (ret)ret = tree_manager_write_node_vec2f(&objvecs, n * sizeof(float) * 2, vecf[0], vecf[1]);
										}
										release_zone_ref(&objvecs);
										release_zone_ref(&myvecs);
									}
									break;
									case NODE_RT_VEC3:
									{
										mem_zone_ref myvec = { PTR_NULL }, vecval = { PTR_NULL };
										float vecf[3];
										ret = tree_manager_find_child_node(&objData, NODE_HASH(KeyName), NODE_JSON_ARRAY, &myvec);

										tree_manager_get_child_at(&myvec, 0, &vecval);
										tree_mamanger_get_node_float(&vecval, 0, &vecf[0]);
										release_zone_ref(&vecval);

										tree_manager_get_child_at(&myvec, 1, &vecval);
										tree_mamanger_get_node_float(&vecval, 0, &vecf[1]);
										release_zone_ref(&vecval);

										tree_manager_get_child_at(&myvec, 2, &vecval);
										tree_mamanger_get_node_float(&vecval, 0, &vecf[2]);
										release_zone_ref(&vecval);

										release_zone_ref(&myvec);


										if (ret)ret = tree_manager_set_child_value_vec3(&newObj, KeyName, vecf[0], vecf[1], vecf[2]);
									}
									break;

									case NODE_RT_VEC3_ARRAY:
									{
										mem_zone_ref	myvecs = { PTR_NULL }, objvecs = { PTR_NULL };

										unsigned int	n, nvecs;
										ret = tree_manager_find_child_node(&objData, NODE_HASH(KeyName), NODE_JSON_ARRAY, &myvecs);

										if (ret)ret = tree_manager_add_child_node(&newObj, KeyName, NODE_RT_VEC3_ARRAY, &objvecs);

										nvecs = tree_manager_get_node_num_children(&myvecs) /3 ;

										tree_manager_allocate_node_data(&objvecs, nvecs * sizeof(float) * 4);
										tree_manager_set_child_value_i64(&objvecs, "nVtx", nvecs);

										for (n = 0; n < nvecs; n++)
										{
											float			vecf[3];
											mem_zone_ref myvec = { PTR_NULL };

											tree_manager_get_child_at(&myvecs, n * 3 + 0, &myvec);
											tree_mamanger_get_node_float(&myvec, 0, &vecf[0]);
											release_zone_ref(&myvec);

											tree_manager_get_child_at(&myvecs, n * 3 + 1, &myvec);
											tree_mamanger_get_node_float(&myvec, 0, &vecf[1]);
											release_zone_ref(&myvec);

											tree_manager_get_child_at(&myvecs, n * 3 + 2, &myvec);
											tree_mamanger_get_node_float(&myvec, 0, &vecf[2]);
											release_zone_ref(&myvec);

											if (ret)ret = tree_manager_write_node_vec3f(&objvecs, n * sizeof(float) * 4, vecf[0], vecf[1], vecf[2]);
										}
										release_zone_ref(&objvecs);
										release_zone_ref(&myvecs);
									}
									break;
									case NODE_RT_INT_ARRAY:
									{
										mem_zone_ref	myvecs = { PTR_NULL }, objvecs = { PTR_NULL };

										unsigned int	n, nvecs;
										ret = tree_manager_find_child_node(&objData, NODE_HASH(KeyName), NODE_JSON_ARRAY, &myvecs);

										if (ret)ret = tree_manager_add_child_node(&newObj, KeyName, NODE_RT_INT_ARRAY, &objvecs);

										nvecs = tree_manager_get_node_num_children(&myvecs);

										tree_manager_allocate_node_data(&objvecs, nvecs * sizeof(unsigned int));
										tree_manager_set_child_value_i64(&objvecs, "nVtx", nvecs);

										for (n = 0; n < nvecs; n++)
										{
											unsigned int myval;
											mem_zone_ref myvec = { PTR_NULL };

											ret = tree_manager_get_child_at(&myvecs, n, &myvec);
											if(ret)ret= tree_mamanger_get_node_dword(&myvec, 0, &myval);
											release_zone_ref(&myvec);
											if (ret)ret = tree_manager_write_node_dword(&objvecs, n * sizeof(unsigned int), myval);
										}
										release_zone_ref(&objvecs);
										release_zone_ref(&myvecs);
									}
									break;
								}
							}
						}
						free_string(&KeyStr);
						if (!ret)break;
					}
					release_zone_ref(&type_outs);
					release_zone_ref(&type);
				}
			}
			release_zone_ref(&app);
		}
	}
	release_zone_ref				(&objData);

	node_aquire_mempool_lock		(&mempool);

	if (ret)ret = check_app_obj_unique		(appName.str, typeId, &newObj);
	if (ret)ret = node_check_mempool_unique	(&mempool, appName.str, typeId, &newObj);
	if (ret)ret = make_app_item_tx			(&new_tx, &appName, 2);
	free_string								(&appName);

	if (!ret)
	{
		release_zone_ref			(&addrs);
		release_zone_ref			(&newObj);
		release_zone_ref			(&mempool);
		node_release_mempool_lock	();
		return 0;
	}

	get_tx_input						 (&new_tx, 0, &vin);
	tree_manager_set_child_value_btcaddr (&vin, "srcaddr", pubaddr);
	release_zone_ref					 (&vin);

	objLen = compute_payload_size		 (&newObj);
	buffer = malloc_c					 (objLen);
	write_node							 (&newObj, buffer);
	release_zone_ref					 (&newObj);


	tree_manager_create_node			("script", NODE_BITCORE_SCRIPT, &script_node);
	create_payment_script_data			(&bKey, 0, &script_node,buffer,objLen);
	free_c								(buffer);

	free_string							(&bKey);

	serialize_script					(&script_node, &oScript);
	release_zone_ref					(&script_node);

	tx_add_output						(&new_tx, 0xFFFFFFFF00000000|typeId, &oScript);
	free_string							(&oScript);

	if (paytxfee>0)
	{
		memset_c(changeAddr, 0, sizeof(btc_addr_t));
		total_unspent = 0;
		for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
		{
			btc_addr_t						my_addr;
			tree_manager_get_node_btcaddr(addr, 0, my_addr);
			get_tx_inputs_from_addr(my_addr, &mempool, &total_unspent, paytxfee, min_conf, max_conf, &new_tx);

			if (changeAddr[0] == 0)memcpy_c(changeAddr, my_addr, sizeof(btc_addr_t));
		}
		tree_manager_set_child_value_i64(result, "total", total_unspent);

		if (total_unspent<paytxfee)
		{
			release_zone_ref			(&addrs);
			release_zone_ref			(&new_tx);
			release_zone_ref			(&mempool);
			node_release_mempool_lock	();
			return 0;
		}

		if (tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node))
		{
			create_p2sh_script(changeAddr, &script_node);
			serialize_script(&script_node, &oScript);
			release_zone_ref(&script_node);
		}

		tx_add_output	(&new_tx, total_unspent - paytxfee, &oScript);
		free_string		(&oScript);
	}

	release_zone_ref			(&mempool);
	node_release_mempool_lock	();

	if (tree_manager_find_child_node(&new_tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &inList))
	{
		mem_zone_ref_ptr	input = PTR_NULL;

		for (nin = 0, tree_manager_get_first_child(&inList, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input), nin++)
		{
			hash_t				txh, hh;
			btc_addr_t			src_addr;
			struct string		script = { PTR_NULL };
			uint64_t			inAmount;
			unsigned int		cnt;


			if (nin != 0)
			{
				hash_t				h;
				unsigned int		oIdx;

				tree_manager_get_child_value_hash(input, NODE_HASH("txid"), h);
				tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &oIdx);
				get_tx_output_script(h, oIdx, &script, &inAmount);

				get_out_script_address				(&script, PTR_NULL, src_addr);
				tree_manager_set_child_value_btcaddr(input, "srcaddr", src_addr);
			}
			else
			{
				mem_zone_ref out = { PTR_NULL };
				get_tx_output		(&new_tx, 0, &out);

				tree_manager_get_child_value_istr(&out, NODE_HASH("script"), &script, 0);

				release_zone_ref(&out);
			}

			tree_manager_set_child_value_i32(input, "index", nin);
			tree_manager_set_child_value_i64(input, "value", inAmount);

			compute_tx_sign_hash(&new_tx, nin, &script, 1, txh);
			free_string			(&script);

			cnt = 32;
			while (cnt--)
				hh[31 - cnt] = txh[cnt];

			tree_manager_set_child_value_hash(input, "signHash", hh);
		}
		release_zone_ref(&inList);
	}

	compute_tx_hash(&new_tx, txh);
	tree_manager_set_child_value_bhash	(&new_tx, "txid", txh);

	tree_manager_node_add_child			(&tmp_txs, &new_tx);
	tree_manager_node_add_child			(result, &new_tx);
	release_zone_ref					(&addrs);
	release_zone_ref					(&new_tx);

	return 1;
}


OS_API_C_FUNC(int) makeappfiletx(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t				 txh,binHash, rbinHash;
	btc_addr_t			changeAddr;
	mem_zone_ref		tmp = { PTR_NULL }, pn = { PTR_NULL }, script_node = { PTR_NULL }, new_tx = { PTR_NULL }, addrs = { PTR_NULL }, my_list = { PTR_NULL }, mempool = { PTR_NULL }, node_files = { PTR_NULL }, file = { PTR_NULL }, inList = { PTR_NULL };
	mem_zone_ref_ptr	addr = PTR_NULL;
	struct	string		bpubkey = { 0 }, bsign = { 0 }, oScript = { 0 }, fileKey = { 0 }, fileSign = { 0 }, fileHash = { 0 }, appName = { 0 };
	uint64_t			inFees, paytxfee, total_unspent;
	unsigned int		n;
	int					ret;
	unsigned char		hash_type = 1;



	ret = tree_manager_get_child_at(params, 0, &pn);
	if (ret)ret = tree_manager_get_node_istr(&pn, 0, &appName, 0);
	release_zone_ref(&pn);

	if (ret)ret = tree_manager_get_child_at(params, 1, &pn);
	if (ret)ret = tree_manager_get_node_istr(&pn, 0, &fileHash, 0);
	release_zone_ref(&pn);

	if (ret)ret = (fileHash.len == 64) ? 1 : 0;
	if (ret)
	{
		n = 0;
		while (n < 32)
		{
			char    hex[3];
			hex[0] = fileHash.str[(31 - n) * 2 + 0];
			hex[1] = fileHash.str[(31 - n) * 2 + 1];
			hex[2] = 0;
			binHash[n] = strtoul_c(hex, PTR_NULL, 16);
			rbinHash[31 - n] = binHash[n];
			n++;
		}
	}
	free_string(&fileHash);



	if (ret)ret = tree_manager_get_child_at(params, 2, &pn);
	if (ret)ret = tree_manager_get_node_istr(&pn, 0, &fileKey, 0);
	release_zone_ref(&pn);

	if (ret)ret = (fileKey.len == 66) ? 1 : 0;

	if (ret)
	{
		char			hex[3];
		unsigned char	*ppk;


		bpubkey.len = 33;
		bpubkey.size = bpubkey.len + 1;
		bpubkey.str = malloc_c(bpubkey.size);
		ppk = (unsigned char *)bpubkey.str;

		n = 0;
		while (n < bpubkey.len)
		{

			hex[0] = fileKey.str[n * 2 + 0];
			hex[1] = fileKey.str[n * 2 + 1];
			hex[2] = 0;
			ppk[n] = strtoul_c(hex, PTR_NULL, 16);
			n++;
		}
	}




	if (ret)ret = tree_manager_get_child_at(params, 3, &pn);
	if (ret)ret = tree_manager_get_node_istr(&pn, 0, &fileSign, 0);
	release_zone_ref(&pn);

	if (ret)
	{
		struct string tsign;
		unsigned char *pps;



		tsign.len = (fileSign.len / 2) + 1;
		tsign.size = tsign.len + 1;
		tsign.str = malloc_c(tsign.size);

		pps = (unsigned char *)tsign.str;

		n = 0;
		while (n < tsign.len)
		{
			char	hex[3];
			hex[0] = fileSign.str[n * 2 + 0];
			hex[1] = fileSign.str[n * 2 + 1];
			hex[2] = 0;
			pps[n] = strtoul_c(hex, PTR_NULL, 16);
			n++;
		}
		tsign.str[tsign.len - 1] = hash_type;


		parse_sig_seq(&tsign, &bsign, &hash_type, 1);
		free_string(&tsign);
	}

	if (ret)ret = tree_manager_find_child_node						(&my_node, NODE_HASH("tmp files"), NODE_JSON_ARRAY, &node_files);
	if (ret)ret = tree_find_child_node_by_member_name_hash			(&node_files, NODE_GFX_OBJECT, "dataHash", binHash, &file);
	if (ret)ret = blk_check_sign									(&bsign, &bpubkey, rbinHash);
	release_zone_ref												(&node_files);

	/*
	if (ret)ret = tree_aquire_node_child_lock(&my_node, NODE_HASH("tmp files"), &node_files);
	if (ret)
	{
		ret = tree_find_child_node_by_member_name_hash	(&node_files, NODE_GFX_OBJECT, "dataHash", binHash, &file);
		if (ret)ret = blk_check_sign					(&bsign, &bpubkey, rbinHash);
		tree_release_node_child_lock					(&my_node, NODE_HASH("tmp files"));
	}
	*/
		
		
	


	free_string(&fileSign);
	free_string(&fileKey);

	if (ret)ret = tree_manager_get_child_at(params, 4, &addrs);

	if (ret)
	{
		get_apps(&tmp);
		ret = tree_manager_find_child_node(&tmp, NODE_HASH(appName.str), NODE_BITCORE_TX, PTR_NULL);
		release_zone_ref(&tmp);
	}

	if (!ret)
	{
		free_string(&bpubkey);
		free_string(&bsign);
		release_zone_ref(&addrs);
		release_zone_ref(&file);
		free_string(&appName);
		return 0;
	}

	if (tree_manager_get_child_at(params, 5, &pn))
	{
		tree_mamanger_get_node_qword(&pn, 0, &inFees);
		release_zone_ref(&pn);
	}
	else
		inFees = 0;

	if (tree_manager_get_child_value_i64(&my_node, NODE_HASH("paytxfee"), &paytxfee))
	{
		if (inFees > paytxfee)
			paytxfee = inFees;
	}
	else if (inFees > 0)
		paytxfee = inFees;
	else
		paytxfee = 0;


	if (ret)
	{
		btc_addr_t	addr;
		mem_zone_ref vout = { PTR_NULL };
		ret = make_app_item_tx(&new_tx, &appName, 3);

		tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node);
		make_script_file(&file, &bpubkey, &bsign, &script_node);

		serialize_script(&script_node, &oScript);
		release_zone_ref(&script_node);

		tx_add_output(&new_tx, 0xFFFFFFFFFFFFFFFF, &oScript);

		get_tx_output(&new_tx, 0, &vout);
		key_to_addr(bpubkey.str, addr);
		tree_manager_set_child_value_btcaddr(&vout, "dstaddr", addr);

		release_zone_ref(&vout);

		free_string(&oScript);
		release_zone_ref(&file);
	}
	free_string(&appName);
	free_string(&bpubkey);
	free_string(&bsign);
	release_zone_ref(&file);

	if (!ret)
	{
		release_zone_ref(&new_tx);
		release_zone_ref(&addrs);
		return 0;
	}

	if (paytxfee > 0)
	{

		node_aquire_mempool_lock(&mempool);
		//tree_manager_find_child_node(&my_node, NODE_HASH("mempool"), NODE_BITCORE_TX_LIST, &mempool);

		memset_c(changeAddr, 0, sizeof(btc_addr_t));
		total_unspent = 0;
		for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
		{
			btc_addr_t						my_addr;
			tree_manager_get_node_btcaddr(addr, 0, my_addr);
			get_tx_inputs_from_addr(my_addr, &mempool, &total_unspent, paytxfee, 10, 9999, &new_tx);

			if (changeAddr[0] == 0)memcpy_c(changeAddr, my_addr, sizeof(btc_addr_t));
		}
		tree_manager_set_child_value_i64(result, "total", total_unspent);

		release_zone_ref(&mempool);

		node_release_mempool_lock();


		if (total_unspent < paytxfee)
		{
			release_zone_ref(&addrs);
			release_zone_ref(&new_tx);
			return 0;
		}

		if (tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node))
		{
			create_p2sh_script(changeAddr, &script_node);
			serialize_script(&script_node, &oScript);
			release_zone_ref(&script_node);
		}

		tx_add_output(&new_tx, total_unspent - paytxfee, &oScript);
		free_string(&oScript);
	}

	if (tree_manager_find_child_node(&new_tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &inList))
	{

		mem_zone_ref_ptr	input = PTR_NULL;
		unsigned int		nin;

		for (nin = 0, tree_manager_get_first_child(&inList, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input), nin++)
		{
			hash_t				txh, hh;
			hash_t				h;
			btc_addr_t			src_addr;
			struct string		script = { PTR_NULL }, appname = { PTR_NULL };
			uint64_t			inAmount;
			unsigned int		oIdx, cnt;

			tree_manager_get_child_value_hash(input, NODE_HASH("txid"), h);
			tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &oIdx);

			get_tx_output_script(h, oIdx, &script, &inAmount);

			if (!tree_manager_get_child_value_istr(input, NODE_HASH("srcapp"), &appname, 0))
			{


				get_out_script_address(&script, PTR_NULL, src_addr);
				tree_manager_set_child_value_btcaddr(input, "srcaddr", src_addr);
				tree_manager_set_child_value_i64(input, "value", inAmount);
			}
			else
			{
				tree_manager_set_child_value_i64(input, "value", 0);
				free_string(&appname);
			}

			tree_manager_set_child_value_i32(input, "index", nin);
			compute_tx_sign_hash(&new_tx, nin, &script, 1, txh);
			free_string(&script);

			cnt = 32;
			while (cnt--)
				hh[31 - cnt] = txh[cnt];

			free_string(&appname);

			tree_manager_set_child_value_hash(input, "signHash", hh);
		}
		release_zone_ref(&inList);
	}

	compute_tx_hash(&new_tx, txh);
	tree_manager_set_child_value_bhash(&new_tx, "txid", txh);


	tree_manager_node_add_child	(&tmp_txs, &new_tx);
	tree_manager_node_add_child	(result, &new_tx);

	release_zone_ref(&addrs);
	release_zone_ref(&new_tx);

	return ret;

}


OS_API_C_FUNC(int) makeapplayouttx(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t				txh,binHash, rbinHash;
	btc_addr_t			changeAddr;
	mem_zone_ref		tmp = { PTR_NULL }, pn = { PTR_NULL }, script_node = { PTR_NULL }, new_tx = { PTR_NULL }, addrs = { PTR_NULL }, my_list = { PTR_NULL }, mempool = { PTR_NULL }, node_files = { PTR_NULL }, file = { PTR_NULL }, inList = { PTR_NULL };
	mem_zone_ref_ptr	addr = PTR_NULL;
	struct	string		oScript = { 0 }, fileHash = { 0 }, appName = { 0 };
	uint64_t			inFees, paytxfee, total_unspent;
	unsigned int		n;
	int					ret;
	unsigned char		hash_type = 1;

	//if (!tree_manager_find_child_node(&my_node, NODE_HASH("tmp files"), NODE_JSON_ARRAY, &node_files))return 0;

	ret = tree_manager_get_child_at(params, 0, &pn);
	if (ret)ret = tree_manager_get_node_istr(&pn, 0, &appName, 0);
	release_zone_ref(&pn);

	if (ret)ret = tree_manager_get_child_at(params, 1, &pn);
	if (ret)ret = tree_manager_get_node_istr(&pn, 0, &fileHash, 0);
	release_zone_ref(&pn);

	if (ret)ret = (fileHash.len == 64) ? 1 : 0;
	if (ret)
	{
		n = 0;
		while (n < 32)
		{
			char    hex[3];
			hex[0] = fileHash.str[(31 - n) * 2 + 0];
			hex[1] = fileHash.str[(31 - n) * 2 + 1];
			hex[2] = 0;
			binHash[n] = strtoul_c(hex, PTR_NULL, 16);
			rbinHash[31 - n] = binHash[n];
			n++;
		}
	}
	free_string(&fileHash);

	if (ret)ret = tree_manager_get_child_at(params, 2, &addrs);

	if (ret)
	{
		get_apps(&tmp);
		ret = tree_manager_find_child_node(&tmp, NODE_HASH(appName.str), NODE_BITCORE_TX, PTR_NULL);
		release_zone_ref(&tmp);
	}

	if (!ret)
	{
		release_zone_ref(&addrs);
		release_zone_ref(&file);
		free_string		(&appName);
		return 0;
	}

	if (tree_manager_get_child_at(params, 3, &pn))
	{
		tree_mamanger_get_node_qword(&pn, 0, &inFees);
		release_zone_ref(&pn);
	}
	else
		inFees = 0;

	if (tree_manager_get_child_value_i64(&my_node, NODE_HASH("paytxfee"), &paytxfee))
	{
		if (inFees > paytxfee)
			paytxfee = inFees;
	}
	else if (inFees > 0)
		paytxfee = inFees;
	else
		paytxfee = 0;

	//if (ret)ret = tree_aquire_node_child_lock(&my_node, NODE_HASH("tmp files"), &node_files);


	if (ret)ret = tree_manager_find_child_node						(&my_node, NODE_HASH("tmp files"), NODE_JSON_ARRAY, &node_files);
	if (ret)ret = tree_find_child_node_by_member_name_hash			(&node_files, NODE_GFX_OBJECT, "dataHash", binHash, &file);
	release_zone_ref												(&node_files);

	//tree_release_node_child_lock							(&my_node, NODE_HASH("tmp files"));


	if(ret)ret = make_app_item_tx			(&new_tx, &appName, 4);
	if(ret)ret = tree_manager_create_node	("script", NODE_BITCORE_SCRIPT, &script_node);
	if(ret)ret = make_script_layout			(&file, &script_node);

	if(ret)ret	= serialize_script	(&script_node, &oScript);
	if (ret)ret = tx_add_output(&new_tx, 0xFFFFFFFFFFFFFFFF, &oScript);

	free_string				(&oScript);
	release_zone_ref		(&file);
	release_zone_ref		(&script_node);
	
	free_string				(&appName);
	release_zone_ref		(&file);

	if (!ret)
	{
		release_zone_ref(&new_tx);
		release_zone_ref(&addrs);
		return 0;
	}

	//tree_manager_find_child_node(&my_node, NODE_HASH("mempool"), NODE_BITCORE_TX_LIST, &mempool);
	node_aquire_mempool_lock(&mempool);

	memset_c(changeAddr, 0, sizeof(btc_addr_t));
	total_unspent = 0;
	for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
	{
		btc_addr_t						my_addr;
		tree_manager_get_node_btcaddr(addr, 0, my_addr);
		get_tx_inputs_from_addr(my_addr, &mempool, &total_unspent, paytxfee, 10, 9999, &new_tx);

		if (changeAddr[0] == 0)memcpy_c(changeAddr, my_addr, sizeof(btc_addr_t));
	}
	tree_manager_set_child_value_i64(result, "total", total_unspent);

	release_zone_ref(&mempool);

	node_release_mempool_lock();

	if (total_unspent<paytxfee)
	{
		release_zone_ref(&addrs);
		release_zone_ref(&new_tx);
		return 0;
	}

	if (tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node))
	{
		create_p2sh_script(changeAddr, &script_node);
		serialize_script(&script_node, &oScript);
		release_zone_ref(&script_node);
	}

	tx_add_output	(&new_tx, total_unspent - paytxfee, &oScript);
	free_string		(&oScript);
	
	if (tree_manager_find_child_node(&new_tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &inList))
	{

		mem_zone_ref_ptr	input = PTR_NULL;
		unsigned int		nin;

		for (nin = 0, tree_manager_get_first_child(&inList, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input), nin++)
		{
			hash_t				txh, hh;
			hash_t				h;
			btc_addr_t			src_addr;
			struct string		script = { PTR_NULL }, appname = { PTR_NULL };
			uint64_t			inAmount;
			unsigned int		oIdx, cnt;

			tree_manager_get_child_value_hash(input, NODE_HASH("txid"), h);
			tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &oIdx);

			get_tx_output_script(h, oIdx, &script, &inAmount);

			get_out_script_address					(&script, PTR_NULL, src_addr);
			tree_manager_set_child_value_btcaddr	(input, "srcaddr", src_addr);

			if (!tree_manager_get_child_value_istr(input, NODE_HASH("srcapp"), &appname, 0))
				tree_manager_set_child_value_i64(input, "value", inAmount);
			else
				tree_manager_set_child_value_i64(input, "value", 0);

			free_string(&appname);

			tree_manager_set_child_value_i32(input, "index", nin);
			compute_tx_sign_hash(&new_tx, nin, &script, 1, txh);
			free_string(&script);

			cnt = 32;
			while (cnt--)
				hh[31 - cnt] = txh[cnt];

			free_string(&appname);

			tree_manager_set_child_value_hash(input, "signHash", hh);
		}
		release_zone_ref(&inList);
	}

	compute_tx_hash(&new_tx, txh);
	tree_manager_set_child_value_bhash(&new_tx, "txid", txh);


	tree_manager_node_add_child	(&tmp_txs, &new_tx);
	tree_manager_node_add_child	(result, &new_tx);

	release_zone_ref(&addrs);
	release_zone_ref(&new_tx);

	return ret;

}

OS_API_C_FUNC(int) makeappmoduletx(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t				txh;
	hash_t				binHash, rbinHash;
	btc_addr_t			changeAddr;
	mem_zone_ref		tmp = { PTR_NULL }, pn = { PTR_NULL }, script_node = { PTR_NULL }, new_tx = { PTR_NULL }, addrs = { PTR_NULL }, my_list = { PTR_NULL }, mempool = { PTR_NULL }, node_files = { PTR_NULL }, file = { PTR_NULL }, inList = { PTR_NULL };
	mem_zone_ref_ptr	addr = PTR_NULL;
	struct	string		oScript = { 0 }, fileHash = { 0 }, appName = { 0 };
	uint64_t			inFees, paytxfee, total_unspent;
	unsigned int		n;
	int					ret;
	unsigned char		hash_type = 1;

	//if (!tree_manager_find_child_node(&my_node, NODE_HASH("tmp files"), NODE_JSON_ARRAY, &node_files))return 0;

	ret = tree_manager_get_child_at(params, 0, &pn);
	if (ret)ret = tree_manager_get_node_istr(&pn, 0, &appName, 0);
	release_zone_ref(&pn);

	if (ret)ret = tree_manager_get_child_at(params, 1, &pn);
	if (ret)ret = tree_manager_get_node_istr(&pn, 0, &fileHash, 0);
	release_zone_ref(&pn);

	if (ret)ret = (fileHash.len == 64) ? 1 : 0;
	if (ret)
	{
		n = 0;
		while (n < 32)
		{
			char    hex[3];
			hex[0] = fileHash.str[(31 - n) * 2 + 0];
			hex[1] = fileHash.str[(31 - n) * 2 + 1];
			hex[2] = 0;
			binHash[n] = strtoul_c(hex, PTR_NULL, 16);
			rbinHash[31 - n] = binHash[n];
			n++;
		}
	}
	free_string(&fileHash);

	if (ret)ret = tree_manager_get_child_at(params, 2, &addrs);

	if (ret)
	{
		get_apps(&tmp);
		ret = tree_manager_find_child_node(&tmp, NODE_HASH(appName.str), NODE_BITCORE_TX, PTR_NULL);
		release_zone_ref(&tmp);
	}

	if (!ret)
	{
		release_zone_ref(&addrs);
		release_zone_ref(&file);
		free_string(&appName);
		return 0;
	}

	if (tree_manager_get_child_at(params, 3, &pn))
	{
		tree_mamanger_get_node_qword(&pn, 0, &inFees);
		release_zone_ref(&pn);
	}
	else
		inFees = 0;

	if (tree_manager_get_child_value_i64(&my_node, NODE_HASH("paytxfee"), &paytxfee))
	{
		if (inFees > paytxfee)
			paytxfee = inFees;
	}
	else if (inFees > 0)
		paytxfee = inFees;
	else
		paytxfee = 0;


	if (ret)ret = tree_manager_find_child_node						(&my_node, NODE_HASH("tmp files"), NODE_JSON_ARRAY, &node_files);
	if (ret)ret = tree_find_child_node_by_member_name_hash			(&node_files, NODE_GFX_OBJECT, "dataHash", binHash, &file);
	release_zone_ref												(&node_files);

	/*
	if (ret)ret = tree_aquire_node_child_lock(&my_node, NODE_HASH("tmp files"), &node_files);
	if (ret)
	{
		ret = tree_find_child_node_by_member_name_hash	(&node_files, NODE_GFX_OBJECT, "dataHash", binHash, &file);
		tree_release_node_child_lock					(&my_node, NODE_HASH("tmp files"));
	}
	*/
	if (ret)ret = make_app_item_tx(&new_tx, &appName, 5);
	if (ret)ret = tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node);
	if (ret)ret = make_script_module(&file, &script_node);
	if (ret)ret = serialize_script(&script_node, &oScript);
	if (ret)ret = tx_add_output(&new_tx, 0xFFFFFFFFFFFFFFFF, &oScript);

	free_string(&oScript);
	release_zone_ref(&file);
	release_zone_ref(&script_node);

	free_string(&appName);
	release_zone_ref(&file);

	if (!ret)
	{
		release_zone_ref(&new_tx);
		release_zone_ref(&addrs);
		return 0;
	}

	//tree_manager_find_child_node(&my_node, NODE_HASH("mempool"), NODE_BITCORE_TX_LIST, &mempool);
	node_aquire_mempool_lock(&mempool);

	memset_c(changeAddr, 0, sizeof(btc_addr_t));
	total_unspent = 0;
	for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
	{
		btc_addr_t						my_addr;
		tree_manager_get_node_btcaddr(addr, 0, my_addr);
		get_tx_inputs_from_addr(my_addr, &mempool, &total_unspent, paytxfee, 10, 9999, &new_tx);

		if (changeAddr[0] == 0)memcpy_c(changeAddr, my_addr, sizeof(btc_addr_t));
	}
	tree_manager_set_child_value_i64(result, "total", total_unspent);

	release_zone_ref(&mempool);

	node_release_mempool_lock();

	if (total_unspent<paytxfee)
	{
		release_zone_ref(&addrs);
		release_zone_ref(&new_tx);
		return 0;
	}

	if (tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node))
	{
		create_p2sh_script(changeAddr, &script_node);
		serialize_script(&script_node, &oScript);
		release_zone_ref(&script_node);
	}

	tx_add_output(&new_tx, total_unspent - paytxfee, &oScript);
	free_string(&oScript);

	if (tree_manager_find_child_node(&new_tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &inList))
	{

		mem_zone_ref_ptr	input = PTR_NULL;
		unsigned int		nin;

		for (nin = 0, tree_manager_get_first_child(&inList, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input), nin++)
		{
			hash_t				txh, hh;
			hash_t				h;
			btc_addr_t			src_addr;
			struct string		script = { PTR_NULL }, appname = { PTR_NULL };
			uint64_t			inAmount;
			unsigned int		oIdx, cnt;

			tree_manager_get_child_value_hash(input, NODE_HASH("txid"), h);
			tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &oIdx);

			get_tx_output_script(h, oIdx, &script, &inAmount);

			get_out_script_address(&script, PTR_NULL, src_addr);
			tree_manager_set_child_value_btcaddr(input, "srcaddr", src_addr);

			if (!tree_manager_get_child_value_istr(input, NODE_HASH("srcapp"), &appname, 0))
				tree_manager_set_child_value_i64(input, "value", inAmount);
			else
				tree_manager_set_child_value_i64(input, "value", 0);

			free_string(&appname);

			tree_manager_set_child_value_i32(input, "index", nin);
			compute_tx_sign_hash(&new_tx, nin, &script, 1, txh);
			free_string(&script);

			cnt = 32;
			while (cnt--)
				hh[31 - cnt] = txh[cnt];

			free_string(&appname);

			tree_manager_set_child_value_hash(input, "signHash", hh);
		}
		release_zone_ref(&inList);
	}
	
	compute_tx_hash						(&new_tx, txh);
	tree_manager_set_child_value_bhash	(&new_tx, "txid", txh);


	tree_manager_node_add_child(&tmp_txs, &new_tx);
	tree_manager_node_add_child(result, &new_tx);

	release_zone_ref(&addrs);
	release_zone_ref(&new_tx);

	return ret;

}

OS_API_C_FUNC(int) addchildobj(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t				txh,ph, ch;
	btc_addr_t			objAddr;

	char				KeyName[32];
	char				pHash[65], cHash[65], Hash[65];
	struct string		appName = { 0 };
	mem_zone_ref		obj = { PTR_NULL }, new_tx = { PTR_NULL }, myobj = { PTR_NULL }, pn = { PTR_NULL }, addrs = { PTR_NULL };
	uint64_t			total_unspent, inFees, paytxfee;
	unsigned int		ktype, objType, flags;
	int					ret,n;

	if (!tree_manager_get_child_at(params, 1, &pn))return 0;
	ret = tree_manager_get_node_str(&pn, 0, Hash, 65, 0);
	release_zone_ref(&pn);
	if (!ret)return 0;

	n = 0;
	while (n < 32)
	{
		pHash[n * 2 + 0] = Hash[(31 - n) * 2 + 0];
		pHash[n * 2 + 1] = Hash[(31 - n) * 2 + 1];
		n++;
	}
	pHash[64] = 0;

	if (!tree_manager_get_child_at(params, 2, &pn))return 0;
	ret = tree_manager_get_node_str(&pn, 0, KeyName, 32, 0);
	release_zone_ref(&pn);
	if (!ret)return 0;

	if (!tree_manager_get_child_at(params, 3, &pn))return 0;
	ret = tree_manager_get_node_str(&pn, 0, Hash, 65, 0);
	release_zone_ref(&pn);
	if (!ret)return 0;

	n = 0;
	while (n < 32)
	{
		cHash[n * 2 + 0] = Hash[(31 - n) * 2 + 0];
		cHash[n * 2 + 1] = Hash[(31 - n) * 2 + 1];
		n++;
	}
	cHash[64] = 0;

	if (!tree_manager_get_child_at(params, 4, &addrs))return 0;


	if (tree_manager_get_child_at(params, 5, &pn))
	{
		tree_mamanger_get_node_qword(&pn, 0, &inFees);
		release_zone_ref(&pn);
	}
	else
		inFees = 0;

	if (tree_manager_get_child_value_i64(&my_node, NODE_HASH("paytxfee"), &paytxfee))
	{
		if (inFees > paytxfee)
			paytxfee = inFees;
	}
	else if (inFees > 0)
		paytxfee = inFees;
	else
		paytxfee = 0;

	if (!tree_manager_get_child_at(params, 0, &pn))return 0;
	ret = tree_manager_get_node_istr(&pn, 0, &appName, 0);
	release_zone_ref(&pn);


	if (ret)
	{
		hex_2_bin(pHash, ph, 32);

		ret = node_mempool_getobj(appName.str, ph, &obj);
		if (ret) {
			objType = tree_mamanger_get_node_type(&obj);
			ret = tree_manager_get_child_value_btcaddr(&obj, NODE_HASH("objAddr"), objAddr);
		}
		if(!ret)ret = load_obj_type(appName.str, pHash, &objType, objAddr);
	}
		
	if (ret)ret = get_app_type_key	(appName.str, objType, KeyName, &ktype,&flags);
	if (ret)ret = ((ktype == NODE_JSON_ARRAY) || (ktype == NODE_PUBCHILDS_ARRAY)) ? 1 : 0;
	/*
	if (ret)ret = load_obj			(appName.str, cHash, "obj", 0, &myobj,PTR_NULL);
	release_zone_ref				(&myobj);
	*/

	if (ret)
	{
		hex_2_bin(cHash, ch, 32);
		ret = make_app_child_obj_tx(&new_tx, appName.str, ph, objType, objAddr,KeyName, ktype, ch);
	}

	
	if (ret)
	{
		btc_addr_t			changeAddr;
		mem_zone_ref		my_list = { PTR_NULL }, mempool = { PTR_NULL }, objtx = { PTR_NULL };
		mem_zone_ref_ptr	addr = PTR_NULL;
		uint64_t			min_conf, max_conf;

		min_conf = 10;
		max_conf = 99999;

		node_aquire_mempool_lock(&mempool);
		//tree_manager_find_child_node(&my_node, NODE_HASH("mempool"), NODE_BITCORE_TX_LIST, &mempool);
		
		if (obj.zone != PTR_NULL)
			tree_find_child_node_by_member_name_hash(&mempool, NODE_BITCORE_TX, "txid", ph, &objtx);

		memset_c(changeAddr, 0, sizeof(btc_addr_t));
		total_unspent = 0;
		for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
		{
			btc_addr_t						my_addr;
			tree_manager_get_node_btcaddr(addr, 0, my_addr);
			get_tx_inputs_from_addr(my_addr, &mempool, &total_unspent, paytxfee, min_conf, max_conf, &new_tx);

			if (changeAddr[0] == 0)memcpy_c(changeAddr, my_addr, sizeof(btc_addr_t));
		}
		tree_manager_set_child_value_i64	(result, "total", total_unspent);
		release_zone_ref					(&mempool);

		node_release_mempool_lock();

		ret = (total_unspent >= paytxfee) ? 1 : 0;


		if (ret)
		{
			struct string oScript = { 0 };
			mem_zone_ref script_node = { PTR_NULL }, inList = { PTR_NULL };
			if (tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node))
			{
				create_p2sh_script(changeAddr, &script_node);
				serialize_script(&script_node, &oScript);
				release_zone_ref(&script_node);
			}

			tx_add_output	(&new_tx, total_unspent - paytxfee, &oScript);
			free_string		(&oScript);


			if (tree_manager_find_child_node(&new_tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &inList))
			{
				mem_zone_ref_ptr	input = PTR_NULL;
				unsigned int		nin = 0;

				for (nin = 0, tree_manager_get_first_child(&inList, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input), nin++)
				{
					hash_t				txh, hh;
					hash_t				h;
					btc_addr_t			src_addr;
					struct string		script = { PTR_NULL };
					uint64_t			inAmount;
					unsigned int		oIdx, cnt;

					

					tree_manager_get_child_value_hash	(input, NODE_HASH("txid"), h);
					tree_manager_get_child_value_i32	(input, NODE_HASH("idx"), &oIdx);

					if (obj.zone == PTR_NULL)
						get_tx_output_script(h, oIdx, &script, &inAmount);
					else
					{
						mem_zone_ref vout = { PTR_NULL };

						if (!get_tx_output(&objtx, oIdx, &vout))continue;

						tree_manager_get_child_value_i64 (&vout, NODE_HASH("value"), &inAmount);
						tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &script,0);

						release_zone_ref(&vout);
					}


					get_out_script_address(&script, PTR_NULL, src_addr);

					if (nin > 0)
					{
						tree_manager_set_child_value_btcaddr(input, "srcaddr", src_addr);
						tree_manager_set_child_value_i64(input, "value", inAmount);
					}

					tree_manager_set_child_value_i32	(input, "index", nin);

					compute_tx_sign_hash				(&new_tx, nin, &script, 1, txh);
					free_string							(&script);

					cnt = 32;
					while (cnt--)hh[31 - cnt] = txh[cnt];

					tree_manager_set_child_value_hash(input, "signHash", hh);
				}
				release_zone_ref(&inList);
			}

			compute_tx_hash						(&new_tx, txh);
			tree_manager_set_child_value_bhash	(&new_tx, "txid", txh);
			tree_manager_node_add_child			(&tmp_txs, &new_tx);
		}

		release_zone_ref(&objtx);
	}
	if (ret)ret = tree_manager_node_add_child(result, &new_tx);
	
	release_zone_ref(&obj);
	
	release_zone_ref(&addrs);
	free_string		(&appName);
	release_zone_ref(&new_tx);


	return ret;
}

OS_API_C_FUNC(int) get_type_obj_list(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	mem_zone_ref	pn = { PTR_NULL }, objList = { PTR_NULL };
	struct string	app_name = { 0 };
	size_t			first, max;
	unsigned int	type_id;
	int				ret;

	if (!tree_manager_get_child_at(params, 0, &pn))return 0;
	tree_manager_get_node_istr(&pn, 0, &app_name, 0);
	release_zone_ref(&pn);

	if (!tree_manager_get_child_at(params, 1, &pn))return 0;
	tree_mamanger_get_node_dword(&pn, 0, &type_id);
	release_zone_ref(&pn);

	if (tree_manager_get_child_at(params, 2, &pn))
	{
		tree_mamanger_get_node_dword(&pn, 0, &first);
		release_zone_ref(&pn);
	}
	else
		first = 0;

	if (tree_manager_get_child_at(params, 3, &pn))
	{
		tree_mamanger_get_node_dword(&pn, 0, &max);
		release_zone_ref(&pn);
	}
	else
		max = 10;




	tree_manager_set_child_value_i32	(result, "typeId", type_id);
	ret = tree_manager_add_child_node	(result, "objs", NODE_JSON_ARRAY, &objList);
	get_app_type_obj_hashes				(app_name.str, type_id,first, max, &objList);

	release_zone_ref				(&objList);
	free_string						(&app_name);

	return ret;
}


OS_API_C_FUNC(int) find_objs(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	char			typeStr[32];
	mem_zone_ref	pn = { PTR_NULL }, objList = { PTR_NULL }, addrs = { PTR_NULL }, my_list = { PTR_NULL };
	struct string	app_name = { 0 };
	size_t			first, max;
	mem_zone_ref_ptr addr = PTR_NULL;
	unsigned int	type_id;
	int				ret;

	if (!tree_manager_get_child_at(params, 0, &pn))return 0;
	tree_manager_get_node_istr(&pn, 0, &app_name, 0);
	release_zone_ref(&pn);

	if (!tree_manager_get_child_at(params, 1, &pn))return 0;
	tree_mamanger_get_node_dword(&pn, 0, &type_id);
	release_zone_ref(&pn);

	if (!tree_manager_get_child_at(params, 2, &addrs))return 0;
	
	if (tree_manager_get_child_at(params, 2, &pn))
	{
		tree_mamanger_get_node_dword(&pn, 0, &first);
		release_zone_ref(&pn);
	}
	else
		first = 0;

	if (tree_manager_get_child_at(params, 3, &pn))
	{
		tree_mamanger_get_node_dword(&pn, 0, &max);
		release_zone_ref(&pn);
	}
	else
		max = 10;

	tree_manager_set_child_value_i32(result, "typeId", type_id);
	ret = tree_manager_add_child_node(result, "objs", NODE_JSON_ARRAY, &objList);

	uitoa_s(type_id, typeStr, 32, 16);


	for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
	{
		btc_addr_t						my_addr;
		if (!tree_manager_get_node_btcaddr(addr, 0, my_addr))continue;
		find_objs_by_addr				(app_name.str, typeStr, "addr", my_addr, &objList);
	}


	release_zone_ref(&addrs	);
	release_zone_ref(&objList);
	free_string(&app_name);

	return ret;
}

OS_API_C_FUNC(int) loadobj(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	char		   Hash[65],hash[65];
	struct string  app_name = { 0 };
	mem_zone_ref   pn = { PTR_NULL }, myobj = { PTR_NULL }, outobj = { PTR_NULL };
	unsigned int   flags;
	int			   ret,n;

	if (!tree_manager_get_child_at	(params, 1, &pn))return 0;
	tree_manager_get_node_str		(&pn, 0, Hash,65, 16);
	release_zone_ref				(&pn);

	n = 0;
	while (n < 32)
	{
		hash[n * 2 + 0] = Hash[(31 - n) * 2 + 0];
		hash[n * 2 + 1] = Hash[(31 - n) * 2 + 1];
		n++;
	}
	hash[64] = 0;


	if (!tree_manager_get_child_at	(params, 0, &pn))return 0;
	tree_manager_get_node_istr		(&pn, 0, &app_name, 0);
	release_zone_ref				(&pn);


	if (tree_manager_get_child_at(params, 2, &pn))
	{
		tree_mamanger_get_node_dword(&pn, 0, &flags);
		release_zone_ref(&pn);
	}
	else
		flags = 1|2|8;


	tree_manager_add_child_node		(result, "obj", NODE_GFX_OBJECT, &outobj);

	ret = load_obj					(app_name.str, hash, "obj", flags, &myobj,PTR_NULL);

	tree_manager_copy_children		(&outobj, &myobj,0xFFFFFFFF);
	

	release_zone_ref				(&outobj);
	release_zone_ref				(&myobj);
	free_string						(&app_name);

	return ret;
}

OS_API_C_FUNC(int) listobjtxfr(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t objH;
	char typeStr[32];
	struct string app_name = { 0 };
	mem_zone_ref pn = { PTR_NULL }, txfrs = { PTR_NULL };

	if (!tree_manager_get_child_at(params, 0, &pn))return 0;
	tree_manager_get_node_istr(&pn, 0, &app_name, 0);
	release_zone_ref(&pn);

	if (!tree_manager_get_child_at(params, 1, &pn))return 0;
	tree_manager_get_node_str(&pn, 0, typeStr, 32,16);
	release_zone_ref(&pn);

	if (!tree_manager_get_child_at(params, 2, &pn))return 0;
	tree_manager_get_node_hash(&pn, 0, objH);
	release_zone_ref(&pn);

	if (tree_manager_add_child_node(result, "txfrs", NODE_JSON_ARRAY, &txfrs))
	{
		list_obj_txfr(app_name.str, typeStr, objH, &txfrs);
		release_zone_ref(&txfrs);
	}


	return 1;

}

OS_API_C_FUNC(int) getappobjs(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	struct string app_name = { 0 };
	mem_zone_ref pn = { PTR_NULL }, list = { PTR_NULL };

	if (!tree_manager_get_child_at(params, 0, &pn))return 0;
	tree_manager_get_node_istr(&pn, 0, &app_name, 0);
	release_zone_ref(&pn);
	

	if (tree_manager_add_child_node(result, "objs", NODE_BITCORE_HASH_LIST, &list))
	{
		get_app_obj_hashes	(app_name.str, &list);
		release_zone_ref	(&list);
	}

	return 1;
}

OS_API_C_FUNC(int) getappfiles(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	struct string app_name = { 0 };
	mem_zone_ref pn = { PTR_NULL }, list = { PTR_NULL };
	size_t first, num;
	int nf;

	if (!tree_manager_get_child_at(params, 0, &pn))return 0;
	tree_manager_get_node_istr(&pn, 0, &app_name, 0);
	release_zone_ref(&pn);


	if (tree_manager_get_child_at(params, 1, &pn))
	{
		tree_mamanger_get_node_dword(&pn, 0, &first);
		release_zone_ref(&pn);
	}
	else
		first = 0;

	if (tree_manager_get_child_at(params, 2, &pn))
	{
		tree_mamanger_get_node_dword(&pn, 0, &num);
		release_zone_ref(&pn);
	}
	else
		num = 30;


	if (tree_manager_add_child_node(result, "files", NODE_JSON_ARRAY, &list))
	{
		nf=get_app_files	(&app_name,first,num, &list);
		release_zone_ref	(&list);
	}

	tree_manager_set_child_value_i32(result, "total", nf);

	free_string(&app_name);

	return 1;
}

OS_API_C_FUNC(int) getappfile(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t	 fileHash,bh;
	char	 file_hash[65];
	
	mem_zone_ref pn = { PTR_NULL }, file_tx = { PTR_NULL };
	int			 ret,n;

	if (!tree_manager_get_child_at(params, 0, &pn))  return 0;
	tree_manager_get_node_str(&pn, 0, file_hash, 65, 0);
	release_zone_ref(&pn);
		
	n = 32;
	while (n--)
	{
		char    hex[3];
		hex[0] = file_hash[(31 - n) * 2 + 0];
		hex[1] = file_hash[(31 - n) * 2 + 1];
		hex[2] = 0;
		fileHash[n] = strtoul_c(hex, PTR_NULL, 16);
	}
	


	ret = load_tx(&file_tx, bh, fileHash);
	if (ret)
	{
		struct string app_name = { 0 };
		mem_zone_ref file = { PTR_NULL };

		tree_manager_add_child_node	(result, "file", NODE_GFX_OBJECT, &file);
		ret = get_app_file			(&file_tx, &app_name, &file);
		
		if (ret)
		{
			hash_t fh;
			char fileh[65];
			struct string appPath = { 0 };

			tree_manager_set_child_value_hash(&file, "txid", fileHash);

			tree_manager_get_child_value_hash(&file, NODE_HASH("dataHash"), fh);

			bin_2_hex_r(fh, 32, fileh);

			make_string(&appPath, "/app/");
			cat_cstring(&appPath, app_name.str);
			cat_cstring(&appPath, "/file/");
			cat_cstring(&appPath, fileh);

			
			tree_manager_set_child_value_vstr(result, "appName", &app_name);
			tree_manager_set_child_value_vstr(result, "filePath", &appPath);

			free_string(&appPath);
		}
		release_zone_ref		(&file);
		free_string				(&app_name);
	}
	release_zone_ref(&file_tx);
	return ret;
}

OS_API_C_FUNC(int) listobjects(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	char				appName[32];
	mem_zone_ref		minconf = { PTR_NULL }, pn = { PTR_NULL }, maxconf = { PTR_NULL }, unspents = { PTR_NULL }, addrs = { PTR_NULL }, mempool = { PTR_NULL };
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref_ptr	addr;
	uint64_t			total = 0;
	unsigned int		appType;
	size_t				min_conf = 0, max_conf = 9999;
	size_t				max = 200, ntx = 0;

	if (!tree_manager_get_child_at(params, 0, &pn))return 0;
	tree_manager_get_node_str(&pn, 0, appName,32,0);
	release_zone_ref(&pn);

	if (!tree_manager_get_child_at(params, 1, &pn))return 0;
	tree_mamanger_get_node_dword(&pn, 0, &appType);
	release_zone_ref(&pn);

	if (tree_manager_get_child_at(params, 2, &minconf))
	{
		tree_mamanger_get_node_dword(&minconf, 0, &min_conf);
		release_zone_ref(&minconf);
	}
	if (tree_manager_get_child_at(params, 3, &maxconf))
	{
		tree_mamanger_get_node_dword(&maxconf, 0, &max_conf);
		release_zone_ref(&maxconf);
	}
	if (!tree_manager_get_child_at(params, 4, &addrs))
		return 0;

	if (!tree_manager_add_child_node(result, "objects", NODE_JSON_ARRAY, &unspents))
	{
		release_zone_ref(&addrs);
		return 0;
	}

	node_aquire_mempool_lock(&mempool);

	for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
	{
		btc_addr_t						my_addr;
		tree_manager_get_node_btcaddr(addr, 0, my_addr);
		list_obj(my_addr,appName,appType, &unspents, &mempool, min_conf, max_conf, &ntx, &max, 0);
	}
	release_zone_ref(&mempool);

	node_release_mempool_lock();

	tree_manager_set_child_value_i64(result, "ntx", ntx);
	

	release_zone_ref(&addrs);
	release_zone_ref(&unspents);


	return 1;
}

OS_API_C_FUNC(int) listunspent(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	mem_zone_ref		minconf = { PTR_NULL }, maxconf = { PTR_NULL }, unspents = { PTR_NULL }, addrs = { PTR_NULL }, mempool = { PTR_NULL };
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref_ptr	addr;
	uint64_t			total=0;
	size_t				min_conf = 0, max_conf = 9999;
	size_t				max = 200,ntx=0;


	if (tree_manager_get_child_at(params, 0, &minconf))
	{
		tree_mamanger_get_node_dword(&minconf, 0, &min_conf);
		release_zone_ref(&minconf);
	}
	if (tree_manager_get_child_at(params, 1, &maxconf))
	{
		tree_mamanger_get_node_dword(&maxconf, 0, &max_conf);
		release_zone_ref(&maxconf);
	}
	if (!tree_manager_get_child_at(params, 2, &addrs))
		return 0;

	if (!tree_manager_add_child_node(result, "unspents", NODE_JSON_ARRAY, &unspents))
	{
		release_zone_ref(&addrs);
		return 0;
	}

	node_aquire_mempool_lock(&mempool);

	for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
	{
		btc_addr_t						my_addr;
		tree_manager_get_node_btcaddr	(addr, 0, my_addr);
		list_unspent					(my_addr, &unspents, &mempool,min_conf, max_conf, &total, &ntx, &max,0);
	}
	release_zone_ref(&mempool);

	node_release_mempool_lock();

	tree_manager_set_child_value_i64(result, "ntx", ntx);
	tree_manager_set_child_value_i64(result, "total",total );

	
	release_zone_ref			(&addrs);
	release_zone_ref			(&unspents);


	return 1;
}

OS_API_C_FUNC(int) listreceivedbyaddress(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	mem_zone_ref	minconf = { PTR_NULL }, maxconf = { PTR_NULL },addr_list = { PTR_NULL };
	struct string	dir_list = { PTR_NULL };
	size_t			min_conf = 0, max_conf = 9999, ntx;
	size_t			cur, nfiles;


	if (!tree_manager_add_child_node(result, "addrs", NODE_JSON_ARRAY, &addr_list))
		return 0;
	
	if (tree_manager_get_child_at(params, 0, &minconf))
	{
		tree_mamanger_get_node_dword(&minconf, 0, &min_conf);
		release_zone_ref(&minconf);
	}
	else
		min_conf = 1;

	if (tree_manager_get_child_at(params, 1, &maxconf))
	{
		tree_mamanger_get_node_dword(&maxconf, 0, &max_conf);
		release_zone_ref(&maxconf);
	}
	else
		max_conf = 0;

	nfiles = get_sub_dirs("adrs", &dir_list);
	if (nfiles > 0)
	{
		const char		*ptr, *optr;
		unsigned int	dir_list_len;

		dir_list_len = dir_list.len;
		optr = dir_list.str;
		cur = 0;
		while (cur < nfiles)
		{
			mem_zone_ref	new_addr = { PTR_NULL };
			size_t			sz;

			ptr = memchr_c(optr, 10, dir_list_len);
			sz = mem_sub(optr, ptr);

			if (tree_manager_add_child_node(&addr_list, "address", NODE_GFX_OBJECT, &new_addr))
			{
				char				addr[35];
				uint64_t			amount;
				size_t				max = 0;

				memcpy_c(addr, optr, (sz<34)?sz:34); 
				addr[34] = 0;
				amount = 0;
				ntx = 0;
				
				list_received					(addr, PTR_NULL, min_conf, max_conf, &amount, &ntx,&max, 0);
				tree_manager_set_child_value_str(&new_addr, "addr", addr);
				tree_manager_set_child_value_i64(&new_addr, "amount", amount);
				tree_manager_set_child_value_i64(&new_addr, "ntx", ntx);
				release_zone_ref				(&new_addr);
			}
			cur++;
			optr = ptr + 1;
			dir_list_len -= sz;
		}
		free_string(&dir_list);
	}
	
	release_zone_ref(&addr_list);
	return 1;
}

OS_API_C_FUNC(int) pubkeytoaddr(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	dh_key_t		pub;
	btc_addr_t		pubaddr;
	mem_zone_ref	pubkey_n = { PTR_NULL };
	struct string   xpubkey = { PTR_NULL };
	size_t keys_data_len = 0;
	unsigned char *keys_data = PTR_NULL;

	if (tree_manager_get_child_at(params, 0, &pubkey_n))
	{
		tree_manager_get_node_istr(&pubkey_n, 0, &xpubkey, 0);
		release_zone_ref(&pubkey_n);
	}

	if (xpubkey.len == 130)
	{
		unsigned char cpub[65];
		int n = 65;
		while (n--)
		{
			char    hex[3];
			hex[0] = xpubkey.str[n * 2 + 0];
			hex[1] = xpubkey.str[n * 2 + 1];
			hex[2] = 0;
			cpub[n] = strtoul_c(hex, PTR_NULL, 16);
		}
		key_to_addr(pub, pubaddr);
	}
	else if (xpubkey.len == 66)
	{
		char    hex[3];
		int n = 32;

		hex[0] = xpubkey.str[0];
		hex[1] = xpubkey.str[1];
		hex[2] = 0;
		pub[0] = strtoul_c(hex, PTR_NULL, 16);
		while (n--)
		{
			hex[0] = xpubkey.str[(n + 1) * 2 + 0];
			hex[1] = xpubkey.str[(n + 1) * 2 + 1];
			hex[2] = 0;
			pub[(n)+1] = strtoul_c(hex, PTR_NULL, 16);
		}
		key_to_addr(pub, pubaddr);
	}
	free_string								(&xpubkey);
	tree_manager_set_child_value_btcaddr	(result,"addr",pubaddr);
	return 1;
}

OS_API_C_FUNC(int) liststaking(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	mem_zone_ref_ptr addr=PTR_NULL;
	mem_zone_ref	mempool = { PTR_NULL }, minconf = { PTR_NULL }, maxconf = { PTR_NULL }, unspents = { PTR_NULL }, addrs = { PTR_NULL };
	mem_zone_ref	my_list = { PTR_NULL };
	mem_zone_ref	last_blk = { PTR_NULL };
	struct string	pos_hash_data = { PTR_NULL };
	int				max = 2000;
	int				ret = 0;
	unsigned int 	block_time;
	unsigned int	target,iminconf=0;

	if (tree_manager_get_child_at(params, 0, &minconf))
	{
		tree_mamanger_get_node_dword(&minconf, 0, &iminconf);
		release_zone_ref			(&minconf);
	}

	if (iminconf < min_staking_depth)
		iminconf = min_staking_depth;

	if (tree_manager_get_child_at(params, 1, &maxconf))
	{
		//tree_mamanger_get_node_dword(&maxconf, 0, &iminconf);
		release_zone_ref(&maxconf);
	}
	if (!tree_manager_get_child_at(params, 2, &addrs))return 0;

	if (!tree_manager_find_child_node(&my_node, NODE_HASH("last_block"), NODE_BITCORE_BLK_HDR, &last_blk))
	{
		release_zone_ref(&addrs);
		return 0;
	}

	if (!tree_manager_add_child_node(result, "unspents", NODE_JSON_ARRAY, &unspents))
	{
		release_zone_ref(&last_blk);
		release_zone_ref(&addrs);
		return 0;
	}

	get_target_spacing				(&target);
	tree_manager_get_child_value_i32(&last_blk, NODE_HASH("time"), &block_time);

	tree_manager_set_child_value_i32(result, "block_target", target);
	tree_manager_set_child_value_i32(result, "now", get_time_c());
	tree_manager_set_child_value_i32(result, "last_block_time", block_time);

	//tree_manager_find_child_node	(&my_node, NODE_HASH("mempool"), NODE_BITCORE_TX_LIST, &mempool);
	node_aquire_mempool_lock(&mempool);
	for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
	{
		if (max > 0)
		{
			btc_addr_t my_addr;
			tree_manager_get_node_btcaddr	(addr, 0, my_addr);
			list_staking_unspent			(&last_blk, my_addr, &mempool, &unspents, iminconf, &max);
		}
	}
	release_zone_ref(&mempool);
	node_release_mempool_lock();

	release_zone_ref(&last_blk);
	release_zone_ref(&unspents);
	release_zone_ref(&addrs);
	return 1;
}




OS_API_C_FUNC(int) rescanaddrs(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	mem_zone_ref	 addrs = { PTR_NULL }, addr_list = { PTR_NULL };
	mem_zone_ref_ptr addr;

	if (!tree_manager_get_child_at(params, 0, &addrs))return 0;

	for (tree_manager_get_first_child(&addrs, &addr_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&addr_list, &addr))
	{
		btc_addr_t			pubaddr;
		memset_c(pubaddr, 0, sizeof(btc_addr_t));
		if (!tree_manager_get_node_btcaddr(addr, 0, pubaddr))continue;
		rescan_addr(pubaddr);
	}
	release_zone_ref(&addrs);

	return 1;
}
OS_API_C_FUNC(int) signstakeblock(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t			rblkHash, blkHash;
	unsigned char	chash[65];
	struct string   signature = { 0 }, vpubk = { 0 }, pubk = { 0 }, sign = { 0 };
	mem_zone_ref	pn = { PTR_NULL }, blk = { PTR_NULL }, node_blks = { PTR_NULL };
	unsigned int	n;
	int				ret=0;
	tree_manager_get_child_at(params, 0, &pn);
	tree_manager_get_node_str(&pn, 0, chash, 65, 0);
	release_zone_ref(&pn);

	tree_manager_get_child_at(params, 1, &pn);
	tree_manager_get_node_istr(&pn, 0, &signature, 0);
	release_zone_ref(&pn);

	tree_manager_get_child_at(params, 2, &pn);
	tree_manager_get_node_istr(&pn, 0, &vpubk, 0);
	release_zone_ref(&pn);

	n = 0;
	while (n < 32)
	{
		char	hex[3];
		hex[0] = chash[n * 2 + 0];
		hex[1] = chash[n * 2 + 1];
		hex[2] = 0;
		blkHash[n] = strtoul_c(hex, PTR_NULL, 16);
		rblkHash[31 - n] = blkHash[n];
		n++;
	}

	sign.len = signature.len / 2;
	sign.size = sign.len + 1;
	sign.str = malloc_c(sign.size);
	n		 = 0;
	while (n < sign.len)
	{
		char	hex[3];
		hex[0] = signature.str[n * 2 + 0];
		hex[1] = signature.str[n * 2 + 1];
		hex[2] = 0;
		sign.str[n] = strtoul_c(hex, PTR_NULL, 16);
		n++;
	}
	sign.str[sign.len] = 0;
	free_string(&signature);


	if (vpubk.len == 66)
	{
		pubk.len  = 33;
		pubk.size = pubk.len + 1;
		pubk.str  = malloc_c(pubk.size);
		n = 0;
		while (n < pubk.len)
		{
			char	hex[3];
			hex[0] = vpubk.str[n * 2 + 0];
			hex[1] = vpubk.str[n * 2 + 1];
			hex[2] = 0;
			pubk.str[n] = strtoul_c(hex, PTR_NULL, 16);
			n++;
		}
		pubk.str[pubk.len] = 0;
	}
	free_string(&vpubk);



	if (tree_manager_find_child_node(&my_node, NODE_HASH("submitted blocks"), NODE_BITCORE_BLOCK_LIST, &node_blks))
	{
		if (tree_find_child_node_by_member_name_hash(&node_blks, NODE_BITCORE_BLK_HDR, "blkHash", blkHash, &blk))
		{
			mem_zone_ref	txs = { PTR_NULL };
			mem_zone_ref	sig = { PTR_NULL };

			if (node_is_next_block(&blk))
			{
				if (pubk.len == 0)
					ret = 1;
				else
				{
					unsigned char	type;
					struct string	bsig = { 0 };

					ret = parse_sig_seq(&sign, &bsig, &type, 1);
					if (ret)
						ret = blk_check_sign(&bsig, &pubk, blkHash);

					free_string(&bsig);
				}
				if (ret)
				{
					uint64_t block_height;
					unsigned int done;

					node_aquire_mining_lock();

					if (!tree_manager_find_child_node(&blk, NODE_HASH("signature"), NODE_BITCORE_ECDSA_SIG, &sig))
						tree_manager_add_child_node(&blk, "signature", NODE_BITCORE_ECDSA_SIG, &sig);

					tree_manager_write_node_sig(&sig, 0, sign.str, sign.len);
					release_zone_ref(&sig);

					tree_manager_set_child_value_i32(&blk, "done", 0);
					tree_manager_set_child_value_i32(&blk, "ready", 1);

					node_release_mining_lock();

					done = 0;
					while (done == 0)
					{
						if (!tree_manager_get_child_value_i32(&blk, NODE_HASH("done"), &done))
							done = 1;
					}

					tree_manager_get_child_value_i64(&my_node, NODE_HASH("block_height"), &block_height);

					if (find_block_hash(blkHash, block_height, PTR_NULL))
						release_zone_ref(&next_block);
				}
			}
			else
				tree_remove_child_by_member_value_hash(&node_blks, NODE_BITCORE_BLK_HDR, "blkHash", blkHash);

			release_zone_ref			(&blk);
		}
		release_zone_ref(&node_blks);
	}

	
	
	free_string(&sign);
	free_string(&pubk);


	return ret;
}


OS_API_C_FUNC(int) signstaketx(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t			blkHash;
	unsigned char	chash[65];
	int				ret = 0;
	unsigned int	n;
	unsigned char   hash_type = 1;
	struct	string  bsign = { PTR_NULL }, bpubkey = { PTR_NULL }, sign = { PTR_NULL }, inPubKey = { PTR_NULL };
	mem_zone_ref	blk = { PTR_NULL }, blk_list = { PTR_NULL },pn = { PTR_NULL }, node_txs = { PTR_NULL }, tx = { PTR_NULL };
	
	if (!tree_manager_get_child_at(params, 0, &pn))return 0;
	tree_manager_get_node_str	(&pn, 0, chash, 65, 0);
	release_zone_ref			(&pn);

	n = 0;
	while (n < 32)
	{
		char	hex[3];
		hex[0] = chash[n * 2 + 0];
		hex[1] = chash[n * 2 + 1];
		hex[2] = 0;
		blkHash[n] = strtoul_c(hex, PTR_NULL, 16);
		n++;
	}



	if (!tree_manager_get_child_at(params, 1, &pn))return 0;
	tree_manager_get_node_istr	(&pn, 0, &sign, 0);
	release_zone_ref			(&pn);

	bsign.len	= (sign.len / 2)+1;
	bsign.size  = bsign.len + 1;
	bsign.str	= malloc_c(bsign.size);
	n = 0;
	while (n < bsign.len)
	{
		char	hex[3];
		hex[0]   = sign.str[n * 2 + 0];
		hex[1]   = sign.str[n * 2 + 1];
		hex[2]   = 0;
		bsign.str[n] = strtoul_c(hex, PTR_NULL, 16);
		n++;
	}
	free_string(&sign);

	bsign.str[bsign.len-1] = hash_type;

	if (tree_manager_get_child_at(params, 2, &pn))
	{
		struct string inPubKey = { PTR_NULL };

		tree_manager_get_node_istr	(&pn, 0, &inPubKey,16);
		release_zone_ref			(&pn);


		if (inPubKey.len == 66)
		{
			bpubkey.len		= 33;
			bpubkey.size	= bpubkey.len + 1;
			bpubkey.str		= malloc_c(bpubkey.size);

			n = 0;
			while (n < bpubkey.len)
			{
				char	hex[3];
				hex[0] = inPubKey.str[n * 2 + 0];
				hex[1] = inPubKey.str[n * 2 + 1];
				hex[2] = 0;
				bpubkey.str[n] = strtoul_c(hex, PTR_NULL, 16);
				n++;
			}
			free_string(&inPubKey);
		}
	}

	node_aquire_mining_lock();

	tree_manager_find_child_node(&my_node, NODE_HASH("submitted blocks"), NODE_BITCORE_BLOCK_LIST, &blk_list);
	
	if (tree_find_child_node_by_member_name_hash(&blk_list, NODE_BITCORE_BLK_HDR, "blkHash", blkHash, &blk))
	{
		mem_zone_ref	txs = { PTR_NULL };
		hash_t			h, rblkh, merkleRoot;

		if(node_is_next_block						(&blk))
		{
			tree_manager_find_child_node			(&blk, NODE_HASH("txs"), NODE_BITCORE_TX_LIST, &txs);
			tree_manager_get_child_at				(&txs, 1, &tx);
			ret = tx_sign							(&tx, 0, hash_type, &bsign, &bpubkey,PTR_NULL);
		
			build_merkel_tree						(&txs, merkleRoot);
			tree_manager_set_child_value_hash		(&blk, "merkle_root", merkleRoot);
			compute_block_hash						(&blk,h);
			tree_manager_set_child_value_hash		(&blk, "blkHash", h);
			n = 32;
			while (n--)rblkh[n] = h[31 - n];

			tree_manager_set_child_value_hash		(result, "newblockhash", rblkh);

			release_zone_ref						(&tx);
			release_zone_ref						(&txs);
		}
		else
			tree_remove_child_by_member_value_hash(&blk_list, NODE_BITCORE_BLK_HDR, "blkHash", blkHash);

		release_zone_ref					(&blk);
	}

	release_zone_ref(&blk_list);

	node_release_mining_lock();

	free_string(&bsign);
	free_string(&bpubkey);
	return ret;

}

OS_API_C_FUNC(int) getstaketx(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	unsigned char	chash[65];
	hash_t			txHash, blkhash;
	hash_t			txh;
	btc_addr_t		pubaddr;
	mem_zone_ref	vout = { PTR_NULL }, prevtx = { PTR_NULL }, newtx = { PTR_NULL }, pn = { PTR_NULL };
	struct string   sPubk = { PTR_NULL }, script = { PTR_NULL }, null_str = { PTR_NULL }, bpubkey = { PTR_NULL };
	uint64_t		amount;
	unsigned int	OutIdx, newTxTime, n, hash_type = 1;
	int				ret;
	char			toto = 0;
	
	null_str.str  = &toto;
	null_str.len  = 0;
	null_str.size = 1;
	
	if (!tree_manager_get_child_at(params, 0, &pn))return 0;
	tree_manager_get_node_str(&pn, 0, chash, 65, 0);
	release_zone_ref(&pn);

	n = 0;
	while (n < 32)
	{
		char	hex[3];
		hex[0] = chash[n * 2 + 0];
		hex[1] = chash[n * 2 + 1];
		hex[2] = 0;
		txHash[31 - n] = strtoul_c(hex, PTR_NULL, 16);
		n++;
	}

	if (!tree_manager_get_child_at(params, 1, &pn))return 0;
	tree_mamanger_get_node_dword(&pn, 0, &OutIdx);
	release_zone_ref(&pn);

	if (!tree_manager_get_child_at(params, 2, &pn))return 0;
	tree_mamanger_get_node_dword(&pn, 0, &newTxTime);
	release_zone_ref(&pn);

	if (tree_manager_get_child_at(params, 3, &pn)){
		struct string inPubKey = { PTR_NULL };

		tree_manager_get_node_istr(&pn, 0, &inPubKey, 16);
		release_zone_ref(&pn);

		if (inPubKey.len == 66)
		{
			bpubkey.len = 33;
			bpubkey.size = bpubkey.len + 1;
			bpubkey.str = malloc_c(bpubkey.size);

			n = 0;
			while (n < bpubkey.len)
			{
				char	hex[3];
				hex[0] = inPubKey.str[n * 2 + 0];
				hex[1] = inPubKey.str[n * 2 + 1];
				hex[2] = 0;
				bpubkey.str[n] = strtoul_c(hex, PTR_NULL, 16);
				n++;
			}
		}
		free_string(&inPubKey);
		release_zone_ref(&pn);
	}


	ret = load_tx(&prevtx, blkhash, txHash);
	
	if (ret)ret = get_tx_output(&prevtx, OutIdx, &vout);
	if (ret)ret = tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &script, 0);
	if (ret)ret = tree_manager_get_child_value_i64(&vout, NODE_HASH("value"), &amount);
	if (ret)ret = get_out_script_address(&script, &sPubk, pubaddr);
	release_zone_ref(&vout);

	if (ret)ret = tree_manager_add_child_node(result, "transaction", NODE_BITCORE_TX, &newtx);
	if (ret)ret = new_transaction(&newtx, newTxTime);
	
	if (ret)
	{
		uint64_t			half_am, rew;
		uint64_t			lb;
		mem_zone_ref		last_blk = { PTR_NULL };
		struct string		oscript = { PTR_NULL };

		node_aquire_mining_lock();

		if (sPubk.str == PTR_NULL)
		{
			mem_zone_ref script_node = { PTR_NULL };

			if (tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node))
			{
				create_payment_script(&bpubkey, 0, &script_node);
				serialize_script(&script_node, &oscript);
				release_zone_ref(&script_node);
			}
		}
		else
		{
			clone_string(&oscript, &script);
		}

		tree_manager_get_child_value_i64(&my_node, NODE_HASH("block_height"), &lb);

		get_stake_reward(lb, &rew);
		half_am = muldiv64(amount + rew, 1, 2);

		tx_add_input	(&newtx, txHash, OutIdx, &script);
		tx_add_output	(&newtx, 0, &null_str);
		tx_add_output	(&newtx, half_am, &oscript);
		tx_add_output	(&newtx, half_am, &oscript);
		free_string		(&oscript);

		if (tree_manager_find_child_node(&my_node, NODE_HASH("last_block"), NODE_BITCORE_BLK_HDR, &last_blk))
		{
			hash_t					pos_hash, out_diff;
			hash_t					lastStakeModifier;
			unsigned int			ModTime, last_diff;
			uint64_t				weight;

			get_last_stake_modifier	(&last_blk, lastStakeModifier, &ModTime);
			ret = compute_tx_pos	(&newtx, lastStakeModifier, newTxTime, pos_hash, &weight);

			if (ret)
			{
				mem_zone_ref			node_txs = { PTR_NULL };

				memset_c(out_diff, 0, sizeof(hash_t));
				last_diff = get_current_pos_difficulty();

				if (last_diff == 0xFFFFFFFF)
				{
					unsigned int					nBits;
					tree_manager_get_child_value_i32(&last_blk, NODE_HASH("bits"), &nBits);
					mul_compact(nBits, weight, out_diff);
				}
				else
					mul_compact(last_diff, weight, out_diff);

				//check proof of stake
				if (cmp_hashle(pos_hash, out_diff) >= 0)
				{
					hash_t					rtxhash;
					uint64_t				height;
					mem_zone_ref			node_txs = { PTR_NULL }, newBlock = { PTR_NULL };

					
					tree_manager_get_child_value_i64(&last_blk, NODE_HASH("height"), &height);

					if (create_pos_block(height, &newtx, &newBlock))
					{
						mem_zone_ref txs = { PTR_NULL }, blk_list = { PTR_NULL };

						node_fill_block_from_mempool(&newBlock);

						if (tree_manager_find_child_node(&my_node, NODE_HASH("submitted blocks"), NODE_BITCORE_BLOCK_LIST, &blk_list))
						{
							hash_t h, rblkh;

							tree_manager_get_child_value_hash(&newBlock, NODE_HASH("blkHash"), h);
							n = 32;
							while (n--)rblkh[n] = h[31 - n];

							tree_manager_set_child_value_hash(result, "newblockhash", rblkh);
							tree_manager_node_add_child		 (&blk_list, &newBlock);
							release_zone_ref				 (&blk_list);
						}
						tree_manager_find_child_node(&newBlock, NODE_HASH("txs"), NODE_BITCORE_TX_LIST, &txs);
						tree_manager_get_child_at	(&txs, 1, &newtx);
						release_zone_ref			(&txs);
						release_zone_ref			(&newBlock);
						compute_tx_sign_hash		(&newtx, 0, &script, hash_type, txh);



						n = 32;
						while (n--)rtxhash[n] = txh[31 - n];
						tree_manager_set_child_value_hash	(result, "txhash"	, rtxhash);
						tree_manager_set_child_value_btcaddr(result, "addr"		, pubaddr);

						release_zone_ref					(&next_block);

						ret = 1;
					}
				}
				else
					ret = 0;
			}
			release_zone_ref(&last_blk);
			
		}
		node_release_mining_lock();
	}
	
	
	free_string		(&sPubk);
	free_string		(&bpubkey);
	release_zone_ref(&newtx);
	release_zone_ref(&prevtx);
	free_string		(&script);
	return ret;
}

OS_API_C_FUNC(int) getrawmempool(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	mem_zone_ref tx_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr tx = PTR_NULL;
	//if (!tree_manager_find_child_node(params, NODE_HASH("mempool"), NODE_BITCORE_TX_LIST, &tx_list))return 0;

	node_aquire_mempool_lock(&tx_list);
	for (tree_manager_get_first_child(&tx_list, &my_list, &tx); ((tx != NULL) && (tx->zone != NULL)); tree_manager_get_next_child(&my_list, &tx))
	{
		mem_zone_ref hn = { PTR_NULL };
		if (tree_manager_find_child_node(tx, NODE_HASH("txid"), 0xFFFFFFF, &hn))
		{
			tree_manager_node_add_child(result, &hn);
			release_zone_ref(&hn);
		}
	}
	release_zone_ref			(&tx_list);
	node_release_mempool_lock();

	return 1;
}


OS_API_C_FUNC(int) getstaking(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	unsigned char	chash[65];
	hash_t			txHash, out_diff;
	mem_zone_ref	last_blk		= { PTR_NULL };
	mem_zone_ref pn = { PTR_NULL };
	struct string	pos_hash_data	= { PTR_NULL };
	unsigned int	OutIdx, target, block_time, n;
	uint64_t		amount;
	int ret = 0;


	if (!tree_manager_get_child_at(params, 0, &pn))return 0;
	tree_manager_get_node_str(&pn, 0, chash, 65, 0);
	release_zone_ref(&pn);

	if (!tree_manager_get_child_at(params, 1, &pn))return 0;
	tree_mamanger_get_node_dword(&pn, 0, &OutIdx);
	release_zone_ref(&pn);

	if (!tree_manager_find_child_node(&my_node, NODE_HASH("last_block"), NODE_BITCORE_BLK_HDR, &last_blk))
		return 0;

	n = 0;
	while (n < 32)
	{
		char	hex[3];
		hex[0]		= chash[n * 2 + 0];
		hex[1]		= chash[n * 2 + 1];
		hex[2]		= 0;
		txHash[31-n]= strtoul_c(hex, PTR_NULL, 16);
		n++;
	}
		
	get_target_spacing				(&target);
	tree_manager_get_child_value_i32(&last_blk, NODE_HASH("time"), &block_time);

	memset_c(out_diff, 0, sizeof(hash_t));

	if (get_tx_pos_hash_data(&last_blk, txHash, OutIdx, &pos_hash_data, &amount, out_diff))
	{
		hash_t rout_diff;

		n = 32;
		while (n--)rout_diff[n]=out_diff[31 - n];

		tree_manager_set_child_value_str (result , "hash_data"		, pos_hash_data.str);
		tree_manager_set_child_value_hash(result , "difficulty"		, rout_diff);
		tree_manager_set_child_value_i64 (result  , "weight"			, amount);
		tree_manager_set_child_value_i32 (result , "block_target"	, target);
		tree_manager_set_child_value_i32 (result , "now"			, get_time_c());
		tree_manager_set_child_value_i32 (result , "last_block_time", block_time);
		ret = 1;
	}
	free_string		(&pos_hash_data);
	release_zone_ref(&last_blk);
	

	return ret;
}


OS_API_C_FUNC(int) importkeypair(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	dh_key_t	pub, priv;
	char		clabel[32];
	btc_addr_t	pubaddr;
	mem_zone_ref username_n = { PTR_NULL }, pubkey_n = { PTR_NULL }, privkey_n = { PTR_NULL }, rescan_n = { PTR_NULL }, label_n = { PTR_NULL };
	mem_zone_ref_ptr tx = PTR_NULL;
	struct string username, xpubkey, xprivkey,  label;
	unsigned int found, rescan;
	unsigned int np;
	int ret;

	create_dir("keypairs");
	if (stat_file("keypairs") != 0)
		return 0;

	np = tree_manager_get_node_num_children(params);
	if (np < 5)return 0;
	
	init_string	(&username);
	init_string	(&xpubkey);
	init_string	(&xprivkey);
	init_string	(&label);
	memset_c(clabel, 0, 32); 
	
	tree_manager_get_child_at(params, 0, &username_n);
	tree_manager_get_child_at(params, 1, &label_n);
	tree_manager_get_child_at(params, 2, &pubkey_n);
	tree_manager_get_child_at(params, 3, &privkey_n);
	tree_manager_get_child_at(params, 4, &rescan_n);
	
	tree_manager_get_node_istr	(&username_n, 0, &username, 0);
	tree_manager_get_node_str	(&label_n, 0, clabel,32, 0);
	tree_manager_get_node_istr	(&pubkey_n, 0, &xpubkey, 0);
	tree_manager_get_node_istr	(&privkey_n, 0, &xprivkey, 0);
	
	if (!tree_mamanger_get_node_dword(&rescan_n, 0, &rescan))
		rescan = 0;

				
	release_zone_ref(&rescan_n);
	release_zone_ref(&privkey_n);
	release_zone_ref(&pubkey_n);
	release_zone_ref(&label_n);
	release_zone_ref(&username_n);

	if ((username.len < 1) || (xpubkey.len < 66))
	{
		free_string(&username);
		free_string(&xpubkey);
		free_string(&xprivkey);
		free_string(&label);
		return 0;
	}
	uname_cleanup(&username);
	if (xpubkey.len == 66)
	{
		int n = 33;
		while (n--)
		{
			char    hex[3];
			hex[0] = xpubkey.str[n * 2 + 0];
			hex[1] = xpubkey.str[n * 2 + 1];
			hex[2] = 0;
			pub[n] = strtoul_c(hex, PTR_NULL, 16);
		}
		key_to_addr(pub, pubaddr);
	}

	memset_c(priv, 0, sizeof(dh_key_t));
	if (xprivkey.len > 0) 
	{
		int n = (xprivkey.len < sizeof(dh_key_t)) ? xprivkey.len : sizeof(dh_key_t);
		while (n--)
		{
			char    hex[3];
			hex[0] = xprivkey.str[n * 2 + 0];
			hex[1] = xprivkey.str[n * 2 + 1];
			hex[2] = 0;
			priv[n] = strtoul_c(hex, PTR_NULL, 16);
		}
	}
	
	ret= add_keypair					(&username, clabel, pubaddr, priv, rescan, &found);

	if (ret)
		tree_manager_set_child_value_i32	(result, "new", found == 1 ? 0 : 1);
	


	free_string(&username);
	free_string(&xpubkey);
	free_string(&xprivkey);
	return ret;
}

OS_API_C_FUNC(int) getprivaddr(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	dh_key_t		prvkey;
	mem_zone_ref	pn					= { PTR_NULL };
	struct string	pubaddr				= { PTR_NULL },	username	= { PTR_NULL };
	int				ret					= 0;

	tree_manager_get_child_at	(params, 0, &pn);
	tree_manager_get_node_istr	(&pn, 0, &username, 0);
	release_zone_ref			(&pn);

	uname_cleanup(&username);

	tree_manager_get_child_at	(params, 1, &pn);
	tree_manager_get_node_istr	(&pn, 0, &pubaddr, 0);
	release_zone_ref			(&pn);
	
	ret=get_privkey				(&username, &pubaddr,prvkey);

	if (ret)
	{
		char hexk[129];
		bin_2_hex(prvkey, 64, hexk);
		tree_manager_set_child_value_str(result, "privkey", hexk);
	}
		
	free_string(&pubaddr);
	free_string(&username);

	return ret;
}

OS_API_C_FUNC(int) setaccountpw(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	mem_zone_ref		pn = { PTR_NULL };
	struct string		username = { PTR_NULL }, pw = { PTR_NULL }, newpw = { PTR_NULL };
	int					ret;

	if (!tree_manager_get_child_at(params, 0, &pn))
		return 0;

	tree_manager_get_node_istr(&pn, 0, &username,0);
	release_zone_ref(&pn);

	uname_cleanup(&username);

	if (!tree_manager_get_child_at(params, 1, &pn))
	{
		free_string(&username);
		return 0;
	}
	tree_manager_get_node_istr(&pn, 0, &pw, 0);
	release_zone_ref(&pn);

	if (tree_manager_get_child_at(params, 2, &pn))
	{
		tree_manager_get_node_istr(&pn, 0, &newpw, 0);
		release_zone_ref(&pn);
	}
	ret = setpassword(&username, &pw, &newpw);

	free_string(&pw);
	free_string(&newpw);
	free_string(&username);
	

	return ret;
}

OS_API_C_FUNC(int) listaccounts(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	mem_zone_ref	page_idx_n = { PTR_NULL }, accnt_list = { PTR_NULL },addr_list = { PTR_NULL };
	size_t			page_idx;


	if (tree_manager_get_child_at(params, 0, &page_idx_n))
	{
		tree_mamanger_get_node_dword(&page_idx_n, 0, &page_idx);
		release_zone_ref(&page_idx_n);
	}
	else
		page_idx = 0;

	if (!tree_manager_add_child_node(result, "accounts", NODE_JSON_ARRAY, &accnt_list))
		return 0;

	
	get_account_list(&accnt_list, page_idx);
	
	release_zone_ref(&accnt_list);
	
	return 1;
}

OS_API_C_FUNC(int) getpubaddrs(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	mem_zone_ref username_n = { PTR_NULL }, param = { PTR_NULL }, addr_list = { PTR_NULL };
	unsigned int shownull;
	
	if (!tree_manager_get_child_at(params, 0, &username_n))
		return 0;

	if (tree_manager_get_child_at(params, 1, &param))
	{
		if(!tree_mamanger_get_node_dword(&param, 0, &shownull))
			shownull = 0;
		release_zone_ref(&param);
	}
	else
		shownull = 0;
		
	if (!tree_manager_add_child_node(result, "addrs", NODE_JSON_ARRAY, &addr_list))
	{
		release_zone_ref(&username_n);
		return 0;
	}
		
	wallet_list_addrs		(&username_n, &addr_list,shownull ? 3 : 1);
	release_zone_ref		(&username_n);
	release_zone_ref		(&addr_list);
	
	
	return 1;
}

OS_API_C_FUNC(int) getmininginfo(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	size_t	 bsize;
	uint64_t height;
	unsigned int bits;
	double    fdiff;

	tree_manager_get_child_value_i64(&my_node, NODE_HASH("block_height"), &height);
	tree_manager_get_child_value_i32(&my_node, NODE_HASH("current_pow_diff"), &bits);
	bsize = 1024;

	/*fdiff = getdifficulty(bits);*/
	fdiff = GetDifficulty(bits);


	tree_manager_set_child_value_i64(result, "blocks", height);
	tree_manager_set_child_value_i32(result, "currentblocksize", bsize);
	tree_manager_set_child_value_i64(result, "currentblockweight", 0);
	tree_manager_set_child_value_float(result, "difficulty", fdiff);
	tree_manager_set_child_value_str(result, "chain", "main");



	return 1;

}

OS_API_C_FUNC(int) getwork(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	unsigned char	xbuffer[257];
	unsigned char	buffer[129];
	hash_t			diffHash, midh;
	struct string   xdata = { 0 }, param = { 0 };
	mem_zone_ref	pn = { PTR_NULL };
	uint64_t		last_blk;
	unsigned int	bits, n, now, next_block_time, gen_new_block;

	if (!has_peers())
		return 0;
	
	tree_manager_get_child_value_i64(&my_node, NODE_HASH("block_height"), &last_blk);

	if (block_pow_limit())
		return 0;

	node_aquire_mining_lock();

	if ((cur_mining_height != 0) && (cur_mining_height!=last_blk))
	{
		release_zone_ref(&next_block);
	}
	
	if (tree_manager_get_child_at(params, 0, &pn))
	{
		struct string	xparam = { 0 };
		unsigned int	ok;
		size_t			rd;

		tree_manager_get_node_istr	(&pn, 0, &xparam, 0);
		release_zone_ref			(&pn);

		tree_manager_dump_node_rec(params, 0, 16);

		ok = 0;

		if (xparam.len > 0)
		{
			mem_zone_ref new_blk = { PTR_NULL };
			unsigned int nonce,done;
			unsigned int *pstr;

			if (next_block.zone == PTR_NULL)
			{
				tree_manager_create_node		("result", NODE_GFX_BOOL, result);
				tree_manager_write_node_dword	(result, 0, 0);
				node_release_mining_lock		();
				return 1;
			}


			param.len = xparam.len / 2;
			param.size = param.len + 1;
			param.str = malloc_c(param.size);

			pstr = (unsigned int *)param.str;

			n = 0;
			while (n < (param.len/4))
			{
				char	hex[9];
				hex[0] = xparam.str[n * 8 + 0];
				hex[1] = xparam.str[n * 8 + 1];

				hex[2] = xparam.str[n * 8 + 2];
				hex[3] = xparam.str[n * 8 + 3];

				hex[4] = xparam.str[n * 8 + 4];
				hex[5] = xparam.str[n * 8 + 5];

				hex[6] = xparam.str[n * 8 + 6];
				hex[7] = xparam.str[n * 8 + 7];

				hex[8] = 0;
				pstr[n] = strtoul_c(hex, PTR_NULL, 16);
				n++;
			}
			free_string				(&xparam);

			tree_manager_create_node("blk", NODE_BITCORE_BLK_HDR, &new_blk);
			rd = read_node			(&new_blk, param.str, param.len);
			free_string				(&param);

			if (rd == 80)
			{
				hash_t			bh;
				uint64_t		block_height;
				mem_zone_ref	node_blks = { PTR_NULL };
				
				tree_manager_get_child_value_i32	(&new_blk, NODE_HASH("nonce"), &nonce);
				tree_manager_set_child_value_i32	(&next_block, "nonce", nonce);

				compute_block_hash					(&next_block, bh);
				tree_manager_set_child_value_hash	(&next_block, "blkHash", bh);

				if (!generate_new_keypair("mining addr", mining_addr))
					memset_c(mining_addr, 0, sizeof(btc_addr_t));

				tree_manager_set_child_value_i32(&next_block, "ready", 1);
				tree_manager_set_child_value_i32(&next_block, "done", 0);


				tree_manager_find_child_node	(&my_node, NODE_HASH("submitted blocks"), NODE_BITCORE_BLOCK_LIST, &node_blks);
				tree_manager_node_add_child		(&node_blks, &next_block);
				release_zone_ref				(&node_blks);
				done = 0;

				while (done==0)
				{
					if (!tree_manager_get_child_value_i32(&next_block, NODE_HASH("done"), &done))
						done = 1;
				}

				tree_manager_get_child_value_i64(&my_node, NODE_HASH("block_height"), &block_height);

				if (find_block_hash(bh,block_height,PTR_NULL))
					ok = 1;
				else
					ok = 0;
			}
			else if (rd == 76)
			{
				ok = 1;
			}
			release_zone_ref(&new_blk);
			release_zone_ref(&next_block);
		}
		tree_manager_create_node		("result", NODE_GFX_BOOL, result);
		tree_manager_write_node_dword	(result,0,ok);
		
		node_release_mining_lock		();
		return 1;
	}

	if (mining_addr[0] == 0)
	{
		//if (!tree_manager_get_child_value_btcaddr(&my_node, NODE_HASH("coinbaseAddr"), mining_addr))
		{
			if (!generate_new_keypair("mining addr", mining_addr))
			{
				node_release_mining_lock();
				return 0;
			}
		}
	}

	now = get_time_c();

	
	if (next_block.zone == PTR_NULL)
		gen_new_block = 1;
	else
	{
		tree_manager_get_child_value_i32(&next_block, NODE_HASH("time"), &next_block_time);
		
		if ((next_block_time + 30) < now)
			gen_new_block = 1;
		else
			gen_new_block = 0;
	}


	if (gen_new_block)
	{
		if (next_block.zone != PTR_NULL)
			release_zone_ref						(&next_block);

		node_create_pow_block					(&next_block,mining_addr);
		node_fill_block_from_mempool			(&next_block);

		cur_mining_height = last_blk;
		

		log_message("generated new block candidate for mining %blkHash%", &next_block);
	}

	

	write_node(&next_block, buffer);
	*((unsigned int *)(buffer + 80)) = 80;
	memset_c(buffer + 84, 0, 128 - 84);

	n = 0;
	while (n<32)
	{
		xbuffer[n * 8 + 0] = hex_chars[buffer[n * 4 + 3] >> 4];
		xbuffer[n * 8 + 1] = hex_chars[buffer[n * 4 + 3] & 0x0F];

		xbuffer[n * 8 + 2] = hex_chars[buffer[n * 4 + 2] >> 4];
		xbuffer[n * 8 + 3] = hex_chars[buffer[n * 4 + 2] & 0x0F];

		xbuffer[n * 8 + 4] = hex_chars[buffer[n * 4 + 1] >> 4];
		xbuffer[n * 8 + 5] = hex_chars[buffer[n * 4 + 1] & 0x0F];

		xbuffer[n * 8 + 6] = hex_chars[buffer[n * 4 + 0] >> 4];
		xbuffer[n * 8 + 7] = hex_chars[buffer[n * 4 + 0] & 0x0F];

		n++;
	}
	xbuffer[n*8] = 0;

	xdata.len  = 256;
	xdata.size = 257;
	xdata.str  = xbuffer;
	
	tree_manager_set_child_value_vstr(result, "data", &xdata);

	memset_c(midh, 0, sizeof(hash_t));

	tree_manager_get_child_value_i32(&my_node, NODE_HASH("current_pow_diff"), &bits);
	SetCompact(bits, diffHash);


	
	
	tree_manager_set_child_value_hash	(result, "target"	, diffHash);

	node_release_mining_lock();

	
	return 1;
}


OS_API_C_FUNC(int) getblocktemplate(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	char			workid[16];
	char			bitsStr[16];
	hash_t			block_hash, diffHash, rdiffHash;
	unsigned char	rnd[8];
	struct	string	param = { 0 }, emptystr = { 0 };
	struct	string	param2 = { 0 };
	mem_zone_ref	cbtx = { PTR_NULL }, txs = { PTR_NULL }, blk_txs = { PTR_NULL }, my_list = { PTR_NULL }, caps = { PTR_NULL }, req_caps = { PTR_NULL }, pn = { PTR_NULL }, reqcaps = { PTR_NULL }, cap = { PTR_NULL };
	mem_zone_ref_ptr tx = PTR_NULL;
	unsigned char	*bbits;
	unsigned int	rbits, mintime;
	unsigned int	n, version, time, bits, gen_new_block, now;
	uint64_t		height, total_fees;
	uint64_t		last_blk;

	if (!has_peers())
		return 0;

	tree_manager_get_child_value_i64(&my_node, NODE_HASH("block_height"), &last_blk);
	
	if (block_pow_limit())
		return 0;

	if ((cur_mining_height != 0) && (cur_mining_height != last_blk))
	{
		release_zone_ref(&next_block);
	}


	

	if (mining_addr[0] == 0)
	{
		if (!generate_new_keypair("mining addr", mining_addr))
			return 0;
	}

	if (tree_manager_get_child_at(params, 0, &pn))
	{
		tree_manager_find_child_node(&pn, NODE_HASH("capabilities"), NODE_JSON_ARRAY, &reqcaps);
		release_zone_ref(&pn);
		tree_manager_dump_node_rec(params, 0, 16);
	}


	default_RNG					(rnd, 4);
	bin_2_hex					(rnd, 4, workid);

	node_aquire_mining_lock();

	now = get_time_c();

	if (next_block.zone == PTR_NULL)
		gen_new_block = 1;
	else
	{
		unsigned int next_block_time;

		tree_manager_get_child_value_i32(&next_block, NODE_HASH("time"), &next_block_time);

		if ((next_block_time + 30) <now)
			gen_new_block = 1;
		else
			gen_new_block = 0;
	}


	if (gen_new_block)
	{
		if (next_block.zone != PTR_NULL)
			release_zone_ref						(&next_block);

		node_create_pow_block			(&next_block, mining_addr);
		node_fill_block_from_mempool	(&next_block);

		log_message("generated new block candidate for mining %blkHash%", &next_block);

		cur_mining_height = last_blk;
	}



	tree_manager_set_child_value_str	(&next_block, "workid", workid);
	tree_manager_get_child_value_hash	(&next_block, NODE_HASH("prev"), block_hash);
	tree_manager_get_child_value_i32	(&next_block, NODE_HASH("version"), &version);
	tree_manager_get_child_value_i32	(&next_block, NODE_HASH("time"), &time);
	tree_manager_get_child_value_i32	(&next_block, NODE_HASH("bits"), &bits);
	tree_manager_get_child_value_i64	(&next_block, NODE_HASH("height"), &height);

	get_block_time						(height-1, &mintime);

	
	bbits = &bits;
	rbits = ((bbits[0] << 24) | (bbits[1] << 16)  | (bbits[2] << 8) | (bbits[3]));
	
	SetCompact							(bits, diffHash);
	uitoa_s								(bits, bitsStr, 16, 16);

	//time += 3600 * 2;

	for (n = 0; n < 32; n++)
		rdiffHash[31 - n] = diffHash[n];

	tree_manager_set_child_value_str	(result, "bits", bitsStr);
	tree_manager_set_child_value_hash	(result, "target", rdiffHash);
	tree_manager_set_child_value_i32	(result, "curtime", time);
	tree_manager_set_child_value_i32	(result, "mintime", mintime);
	
	tree_manager_set_child_value_i32	(result, "expires", 30);
	tree_manager_set_child_value_i32	(result, "sizelimit", 1024*1024);
	tree_manager_set_child_value_i32	(result, "sigoplimit", 2000);
	tree_manager_set_child_value_i32	(result, "sigops", 2000);
	tree_manager_set_child_value_i32	(result, "weightlimit", 4000000);
	tree_manager_set_child_value_i32	(result, "vbrequired", 0);
	
	
	tree_manager_set_child_value_i64	(result, "height", height);
	tree_manager_set_child_value_hash	(result, "previousblockhash",block_hash);
	tree_manager_set_child_value_i32	(result, "version", 2);
	tree_manager_set_child_value_str	(result, "noncerange", "00000000ffffffff");
	tree_manager_set_child_value_str	(result, "workid", workid);

	tree_manager_add_child_node			(result, "mutable", NODE_JSON_ARRAY, PTR_NULL);

	emptystr.str = "";
	emptystr.len = 0;
	emptystr.size = 1;


	tree_manager_add_child_node			(result, "coinbaseaux", NODE_GFX_OBJECT, &caps);
	release_zone_ref					(&caps);


	tree_manager_add_child_node			(result, "rules", NODE_JSON_ARRAY, &caps);
	tree_manager_set_child_value_str	(&caps, "0", "csv");
	release_zone_ref					(&caps);

	tree_manager_add_child_node		   (result, "capabilities", NODE_JSON_ARRAY, &caps);
	
	tree_manager_add_child_node		   (&caps, "0", NODE_GFX_STR, &cap);
	tree_manager_write_node_str		   (&cap,0,"proposal");
	release_zone_ref				   (&cap);
	
	tree_manager_add_child_node		   (&caps, "1", NODE_GFX_STR, &cap);
	tree_manager_write_node_str		   (&cap, 0, "workid");
	release_zone_ref				   (&cap);

	
	tree_manager_add_child_node		   (&caps, "2", NODE_GFX_STR, &cap);
	tree_manager_write_node_str		   (&cap, 0, "coinbasevalue");
	release_zone_ref				   (&cap);
	

	tree_manager_add_child_node		   (&caps, "3", NODE_GFX_STR, &cap);
	tree_manager_write_node_str		   (&cap, 0, "coinbasetxn");
	release_zone_ref				   (&cap);
	
	
	release_zone_ref				   (&caps);
		
	
	if(tree_manager_find_child_node		(&next_block, NODE_HASH("txs"), NODE_BITCORE_TX_LIST, &blk_txs))
	{ 
		tree_manager_get_first_child		(&blk_txs, &my_list, &tx);
		tree_manager_add_child_node			(result, "coinbasetxn", NODE_GFX_OBJECT, &cbtx);
		get_tx_data							(tx, &cbtx);
		release_zone_ref					(&cbtx);

		tree_manager_get_next_child			(&my_list, &tx);
	}


	tree_manager_add_child_node			(result, "transactions", NODE_JSON_ARRAY, &txs);
	
	get_blockreward(height, &total_fees);

	for (; ((tx != NULL) && (tx->zone != NULL)); tree_manager_get_next_child(&my_list, &tx))
	{
		mem_zone_ref	newtx = { PTR_NULL };
		uint64_t		fee;

		if (tree_manager_add_child_node(&txs, "tx", NODE_GFX_OBJECT, &newtx))
		{
			get_tx_data(tx, &newtx);
			release_zone_ref(&newtx);
		}
		if (tree_manager_get_child_value_i64(tx, NODE_HASH("fee"), &fee))
			total_fees += fee;
	}

	release_zone_ref					(&blk_txs);
	release_zone_ref					(&txs);
	release_zone_ref					(&caps);
	release_zone_ref					(&reqcaps);

	tree_manager_set_child_value_i64	(result, "coinbasevalue", total_fees);
	
	node_release_mining_lock();

	return 1;
}


OS_API_C_FUNC(int) submitblock(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t			hash;
	mem_zone_ref	last_blk = { PTR_NULL };
	struct string	xparam = { 0 };
	struct string	xdata = { 0 }, data = { 0 };
	mem_zone_ref	blk = { PTR_NULL }, pn = { PTR_NULL };
	uint64_t		last_blkh;
	size_t			rd;
	unsigned int	n;
	int				ok = 0;

	if (!has_peers())
		return 0;

	tree_manager_get_child_value_i64(&my_node, NODE_HASH("block_height"), &last_blkh);

	if (block_pow_limit())
		return 0;

	if (!tree_manager_get_child_at(params, 0, &pn))return 0;
	{
		tree_manager_get_node_istr	(&pn, 0, &xparam, 0);
		release_zone_ref			(&pn);

		if (tree_manager_get_child_at(params, 1, &pn))
		{
			struct string workidStr = { 0 };

			if (tree_manager_get_child_value_istr(&pn, NODE_HASH("workid"), &workidStr, 0))
			{
				free_string(&workidStr);
			}
			release_zone_ref(&pn);
		}
		
		tree_manager_dump_node_rec(params, 0, 4);

		if (xparam.len > 0)
		{
			hash_t  ph;
			char	hex[9];
			struct string param = { PTR_NULL }, new_sig = { PTR_NULL };
			mem_zone_ref new_blk = { PTR_NULL },new_txs = { PTR_NULL };
			unsigned int done, rd2,mcnt;
			unsigned int *pstr;

			tree_manager_find_child_node(&my_node, NODE_HASH("last_block"), NODE_BITCORE_BLK_HDR, &last_blk);
			tree_manager_get_child_value_hash(&last_blk, NODE_HASH("blkHash"), hash);
			release_zone_ref(&last_blk);

			param.len  = xparam.len / 2;

			if (param.len & 3)
				param.len = (param.len & (~3)) + 4;

			param.size = param.len + 1;
			param.str = malloc_c(param.size);

			pstr = (unsigned int *)param.str;
		
			/*
			log_output("\n------------------------------------------------\n");
			log_output(xparam.str);
			log_output("\n------------------------------------------------\n");
			*/

			hex[8] = 0;
			
			for (n=0; n < (xparam.len / 8); n++)
			{
				hex[0] = xparam.str[n * 8 + 6];
				hex[1] = xparam.str[n * 8 + 7];

				hex[2] = xparam.str[n * 8 + 4];
				hex[3] = xparam.str[n * 8 + 5];

				hex[4] = xparam.str[n * 8 + 2];
				hex[5] = xparam.str[n * 8 + 3];

				hex[6] = xparam.str[n * 8 + 0];
				hex[7] = xparam.str[n * 8 + 1];
				
				pstr[n] = strtoul_c(hex, PTR_NULL, 16);
			}

			if ((n * 8) < xparam.len)
			{
				memset_c(hex, '0', 9);
				mcnt = xparam.len - (n * 8);
				
				while (mcnt>0)
				{
					hex[(8 - mcnt) + 0] = xparam.str[n * 8 + ( mcnt - 2 )];
					hex[(8 - mcnt) + 1] = xparam.str[n * 8 + ( mcnt - 1 )];
					mcnt -= 2;
				}

				pstr[n] = strtoul_c(hex, PTR_NULL, 16);
			}

			free_string				(&xparam);

			tree_manager_create_node("blk", NODE_BITCORE_BLK_HDR, &new_blk);
			tree_manager_create_node("txs", NODE_BITCORE_TX_LIST, &new_txs);
			

			rd = read_node			(&new_blk, param.str, 80);
			if(param.len>80)
				rd2 = read_node		(&new_txs, &param.str[80], param.len-80);

			free_string				(&param);

			tree_manager_node_add_child(&new_blk, &new_txs);
			release_zone_ref(&new_txs);
			
			tree_manager_get_child_value_hash(&new_blk, NODE_HASH("prev"), ph);

			tree_manager_dump_node_rec(&new_blk, 0, 8); 

			
			if ((rd >= 80) && (!memcmp_c(ph,hash,sizeof(hash_t))))
			{
				hash_t			bh;
				uint64_t		block_height;
				mem_zone_ref	node_blks = { PTR_NULL }, tx_list = { PTR_NULL };


				compute_block_hash				 (&new_blk, bh);

				node_aquire_mining_lock			 ();

				/*
				unsigned char   *bbits;
				rnonce,
				tree_manager_get_child_value_i32(&new_blk, NODE_HASH("nonce"), &nonce);
				bbits = &nonce;
				rnonce = ((bbits[0] << 24) | (bbits[1] << 16) | (bbits[2] << 8) | (bbits[3]));
				tree_manager_set_child_value_i32(&next_block, "nonce", rnonce);
				*/

				tree_manager_set_child_value_hash(&new_blk, "blkHash", bh);
				tree_manager_set_child_value_i32 (&new_blk, "ready", 1);
				tree_manager_set_child_value_i32 (&new_blk, "done", 0);

				tree_manager_find_child_node	(&my_node, NODE_HASH("submitted blocks"), NODE_BITCORE_BLOCK_LIST, &node_blks);
				tree_manager_node_add_child		(&node_blks, &new_blk);
				release_zone_ref				(&node_blks);
				done = 0;

				while (done == 0)
				{
					if (!tree_manager_get_child_value_i32(&new_blk, NODE_HASH("done"), &done))
						done = 1;
				}

				tree_manager_get_child_value_i64(&my_node, NODE_HASH("block_height"), &block_height);

				if (find_block_hash(bh, block_height+1, PTR_NULL))
					ok = 1;
				else
					ok = 0;

				node_release_mining_lock();
			}
			release_zone_ref(&new_blk);
			release_zone_ref(&next_block);

		}

		if (ok)
		{
			tree_manager_create_node("result", NODE_GFX_STR, result);
			tree_manager_write_node_str(result, 0, "");
		}
		else
		{
			tree_manager_create_node	("result", NODE_GFX_STR, result);
			tree_manager_write_node_str	(result, 0, "rejected");
		}

		return 1;
	}

	tree_manager_create_node("result", NODE_GFX_BOOL, result);
	tree_manager_write_node_dword(result, 0, 0);

	return 1;
}