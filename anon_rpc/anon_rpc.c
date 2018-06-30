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

mem_zone_ref			my_node = { PTR_INVALID };

OS_API_C_FUNC(int) set_node_anon_wallet(mem_zone_ref_ptr node, tpo_mod_file *pos_mod)
{
	my_node.zone = PTR_NULL;
	copy_zone_ref(&my_node, node);
	return 1;
}


int sign_tx_inputs(mem_zone_ref_ptr new_tx)
{
	mem_zone_ref inList = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr	input = PTR_NULL;
	unsigned int		nin;
	int				ret;

	if (!tree_manager_find_child_node(new_tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &inList))
		return 0;
	

	for (nin = 0, tree_manager_get_first_child(&inList, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input), nin++)
	{
		dh_key_t			privkey;
		hash_t				txh;
		hash_t				h;
		btc_addr_t			src_addr;
		struct string		script = { PTR_NULL }, pubk = { PTR_NULL }, signature = { 0 };
		uint64_t			inAmount;
		unsigned int		oIdx, cnt;

		tree_manager_get_child_value_hash	(input, NODE_HASH("txid"), h);
		tree_manager_get_child_value_i32	(input, NODE_HASH("idx"), &oIdx);

		ret = get_tx_output_script				(h, oIdx, &script, &inAmount);
		if(ret)ret= get_out_script_address		(&script, &pubk, src_addr);

		if(ret)
			compute_tx_sign_hash	(new_tx, nin, &script, 1, txh);

		free_string					(&script);

		if (ret) ret = get_anon_key	(src_addr, privkey);

		if (ret)
		{
			hash_t hh;
			
			for (cnt = 0; cnt < 32; cnt++)
			{
				hh[31 - cnt] = txh[cnt];
			}

			ret = sign_hash(hh, privkey, &signature);
		}

		if (ret)
		{
			struct string sscript = { PTR_NULL }, sign_seq = { PTR_NULL };
			mem_zone_ref script_node = { PTR_NULL };

			encode_DER_sig						(&signature, &sign_seq, 1, 1);
			tree_manager_create_node			("script", NODE_BITCORE_SCRIPT, &script_node);
			tree_manager_set_child_value_vstr	(&script_node, "var1", &sign_seq);

			if ((pubk.len == 0)||(pubk.str == PTR_NULL))
			{
				dh_key_t				mypub,mycpub, rpkey;
				mem_zone_ref			pkvar = { PTR_NULL };

				extract_pub		(privkey, mypub);
				compress_key	(mypub, mycpub);

				rpkey[0] = mycpub[0];
				for (cnt = 1; cnt < 33; cnt++)
				{
					rpkey[(32 - cnt) + 1] = mycpub[cnt];
				}

				if (tree_manager_add_child_node(&script_node, "pk", NODE_BITCORE_PUBKEY, &pkvar))
				{
					tree_manager_write_node_data(&pkvar, rpkey, 0, 33);
					release_zone_ref			(&pkvar);
				}
			}

			serialize_script					(&script_node, &sscript);
			tree_manager_set_child_value_vstr	(input, "script", &sscript);
			free_string							(&sscript);


			release_zone_ref	(&script_node);
			free_string			(&pubk);
			free_string			(&signature);
			free_string			(&sign_seq);
		}

		if (!ret)
		{
			dec_zone_ref(input);
			release_zone_ref(&my_list);
			break;
		}
	}
	release_zone_ref(&inList);
	
	return ret;
}


OS_API_C_FUNC(int) walletlock(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	return reset_anon_pw();
}




OS_API_C_FUNC(int) sendfrom(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	hash_t			 txHash;
	uint64_t		 total_unspent, nAmount, min_conf, paytxfee;
	btc_addr_t		 dstAddr,changeAddr;
	struct string	 oScript = { 0 };
	mem_zone_ref	 param = { PTR_NULL };
	mem_zone_ref	 account_name = { PTR_NULL }, mempool = { PTR_NULL }, new_tx = { PTR_NULL }, addrs = { PTR_NULL }, my_list = { PTR_NULL }, script_node = { PTR_NULL }, etx = { PTR_NULL };
	mem_zone_ref_ptr addr=PTR_NULL;
	int				 ret;
	
	
	if (tree_manager_get_node_num_children(params) < 3)return 0;

	memset_c						(changeAddr, 0, sizeof(btc_addr_t));

	tree_manager_get_child_at		(params, 0, &account_name);

	tree_manager_get_child_at		(params, 1, &param);
	tree_manager_get_node_btcaddr	(&param, 0, dstAddr);
	release_zone_ref				(&param);

	tree_manager_get_child_at		(params, 2, &param);
	tree_mamanger_get_node_qword	(&param, 0, &nAmount);
	release_zone_ref				(&param);

	if (tree_manager_get_child_at(params, 3, &param))
	{
		tree_mamanger_get_node_qword(&param, 0, &min_conf);
		release_zone_ref			(&param);
	}
	else
		min_conf = 1;

	if (!tree_manager_get_child_value_i64(&my_node, NODE_HASH("paytxfee"), &paytxfee))
		paytxfee = 0;


	tree_manager_create_node		("addrs", NODE_BITCORE_WALLET_ADDR_LIST, &addrs);

	wallet_list_addrs				(&account_name,&addrs,0);
	release_zone_ref				(&account_name);

	node_aquire_mempool_lock		(&mempool);
	new_transaction					(&new_tx, get_time_c());

	total_unspent = 0;
	for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
	{
		btc_addr_t						my_addr;

		tree_manager_get_child_value_btcaddr(addr, NODE_HASH("address"), my_addr);
		get_tx_inputs_from_addr				(my_addr, &mempool, &total_unspent, nAmount + paytxfee, min_conf, 9999999, &new_tx);

		if (changeAddr[0] == 0)
			memcpy_c (changeAddr, my_addr, sizeof(btc_addr_t));

		if (total_unspent > (nAmount + paytxfee))
		{
			dec_zone_ref(addr);
			release_zone_ref(&my_list);
			break;
		}
	}
	
	release_zone_ref			(&mempool);
	node_release_mempool_lock	();
	release_zone_ref			(&addrs);

	if (total_unspent < nAmount)
	{
		release_zone_ref(&new_tx);
		return 0;
	}

	if (tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node))
	{
		create_p2sh_script		(dstAddr, &script_node);
		serialize_script		(&script_node, &oScript);
		release_zone_ref		(&script_node);
	}

	tx_add_output				(&new_tx, nAmount, &oScript);
	free_string					(&oScript);

	if (total_unspent > nAmount)
	{
		if (tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node))
		{
			create_p2sh_script	(changeAddr, &script_node);
			serialize_script	(&script_node, &oScript);
			release_zone_ref	(&script_node);
		}

		tx_add_output	(&new_tx, total_unspent - (nAmount + paytxfee), &oScript);
		free_string		(&oScript);
	}

	ret = sign_tx_inputs	(&new_tx);

	if (ret)
	{
		compute_tx_hash						(&new_tx, txHash);
		tree_manager_set_child_value_hash	(&new_tx, "txid", txHash);
		ret = tree_manager_find_child_node	(&my_node, NODE_HASH("submitted txs"), NODE_BITCORE_TX_LIST, &etx);
	}

	if(ret)
	{
		tree_manager_node_add_child(&etx, &new_tx);
		release_zone_ref(&etx);
	}

	release_zone_ref(&new_tx);

	if(ret)
		tree_manager_set_child_value_hash(result, "txid", txHash);

	return ret;

}

OS_API_C_FUNC(int) dumpprivkey(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	dh_key_t	 key;
	struct string ekey = { 0 };
	btc_addr_t   pubaddr;
	mem_zone_ref param = { PTR_NULL };



	if (!tree_manager_get_child_at(params, 0, &param))
		return 0;

	tree_manager_get_node_btcaddr	(&param, 0, pubaddr);
	release_zone_ref				(&param);

	if (!get_anon_key(pubaddr, key))
		return 0;

	privkey_to_addr(key, &ekey);
	tree_manager_set_child_value_vstr(result, "privkey", &ekey);


	free_string(&ekey);

	return 1;

}
OS_API_C_FUNC(int) generatekeys(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	
	btc_addr_t   pubaddr;
	char         clabel[32];
	mem_zone_ref label_n = { PTR_NULL };
	int			 ret;

	memset_c					 (clabel, 0, 32);

	if (tree_manager_get_child_at(params, 0, &label_n))
	{
		tree_manager_get_node_str(&label_n, 0, clabel, 32, 0);
		release_zone_ref(&label_n);
	}
	else
	{
		strcat_cs(clabel, 32, "new address");
	}

	ret = generate_new_keypair(clabel, pubaddr);
	if (ret)
		tree_manager_set_child_value_btcaddr(result, "address", pubaddr);
	

	return ret;
}

OS_API_C_FUNC(int) accesstest(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	tree_manager_create_node		("result", NODE_GFX_BOOL, result);
	tree_manager_write_node_dword	(result, 0, 1);
	return 1;
}


OS_API_C_FUNC(int) walletpassphrase(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	struct string	password = { 0 };
	mem_zone_ref	pn = { PTR_NULL };
	unsigned int	timeout;
	int				ret;


	if (tree_manager_get_node_num_children(params) < 2)return 0;

	tree_manager_get_child_at	(params, 0, &pn);
	tree_manager_get_node_istr	(&pn, 0, &password, 0);
	release_zone_ref			(&pn);

	tree_manager_get_child_at	(params, 1, &pn);
	tree_mamanger_get_node_dword(&pn, 0, &timeout);
	release_zone_ref			(&pn);

	ret=set_anon_pw				(password.str, timeout);

	free_string					(&password);

	
	return ret;
}