//copyright antoine bentue-ferrer 2016
#include <base/std_def.h>
#include <base/std_mem.h>
#include <base/mem_base.h>
#include <base/std_str.h>


#include <strs.h>
#include <tree.h>

#include <http.h>
#include <fsio.h>
#include <mem_stream.h>
#include <tpo_mod.h>

#include <crypto.h>
#include "../block_adx/block_api.h"
#include "../node_adx/node_api.h"

C_IMPORT size_t			C_API_FUNC	  get_node_size(mem_zone_ref_ptr key);


typedef int  C_API_FUNC get_blk_staking_infos_func(mem_zone_ref_ptr blk, mem_zone_ref_ptr infos);
typedef get_blk_staking_infos_func *get_blk_staking_infos_func_ptr;


#ifdef _DEBUG
	C_IMPORT int					C_API_FUNC get_blk_staking_infos(mem_zone_ref_ptr blk, mem_zone_ref_ptr infos);
	get_blk_staking_infos_func_ptr  _get_blk_staking_infos = PTR_INVALID;
#else
	get_blk_staking_infos_func_ptr  get_blk_staking_infos = PTR_INVALID;
#endif
	

unsigned int			WALLET_VERSION = 60000;
mem_zone_ref			my_node = { PTR_INVALID };


OS_API_C_FUNC(int) set_node_block_explorer(mem_zone_ref_ptr node, tpo_mod_file *pos_mod)
{
	log_output("init block explorer module\n");
	
	my_node.zone = PTR_NULL;
	copy_zone_ref	(&my_node, node);

#ifndef _DEBUG
	get_blk_staking_infos = (get_blk_staking_infos_func_ptr)get_tpo_mod_exp_addr_name(pos_mod, "get_blk_staking_infos", 0);
#endif
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

OS_API_C_FUNC(int) block_index(const char *params, const struct http_req *req, mem_zone_ref_ptr result)
{
	unsigned int idx;
	hash_t		 hash;
	idx = strtoul_c(params, PTR_NULL, 10);
	

	if (!get_block_hash( idx, hash))return 0;
	tree_manager_set_child_value_hash(result, "blockHash", hash);
	return 1;
}

OS_API_C_FUNC(int) test(const char *params, const struct http_req *req, mem_zone_ref_ptr result)
{
	tree_manager_set_child_value_str(result, "text", "hello world");
	return 1;
}

int get_block_infos(mem_zone_ref_ptr block, const struct http_req *req, mem_zone_ref_ptr result)
{
	hash_t block_hash;
	hash_t merkle, proof, nullhash, nexthash, rdiff, hdiff, prev;
	mem_zone_ref txs = { PTR_NULL };
	size_t		n = 0;
	size_t		size, qv;
	unsigned int version, time, bits, nonce;
	uint64_t	 height;
	uint64_t	nblks;
	uint64_t reward;
	tree_manager_get_child_value_i64(&my_node, NODE_HASH("block_height"), &nblks);
	

	tree_manager_get_child_value_hash(block, NODE_HASH("blkHash"), block_hash);

	if (!get_block_size(block_hash, &size))
		size = 0;

	tree_manager_get_child_value_i64(block, NODE_HASH("height"), &height);

	if (tree_manager_find_child_node(block, NODE_HASH("blk pow"), NODE_BITCORE_HASH, PTR_NULL))
	{
		get_blockreward(height, &reward);
		tree_manager_set_child_value_i64(result, "reward", reward);
	}
	else
	{
		get_blk_staking_infos(block, result);
	}

	tree_manager_get_child_value_hash(block, NODE_HASH("merkle_root"), merkle);
	tree_manager_get_child_value_hash(block, NODE_HASH("prev"), prev);
	tree_manager_get_child_value_i32(block, NODE_HASH("version"), &version);
	tree_manager_get_child_value_i32(block, NODE_HASH("time"), &time);
	tree_manager_get_child_value_i32(block, NODE_HASH("bits"), &bits);
	tree_manager_get_child_value_i32(block, NODE_HASH("nonce"), &nonce);


	tree_manager_set_child_value_i64(result, "height", height);
	tree_manager_set_child_value_i32(result, "size", size);
	tree_manager_set_child_value_hash(result, "hash", block_hash);
	tree_manager_set_child_value_i32(result, "confirmations", nblks - height);
	tree_manager_set_child_value_i32(result, "time", time);
	tree_manager_set_child_value_i32(result, "version", version);
	tree_manager_set_child_value_i32(result, "bits", bits);
	tree_manager_set_child_value_i32(result, "nonce", nonce);
	if (req != PTR_NULL)
	{
		for (qv = 0; req->query_vars[qv].value.len > 0; qv++)
		{
			if (!tree_node_keval_i64(result, &req->query_vars[qv]))
				return 0;
		}
	}

	if (get_block_hash(height+1, nexthash) <= 0)
		memcpy_c(nexthash, nullhash, sizeof(hash_t));

	tree_manager_set_child_value_hash(result, "merkleroot", merkle);
	tree_manager_set_child_value_hash(result, "previousblockhash", prev);
	tree_manager_set_child_value_hash(result, "nextblockhash", nexthash);
	tree_manager_set_child_value_float(result, "difficulty", GetDifficulty(bits));


	if (tree_manager_get_child_value_hash(block, NODE_HASH("blk pow"), proof))
	{
		SetCompact(bits, hdiff);
		n = 32;
		while (n--)
		{
			rdiff[n] = hdiff[31 - n];
		}

		tree_manager_set_child_value_hash(result, "proofhash", proof);
		tree_manager_set_child_value_hash(result, "hbits", rdiff);
		tree_manager_set_child_value_bool(result, "isCoinbase", 1);
	}
	else if (get_blk_staking_infos)
	{
		tree_manager_set_child_value_bool(result, "isCoinbase", 0);
	}


	if (tree_manager_add_child_node(result, "tx", NODE_JSON_ARRAY, &txs))
	{
		get_blk_txs(block_hash, nblks, &txs, 1000);
		release_zone_ref(&txs);
	}

	return 1;

}

OS_API_C_FUNC(int) block(const char *params, const struct http_req *req, mem_zone_ref_ptr result)
{
	hash_t		block_hash;
	mem_zone_ref	blk = { PTR_NULL };
	size_t		n = 0;
	size_t		 nxt_prm_pos;
	int			ret;
	uint64_t	nblks;

	tree_manager_get_child_value_i64(&my_node, NODE_HASH("block_height"), &nblks);
	
	nxt_prm_pos = strlpos_c(params, 0, '/');
	if (nxt_prm_pos == INVALID_SIZE)
		nxt_prm_pos = strlen_c(params);

	if (nxt_prm_pos >= 64)
	{
		n = 0;
		while (n < 32)
		{
			char    hex[3];
			hex[0] = params[n * 2 + 0];
			hex[1] = params[n * 2 + 1];
			hex[2] = 0;
			block_hash[31-n] = strtoul_c(hex, PTR_NULL, 16);
			n++;
		}
	}
	else
	{
		get_block_hash(nblks - 1, block_hash);
	}
	if (!load_blk_hdr(&blk, block_hash))return 0;
	ret=get_block_infos(&blk, PTR_NULL, result);
	release_zone_ref(&blk);

	return ret;
	/*
	{   
		"chainwork" : "0000000000000000000000000000000000000000000998b7adec271cd0ea7258", 
		"nextblockhash" : "000000000000000013677449d7375ed22f9c66a94940328081412179795a1ac5", 
		"reward" : 25, 
		"isMainChain" : true, 
		"poolInfo" : {}
	}
	*/
}

int		get_tx	(mem_zone_ref_ptr my_tx,mem_zone_ref_ptr result)
{
	char			hexscript[2048];
	hash_t			tx_hash, nullhash, app_root_hash;
	mem_zone_ref	txout_list = { PTR_NULL }, txin_list = { PTR_NULL }, appRootHash = { PTR_NULL };
	uint64_t		height, nblks, blk_time;
	unsigned int	version, locktime, tx_time,has_app_root;
	size_t			size;

	size = get_node_size(my_tx);

	memset_c(nullhash, 0, sizeof(hash_t));

	compute_tx_hash					 (my_tx,tx_hash);
	tree_manager_set_child_value_hash(my_tx,"txid",tx_hash);

	get_tx_blk_height(tx_hash, &height, &blk_time, &tx_time);

	tree_manager_get_child_value_i64(&my_node, NODE_HASH("block_height"), &nblks);



	tree_manager_get_child_value_i32(my_tx, NODE_HASH("version")	, &version);
	tree_manager_get_child_value_i32(my_tx, NODE_HASH("locktime")	, &locktime);
	tree_manager_get_child_value_i32(my_tx, NODE_HASH("time")		, &tx_time);
	

	tree_manager_set_child_value_hash(result, "txid", tx_hash);

	tree_manager_set_child_value_i64 (result, "blockheight", height);
	tree_manager_set_child_value_i64 (result, "confirmations", nblks- height);
	tree_manager_set_child_value_i32 (result, "blocktime", blk_time);
	tree_manager_set_child_value_i32(result, "size", size);
	
	tree_manager_set_child_value_i32 (result, "time", tx_time);

	has_app_root = 0;

	if (tree_manager_create_node("hash", NODE_BITCORE_HASH, &appRootHash))
	{
		if (get_root_app(&appRootHash))
		{
			if (tree_manager_get_node_hash(&appRootHash, 0, app_root_hash))
			{
				has_app_root = 1;
			}
		}
		release_zone_ref(&appRootHash);
	}
	
	if (is_tx_null(my_tx))
	{
		tree_manager_set_child_value_bool(result, "isNull", 1);
		release_zone_ref(my_tx);
		return 1;
	}
	if (tree_manager_find_child_node(my_tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))
	{
		mem_zone_ref my_list = { PTR_NULL }, vin_list = { PTR_NULL };
		mem_zone_ref_ptr in = PTR_NULL;
		unsigned int nin = 0;

		tree_manager_add_child_node(result, "vin", NODE_JSON_ARRAY, &vin_list);
		for (nin = 0, tree_manager_get_first_child(&txin_list, &my_list, &in); ((in != PTR_NULL) && (in->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &in), nin++)
		{
			mem_zone_ref new_vin = { PTR_NULL };
			if (tree_manager_add_child_node(&vin_list, "vin", NODE_GFX_OBJECT, &new_vin))
			{
				hash_t phash;
				
				struct string script = { PTR_NULL };
				unsigned int seq,idx;

				tree_manager_get_child_value_hash(in, NODE_HASH("txid")	, phash);
				tree_manager_get_child_value_i32 (in, NODE_HASH("idx")		, &idx);
				tree_manager_get_child_value_i32 (in, NODE_HASH("sequence")	, &seq);
				

				if (!memcmp_c(phash, nullhash, 32))
				{
					tree_manager_get_child_value_istr(in, NODE_HASH("script"), &script,0);

					if ((script.len > 0)&&(script.len<1024))
						bin_2_hex(script.str, script.len, hexscript);
					else
						hexscript[0] = 0;

					tree_manager_set_child_value_str(&new_vin, "coinbase", hexscript);
					tree_manager_set_child_value_bool(result, "isCoinBase", 1);

					free_string(&script);
				}
				else if ((has_app_root == 1) && (!memcmp_c(app_root_hash, phash, sizeof(hash_t))))
				{
					struct string app_name = { 0 };
					tree_manager_get_child_value_istr(in, NODE_HASH("script"), &script, 0);

					if(get_app_name(&script, &app_name))
					{
						tree_manager_set_child_value_bool	(result, "isApp", 1);
						tree_manager_set_child_value_vstr	(result, "AppName", &app_name);

						tree_manager_set_child_value_bool	(&new_vin, "isApp", 1);
						tree_manager_set_child_value_vstr	(&new_vin, "appName", &app_name);
						free_string							(&app_name);
					}
					free_string(&script);
				}
				else
				{
					hash_t			prev_bhash;
					mem_zone_ref	addrs = { PTR_NULL }, prev_tx = { PTR_NULL };

					if (load_tx(&prev_tx, prev_bhash, phash))
					{
						mem_zone_ref vout = { PTR_NULL };
						uint64_t value;

						if (get_tx_output(&prev_tx, idx, &vout))
						{
							tree_manager_get_child_value_i64(&vout, NODE_HASH("value"), &value);
							tree_manager_set_child_value_i64(&new_vin, "value", value);

							if (tree_manager_add_child_node(&new_vin, "addresses", NODE_JSON_ARRAY, &addrs))
							{
								btc_addr_t addr;
								mem_zone_ref ad = { PTR_NULL };
								tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &script, 0);
								if (get_out_script_address(&script, PTR_NULL,addr))
								{
									if (tree_manager_add_child_node(&addrs, "addr", NODE_BITCORE_WALLET_ADDR, &ad))
									{
										tree_manager_write_node_btcaddr(&ad, 0, addr);
										release_zone_ref(&ad);
									}
								}
								free_string(&script);
								release_zone_ref(&addrs);
							}
							release_zone_ref(&vout);
						}
						release_zone_ref(&prev_tx);
					}
					tree_manager_set_child_value_hash	(&new_vin, "prevhash", phash);
					tree_manager_set_child_value_i32	(&new_vin, "idx", idx);
					tree_manager_set_child_value_bool	(result, "isCoinBase", 0);
				}
				
				tree_manager_set_child_value_i32(&new_vin, "n", nin);
				tree_manager_set_child_value_i32(&new_vin, "sequence", seq);
				release_zone_ref(&new_vin);
			}
		}
		release_zone_ref(&vin_list);
		release_zone_ref(&txin_list);
	}
	if (tree_manager_find_child_node(my_tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))
	{
		mem_zone_ref vout_list = { PTR_NULL }, my_list = { PTR_NULL };
		mem_zone_ref_ptr out = PTR_NULL;
		unsigned int nout = 0;

		tree_manager_add_child_node(result, "vout", NODE_JSON_ARRAY, &vout_list);
		for (nout = 0, tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != PTR_NULL) && (out->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &out), nout++)
		{
			mem_zone_ref new_vout = { PTR_NULL };
			struct		 string script = { PTR_NULL };
			uint64_t	 value;


			tree_manager_get_child_value_i64(out, NODE_HASH("value"), &value);
			tree_manager_get_child_value_istr(out, NODE_HASH("script"), &script, 0);


	

			if (tree_manager_add_child_node(&vout_list, "vout", NODE_GFX_OBJECT, &new_vout))
			{
				mem_zone_ref scriptkey = { PTR_NULL };
				tree_manager_set_child_value_i32(&new_vout, "n", nout);

				if ((value == 0xFFFFFFFF00000000) || (value == 0xFFFFFFFFFFFFFFFF))
				{
					tree_manager_set_child_value_i64(&new_vout, "value", 0);
					tree_manager_set_child_value_bool(&new_vout, "isNull", 0);
					tree_manager_add_child_node		(&new_vout, "addresses", NODE_JSON_ARRAY, PTR_NULL);
					hexscript[0] = 0;

					tree_manager_add_child_node		(&new_vout, "scriptPubKey", NODE_GFX_OBJECT, &scriptkey);
					tree_manager_set_child_value_str(&scriptkey, "hex", hexscript);
					release_zone_ref				(&scriptkey);
				}
				else
				{


					tree_manager_set_child_value_i64(&new_vout, "value", value);


					if ((value == 0) && (script.len == 0))
					{
						tree_manager_set_child_value_bool(&new_vout, "isNull", 1);
					}
					else
					{
						tree_manager_set_child_value_bool(&new_vout, "isNull", 0);
						if (tree_manager_add_child_node(&new_vout, "scriptPubKey", NODE_GFX_OBJECT, &scriptkey))
						{
							mem_zone_ref addrs = { PTR_NULL };

							if (script.len < 1024)
							{
								bin_2_hex(script.str, script.len, hexscript);
							}
							else
								hexscript[0] = 0;

							tree_manager_set_child_value_str(&scriptkey, "hex", hexscript);


							if (tree_manager_add_child_node(&new_vout, "addresses", NODE_JSON_ARRAY, &addrs))
							{
								btc_addr_t addr;
								mem_zone_ref ad = { PTR_NULL };
								int ret;

								ret = get_out_script_address(&script, PTR_NULL, addr);

								if (ret == 1)
									tree_manager_set_child_value_str(&scriptkey, "type", "pubkeyhash");

								if (ret == 2)
									tree_manager_set_child_value_str(&scriptkey, "type", "paytoscript");

								if (tree_manager_add_child_node(&addrs, "addr", NODE_BITCORE_WALLET_ADDR, &ad))
								{
									if (!tree_manager_write_node_btcaddr(&ad, 0, addr))
										memset_c(addr, 0, 32);
									release_zone_ref(&ad);
								}
								release_zone_ref(&addrs);
							}
							release_zone_ref(&scriptkey);
						}
					}
				}
				free_string(&script);
				release_zone_ref(&new_vout);
			}
		}
		release_zone_ref(&vout_list);
		release_zone_ref(&txout_list);
	}
	release_zone_ref(my_tx);
	return 1;
}


OS_API_C_FUNC(int) tx(const char *params, const struct http_req *req, mem_zone_ref_ptr result)
{
	hash_t			blk_hash, tx_hash;
	mem_zone_ref	my_tx = { PTR_NULL };
	size_t			nxt_prm_pos;
	int				ret,n;

	nxt_prm_pos = strlpos_c(params, 0, '/');
	if (nxt_prm_pos == INVALID_SIZE)
		nxt_prm_pos = strlen_c(params);

	if (nxt_prm_pos < 64)return 0;

	n = 0;
	while (n<32)
	{
		char    hex[3];
		hex[0] = params[(31 - n) * 2 + 0];
		hex[1] = params[(31 - n) * 2 + 1];
		hex[2] = 0;
		tx_hash[n] = strtoul_c(hex, PTR_NULL, 16);
		n++;
	}
	if (!load_tx(&my_tx, blk_hash, tx_hash))return 0;

	tree_manager_set_child_value_hash(result, "blockhash", blk_hash);
	ret=get_tx(&my_tx, result);
	release_zone_ref(&my_tx);
	
	return ret;
	/*
	{	
		"txid":"5756ff16e2b9f881cd15b8a7e478b4899965f87f553b6210d0f8e5bf5be7df1d", 
		"version" : 1, 
		"locktime" : 981825022, 
		"confirmations" : 62823,
		"time" : 1440604784,
		"isCoinBase" : true,
		"valueOut" : 25.37726812,
		"size" : 185

		"blockhash" : "0000000000000000027d0985fef71cbc05a5ee5cdbdc4c6baf2307e6c5db8591",
		"blockheight" : 371622,
		"blocktime" : 1440604784,
		"vin" : 
		[
			{"coinbase":"03a6ab05e4b883e5bda9e7a59ee4bb99e9b1bc76a3a2bb0e9c92f06e4a6349de9ccc8fbe0fad11133ed73c78ee12876334c13c02000000f09f909f2f4249503130302f4d696e65642062792073647a686162636400000000000000000000000000000000", 
			 "sequence" : 2765846367, 
			 "n" : 0}
		], 
		"vout" : 
		[
			{   
				"value":"25.37726812", 
				"n" : 0, 
				"scriptPubKey" : 
				{
					"hex":"76a914c825a1ecf2a6830c4401620c3a16f1995057c2ab88ac", 
					"asm" : "OP_DUP OP_HASH160 c825a1ecf2a6830c4401620c3a16f1995057c2ab OP_EQUALVERIFY OP_CHECKSIG", 
					"addresses" : ["1KFHE7w8BhaENAswwryaoccDb6qcT6DbYY"], 
					"type" : "pubkeyhash"
				}, 
				"spentTxId" : "a01b32246795ca47ed77ef78d56736677ec2f2aae7b400ebbcc95cd784492dc2", 
				"spentIndex" : 6, 
				"spentHeight" : 371831
			}
		], 
	}
	*/
}

OS_API_C_FUNC(int) addrbalance(const char *params, const struct http_req *req, mem_zone_ref_ptr result)
{
	btc_addr_t			addr;
	mem_zone_ref		txs = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr	ptx = PTR_NULL;
	uint64_t			recv, sent,ntx;
	size_t				nxt_prm_pos;

	nxt_prm_pos = strlpos_c(params, 0, '/');
	if (nxt_prm_pos == INVALID_SIZE)
		nxt_prm_pos = strlen_c(params);

	if (nxt_prm_pos < sizeof(btc_addr_t))return 0;

	memcpy_c(addr, params, sizeof(btc_addr_t));

	tree_manager_create_node("txs", NODE_BITCORE_HASH_LIST, &txs);
	load_tx_addresses(addr, &txs, 0, 1000);
	
	recv = 0;
	sent = 0;
	ntx = 0;
	
	for (tree_manager_get_first_child(&txs, &my_list, &ptx); ((ptx != PTR_NULL) && (ptx->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &ptx))
	{
		hash_t			txhash, blk_hash;
		mem_zone_ref	tx = { PTR_NULL };

		tree_manager_get_node_hash(ptx, 0, txhash);
		if (!load_tx(&tx, blk_hash, txhash))continue;

		get_tx_value	(&tx, addr, &recv, &sent);
		release_zone_ref(&tx);

		ntx++;
	}
	release_zone_ref(&txs);

	tree_manager_set_child_value_i64(result, "recv", recv);
	tree_manager_set_child_value_i64(result, "sent", sent);
	tree_manager_set_child_value_i32(result, "numtx", ntx);


	return 1;
}

OS_API_C_FUNC(int) txs(const char *params, const struct http_req *req, mem_zone_ref_ptr result)
{
	const struct key_val   *hdr;
	mem_zone_ref	  new_block = { PTR_NULL }, tx_list = { PTR_NULL };
	mem_zone_ref	  my_list	= { PTR_NULL };
	mem_zone_ref	  txs		= { PTR_NULL };
	mem_zone_ref_ptr  ptx		= PTR_NULL;
	size_t			  ntx,cur,idx,page_num, limit;
	uint64_t			nblks;
	tree_manager_add_child_node(result, "txs", NODE_JSON_ARRAY, &tx_list);

	if ((hdr = find_key(req->query_vars, "pageNum")) != PTR_NULL)
		page_num = strtoul_c(hdr->value.str,PTR_NULL,10);
	else
		page_num = 0;

	if ((hdr = find_key(req->query_vars, "limit")) != PTR_NULL)
		limit = strtoul_c(hdr->value.str,PTR_NULL,10);
	else
		limit = 10;

	tree_manager_get_child_value_i64(&my_node, NODE_HASH("block_height"), &nblks);

	if ((hdr = find_key(req->query_vars, "block")) != PTR_NULL)
	{
		hash_t block_hash;

		if (hdr->value.len == 64)
		{
			hex_2_bin_r(hdr->value.str, block_hash);
			tree_manager_create_node("txs", NODE_BITCORE_HASH_LIST, &txs);
			get_blk_txs(block_hash, nblks, &txs, limit);
		}
	}
	else if ((hdr = find_key(req->query_vars, "address")) != PTR_NULL)
	{
		tree_manager_create_node("txs", NODE_BITCORE_HASH_LIST, &txs);
		load_tx_addresses		(hdr->value.str, &txs, 0, 1000);
	}
	else
	{
		hash_t		 hash;
		
		unsigned int tidx;
		unsigned int time;
		unsigned int block_time;

		


		
		block_time = 0xFFFFFFFF;
		idx = nblks-1;
		if ((hdr = find_key(req->query_vars, "BlockDate")) != PTR_NULL)
		{
			time = parseDate(hdr->value.str);
			while ((block_time > (time + 24 * 3600)) && (idx > 1))
			{
				if (!get_block_time(idx, &block_time))
					break;

				idx--;
			}
			if (idx <= 1)
			{
				release_zone_ref(&tx_list);
				return 1;
			}
		}
		else
		{
			time = 0;
			get_block_time(idx, &block_time);
		}
			

		if (time > 0)
		{
			tree_manager_set_child_value_i32(result, "to"		, time);
			tree_manager_set_child_value_i32(result, "from"		, block_time);
		}
		tidx = 0;
		cur = 0;
		
	
		
		while ((block_time >= time) && (idx>1))
		{
			int ntx;
			
			get_block_hash				(idx,hash);
			ntx = get_blk_ntxs			(hash, nblks);

			if (((tidx + ntx) >= page_num*limit) && (cur<limit))
			{
				if (tree_manager_create_node("txs", NODE_BITCORE_HASH_LIST, &txs))
				{
					get_blk_txs(hash, nblks,&txs, limit - cur);
					
					for (tree_manager_get_first_child(&txs, &my_list, &ptx); ((ptx != PTR_NULL) && (ptx->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &ptx), tidx++)
					{
						char			prm[65],tx_hash[65];
						mem_zone_ref	my_tx = { PTR_NULL };
						unsigned int	n;

						memset_c(tx_hash, 0, 65);

						if (tidx < page_num*limit)continue;
						if (cur >= limit)continue;
						if (!tree_manager_get_node_str(ptx, 0, tx_hash, 65, 16))continue;
						for(n = 0;n < 32; n++)
						{
							prm[(31 - n) * 2 + 0] = tx_hash[n * 2 + 0];
							prm[(31 - n) * 2 + 1] = tx_hash[n * 2 + 1];
						}
						prm[64] = 0;

						if (tree_manager_add_child_node(&tx_list, "tx", NODE_GFX_OBJECT, &my_tx))
						{
							tx(prm, PTR_NULL, &my_tx);
							release_zone_ref(&my_tx);
						}
						cur++;
					}

					release_zone_ref(&txs);
				
				}
			}
			else
		
				tidx += ntx;
			
			if ((time == 0) && (cur>=limit))
				break;

			idx--;

			if (!get_block_time(idx, &block_time))
				break;
		}
	
		release_zone_ref(&tx_list);

		tree_manager_set_child_value_i32(result, "limit", limit);
		tree_manager_set_child_value_i32(result, "page_num", page_num);
		tree_manager_set_child_value_i32(result, "numtxs", tidx);
		return 1;
	}


	ntx = tree_manager_get_node_num_children(&txs);
	cur = 0;
	for (idx=0,tree_manager_get_first_child(&txs, &my_list, &ptx);((ptx != PTR_NULL) && (ptx->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &ptx),idx++)
	{
		hash_t			txhash, blk_hash;
		mem_zone_ref	tx = { PTR_NULL },my_tx = { PTR_NULL };
		int				n;

		if ((idx >= page_num*limit))
		{
			tree_manager_get_node_hash(ptx, 0, txhash);
			if (!load_tx(&tx, blk_hash, txhash))continue;

			tree_manager_add_child_node			(&tx_list, "tx", NODE_GFX_OBJECT, &my_tx);
			get_tx								(&tx	, &my_tx);
			tree_manager_set_child_value_hash	(&my_tx	, "blockhash", blk_hash);
			release_zone_ref					(&my_tx);
			release_zone_ref					(&tx);
			cur++;
		}

		if (cur >= limit)
		{
			release_zone_ref(&my_list);
			dec_zone_ref(ptx);
			break;
		}
	}

	tree_manager_set_child_value_i32(result, "numtx", ntx);


	release_zone_ref(&txs);
	release_zone_ref(&tx_list);

	return 1;
}

OS_API_C_FUNC(int) blocks(const char *params, const struct http_req *req, mem_zone_ref_ptr result)
{
	hash_t			nullhash;
	char			prm[65];
	mem_zone_ref	new_block = { PTR_NULL }, block_list = { PTR_NULL };
	struct string	blk_path = { PTR_NULL };
	ctime_t			time;
	uint64_t		nblks=0;
	size_t			page_num;
	unsigned int	 block_time, limit, num, idx,tidx,n,start_time,cur_time;
	const struct key_val *blockdate, *pageNum, *sinceblock, *beforeblock, *txl;

	tree_manager_get_child_value_i64	(&my_node, NODE_HASH("block_height"), &nblks);
	
	memset_c(nullhash, 0, sizeof(hash_t));

	if ((pageNum = find_key(req->query_vars, "pageNum")) != PTR_NULL)
		page_num = strtoul_c(pageNum->value.str,PTR_NULL,10);
	else
		page_num = 0;

	if (isdigit_c(params[0]))
		limit = strtoul_c(params, PTR_NULL, 10);
	else
		limit = 0;

	if ((limit < 1) || (limit > 10))
		limit = 10;

	txl = find_key(req->query_vars, "tx");


	tree_manager_add_child_node(result, "blocks", NODE_JSON_ARRAY, &block_list);
	
	if ((blockdate = find_key(req->query_vars, "BlockDate")) != PTR_NULL)
	{
		ctime_t		next_day;
		mem_zone_ref	my_blocks = { PTR_NULL }, my_list = { PTR_NULL };
		mem_zone_ref_ptr my_block = PTR_NULL;
		tidx = 0;
		time		= parseDate(blockdate->value.str);
		block_time	= 0xFFFFFFFF;
		next_day	= time + 24 * 3600;
		tree_manager_create_node		("blocks", NODE_BITCORE_BLK_HDR_LIST, &my_blocks);
		
		if (load_block_time_range(time, next_day, page_num*limit, limit, &tidx, &my_blocks))
		{
			for (tree_manager_get_first_child(&my_blocks, &my_list, &my_block); ((my_block != PTR_NULL) && (my_block->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &my_block))
			{
				if (tree_manager_add_child_node(&block_list, "block", NODE_GFX_OBJECT, &new_block))
				{
					get_block_infos(my_block, req, &new_block);
					release_zone_ref(&new_block);
				}
			}
		}

		release_zone_ref(&my_blocks);


		tree_manager_set_child_value_i32(result, "limit", limit);
		tree_manager_set_child_value_i32(result, "page_num", page_num);
		tree_manager_set_child_value_i32(result, "numblocks", tidx);
	}
	else 
	{
		hash_t phash, bhash,hash;
		size_t cur, qv, min_idx, max_idx;
		int		idx_inc;
		sinceblock  = find_key(req->query_vars, "SinceBlock");
		

		if ((sinceblock!=PTR_NULL)&&(sinceblock->value.len == 64))
		{
			n = 0;
			while (n < 32)
			{
				char    hex[3];

				hex[0]					= sinceblock->value.str[n * 2 + 0];
				hex[1]					= sinceblock->value.str[n * 2 + 1];
				hex[2]					= 0;

				phash[31 - n]			= strtoul_c(hex, PTR_NULL, 16);
				n++;
			}
		}
		idx = nblks;
		idx_inc = -1;
		min_idx = 0;
		max_idx = nblks;
		memset_c(hash, 0, sizeof(hash_t));


		beforeblock = find_key(req->query_vars, "BeforeBlock");
		if ((beforeblock != PTR_NULL) && (beforeblock->value.len == 64))
		{
			n = 0;
			while (n < 32)
			{
				char    hex[3];

				hex[0] = beforeblock->value.str[n * 2 + 0];
				hex[1] = beforeblock->value.str[n * 2 + 1];
				hex[2] = 0;

				bhash[31-n] = strtoul_c(hex, PTR_NULL, 16);
				n++;
			}
			while ((--idx) > 1)
			{
				hash_t block_hash;

				if (!get_block_hash(idx,block_hash))
					break;
				if (!memcmp_c(block_hash, bhash,32))
					break;
			}
		}

		for (qv = 0; req->query_vars[qv].value.len > 0; qv++)
		{
			if (!stricmp_c(req->query_vars[qv].key, "height"))
			{
				switch (req->query_vars[qv].op)
				{
					case CMP_L:idx_inc = -1; idx = strtoll_c(req->query_vars[qv].value.str, PTR_NULL, 10); break;
					case CMP_G:idx_inc =  1; idx = strtoll_c(req->query_vars[qv].value.str, PTR_NULL, 10)-1; break;
					case CMP_E:idx_inc = -1; idx = strtoll_c(req->query_vars[qv].value.str, PTR_NULL, 10); limit = 1; break;
				}
			}
			if (!stricmp_c(req->query_vars[qv].key, "time"))
			{
				ctime_t vtime;
				vtime = strtoll_c(req->query_vars[qv].value.str, PTR_NULL, 10);

				switch (req->query_vars[qv].op)
				{
					case CMP_L:
						while ((--idx) > 1)
						{
							if(!get_block_time(idx,&block_time))
								break;

							if (block_time < vtime)break;
						}
						idx_inc = -1;
					break;
					case CMP_E:
						while ((--idx) > 1)
						{
							if (!get_block_time(idx, &block_time))
								break;

							if (block_time == vtime)break;
						}
						idx_inc = -1;
						limit = 1;
					break;
					case CMP_G:
						idx = 0;
						while ((++idx) < nblks)
						{
							if (!get_block_time(idx, &block_time))
								break;

							if (block_time > vtime)
								break;
						}
						idx_inc = 1;
					break;
				}
			}
		}

		cur = 0;
		num = 0;

		start_time = get_time_c();

		while (( (cur_time = get_time_c()) - start_time) < 5)
		{
			int				ok;
			
			idx += idx_inc;
			if ((idx_inc < 0) && (idx < min_idx))break;
			if ((idx_inc > 0) && (idx > max_idx))break;

			if (!get_block_hash(idx, hash))break;
			if ((sinceblock != PTR_NULL) && (!memcmp_c(hash, phash, sizeof(hash_t))))break;
			
			ok = 1;
			if (txl != PTR_NULL)
			{
				int		ntx, ival;

				ival = strtol_c		(txl->value.str, PTR_NULL, 10);
				ntx  = get_blk_ntxs	(hash,nblks);
				switch (txl->op)
				{
					case CMPL_N:ok = (ntx != ival); break;
					case CMPL_E:ok = (ntx == ival); break;
					case CMPL_L:ok = (ntx < ival); break;
					case CMPL_G:ok = (ntx > ival); break;
					default:ok = 0; break;
				}
			}

			if (ok)
			{
				if (tree_manager_create_node("block", NODE_GFX_OBJECT, &new_block))
				{
					bin_2_hex_r(hash, 32, prm);
					if (block(prm, req, &new_block))
					{
						if ((num >= page_num*limit) && (cur < limit))
						{
							tree_manager_node_add_child(&block_list, &new_block);
							cur++;
						}
						num++;
					}
					release_zone_ref(&new_block);
				}
			}
		}

		tree_manager_set_child_value_hash(result, "lastblock", hash);
		tree_manager_set_child_value_i32(result, "lastblockidx", idx);
		tree_manager_set_child_value_i32(result, "limit", limit);
		tree_manager_set_child_value_i32(result, "page_num", page_num);
		tree_manager_set_child_value_i32(result, "numblocks", num);
	}
	release_zone_ref(&block_list);
	

	return 1;
}

