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
#include <cypher.h>
#include <mem_stream.h>
#include <tpo_mod.h>

#include "../block_adx/block_api.h"

#define WALLET_API C_EXPORT

#include "wallet_api.h"

struct key_entry
{
	char label[32];
	btc_addr_t addr;
	dh_key_t key;
};

mem_zone_ref			my_node = { PTR_INVALID };
mem_zone_ref			locked_input = { PTR_INVALID };
mem_zone_ref			wallet_messages = { PTR_INVALID };


C_IMPORT size_t			C_API_FUNC	compute_payload_size(mem_zone_ref_ptr key);


#ifdef _NATIVE_LINK_
C_IMPORT int			C_API_FUNC		get_last_stake_modifier(mem_zone_ref_ptr pindex, hash_t nStakeModifier, unsigned int *nModifierTime);
C_IMPORT int			C_API_FUNC		get_tx_pos_hash_data(mem_zone_ref_ptr hdr, const hash_t txHash, unsigned int OutIdx, struct string *hash_data, uint64_t *amount, hash_t out_diff);
C_IMPORT int			C_API_FUNC		get_blk_staking_infos(mem_zone_ref_ptr blk, mem_zone_ref_ptr infos);
C_IMPORT int			C_API_FUNC		store_tx_staking(mem_zone_ref_ptr tx, hash_t tx_hash, btc_addr_t stake_addr, uint64_t	stake_in);
C_IMPORT int			C_API_FUNC		get_target_spacing(unsigned int *target);
C_IMPORT unsigned int	C_API_FUNC		get_current_pos_difficulty();
C_IMPORT int			C_API_FUNC		get_stake_reward(uint64_t height, uint64_t *reward);
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
get_min_stake_depth_func_ptr		get_min_stake_depth = PTR_INVALID;
#endif




hash_t					nullh = { 0xFF };
btc_addr_t				nulladdr = {'0' };
unsigned int			WALLET_VERSION = 60000;
unsigned int			min_staking_depth = 2;
btc_addr_t				src_addr_list[4096] = { 0xCDFF };

char					anon_wallet_pw[1024] = { 0xFF };
unsigned int			wallet_pw_set = 0xFFFFFFFF;
unsigned int			wallet_pw_timeout = 0xFFFFFFFF;
unsigned int			last_wallet_msg_id = 1;
unsigned int			staking_enabled = 1;
ctime_t					wallet_last_staking = 0xFFFFFFFF;

OS_API_C_FUNC(int) reset_anon_pw()
{
	memset_c(anon_wallet_pw, 0, sizeof(anon_wallet_pw));
	wallet_pw_set = 0;
	wallet_pw_timeout = 0;
	staking_enabled = 0;

	return 1;
}
int has_valid_pw()
{
	if (!wallet_pw_set)
		return 0;

	if (get_time_c() > wallet_pw_timeout)
	{
		reset_anon_pw();
		return 0;
	}
	return 1;
}

int find_mempool_inputs(hash_t txid, unsigned int oidx)
{
	mem_zone_ref my_txlist = { PTR_NULL }, txlist = { PTR_NULL };
	mem_zone_ref_ptr tx = PTR_NULL;
	int ret = 0;

	mem_ptr mempool_lock;

	tree_manager_get_child_data_ptr(&my_node, NODE_HASH("mempool_lck"), &mempool_lock);
	while (!compare_z_exchange_c(mempool_lock, 1)) {}

	tree_manager_find_child_node(&my_node, NODE_HASH("mempool"), NODE_BITCORE_TX_LIST, &txlist);

	for (tree_manager_get_first_child(&txlist, &my_txlist, &tx); ((tx != PTR_NULL) && (tx->zone != PTR_NULL)); tree_manager_get_next_child(&my_txlist, &tx))
	{
		mem_zone_ref		txin_list = { PTR_NULL }, my_ilist = { PTR_NULL };
		mem_zone_ref_ptr	input = PTR_NULL;

		if (!tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))
			continue;

		ret = find_inner_inputs(&txin_list, txid, oidx);

		release_zone_ref(&txin_list);
		if (ret == 1)
		{
			dec_zone_ref(tx);
			release_zone_ref(&my_txlist);
			break;
		}
	}

	tree_manager_set_child_value_i32(&my_node, "mempool_lck", 0);
	return ret;
}
OS_API_C_FUNC(int) wallet_infos(unsigned int *staking,unsigned int *timeout)
{
	ctime_t now;

	if (!wallet_pw_set)
		return 0;

	now = get_time_c();

	if (now > wallet_pw_timeout)
	{
		reset_anon_pw();
		return 0;
	}

	*staking = staking_enabled;
	*timeout = wallet_pw_timeout - now;
	return 1;
}



OS_API_C_FUNC(int) init_wallet(mem_zone_ref_ptr node, tpo_mod_file *pos_mod)
{
	my_node.zone = PTR_NULL;
	copy_zone_ref(&my_node, node);

	locked_input.zone = PTR_NULL;
	wallet_messages.zone = PTR_NULL;

	tree_manager_create_node("locked", NODE_BITCORE_VINLIST, &locked_input);
	tree_manager_create_node("messages", NODE_BITCORE_MSG_LIST, &wallet_messages);



	
	
	reset_anon_pw();

	wallet_last_staking = get_time_c();
	

#ifndef _NATIVE_LINK_
	get_blk_staking_infos = (get_blk_staking_infos_func_ptr)get_tpo_mod_exp_addr_name(pos_mod, "get_blk_staking_infos", 0);
	store_tx_staking = (store_tx_staking_func_ptr)get_tpo_mod_exp_addr_name(pos_mod, "store_tx_staking", 0);
	get_tx_pos_hash_data = (get_tx_pos_hash_data_func_ptr)get_tpo_mod_exp_addr_name(pos_mod, "get_tx_pos_hash_data", 0);
	get_target_spacing = (get_target_spacing_func_ptr)get_tpo_mod_exp_addr_name(pos_mod, "get_target_spacing", 0);
	get_stake_reward = (get_stake_reward_func_ptr)get_tpo_mod_exp_addr_name(pos_mod, "get_stake_reward", 0);
	get_last_stake_modifier = (get_last_stake_modifier_func_ptr)get_tpo_mod_exp_addr_name(pos_mod, "get_last_stake_modifier", 0);
	get_current_pos_difficulty = (get_current_pos_difficulty_func_ptr)get_tpo_mod_exp_addr_name(pos_mod, "get_current_pos_difficulty", 0);
	check_tx_pos = (check_tx_pos_func_ptr)get_tpo_mod_exp_addr_name(pos_mod, "check_tx_pos", 0);
	create_pos_block = (create_pos_block_func_ptr)get_tpo_mod_exp_addr_name(pos_mod, "create_pos_block", 0);
	get_min_stake_depth = (get_min_stake_depth_func_ptr)get_tpo_mod_exp_addr_name(pos_mod, "get_min_stake_depth", 0);
#endif
	if (get_min_stake_depth != PTR_NULL)
		get_min_stake_depth(&min_staking_depth);

	memset_c(nullh, 0, sizeof(hash_t));
	memset_c(nulladdr, '0', sizeof(btc_addr_t));
	return 1;
}

OS_API_C_FUNC(int)  find_stake_hash(hash_t hash, unsigned char *stakes, unsigned int len)
{
	unsigned int n = 0;
	if (len == 0)return 0;
	if (stakes == PTR_NULL)return 0;
	while (n<len)
	{
		if (!memcmp_c(&stakes[n + 8], hash, sizeof(hash_t)))
			return 1;
		n += 40;
	}
	return 0;
}


int create_new_unspent_message(mem_zone_ref_ptr unspent, mem_zone_ref_ptr new_unspent_pack)
{
	mem_zone_ref		payload = { PTR_NULL };
	size_t				pl_size;

	if (!tree_manager_create_node("message", NODE_BITCORE_MSG, new_unspent_pack))return 0;

	tree_manager_set_child_value_str(new_unspent_pack, "cmd", "newunspent");
	tree_manager_add_child_node(new_unspent_pack, "payload", NODE_BITCORE_PAYLOAD, &payload);
	tree_manager_node_add_child(&payload, unspent);
	pl_size = compute_payload_size(&payload);
	release_zone_ref(&payload);

	tree_manager_set_child_value_i32(new_unspent_pack, "size", pl_size);
	tree_manager_set_child_value_i32(new_unspent_pack, "sent", 0);

	return 1;

}


int create_new_spent_message(mem_zone_ref_ptr spent, mem_zone_ref_ptr new_spent_pack)
{
	mem_zone_ref		payload = { PTR_NULL };
	size_t				pl_size;

	if (!tree_manager_create_node("message", NODE_BITCORE_MSG, new_spent_pack))return 0;

	tree_manager_set_child_value_str(new_spent_pack, "cmd", "newspent");
	tree_manager_add_child_node(new_spent_pack, "payload", NODE_BITCORE_PAYLOAD, &payload);
	tree_manager_node_add_child(&payload, spent);
	pl_size = compute_payload_size(&payload);
	release_zone_ref(&payload);

	tree_manager_set_child_value_i32(new_spent_pack, "size", pl_size);
	tree_manager_set_child_value_i32(new_spent_pack, "sent", 0);

	return 1;

}


int  lock_unspent(hash_t txid,unsigned int oidx, unsigned int time)
{
	char			txh[65];
	btc_addr_t		srcAddr,addr;
	struct string	script = { 0 };
	uint64_t		amnt;
	mem_zone_ref	new_lock = { PTR_NULL };
	ctime_t			now;
	int				ret;

	bin_2_hex(txid, 32, txh);

	if (!load_utxo(txh, oidx, &amnt, addr, PTR_NULL))
		return 0;

	if (get_tx_output_script(txid, oidx, &script, &amnt))
	{
		get_out_script_address(&script, PTR_NULL, srcAddr);
		free_string(&script);
	}
	else
	{
		memset_c(srcAddr, '0', sizeof(btc_addr_t));
	}

	now = get_time_c();
	
	tree_manager_create_node				("spent", NODE_BITCORE_TXIN, &new_lock);
	tree_manager_set_child_value_hash		(&new_lock, "txid", txid);
	tree_manager_set_child_value_i32		(&new_lock, "idx", oidx);
	tree_manager_set_child_value_btcaddr	(&new_lock, "addr", srcAddr);
	tree_manager_set_child_value_i64		(&new_lock, "amount", amnt);

	tree_manager_set_child_value_i32		(&new_lock, "time", now + time);
	tree_manager_set_child_value_i32		(&new_lock, "done", 0);
	ret = tree_manager_node_add_unique_child(&locked_input, &new_lock, "[txid,idx]");

	if (ret)
	{
		mem_zone_ref new_spent_message = { PTR_NULL };
		if (create_new_spent_message(&new_lock, &new_spent_message))
		{
			tree_manager_set_child_value_i32(&new_spent_message, "id", last_wallet_msg_id++);
			tree_manager_set_child_value_i32(&new_spent_message, "sent_time", get_time_c());
			ret = tree_manager_node_ins_child(&wallet_messages, &new_spent_message);
			release_zone_ref(&new_spent_message);
		}
	}

	release_zone_ref					(&new_lock);

	return ret;

}


int  is_locked_unspent(hash_t txid, unsigned int oidx)
{
	return find_inner_inputs(&locked_input, txid, oidx);
}
OS_API_C_FUNC(int) cancel_tx_lock(mem_zone_ref_ptr tx)
{
	mem_zone_ref	 my_list = { PTR_NULL }, inList = { PTR_NULL };
	mem_zone_ref_ptr input = PTR_NULL;

	if (!tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &inList))
		return 0;


	for (tree_manager_get_first_child(&inList, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input))
	{
		hash_t			 h1;
		mem_zone_ref	 locked_input = { PTR_NULL }, my_ilist = { PTR_NULL };
		unsigned int	 i1;

		tree_manager_get_child_value_hash(input, NODE_HASH("txid"), h1);
		tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &i1);
		unlock_unspent(h1, i1, 1);

	}

	release_zone_ref(&inList);

	return 1;

}

OS_API_C_FUNC(int)  unlock_unspent(hash_t txid, unsigned int oidx, unsigned int send_msg)
{
	mem_zone_ref	 my_ilist = { PTR_NULL };
	mem_zone_ref_ptr iinput = PTR_NULL, iilist_ptr = PTR_NULL;

	if ((get_zone_area_type(&locked_input) & 0x10) == 0)
		iilist_ptr = &my_ilist;

	for (tree_manager_get_first_child_shared(&locked_input, &iilist_ptr, &iinput); ((iinput != NULL) && (iinput->zone != NULL)); tree_manager_get_next_child_shared(&iilist_ptr, &iinput))
	{
		hash_t h;
		mem_zone_ref new_spent_message = { PTR_NULL };
		unsigned int idx, done;

		if (!tree_manager_get_child_value_i32(iinput, NODE_HASH("done"), &done))
			done = 0;

		if (done != 0)
			continue;

		tree_manager_get_child_value_i32(iinput, NODE_HASH("idx"), &idx);

		if (oidx != idx)
			continue;

		tree_manager_get_child_value_hash(iinput, NODE_HASH("txid"), h);

		if (memcmp_c(txid, h, 32))
			continue;


		tree_manager_set_child_value_i32(iinput, "done", 1);

		if (!send_msg)
			continue;

		tree_manager_set_child_value_i32(iinput, "canceled", 1);

		if (create_new_spent_message(iinput, &new_spent_message))
		{
			tree_manager_set_child_value_i32(&new_spent_message, "id", last_wallet_msg_id++);
			tree_manager_set_child_value_i32(&new_spent_message, "sent_time", get_time_c());
			tree_manager_node_ins_child(&wallet_messages, &new_spent_message);
			release_zone_ref(&new_spent_message);
		}

	}

	return 1;
}

OS_API_C_FUNC(int) send_tx_messages(mem_zone_ref_ptr tx)
{
	mem_zone_ref inList = { PTR_NULL }, outList = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr input = PTR_NULL, output = PTR_NULL;
	ctime_t now;

	now = get_time_c();

	tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &inList);
	for (tree_manager_get_first_child(&inList, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input))
	{
		hash_t			 h1;
		unsigned int	 i1;

		tree_manager_get_child_value_hash(input, NODE_HASH("txid"), h1);
		tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &i1);

		if (i1 == 0xFFFFFFFF)
			continue;
		
		if (!is_locked_unspent(h1, i1))
		{
			btc_addr_t srcAddr;
			mem_zone_ref new_spent = { PTR_NULL },new_spent_message = { PTR_NULL };
			uint64_t value;

			tree_manager_get_child_value_btcaddr(input, NODE_HASH("srcaddr"), srcAddr);
			tree_manager_get_child_value_i64	(input, NODE_HASH("value"), &value);
			


			tree_manager_create_node("spent", NODE_BITCORE_TXIN, &new_spent);
			tree_manager_set_child_value_hash(&new_spent, "txid", h1);
			tree_manager_set_child_value_i32(&new_spent, "idx", i1);

			tree_manager_set_child_value_btcaddr(&new_spent, "addr", srcAddr);
			tree_manager_set_child_value_i64(&new_spent, "amount", value);

			tree_manager_set_child_value_i32(&new_spent, "time", now );
			tree_manager_set_child_value_i32(&new_spent, "done", 0);

			if (create_new_spent_message(&new_spent, &new_spent_message))
			{
				tree_manager_set_child_value_i32(&new_spent_message, "id", last_wallet_msg_id++);
				tree_manager_set_child_value_i32(&new_spent_message, "sent_time", get_time_c());
				tree_manager_node_ins_child(&wallet_messages, &new_spent_message);
				release_zone_ref(&new_spent_message);
			}

			release_zone_ref(&new_spent);
		}
	}
	release_zone_ref(&inList);

	tree_manager_find_child_node(tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &outList);

	for (tree_manager_get_first_child(&outList, &my_list, &output); ((output != PTR_NULL) && (output->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &output))
	{
		btc_addr_t		addr;
		uint64_t		value;
		mem_zone_ref	unspent = { PTR_NULL };
		mem_zone_ref new_unspent_message = { PTR_NULL };

		tree_manager_get_child_value_i64(output, NODE_HASH("value"), &value);

		if ((value == 0) || (value == 0xFFFFFFFFFFFFFFFF) || ((value & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000))
			continue;

		if (!tree_manager_get_child_value_btcaddr(output, NODE_HASH("dstaddr"), addr))
			continue;

		tree_manager_create_node("unspent", NODE_GFX_OBJECT, &unspent);

		tree_manager_set_child_value_i64(&unspent, "amount", value);
		tree_manager_set_child_value_btcaddr(&unspent, "addr", addr);

		if (create_new_unspent_message(&unspent, &new_unspent_message))
		{
			tree_manager_set_child_value_i32(&new_unspent_message, "id", last_wallet_msg_id++);
			tree_manager_set_child_value_i32(&new_unspent_message, "sent_time", get_time_c());
			tree_manager_node_ins_child(&wallet_messages, &new_unspent_message);
			release_zone_ref(&new_unspent_message);
		}

		release_zone_ref(&unspent);
	}
	release_zone_ref(&outList);

	return 1;
}

OS_API_C_FUNC(int)  wallet_pop_message(mem_zone_ref_ptr msg)
{
	return tree_manager_pop_child(&wallet_messages, msg);
}

OS_API_C_FUNC(int)  wallet_clear_locked_inputs()
{
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref_ptr	child_list_ptr = PTR_NULL, input = PTR_NULL;
	ctime_t				now=get_time_c();

	if ((get_zone_area_type(&locked_input) & 0x10) == 0)
		child_list_ptr = &my_list;

	for (tree_manager_get_first_child_shared(&locked_input, &child_list_ptr, &input); ((input != NULL) && (input->zone != NULL)); tree_manager_get_next_child_shared(&child_list_ptr, &input))
	{
		unsigned int done, t;

		if (!tree_manager_get_child_value_i32(input, NODE_HASH("done"), &done))
			done = 0;

		if (done != 0)
			continue;

		if (!tree_manager_get_child_value_i32(input, NODE_HASH("time"), &t))
			t = 0;

		if (t < now)
		{
			mem_zone_ref new_spent_message = { PTR_NULL };

			tree_manager_set_child_value_i32(input, "canceled", 1);

			if (create_new_spent_message(input, &new_spent_message))
			{
				tree_manager_set_child_value_i32(&new_spent_message, "id", last_wallet_msg_id++);
				tree_manager_set_child_value_i32(&new_spent_message, "sent_time", get_time_c());
				tree_manager_node_ins_child(&wallet_messages, &new_spent_message);
				release_zone_ref(&new_spent_message);
			}
			tree_manager_set_child_value_i32(input, "done", 1);
		}

	}

	tree_remove_child_by_member_value_dword(&locked_input, NODE_BITCORE_TXIN, "done", 1);

	return 1;
}

OS_API_C_FUNC(int)  get_tx_inputs_from_addr(btc_addr_t addr, uint64_t *total_unspent, uint64_t min_amount, size_t min_conf, size_t max_conf, mem_zone_ref_ptr tx)
{
	mem_zone_ref		new_addr = { PTR_NULL };
	struct string		unspent_path = { 0 }, user_key_file = { 0 };
	unsigned int		n;
	unsigned int		dir_list_len;
	struct string		dir_list = { PTR_NULL };
	const char			*ptr, *optr;
	size_t				cur, nfiles;
	unsigned int		sheight;

	tree_manager_get_child_value_i32(&my_node, NODE_HASH("block_height"), &sheight);


	make_string		(&unspent_path, "adrs");
	cat_ncstring_p	(&unspent_path, addr, 34);
	cat_cstring_p	(&unspent_path, "unspent");

	if (stat_file(unspent_path.str) != 0)
	{
		free_string(&unspent_path);
		return 0;
	}

	nfiles = get_sub_files(unspent_path.str, &dir_list);
	dir_list_len = dir_list.len;
	optr = dir_list.str;
	cur = 0;

	while ((cur < nfiles) && ((*total_unspent)<min_amount))
	{
		hash_t			hash;
		uint64_t		height, block_time, nconf;
		struct string	tx_path = { 0 };
		unsigned int	output = 0xFFFFFFFF, tx_time;
		size_t			sz, len;
		unsigned char	*data;

		ptr = memchr_c(optr, 10, dir_list_len);
		sz = mem_sub(optr, ptr);

		if (optr[64] == '_')
			output = strtoul_c(&optr[65], PTR_NULL, 10);
		else
			output = 0xFFFFFFFF;

		n = 0;
		while (n < 32)
		{
			char    hex[3];
			hex[0] = optr[n * 2 + 0];
			hex[1] = optr[n * 2 + 1];
			hex[2] = 0;
			hash[n] = strtoul_c(hex, PTR_NULL, 16);
			n++;
		}

		if ((!find_mempool_inputs(hash, output)) && (check_utxo(optr, output)))
		{
			if (get_tx_blk_height(hash, &height, &block_time, &tx_time))
				nconf = sheight - height;
			else
			{
				block_time = 0;
				tx_time = 0;
				nconf = 0;
			}

			clone_string(&tx_path, &unspent_path);
			cat_ncstring_p(&tx_path, optr, sz);

			if ((nconf >= min_conf) && (nconf <= max_conf))
			{
				if (get_file(tx_path.str, &data, &len) > 0)
				{
					if (len >= sizeof(uint64_t))
					{
						if (lock_unspent(hash, output, 120))
						{
							*total_unspent += *((uint64_t*)data);
							tx_add_input(tx, hash, output, PTR_NULL);
						}
					}
					free_c(data);
				}
			}
			free_string(&tx_path);

			if ((*total_unspent) >= min_amount)break;
		}
		cur++;
		optr = ptr + 1;
		dir_list_len -= sz;

		do_mark_sweep(get_tree_mem_area_id(), 250);
	}
	free_string(&dir_list);
	free_string(&unspent_path);

	return 1;
}
OS_API_C_FUNC(int)  list_obj(btc_addr_t addr,const char *appName, unsigned int appType, mem_zone_ref_ptr unspents, size_t min_conf, size_t max_conf, size_t *ntx, size_t *max, size_t first)
{
	struct string		unspent_path = { 0 };
	unsigned int		n;
	unsigned int		dir_list_len;
	struct string		dir_list = { PTR_NULL };
	const char			*ptr, *optr;
	size_t				cur, nfiles;
	uint64_t			sheight;

	make_string(&unspent_path, "adrs");
	cat_ncstring_p(&unspent_path, addr, 34);
	cat_cstring_p(&unspent_path, "objects");

	if (stat_file(unspent_path.str) != 0)
	{
		free_string(&unspent_path);
		return 0;
	}

	sheight = get_last_block_height();
	nfiles = get_sub_files(unspent_path.str, &dir_list);

	dir_list_len = dir_list.len;
	optr = dir_list.str;
	cur = 0;
	while (cur < nfiles)
	{
		hash_t			hash;
		uint64_t		height, block_time, nconf;
		unsigned int	tx_time;
		struct string	tx_path = { 0 };
		unsigned int	output = 0xFFFFFFFF;
		size_t			sz, len;
		unsigned char	*data;

		ptr = memchr_c(optr, 10, dir_list_len);
		sz = mem_sub(optr, ptr);

		if (optr[64] == '_')
			output = strtoul_c(&optr[65], PTR_NULL, 10);
		else
			output = 0xFFFFFFFF;


		n = 0;
		while (n<32)
		{
			char    hex[3];
			hex[0] = optr[n * 2 + 0];
			hex[1] = optr[n * 2 + 1];
			hex[2] = 0;
			hash[n] = strtoul_c(hex, PTR_NULL, 16);
			n++;
		}
		if (!find_mempool_inputs(hash, output))
		{
			if (get_tx_blk_height(hash, &height, &block_time, &tx_time))
				nconf = sheight - height;
			else
			{
				block_time = 0;
				tx_time = 0;
				nconf = 0;
			}

			clone_string(&tx_path, &unspent_path);
			cat_ncstring_p(&tx_path, optr, sz);

			if (get_file(tx_path.str, &data, &len) > 0)
			{
				if (len >= 68)
				{
					if (	((appName == PTR_NULL)  ||	(!strcmp_c(appName, data)))&&
							((appType == 0)			||	(appType== (*((unsigned int *)(data + 32))))))
					{
						(*ntx)++;
						if (((*ntx) >= first) && ((*max) > 0) && (nconf >= min_conf) && (nconf <= max_conf))
						{
							mem_zone_ref	unspent = { PTR_NULL };
							(*max)--;
							if (tree_manager_add_child_node(unspents, "object", NODE_GFX_OBJECT, &unspent))
							{
								tree_manager_set_child_value_hash(&unspent, "txid", hash);
								tree_manager_set_child_value_i32(&unspent, "vout", output);
								tree_manager_set_child_value_str(&unspent, "appObj", data);
								tree_manager_set_child_value_i32(&unspent, "objType", *((unsigned int *)(data + 32)));
								tree_manager_set_child_value_hash(&unspent, "objHash", data + 36);
								tree_manager_set_child_value_i32(&unspent, "time", tx_time);
								tree_manager_set_child_value_i64(&unspent, "confirmations", nconf);

								tree_manager_set_child_value_btcaddr(&unspent, "dstaddr", addr);
								tree_manager_set_child_value_btcaddr(&unspent, "srcaddr", data + 68);

								len -= 102;
								release_zone_ref(&unspent);
							}
						}
					}
					free_c(data);
				}
			}
			free_string(&tx_path);
		}
		cur++;
		optr = ptr + 1;
		dir_list_len -= sz;
	}
	free_string(&dir_list);
	free_string(&unspent_path);

	return 1;
}



OS_API_C_FUNC(int)  list_unspent(btc_addr_t addr, mem_zone_ref_ptr unspents, size_t min_conf, size_t max_conf, uint64_t *total_unspent, size_t *ntx, size_t *max, size_t first)
{
	struct string		unspent_path = { 0 };
	unsigned int		n;
	unsigned int		dir_list_len;
	struct string		dir_list = { PTR_NULL };
	const char			*ptr, *optr;
	size_t				cur, nfiles;
	uint64_t			sheight;

	make_string(&unspent_path, "adrs");
	cat_ncstring_p(&unspent_path, addr, 34);
	cat_cstring_p(&unspent_path, "unspent");

	if (stat_file(unspent_path.str) != 0)
	{
		free_string(&unspent_path);
		return 0;
	}

	sheight = get_last_block_height();
	nfiles = get_sub_files(unspent_path.str, &dir_list);

	dir_list_len = dir_list.len;
	optr = dir_list.str;
	cur = 0;
	while (cur < nfiles)
	{
		hash_t			hash;
		uint64_t		height, block_time, nconf;
		unsigned int	n_addrs, tx_time;
		struct string	tx_path = { 0 };
		unsigned int	output = 0xFFFFFFFF;
		size_t			sz, len;
		unsigned char	*data;

		ptr = memchr_c(optr, 10, dir_list_len);
		sz = mem_sub(optr, ptr);

		if (optr[64] == '_')
			output = strtoul_c(&optr[65], PTR_NULL, 10);
		else
			output = 0xFFFFFFFF;


		n = 0;
		while (n<32)
		{
			char    hex[3];
			hex[0] = optr[n * 2 + 0];
			hex[1] = optr[n * 2 + 1];
			hex[2] = 0;
			hash[n] = strtoul_c(hex, PTR_NULL, 16);
			n++;
		}
		if (!find_mempool_inputs(hash, output))
		{
			if (get_tx_blk_height(hash, &height, &block_time, &tx_time))
				nconf = sheight - height;
			else
			{
				block_time = 0;
				tx_time = 0;
				nconf = 0;
			}

			clone_string(&tx_path, &unspent_path);
			cat_ncstring_p(&tx_path, optr, sz);

			if (get_file(tx_path.str, &data, &len) > 0)
			{
				if (len >= sizeof(uint64_t))
				{
					(*ntx)++;
					*total_unspent += *((uint64_t*)data);
					if (((*ntx) >= first) && ((*max) > 0) && (nconf >= min_conf) && (nconf <= max_conf))
					{
						mem_zone_ref	unspent = { PTR_NULL };
						(*max)--;
						if (tree_manager_add_child_node(unspents, "unspent", NODE_GFX_OBJECT, &unspent))
						{
							tree_manager_set_child_value_hash(&unspent, "txid", hash);
							tree_manager_set_child_value_i32(&unspent, "vout", output);
							tree_manager_set_child_value_i64(&unspent, "amount", *((uint64_t*)data));

							tree_manager_set_child_value_i32(&unspent, "time", tx_time);
							tree_manager_set_child_value_i64(&unspent, "confirmations", nconf);

							tree_manager_set_child_value_btcaddr(&unspent, "dstaddr", addr);

							len -= sizeof(uint64_t);
							if (len > 4)
							{
								n_addrs = *((unsigned int *)(data + sizeof(uint64_t)));
								if (n_addrs > 0)
								{
									mem_zone_ref addr_list = { PTR_NULL };

									if (tree_manager_add_child_node(&unspent, "addresses", NODE_JSON_ARRAY, &addr_list))
									{
										mem_ptr addrs;
										addrs = data + sizeof(uint64_t) + sizeof(unsigned int);
										for (n = 0; n < n_addrs; n++)
										{
											mem_zone_ref new_addr = { PTR_NULL };
											if (tree_manager_add_child_node(&addr_list, "addr", NODE_BITCORE_WALLET_ADDR, &new_addr))
											{
												tree_manager_write_node_btcaddr(&new_addr, 0, addrs);
												release_zone_ref(&new_addr);
											}
											addrs = mem_add(addrs, sizeof(btc_addr_t));
										}
										release_zone_ref(&addr_list);
									}
								}
							}
							release_zone_ref(&unspent);
						}
					}

					do_mark_sweep(get_tree_mem_area_id(), 500);
					free_c(data);
				}
			}
			free_string(&tx_path);
		}
		cur++;
		optr = ptr + 1;
		dir_list_len -= sz;
	}
	free_string(&dir_list);
	free_string(&unspent_path);

	return 1;
}



OS_API_C_FUNC(int)  list_spent(btc_addr_t addr, mem_zone_ref_ptr spents, size_t min_conf, size_t max_conf, uint64_t *total_spent, size_t *ntx, size_t *max,size_t first)
{
	struct string		spent_path = { 0 };
	unsigned int		dir_list_len;
	struct string		dir_list = { PTR_NULL };
	const char			*ptr, *optr;
	size_t				len_stakes;
	unsigned char		*stakes= PTR_NULL;
	size_t				cur, nfiles;
	uint64_t			sheight;
	struct string		stake_path = { 0 };

	make_string(&spent_path, "adrs");
	cat_ncstring_p(&spent_path, addr, 34);
	cat_cstring_p(&spent_path, "spent");

	if (stat_file(spent_path.str) != 0)
	{
		free_string(&spent_path);
		return 0;
	}

	sheight = get_last_block_height();


	make_string(&stake_path, "adrs");
	cat_ncstring_p(&stake_path, addr, 34);
	cat_cstring_p(&stake_path, "stakes");
	get_file(stake_path.str, &stakes, &len_stakes);
	free_string(&stake_path);

	nfiles = get_sub_files(spent_path.str, &dir_list);

	dir_list_len = dir_list.len;
	optr = dir_list.str;
	cur = 0;
	while (cur < nfiles)
	{
		struct string	tx_path = { 0 };
		unsigned int	vin = 0xFFFFFFFF;
		unsigned int	prev_out = 0xFFFFFFFF;
		size_t			sz, len;
		unsigned char	*data;
		int				n;


		ptr = memchr_c(optr, 10, dir_list_len);
		sz = mem_sub(optr, ptr);

		clone_string(&tx_path, &spent_path);
		cat_ncstring_p(&tx_path, optr, sz);
		if (get_file(tx_path.str, &data, &len) > 0)
		{
			hash_t		thash;
			uint64_t	height, block_time, nconf;
			unsigned int tx_time;

			if (optr[64] == '_')
				prev_out = strtoul_c(&optr[65], PTR_NULL, 10);
			else
				prev_out = 0xFFFFFFFF;

			n = 0;
			while (n<32)
			{
				char    hex[3];
				hex[0] = optr[n * 2 + 0];
				hex[1] = optr[n * 2 + 1];
				hex[2] = 0;
				thash[n] = strtoul_c(hex, PTR_NULL, 16);
				n++;
			}

			if (get_tx_blk_height(thash, &height, &block_time, &tx_time))
				nconf = sheight - height;
			else
			{
				block_time = 0;
				tx_time = 0;
				nconf = 0;
			}

			if (len >= sizeof(uint64_t))
			{
				hash_t		  hash;
				mem_zone_ref  spent = { PTR_NULL };
				unsigned int  n_in_addr;
				unsigned char *cdata;

				cdata = data + sizeof(uint64_t);
				n_in_addr = *((unsigned int *)(cdata));

				if ((n_in_addr * sizeof(btc_addr_t)) > len)
				{
					free_c(data);
					free_string(&tx_path);
					cur++;
					optr = ptr + 1;
					dir_list_len -= sz;
					continue;
				}

				cdata += sizeof(unsigned int) + n_in_addr*sizeof(btc_addr_t);
				memcpy_c(hash, cdata, sizeof(hash_t));
				cdata += sizeof(hash_t);
				vin = *((unsigned int *)(cdata));
				cdata += sizeof(unsigned int);

				if (!find_stake_hash(hash, stakes, len_stakes))
				{
					(*ntx)++;
					*total_spent += *((uint64_t*)data);

					if (((*ntx) >= first) && ((*max) > 0) && (nconf >= min_conf) && (nconf <= max_conf))
					{
						(*max)--;
						if (tree_manager_add_child_node(spents, "spent", NODE_GFX_OBJECT, &spent))
						{
							mem_zone_ref  addr_list = { PTR_NULL };


							tree_manager_set_child_value_btcaddr(&spent, "srcaddr", addr);
							tree_manager_set_child_value_hash(&spent, "txid", hash);
							tree_manager_set_child_value_i32(&spent, "vin", vin);
							tree_manager_set_child_value_i64(&spent, "amount", *((uint64_t*)data));
							tree_manager_set_child_value_i32(&spent, "time", tx_time);
							tree_manager_set_child_value_i64(&spent, "confirmations", nconf);

							if (tree_manager_add_child_node(&spent, "addresses", NODE_JSON_ARRAY, &addr_list))
							{
								while (cdata < (data + len))
								{
									mem_zone_ref new_addr = { PTR_NULL };
									if (tree_manager_add_child_node(&addr_list, "address", NODE_BITCORE_WALLET_ADDR, &new_addr))
									{
										tree_manager_write_node_btcaddr(&new_addr, 0, cdata);
										release_zone_ref(&new_addr);
									}
									cdata = mem_add(cdata, sizeof(btc_addr_t));
								}
								release_zone_ref(&addr_list);
							}
							release_zone_ref(&spent);
						}
					}
				}
			}
			free_c(data);
		}
		free_string(&tx_path);
		cur++;
		optr = ptr + 1;
		dir_list_len -= sz;
	}

	free_c(stakes);
	free_string(&dir_list);
	free_string(&spent_path);
	return 1;
}



OS_API_C_FUNC(int)  list_sentobjs(btc_addr_t addr, const char *appName, unsigned int appType, mem_zone_ref_ptr spents, size_t min_conf, size_t max_conf, size_t *ntx, size_t *max, size_t first)
{
	mem_zone_ref		apps = { PTR_NULL };
	struct string		spent_path = { 0 };
	struct string		dir_list = { PTR_NULL };
	const char			*ptr, *optr;
	unsigned int		dir_list_len;
	size_t				cur, nfiles;
	uint64_t			sheight;


	make_string(&spent_path, "adrs");
	cat_ncstring_p(&spent_path, addr, 34);
	cat_cstring_p(&spent_path, "sentobjs");

	if (stat_file(spent_path.str) != 0)
	{
		free_string(&spent_path);
		return 0;
	}

	sheight = get_last_block_height();

	get_apps(&apps);


	nfiles = get_sub_files(spent_path.str, &dir_list);

	dir_list_len = dir_list.len;
	optr = dir_list.str;
	cur = 0;
	while (cur < nfiles)
	{
		struct string	tx_path = { 0 };
		unsigned int	vin = 0xFFFFFFFF;
		unsigned int	prev_out = 0xFFFFFFFF;
		size_t			sz, len;
		unsigned char	*data;
		int				n;


		ptr = memchr_c(optr, 10, dir_list_len);
		sz = mem_sub(optr, ptr);

		clone_string(&tx_path, &spent_path);
		cat_ncstring_p(&tx_path, optr, sz);
		if (get_file(tx_path.str, &data, &len) > 0)
		{
			hash_t		thash;
			uint64_t	height, block_time, nconf;
			unsigned int tx_time;

			if (optr[64] == '_')
				prev_out = strtoul_c(&optr[65], PTR_NULL, 10);
			else
				prev_out = 0xFFFFFFFF;

			n = 0;
			while (n<32)
			{
				char    hex[3];
				hex[0] = optr[n * 2 + 0];
				hex[1] = optr[n * 2 + 1];
				hex[2] = 0;
				thash[n] = strtoul_c(hex, PTR_NULL, 16);
				n++;
			}

			//32 app + 4 type + 32 objHash
			if (len >= 172)
			{
				char		  appName[32];
				hash_t		  objHash,hash;
				mem_zone_ref  spent = { PTR_NULL };
				unsigned int  type;
				unsigned char *cdata;

				strcpy_cs(appName, 32, data);

				type = *((unsigned int *)(data + 32));
				memcpy_c(objHash, data + 36, 32);

				cdata = data + 102;
				memcpy_c(hash, cdata, sizeof(hash_t));
				cdata += sizeof(hash_t);
				vin = *((unsigned int *)(cdata));
				cdata += sizeof(unsigned int);

				if (get_tx_blk_height(hash, &height, &block_time, &tx_time))
					nconf = sheight - height;
				else
				{
					block_time = 0;
					tx_time = 0;
					nconf = 0;
				}

	
				(*ntx)++;

				if (((*ntx) >= first) && ((*max) > 0) && (nconf >= min_conf) && (nconf <= max_conf))
				{
					(*max)--;
					if (tree_manager_add_child_node(spents, "spent", NODE_GFX_OBJECT, &spent))
					{
						mem_zone_ref  addr_list = { PTR_NULL };

						if (tree_find_child_node_by_member_name_hash(&apps, NODE_BITCORE_TX, "txid", thash, PTR_NULL))
						{

						}

						tree_manager_set_child_value_btcaddr(&spent, "srcaddr", addr);
						tree_manager_set_child_value_btcaddr(&spent, "dstaddr", cdata);
						tree_manager_set_child_value_hash(&spent, "objHash", objHash);
						tree_manager_set_child_value_hash(&spent, "txid", hash);
						tree_manager_set_child_value_i32(&spent, "vin", vin);
						tree_manager_set_child_value_i32(&spent, "time", tx_time);
						tree_manager_set_child_value_i64(&spent, "confirmations", nconf);

		
						release_zone_ref(&spent);
					}
				}
			}
			free_c(data);
		}
		free_string(&tx_path);
		cur++;
		optr = ptr + 1;
		dir_list_len -= sz;
	}
	release_zone_ref(&apps);
	free_string(&dir_list);
	free_string(&spent_path);
	return 1;
}

OS_API_C_FUNC(int) list_staking_unspent(mem_zone_ref_ptr last_blk, btc_addr_t addr, mem_zone_ref_ptr unspents, unsigned int min_depth, int *max)
{
	struct string		unspent_path = { 0 };
	unsigned int		n;
	unsigned int		dir_list_len;
	struct string		dir_list = { PTR_NULL };
	const char			*ptr, *optr;
	size_t				cur, nfiles;
	uint64_t			sheight;

	make_string(&unspent_path, "adrs");
	cat_ncstring_p(&unspent_path, addr, 34);
	cat_cstring_p(&unspent_path, "unspent");

	if (stat_file(unspent_path.str) != 0)
	{
		free_string(&unspent_path);
		return 0;
	}

	sheight = get_last_block_height();
	nfiles = get_sub_files(unspent_path.str, &dir_list);

	free_string(&unspent_path);

	dir_list_len = dir_list.len;
	optr = dir_list.str;
	cur = 0;
	while (cur < nfiles)
	{
		hash_t			hash, rhash;
		mem_zone_ref	unspent = { PTR_NULL };
		unsigned int	output = 0xFFFFFFFF, tx_time;
		uint64_t		height, blk_time, nconf;
		size_t			sz;

		if (((*max)--) <= 0)
			break;

		ptr = memchr_c(optr, 10, dir_list_len);
		sz = mem_sub(optr, ptr);

		if (optr[64] == '_')
			output = strtoul_c(&optr[65], PTR_NULL, 10);
		else
			output = 0xFFFFFFFF;

		n = 0;
		while (n<32)
		{
			char    hex[3];
			hex[0] = optr[n * 2 + 0];
			hex[1] = optr[n * 2 + 1];
			hex[2] = 0;
			hash[n] = strtoul_c(hex, PTR_NULL, 16);
			rhash[31 - n] = hash[n];
			n++;
		}

		if ((!is_locked_unspent(hash, output)) && (!find_mempool_inputs(hash, output)) && (check_utxo(optr, output)) )
		{
			if (get_tx_blk_height(hash, &height, &blk_time, &tx_time))
			{
				nconf = sheight - height;
				if (nconf > min_depth)
				{
					if (tree_manager_add_child_node(unspents, "unspent", NODE_GFX_OBJECT, &unspent))
					{
						hash_t			out_diff;
						struct string	pos_hash_data = { PTR_NULL };
						uint64_t		amount;

						memset_c(out_diff, 0, sizeof(hash_t));
						if (get_tx_pos_hash_data(last_blk, hash, output, &pos_hash_data, &amount, out_diff))
						{
							hash_t rout_diff;
							n = 32;
							while (n--)rout_diff[n] = out_diff[31 - n];
							tree_manager_set_child_value_hash(&unspent, "txid", hash);
							tree_manager_set_child_value_i32(&unspent, "vout", output);
							tree_manager_set_child_value_i32(&unspent, "nconf", nconf);
							tree_manager_set_child_value_i64(&unspent, "weight", amount);
							tree_manager_set_child_value_btcaddr(&unspent, "dstaddr", addr);
							tree_manager_set_child_value_vstr(&unspent, "hash_data", &pos_hash_data);
							tree_manager_set_child_value_hash(&unspent, "difficulty", rout_diff);
							free_string(&pos_hash_data);
						}
						release_zone_ref(&unspent);
					}
				}
			}
		}
		cur++;
		optr = ptr + 1;
		dir_list_len -= sz;

		do_mark_sweep(get_tree_mem_area_id(), 500);
	}
	free_string(&dir_list);


	return 1;
}


int get_addr_balance	(btc_addr_t myaddr, uint64_t *out, uint64_t *in)
{
	mem_zone_ref		my_list = { PTR_NULL },mempool = { PTR_NULL };
	mem_zone_ref_ptr	tx = PTR_NULL;

	tree_manager_find_child_node(&my_node, NODE_HASH("mempool"), NODE_BITCORE_TX_LIST, &mempool);

	for (tree_manager_get_first_child(&mempool, &my_list, &tx); ((tx != NULL) && (tx->zone != NULL)); tree_manager_get_next_child(&my_list, &tx))
	{
		mem_zone_ref		my_ilist = { PTR_NULL }, inputs = { PTR_NULL };
		mem_zone_ref_ptr	input = PTR_NULL;

		mem_zone_ref		my_olist = { PTR_NULL }, outputs = { PTR_NULL };
		mem_zone_ref_ptr	output = PTR_NULL;

		tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &inputs);
		for (tree_manager_get_first_child(&inputs, &my_ilist, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_ilist, &input))
		{
			mem_ptr		addr;
			uint64_t	value;

			if (!tree_manager_get_child_value_i64(input, NODE_HASH("value"), &value))
				value = 0;

			if ((value == 0) || (value == 0xFFFFFFFFFFFFFFFF) || ((value & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000))
				continue;

			if (tree_manager_get_child_data_ptr(input, NODE_HASH("srcaddr"), &addr))
			{
				if (!memcmp_c(addr, myaddr, sizeof(btc_addr_t)))
				{
					(*out) += value;
				}
			}
		}

		release_zone_ref(&inputs);

		tree_manager_find_child_node(tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &outputs);
		for (tree_manager_get_first_child(&outputs, &my_olist, &output); ((output != PTR_NULL) && (output->zone != PTR_NULL)); tree_manager_get_next_child(&my_olist, &output))
		{
			mem_ptr		addr;
			uint64_t	value;

			if (!tree_manager_get_child_value_i64(output, NODE_HASH("value"), &value))
				value = 0;

			if ((value == 0) || (value == 0xFFFFFFFFFFFFFFFF) || ((value & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000))
				continue;

			if (tree_manager_get_child_data_ptr(output, NODE_HASH("dstaddr"), &addr))
			{
				if (!memcmp_c(addr, myaddr, sizeof(btc_addr_t)))
				{
					(*in) += value;
				}
			}
		}
		release_zone_ref(&outputs);
	}

	release_zone_ref(&mempool);
	return 1;
}

int  get_balance(btc_addr_t addr, uint64_t *conf_amount, uint64_t *amount, unsigned int minconf)
{
	struct string		unspent_path;
	unsigned int		n;
	unsigned int		dir_list_len;
	struct string		dir_list = { PTR_NULL };
	const char			*ptr, *optr;
	size_t				cur, nfiles;
	unsigned int		sheight;

	init_string(&unspent_path);
	make_string(&unspent_path, "adrs");
	cat_ncstring_p(&unspent_path, addr, 34);
	cat_cstring_p(&unspent_path, "unspent");

	if (stat_file(unspent_path.str) != 0)
	{
		free_string(&unspent_path);
		return 0;
	}




	sheight = get_last_block_height();

	nfiles = get_sub_files(unspent_path.str, &dir_list);

	dir_list_len = dir_list.len;
	optr = dir_list.str;
	cur = 0;
	while ((cur < nfiles) && (dir_list_len>0))
	{
		struct string	tx_path;
		unsigned int	output = 0xFFFFFFFF;
		size_t			sz, len;
		unsigned char	*data;

		ptr = memchr_c(optr, 10, dir_list_len);
		if (ptr == PTR_NULL)break;
		sz = mem_sub(optr, ptr);

		init_string(&tx_path);
		clone_string(&tx_path, &unspent_path);
		cat_ncstring_p(&tx_path, optr, sz);

		if (get_file(tx_path.str, &data, &len)>0)
		{
			unsigned int nconf;
			if (len >= sizeof(uint64_t))
			{
				hash_t			hash;
				mem_zone_ref	unspent = { PTR_NULL };
				uint64_t		height, block_time;
				unsigned int    tx_time;
				n = 0;
				while (n<32)
				{
					char    hex[3];
					hex[0] = optr[n * 2 + 0];
					hex[1] = optr[n * 2 + 1];
					hex[2] = 0;
					hash[n] = strtoul_c(hex, PTR_NULL, 16);
					n++;
				}

				if (optr[64] == '_')
					output = strtoul_c(&optr[65], PTR_NULL, 10);
				else
					output = 0xFFFFFFFF;

				if (get_tx_blk_height(hash, &height, &block_time, &tx_time))
					nconf = sheight - height;
				else
				{
					block_time = 0;
					tx_time = 0;
					nconf = 0;
				}
				if (nconf < minconf)
					(*amount) += *((uint64_t*)data);
				else
					(*conf_amount) += *((uint64_t*)data);
			}
			free_c(data);
			do_mark_sweep(get_tree_mem_area_id(), 500);
		}
		free_string(&tx_path);
		cur++;
		optr = ptr + 1;
		dir_list_len -= (sz + 1);
	}
	free_string(&dir_list);
	free_string(&unspent_path);
	return 1;
}


OS_API_C_FUNC(int)  list_received(btc_addr_t addr, mem_zone_ref_ptr received, size_t min_conf, size_t max_conf, uint64_t *amount, size_t *ntx,size_t *max, size_t first)
{
	btc_addr_t			null_addr;
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref_ptr	ptx = PTR_NULL;
	struct string		unspent_path = { 0 };
	struct string		spent_path = { 0 };
	struct string		stake_path = { 0 };
	unsigned int		dir_list_len;
	struct string		dir_list = { PTR_NULL };
	uint64_t			sheight;
	const char			*ptr, *optr;
	size_t				cur, nfiles, nStakes;
	size_t				len_stakes;
	unsigned char		*stakes = PTR_NULL;

	memset_c(null_addr, '0', sizeof(btc_addr_t));

	sheight = get_last_block_height();

	make_string(&unspent_path, "adrs");
	cat_ncstring_p(&unspent_path, addr, 34);

	clone_string(&spent_path, &unspent_path);
	clone_string(&stake_path, &unspent_path);
	cat_cstring_p(&spent_path, "spent");
	cat_cstring_p(&stake_path, "stakes");
	cat_cstring_p(&unspent_path, "unspent");

	*amount = 0;

	if (get_file(stake_path.str, &stakes, &len_stakes))
	{
		nStakes = len_stakes / 40;
		cur = 0;
		while (cur < len_stakes)
		{
			*amount += *((uint64_t*)(stakes + cur));
			(*ntx)++;

			if ((received != PTR_NULL) && ((*ntx)>=first) && ((*max)>0))
			{
				mem_zone_ref recv = { PTR_NULL };
				(*max)--;
				if (tree_manager_add_child_node(received, "recv", NODE_GFX_OBJECT, &recv))
				{
					mem_zone_ref addr_list = { PTR_NULL };
					uint64_t	height, block_time, nconf;
					unsigned int tx_time;
					if (get_tx_blk_height(&stakes[cur + 8], &height, &block_time, &tx_time))
						nconf = sheight - height;
					else
					{
						tx_time = 0;
						block_time = 0;
						nconf = 0;
					}

					tree_manager_set_child_value_btcaddr(&recv, "dstaddr", addr);

					tree_manager_set_child_value_hash(&recv, "txid", &stakes[cur + 8]);
					tree_manager_set_child_value_i64(&recv, "amount", *((uint64_t*)(stakes + cur)));
					tree_manager_set_child_value_i32(&recv, "time", tx_time);
					tree_manager_set_child_value_i64(&recv, "confirmations", nconf);
					if (tree_manager_add_child_node(&recv, "addresses", NODE_JSON_ARRAY, &addr_list))
					{
						mem_zone_ref new_addr = { PTR_NULL };
						if (tree_manager_add_child_node(&addr_list, "address", NODE_BITCORE_WALLET_ADDR, &new_addr))
						{
							tree_manager_write_node_btcaddr(&new_addr, 0, addr);
							release_zone_ref(&new_addr);
						}
						release_zone_ref(&addr_list);
					}
					release_zone_ref(&recv);
				}
			}
			cur += 40;
		}
	}
	else
	{
		len_stakes = 0;
		nStakes = 0;
	}
	free_string(&stake_path);

	nfiles = get_sub_files(unspent_path.str, &dir_list);

	dir_list_len = dir_list.len;
	optr = dir_list.str;
	cur = 0;
	while (cur < nfiles)
	{
		hash_t			hash;
		struct string	tx_path = { 0 };
		unsigned int	output = 0xFFFFFFFF;
		int				n;
		size_t			sz, len;
		unsigned char	*data;

		ptr = memchr_c(optr, 10, dir_list_len);
		sz = mem_sub(optr, ptr);

		n = 0;
		while (n < 32)
		{
			char    hex[3];
			hex[0] = optr[n * 2 + 0];
			hex[1] = optr[n * 2 + 1];
			hex[2] = 0;
			hash[n] = strtoul_c(hex, PTR_NULL, 16);
			n++;
		}

		if (!find_stake_hash(hash, stakes, len_stakes))
		{
			if (optr[64] == '_')
				output = strtoul_c(&optr[65], PTR_NULL, 10);

			clone_string(&tx_path, &unspent_path);
			cat_ncstring_p(&tx_path, optr, sz);

			if (get_file(tx_path.str, &data, &len)>0)
			{
				if (len >= sizeof(uint64_t)){
					*amount += *((uint64_t*)data);
					(*ntx)++;
				}


				if ((received != PTR_NULL) && ((*ntx) >= first) && ((*max)>0))
				{
					mem_zone_ref recv = { PTR_NULL };
					(*max)--;
					if (tree_manager_add_child_node(received, "recv", NODE_GFX_OBJECT, &recv))
					{
						hash_t		 hash;
						uint64_t	 height, block_time, nconf;
						unsigned int n, tx_time;
						unsigned int n_addrs;
						n = 0;
						while (n < 32)
						{
							char    hex[3];
							hex[0] = optr[n * 2 + 0];
							hex[1] = optr[n * 2 + 1];
							hex[2] = 0;
							hash[n] = strtoul_c(hex, PTR_NULL, 16);
							n++;
						}


						if (get_tx_blk_height(hash, &height, &block_time, &tx_time))
							nconf = sheight - height;
						else
						{
							block_time = 0;
							tx_time = 0;
							nconf = 0;
						}

						tree_manager_set_child_value_btcaddr(&recv, "dstaddr", addr);
						tree_manager_set_child_value_hash(&recv, "txid", hash);
						tree_manager_set_child_value_i64(&recv, "amount", *((uint64_t*)data));
						tree_manager_set_child_value_i32(&recv, "time", tx_time);
						tree_manager_set_child_value_i64(&recv, "confirmations", nconf);


						n_addrs = *((unsigned int *)(data + sizeof(uint64_t)));
						if (n_addrs > 0)
						{
							mem_zone_ref addr_list = { PTR_NULL };

							if (tree_manager_add_child_node(&recv, "addresses", NODE_JSON_ARRAY, &addr_list))
							{
								mem_ptr addrs;
								addrs = data + sizeof(uint64_t) + sizeof(unsigned int);
								for (n = 0; n < n_addrs; n++)
								{
									mem_zone_ref new_addr = { PTR_NULL };
									if (tree_manager_add_child_node(&addr_list, "address", NODE_BITCORE_WALLET_ADDR, &new_addr))
									{
										tree_manager_write_node_btcaddr(&new_addr, 0, addrs);
										release_zone_ref(&new_addr);
									}
									addrs = mem_add(addrs, sizeof(btc_addr_t));
								}
								release_zone_ref(&addr_list);
							}
						}

						release_zone_ref(&recv);
					}
				}
				free_c(data);
			}
			free_string(&tx_path);
		}
		cur++;
		optr = ptr + 1;
		dir_list_len -= sz;
	}
	free_string(&dir_list);
	nfiles = get_sub_files(spent_path.str, &dir_list);

	dir_list_len = dir_list.len;
	optr = dir_list.str;
	cur = 0;
	while (cur < nfiles)
	{
		hash_t			hash;
		struct string	tx_path = { 0 };
		int				n;
		size_t			sz, len;
		unsigned char	*data;

		ptr = memchr_c(optr, 10, dir_list_len);
		sz = mem_sub(optr, ptr);

		n = 0;
		while (n < 32)
		{
			char    hex[3];
			hex[0] = optr[n * 2 + 0];
			hex[1] = optr[n * 2 + 1];
			hex[2] = 0;
			hash[n] = strtoul_c(hex, PTR_NULL, 16);
			n++;
		}
		if (!find_stake_hash(hash, stakes, len_stakes))
		{
			unsigned int prev_output;

			clone_string(&tx_path, &spent_path);
			cat_ncstring_p(&tx_path, optr, sz);

			if (optr[64] == '_')
				prev_output = strtoul_c(&optr[65], PTR_NULL, 10);

			if (get_file(tx_path.str, &data, &len)>0)
			{
				if (len >= sizeof(uint64_t))
				{
					(*ntx)++;
					*amount += *((uint64_t*)data);
				}

				if ((received != PTR_NULL) && ((*ntx) >= first) && ((*max)>0))
				{
					mem_zone_ref recv = { PTR_NULL };
					size_t total_len;
					size_t naddrs;

					naddrs		= *((unsigned int *)(data + sizeof(uint64_t)));
					total_len	= sizeof(uint64_t)+ sizeof(unsigned int) + naddrs*sizeof(btc_addr_t);

					if (len >= total_len)
					{
						(*max)--;
						if (tree_manager_add_child_node(received, "recv", NODE_GFX_OBJECT, &recv))
						{
							mem_zone_ref addr_list = { PTR_NULL };
							unsigned int n_in_addr, vin;
							hash_t		 hash;
							uint64_t	 height, block_time, nconf;
							unsigned int  n, tx_time;
							unsigned char *cdata;

							cdata = data + sizeof(uint64_t);
							n_in_addr = *((unsigned int *)(cdata));
							cdata += sizeof(unsigned int);
							if (tree_manager_add_child_node(&recv, "addresses", NODE_JSON_ARRAY, &addr_list))
							{
								for (n = 0; n < n_in_addr; n++)
								{
									mem_zone_ref new_addr = { PTR_NULL };
									if (tree_manager_add_child_node(&addr_list, "address", NODE_BITCORE_WALLET_ADDR, &new_addr))
									{
										tree_manager_write_node_btcaddr(&new_addr, 0, cdata);
										release_zone_ref(&new_addr);
									}
									cdata = mem_add(cdata, sizeof(btc_addr_t));
								}
								release_zone_ref(&addr_list);
							}
							memcpy_c(hash, cdata, sizeof(hash_t));
							cdata += sizeof(hash_t);
							vin = *((unsigned int *)(cdata));
							cdata += sizeof(unsigned int);

							if (get_tx_blk_height(hash, &height, &block_time, &tx_time))
								nconf = sheight - height;
							else
							{
								block_time = 0;
								tx_time = 0;
								nconf = 0;
							}
							tree_manager_set_child_value_btcaddr(&recv, "dstaddr", addr);
							tree_manager_set_child_value_hash(&recv, "txid", hash);
							tree_manager_set_child_value_i64(&recv, "amount", *((uint64_t*)data));
							tree_manager_set_child_value_i32(&recv, "time", tx_time);
							tree_manager_set_child_value_i64(&recv, "confirmations", nconf);

							release_zone_ref(&recv);
						}
					}
				}
				free_c(data);
			}
			free_string(&tx_path);
		}
		
		cur++;
		optr = ptr + 1;
		dir_list_len -= sz;
	}
	free_string(&unspent_path);
	free_string(&dir_list);
	free_string(&stake_path);

	if (stakes != PTR_NULL)
		free_c(stakes);
	return 1;

}

OS_API_C_FUNC(int) cancel_unspend_obj_addr(btc_addr_t addr, const char *tx_hash, unsigned int oidx)
{
	struct string	unspent_path = { 0 };
	int ret;
	make_string(&unspent_path, "adrs");
	cat_ncstring_p(&unspent_path, addr, 34);
	cat_cstring_p(&unspent_path, "objects");
	cat_cstring_p(&unspent_path, tx_hash);
	cat_cstring(&unspent_path, "_");
	strcat_int(&unspent_path, oidx);
	ret = del_file(unspent_path.str);
	free_string(&unspent_path);

	return ret;
}

OS_API_C_FUNC(int) cancel_unspend_tx_addr(btc_addr_t addr, const char *tx_hash, unsigned int oidx)
{
	struct string	unspent_path = { 0 };
	int ret;
	make_string(&unspent_path, "adrs");
	cat_ncstring_p(&unspent_path, addr, 34);
	cat_cstring_p(&unspent_path, "unspent");
	cat_cstring_p(&unspent_path, tx_hash);
	cat_cstring(&unspent_path, "_");
	strcat_int(&unspent_path, oidx);
	ret=del_file(unspent_path.str);
	free_string(&unspent_path);

	return ret;
}

OS_API_C_FUNC(int) cancel_spend_tx_addr(btc_addr_t addr, const char *tx_hash, unsigned int oidx)
{
	struct string	spent_path = { 0 };
	int ret;


	//check if the address is monitored on the local wallet
	make_string(&spent_path, "adrs");
	cat_ncstring_p(&spent_path, addr, 34);
	cat_cstring_p(&spent_path, "spent");
	cat_cstring_p(&spent_path, tx_hash);
	cat_cstring(&spent_path, "_");
	strcat_int(&spent_path, oidx);

	ret = (stat_file(spent_path.str) == 0) ? 1 : 0;
	if (ret)
	{
		struct string	 unspent_path = { 0 };
		unsigned char	 *data;
		size_t			 len;

		//create unspent directory for the address
		make_string(&unspent_path, "adrs");
		cat_ncstring_p(&unspent_path, addr, 34);
		cat_cstring_p(&unspent_path, "unspent");
		create_dir(unspent_path.str);


		//move the spent back in the unspent
		cat_cstring_p(&unspent_path, tx_hash);
		cat_cstring(&unspent_path, "_");
		strcat_int(&unspent_path, oidx);
		ret = move_file(spent_path.str, unspent_path.str);

		//remove spending data from the unspent file
		if (get_file(unspent_path.str, &data, &len)>0)
		{
			unsigned int n_addr;
			if (len >= (sizeof(uint64_t) + sizeof(unsigned int)))
			{
				size_t		unspent_len;
				n_addr = *((unsigned int *)(data + sizeof(uint64_t)));
				unspent_len = (sizeof(uint64_t) + sizeof(unsigned int) + n_addr*sizeof(btc_addr_t));

				if (len > unspent_len)
					truncate_file(unspent_path.str, unspent_len, PTR_NULL, 0);
			}
			free_c(data);
		}
		free_string(&unspent_path);
	}
	free_string(&spent_path);

	return ret;
}

OS_API_C_FUNC(int) cancel_spend_obj_addr(btc_addr_t addr, const char *tx_hash, unsigned int oidx)
{
	struct string	spent_path = { 0 };
	int ret;


	//check if the address is monitored on the local wallet
	make_string(&spent_path, "adrs");
	cat_ncstring_p(&spent_path, addr, 34);
	cat_cstring_p(&spent_path, "sentobjs");
	cat_cstring_p(&spent_path, tx_hash);
	cat_cstring(&spent_path, "_");
	strcat_int(&spent_path, oidx);

	ret = (stat_file(spent_path.str) == 0) ? 1 : 0;
	if (ret)
	{
		struct string	 unspent_path = { 0 };
		unsigned char	 *data;
		size_t			 len;

		//create unspent directory for the address
		make_string(&unspent_path, "adrs");
		cat_ncstring_p(&unspent_path, addr, 34);
		cat_cstring_p(&unspent_path, "objects");
		create_dir(unspent_path.str);


		//move the spent back in the unspent
		cat_cstring_p(&unspent_path, tx_hash);
		cat_cstring(&unspent_path, "_");
		strcat_int(&unspent_path, oidx);
		ret = move_file(spent_path.str, unspent_path.str);

		//remove spending data from the unspent file
		if (get_file(unspent_path.str, &data, &len)>0)
		{
			if (len >= 102)
				truncate_file(unspent_path.str, 102, PTR_NULL, 0);

			free_c(data);
		}
		free_string(&unspent_path);
	}
	free_string(&spent_path);

	return ret;
}

int remove_tx_staking(const btc_addr_t stake_addr, const hash_t tx_hash)
{
	struct string	 stake_path = { 0 };
	mem_zone_ref	 txout_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr out = PTR_NULL;
	unsigned char	*data;
	size_t			len;

	make_string(&stake_path, "adrs");
	cat_ncstring_p(&stake_path, stake_addr, 34);
	if (stat_file(stake_path.str) != 0)
	{
		free_string(&stake_path);
		return 0;
	}

	cat_cstring_p(&stake_path, "stakes");
	if (get_file(stake_path.str, &data, &len) > 0)
	{
		size_t n = 0;
		while ((n + sizeof(hash_t) + sizeof(uint64_t))<len)
		{
			if (!memcmp_c(&data[n + sizeof(uint64_t)], tx_hash, sizeof(hash_t)))
			{
				put_file("NewFile", data, n);
				put_file("NewFile", data + n + sizeof(hash_t) + sizeof(uint64_t), len - (n + sizeof(hash_t) + sizeof(uint64_t)));
				del_file(stake_path.str);
				move_file("NewFile", stake_path.str);
				break;
			}
			n += (sizeof(hash_t) + sizeof(uint64_t));
		}
		free_c(data);
	}
	free_string(&stake_path);
	return 1;
}

OS_API_C_FUNC(int) remove_wallet_tx(const hash_t tx_hash)
{
	hash_t				blkh;
	mem_zone_ref	    txin_list = { PTR_NULL }, txout_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr    input = PTR_NULL,output = PTR_NULL;
	unsigned int		oidx;
	mem_zone_ref	    tx = { PTR_NULL };

	if (!load_tx(&tx, blkh, tx_hash))return 0;

	if (!tree_manager_find_child_node(&tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))
	{
		release_zone_ref(&tx);
		return 0;
	}

	for (oidx = 0, tree_manager_get_first_child(&txout_list, &my_list, &output); ((output != NULL) && (output->zone != NULL)); tree_manager_get_next_child(&my_list, &output), oidx++)
	{
		btc_addr_t		 out_addr;
		struct string	 pubk = { PTR_NULL }, script = { PTR_NULL };
		uint64_t		 amount;

		tree_manager_get_child_value_i64	(output, NODE_HASH("value"), &amount);

		if (amount == 0)continue;
		if (amount == 0xFFFFFFFFFFFFFFFF)continue;

		tree_manager_get_child_value_istr	(output, NODE_HASH("script"), &script, 0);

		if (is_opfn_script(&script,PTR_NULL))
		{
			free_string(&script);
			continue;
		}

		if (get_out_script_address(&script, &pubk, out_addr))
		{
			char chash[65];

			bin_2_hex(tx_hash, 32, chash);

			if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
			{
				struct string DataStr = { 0 };
				if (get_out_script_return_val(&script, &DataStr))
				{
					cancel_unspend_obj_addr(out_addr, chash, oidx);
					free_string(&DataStr);
				}
			}
			else
				cancel_unspend_tx_addr	(out_addr, chash, oidx);

			remove_tx_staking		(out_addr, tx_hash);
			remove_tx_addresses		(out_addr, tx_hash);

			if (pubk.str != PTR_NULL)
				free_string(&pubk);
		}
		free_string(&script);
	}
	release_zone_ref(&txout_list);


	if (!tree_manager_find_child_node(&tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))
	{
		release_zone_ref(&tx);
		return 0;
	}

	//process tx inputs
	for (tree_manager_get_first_child(&txin_list, &my_list, &input); ((input != NULL) && (input->zone != NULL)); tree_manager_get_next_child(&my_list, &input))
	{
		mem_zone_ref	 ptx = { PTR_NULL };
		hash_t			 prev_hash, pblk_hash;
		unsigned int	 oidx;

		tree_manager_get_child_value_hash(input, NODE_HASH("txid"), prev_hash);
		tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &oidx);


		/*load the transaction with the spent output*/
		if (load_tx(&ptx, pblk_hash, prev_hash))
		{
			char			 pchash[65];
			btc_addr_t		 out_addr;
			struct string	 script = { PTR_NULL }, pubk = { PTR_NULL };
			mem_zone_ref	 vout = { PTR_NULL };


			memset_c(out_addr, '0', sizeof(btc_addr_t));

			/*load the spent output from the parent transaction*/
			if (get_tx_output(&ptx, oidx, &vout))
			{
				uint64_t amount;
				

				tree_manager_get_child_value_i64(&vout, NODE_HASH("value"), &amount);

				if ((amount != 0) && (amount != 0xFFFFFFFFFFFFFFFF))
				{
					bin_2_hex(prev_hash, 32, pchash);
		
					if (tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &script, 16))
					{
						if (!is_opfn_script(&script,PTR_NULL))
						{
							if (get_out_script_address(&script, PTR_NULL, out_addr))
							{
								//cancel the spent in the wallet

								if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
								{
									struct string DataStr = { PTR_NULL };
									if (get_out_script_return_val(&script, &DataStr))
									{
										cancel_spend_obj_addr(out_addr, pchash, oidx);
										free_string(&DataStr);
									}
								}
								else
									cancel_spend_tx_addr(out_addr, pchash, oidx);

								//remove tx from the address index
								remove_tx_addresses(out_addr, tx_hash);
							}
						}

						free_string(&script);
					}
				}
				release_zone_ref(&vout);
			}
			release_zone_ref(&ptx);
		}
	}
	release_zone_ref(&txin_list);
	release_zone_ref(&tx);
	return 1;
}



OS_API_C_FUNC(int) add_unspent(btc_addr_t	addr, const char *tx_hash, unsigned int oidx, uint64_t amount, btc_addr_t *src_addrs, unsigned int n_addrs)
{
	struct string	out_path = { 0 };
	int ret;

	make_string		(&out_path, "adrs");
	cat_ncstring_p	(&out_path, addr, 34);
	cat_cstring_p	(&out_path, "unspent");
	create_dir		(out_path.str);
	cat_cstring_p	(&out_path, tx_hash);
	cat_cstring		(&out_path, "_");
	strcat_int		(&out_path, oidx);

	ret = put_file(out_path.str, &amount, sizeof(uint64_t));
	if (n_addrs > 0)
	{
		append_file(out_path.str, &n_addrs, sizeof(unsigned int));
		append_file(out_path.str, src_addrs, n_addrs*sizeof(btc_addr_t));
	}
	free_string(&out_path);
	return (ret>0);
}

OS_API_C_FUNC(int) add_wallet_obj(btc_addr_t addr, const char *txhash, const char *app, unsigned int type, const hash_t objhash, btc_addr_t src_addrs)
{
	struct string	out_path = { 0 };
	int ret;


	make_string(&out_path, "adrs");
	cat_ncstring_p(&out_path, addr, 34);
	cat_cstring_p(&out_path, "objects");
	create_dir(out_path.str);
	cat_cstring_p(&out_path, txhash);
	cat_cstring(&out_path, "_");
	strcat_int(&out_path, 0);

	ret = put_file(out_path.str, app, 32);
	ret = append_file(out_path.str, &type, 4);
	ret = append_file(out_path.str, objhash, sizeof(hash_t));
	if(ret)ret=append_file(out_path.str, src_addrs, sizeof(btc_addr_t));

	free_string(&out_path);
	return (ret>0);
}

OS_API_C_FUNC(int) spend_tx_addr(btc_addr_t addr, const char *tx_hash, unsigned int vin, const char *ptx_hash, unsigned int oidx, btc_addr_t *addrs_to, unsigned int n_addrs_to)
{
	struct string	 unspent_path = { 0 };
	unsigned char	*sp_buf;
	unsigned int	len;

	make_string(&unspent_path, "adrs");
	cat_ncstring_p(&unspent_path, addr, 34);
	if (stat_file(unspent_path.str) != 0)
	{
		free_string(&unspent_path);
		return 0;
	}

	cat_cstring_p(&unspent_path, "unspent");
	cat_cstring_p(&unspent_path, ptx_hash);
	cat_cstring(&unspent_path, "_");
	strcat_int(&unspent_path, oidx);

	if (get_file(unspent_path.str, &sp_buf, &len)>0)
	{
		hash_t th;
		int n = 0;
		struct string	spent_path = { 0 };

		make_string(&spent_path, "adrs");
		cat_ncstring_p(&spent_path, addr, 34);
		cat_cstring_p(&spent_path, "spent");
		create_dir(spent_path.str);
		cat_cstring_p(&spent_path, ptx_hash);
		cat_cstring(&spent_path, "_");
		strcat_int(&spent_path, oidx);
		move_file(unspent_path.str, spent_path.str);

		while (n<32)
		{
			char    hex[3];
			hex[0] = tx_hash[n * 2 + 0];
			hex[1] = tx_hash[n * 2 + 1];
			hex[2] = 0;
			th[n] = strtoul_c(hex, PTR_NULL, 16);
			n++;
		}

		append_file(spent_path.str, th, sizeof(hash_t));
		append_file(spent_path.str, &vin, sizeof(unsigned int));
		append_file(spent_path.str, addrs_to, n_addrs_to*sizeof(btc_addr_t));

		free_string(&spent_path);
		free_c(sp_buf);
		del_file(unspent_path.str);
	}

	free_string(&unspent_path);


	return 1;
}

OS_API_C_FUNC(int) send_obj(btc_addr_t addr, const char *tx_hash, unsigned int vin, const char *ptx_hash, unsigned int oidx, btc_addr_t addrs_to)
{
	struct string	 unspent_path = { 0 };
	unsigned char	*sp_buf;
	unsigned int	len;

	make_string(&unspent_path, "adrs");
	cat_ncstring_p(&unspent_path, addr, 34);
	if (stat_file(unspent_path.str) != 0)
	{
		free_string(&unspent_path);
		return 0;
	}

	cat_cstring_p(&unspent_path, "objects");
	cat_cstring_p(&unspent_path, ptx_hash);
	cat_cstring(&unspent_path, "_");
	strcat_int(&unspent_path, oidx);

	if (get_file(unspent_path.str, &sp_buf, &len)>0)
	{
		hash_t th;
		int n = 0;
		struct string	spent_path = { 0 };

		make_string(&spent_path, "adrs");
		cat_ncstring_p(&spent_path, addr, 34);
		cat_cstring_p(&spent_path, "sentobjs");
		create_dir(spent_path.str);
		cat_cstring_p(&spent_path, ptx_hash);
		cat_cstring(&spent_path, "_");
		strcat_int(&spent_path, oidx);
		move_file(unspent_path.str, spent_path.str);

		while (n<32)
		{
			char    hex[3];
			hex[0] = tx_hash[n * 2 + 0];
			hex[1] = tx_hash[n * 2 + 1];
			hex[2] = 0;
			th[n] = strtoul_c(hex, PTR_NULL, 16);
			n++;
		}

		append_file(spent_path.str, th, sizeof(hash_t));
		append_file(spent_path.str, &vin, sizeof(unsigned int));
		append_file(spent_path.str, addrs_to, sizeof(btc_addr_t));

		free_string(&spent_path);
		free_c(sp_buf);
		del_file(unspent_path.str);
	}

	free_string(&unspent_path);


	return 1;
}
OS_API_C_FUNC(int) store_tx_wallet(btc_addr_t addr, hash_t tx_hash)
{
	btc_addr_t	 	 to_addr_list[64];
	char			 tchash[65];
	hash_t			 blk_hash, null_hash, ff_hash;
	struct string	 tx_path = { 0 };
	mem_zone_ref	 txin_list = { PTR_NULL }, txout_list = { PTR_NULL }, my_list = { PTR_NULL }, tx = { PTR_NULL };
	mem_zone_ref_ptr input = PTR_NULL, out = PTR_NULL;
	unsigned int	 oidx, iidx, hasobjtxfr;
	unsigned int	 n_to_addrs;
	unsigned int	 n_in_addr;


	memset_c(null_hash, 0, sizeof(hash_t));
	memset_c(ff_hash, 0xFF, sizeof(hash_t));
	

	if (!load_tx(&tx, blk_hash, tx_hash))return 0;
	if (is_tx_null(&tx))
	{
		release_zone_ref(&tx);
		return 0;
	}
	if (!tree_manager_find_child_node(&tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))return 0;
	if (!tree_manager_find_child_node(&tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list)){ release_zone_ref(&txin_list); return 0; }

	

	bin_2_hex(tx_hash, 32, tchash);


	n_to_addrs = 0;
	n_in_addr = 0;

	for (tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); tree_manager_get_next_child(&my_list, &out))
	{
		struct string script = { 0 };
		if (!tree_manager_get_child_value_istr(out, NODE_HASH("script"), &script, 16))continue;
		if (script.len == 0){ free_string(&script); continue; }
		if (get_out_script_address(&script, PTR_NULL, to_addr_list[n_to_addrs]))
		{
			unsigned int n, f;
			f = 0;
			for (n = 0; n < n_to_addrs; n++)
			{
				if (!memcmp_c(to_addr_list[n_to_addrs], to_addr_list[n], sizeof(btc_addr_t)))
				{
					f = 1;
					break;
				}
			}
			if (f == 0)
				n_to_addrs++;
		}
		free_string(&script);
	}
	
	for (iidx = 0, tree_manager_get_first_child(&txin_list, &my_list, &input); ((input != NULL) && (input->zone != NULL)); tree_manager_get_next_child(&my_list, &input), iidx++)
	{
		hash_t			prev_hash = { 0xFF };

		tree_manager_get_child_value_hash(input, NODE_HASH("txid"), prev_hash);
		tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &oidx);

		if (get_tx_output_addr(prev_hash, oidx, src_addr_list[n_in_addr]))
		{
			unsigned int nn, f;

			tree_manager_set_child_value_btcaddr(input, "srcaddr", src_addr_list[n_in_addr]);

			f = 0;
			for (nn = 0; nn < n_in_addr; nn++)
			{
				if (!memcmp_c(src_addr_list[n_in_addr], src_addr_list[nn], sizeof(btc_addr_t)))
				{
					f = 1;
					break;
				}
			}
			if (f == 0)
				n_in_addr++;
		}
	}


	hasobjtxfr = 0;
	out = PTR_NULL;
	for (oidx = 0, tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); oidx++, tree_manager_get_next_child(&my_list, &out))
	{
		hash_t			bh, appH;
		char			appName[32];
		btc_addr_t		out_addr = { 0 };
		struct string	objHashStr = { 0 };
		struct string	out_path = { 0 }, script = { 0 };
		
		uint64_t		amount = 0;
		int				ret, isobj;

		

		tree_manager_get_child_value_i64(out, NODE_HASH("value"), &amount);

		if (amount == 0)continue;
		if (amount == 0xFFFFFFFFFFFFFFFF)continue;

		tree_manager_get_child_value_istr(out, NODE_HASH("script"), &script, 0);
		if (is_opfn_script(&script, PTR_NULL))continue;


		ret = get_out_script_address(&script, PTR_NULL, out_addr);


		

		isobj = 0;
		if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
		{
			if (get_out_script_return_val(&script, &objHashStr))
			{
				if (objHashStr.len == 32)
				{
					mem_zone_ref	objTx = { PTR_NULL };
					if (load_tx(&objTx, bh, objHashStr.str))
					{
						get_tx_input_hash(&objTx, 0, appH);
						release_zone_ref(&objTx);
						hasobjtxfr = 1;
					}
				}
				else
				{
					free_string(&objHashStr);
					get_tx_input_hash(&tx, 0, appH);

					objHashStr.str = malloc_c(32);
					objHashStr.len = 32;
					memcpy_c(objHashStr.str, tx_hash, 32);
				}

				isobj = 1;
			}
		}

		if (ret)
		{
			make_string(&out_path, "adrs");
			cat_ncstring_p(&out_path, out_addr, 34);

			if ((stat_file(out_path.str) == 0)&& (!memcmp_c(addr, out_addr, sizeof(btc_addr_t))))
			{
				if(isobj)
				{
					mem_zone_ref	apps = { PTR_NULL }, app = { PTR_NULL };
					get_apps(&apps);
					if (tree_find_child_node_by_member_name_hash(&apps, NODE_BITCORE_TX, "txid", appH, &app))
					{
						tree_manager_get_child_value_str(&app, NODE_HASH("appName"), appName, 32, 0);
						release_zone_ref(&app);
					}
					release_zone_ref(&apps);
					add_wallet_obj(out_addr, tchash, appName, amount & 0xFFFFFFFF, objHashStr.str, src_addr_list[0]);
				}
				else 
					add_unspent(out_addr, tchash, oidx, amount, src_addr_list, n_in_addr);
			}
			free_string(&out_path);
		}
		free_string(&objHashStr);
		free_string(&script);
	}
	release_zone_ref(&my_list);

	for (iidx = 0, tree_manager_get_first_child(&txin_list, &my_list, &input); ((input != NULL) && (input->zone != NULL)); tree_manager_get_next_child(&my_list, &input), iidx++)
	{
		hash_t			prev_hash = { 0xFF };
		char			ptchash[65];
		btc_addr_t		srcaddr;
		mem_zone_ref	prev_tx = { PTR_NULL };
		struct string	out_path = { 0 };
		uint64_t		amount;
		unsigned char	app_item;


		tree_manager_get_child_value_hash(input, NODE_HASH("txid"), prev_hash);

		if (!memcmp_c(prev_hash, ff_hash, sizeof(hash_t)))
			continue;

		//coin base
		if (!memcmp_c(prev_hash, null_hash, sizeof(hash_t)))
		{
			memset_c(src_addr_list[0], '0', sizeof(btc_addr_t));
			n_in_addr = 1;
			continue;
		}


		tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &oidx);
			   
		/*
		if (tree_manager_find_child_node(input, NODE_HASH("keyName"), 0xFFFFFFFF, PTR_NULL))
			continue;
		if (tree_manager_find_child_node(input, NODE_HASH("op_name"), 0xFFFFFFFF, PTR_NULL))
			continue;
		if (tree_manager_find_child_node(input, NODE_HASH("fn_name"), 0xFFFFFFFF, PTR_NULL))
			continue;
		*/

		if (tx_is_app_item(prev_hash, oidx, &prev_tx, &app_item))
		{
			char appName[32];

			if (app_item == 2)
			{
				tree_manager_get_child_value_str(&prev_tx, NODE_HASH("appName"), appName, 32, 0);
				tree_manager_set_child_value_str(&tx, "appObj", appName);
			}
			release_zone_ref(&prev_tx);
		}

		tree_manager_get_child_value_btcaddr(input, NODE_HASH("srcaddr"), srcaddr);

		if (!memcmp_c(addr, srcaddr, sizeof(btc_addr_t)))
		{
			struct string script = { PTR_NULL };

			bin_2_hex(prev_hash, 32, ptchash);

			if (load_tx_output_amount(prev_hash, oidx, &amount))
				tree_manager_set_child_value_i64(input, "amount", amount);

			if (((hasobjtxfr)&&(amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000))
				send_obj	 (addr, tchash, iidx, ptchash, oidx, to_addr_list[0]);
			else
				spend_tx_addr(addr, tchash, iidx, ptchash, oidx, to_addr_list, n_to_addrs);

			tree_manager_set_child_value_btcaddr(input, "srcaddr", addr);
		}
	}


	if (is_vout_null(&tx, 0))
	{
		mem_zone_ref	vin = { PTR_NULL };
		btc_addr_t		stake_addr = { 0 };
		uint64_t		stake_in = 0;

		if (tree_manager_get_child_at(&txin_list, 0, &vin))
		{
			tree_manager_get_child_value_btcaddr(&vin, NODE_HASH("srcaddr"), stake_addr);
			tree_manager_get_child_value_i64(&vin, NODE_HASH("amount"), &stake_in);
			release_zone_ref(&vin);
		}
		release_zone_ref(&txin_list);

		if (!memcmp_c(stake_addr, addr, sizeof(btc_addr_t)))
			store_tx_staking(&tx, tx_hash, stake_addr, stake_in);
	}

	release_zone_ref(&txin_list);
	release_zone_ref(&txout_list);
	release_zone_ref(&tx);

	return 1;

}


OS_API_C_FUNC(int) store_wallet_tx(mem_zone_ref_ptr tx)
{
	hash_t				ffh;
	btc_addr_t			to_addr_list[16];
	char				tx_hash[65];
	mem_zone_ref		txout_list = { PTR_NULL }, txin_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr	out = PTR_NULL, input = PTR_NULL;
	unsigned int		oidx, iidx, hasobjtxfr, isobj;
	unsigned int		n_to_addrs, n_in_addr;
	
	

	if (!tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))return 0;
	if (!tree_manager_find_child_node(tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list)){ release_zone_ref(&txin_list); return 0; }
	
	tree_manager_get_child_value_str (tx, NODE_HASH("txid"), tx_hash, 65, 0);

	memset_c(ffh, 0xFF, sizeof(hash_t));
	
	n_in_addr = 0;
	n_to_addrs = 0;

	for (iidx = 0, tree_manager_get_first_child(&txin_list, &my_list, &input); ((input != NULL) && (input->zone != NULL)); tree_manager_get_next_child(&my_list, &input), iidx++)
	{
		hash_t			 prev_hash = { 0 };
		struct string	 script = { PTR_NULL };
		struct string	 out_path = { PTR_NULL };



		//add source address to the transaction list
		if (tree_manager_get_child_value_btcaddr(input, NODE_HASH("srcaddr"), src_addr_list[n_in_addr]))
		{
			unsigned int n, f;
			f = 0;
			for (n = 0; n < n_in_addr; n++)
			{
				if (!memcmp_c(src_addr_list[n_in_addr], src_addr_list[n], sizeof(btc_addr_t)))
				{
					f = 1;
					break;
				}
			}
			if (f == 0)n_in_addr++;
		}
	}

	
	for (tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); tree_manager_get_next_child(&my_list, &out))
	{
		struct string script = { 0 };
		if (!tree_manager_get_child_value_istr(out, NODE_HASH("script"), &script, 16))continue;
		if (script.len == 0){ free_string(&script); continue; }
		if (get_out_script_address(&script, PTR_NULL, to_addr_list[n_to_addrs]))
		{
			unsigned int n, f;
			f = 0;
			for (n = 0; n < n_to_addrs; n++)
			{
				if (!memcmp_c(to_addr_list[n_to_addrs], to_addr_list[n], sizeof(btc_addr_t)))
				{
					f = 1;
					break;
				}
			}
			if ((f == 0) && (n_to_addrs<15))
				n_to_addrs++;
		}
		free_string(&script);
	}
	hasobjtxfr = 0;
	isobj = 0;
	
	for (oidx = 0, tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); oidx++, tree_manager_get_next_child(&my_list, &out))
	{
		hash_t			bh, appH;
		mem_zone_ref	apps = { PTR_NULL }, app = { PTR_NULL };
		btc_addr_t		out_addr = { 0 };
		struct string	out_path = { 0 };
		struct string	objHashStr = { 0 };
		struct string   script = { PTR_NULL };
		struct string	pk = { PTR_NULL };
		uint64_t		amount = 0;
		int				addret = 0;

		tree_manager_get_child_value_i64(out, NODE_HASH("value"), &amount);

		if (amount == 0)continue;
		if (amount == 0xFFFFFFFFFFFFFFFF)continue;
		
		if (!tree_manager_get_child_value_istr(out, NODE_HASH("script"), &script, 0))
			continue;

		if (is_opfn_script(&script, PTR_NULL))
		{
			free_string(&script);
			continue;
		}

		addret = get_out_script_address(&script, &pk, out_addr);
		free_string(&pk);

		if (!addret)
		{
			free_string(&script);
			continue;
		}

		isobj = 0;

		if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
		{
			if (get_out_script_return_val(&script, &objHashStr))
			{
				if (objHashStr.len == 32)
				{
					mem_zone_ref	objTx = { PTR_NULL };
					if (load_tx(&objTx, bh, objHashStr.str))
					{
						hasobjtxfr = 1;
						get_tx_input_hash(&objTx, 0, appH);
						release_zone_ref(&objTx);
					}
				}
				else
				{
					free_string(&objHashStr);
					get_tx_input_hash(tx, 0, appH);

					objHashStr.str = malloc_c(32);
					objHashStr.len = 32;
					hex_2_bin(tx_hash, objHashStr.str, 32);
				}
				isobj = 1;
			}
		}
			
		make_string		(&out_path, "adrs");
		cat_ncstring_p	(&out_path, out_addr, 34);

		if (stat_file(out_path.str) == 0)
		{
			if (isobj)
			{
				char			appName[32];
				get_apps(&apps);
				if (tree_find_child_node_by_member_name_hash(&apps, NODE_BITCORE_TX, "txid", appH, &app))
				{
					tree_manager_get_child_value_str(&app, NODE_HASH("appName"), appName, 32, 0);
					release_zone_ref(&app);
				}
				release_zone_ref(&apps);
				add_wallet_obj(out_addr, tx_hash, appName, amount & 0xFFFFFFFF, objHashStr.str, src_addr_list[0]);
				
			}
			else
				add_unspent(out_addr, tx_hash, oidx, amount, src_addr_list, n_in_addr);
		}

		free_string(&out_path);
		free_string(&objHashStr);
		free_string(&script);
		
	}

	for (iidx = 0, tree_manager_get_first_child(&txin_list, &my_list, &input); ((input != NULL) && (input->zone != NULL)); tree_manager_get_next_child(&my_list, &input), iidx++)
	{
		hash_t			 prev_hash = { 0 };
		char			 ptchash[65];
		btc_addr_t		 out_addr;
		struct string	 pubk = { PTR_NULL };
		struct string	 script = { PTR_NULL };
		struct string	 out_path = { PTR_NULL };
		uint64_t		 amount;
		unsigned int	 oidx;
		

		tree_manager_get_child_value_hash(input, NODE_HASH("txid"), prev_hash);

		if (!memcmp_c(prev_hash, ffh, sizeof(hash_t)))
			continue;

		if (!memcmp_c(prev_hash, nullh, sizeof(hash_t)))
		{
			btc_addr_t coinbase;
			memset_c(coinbase, '0', sizeof(btc_addr_t));
			tree_manager_set_child_value_btcaddr(input, "srcaddr", coinbase);
			continue;
		}

		tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &oidx);

		bin_2_hex(prev_hash, 32, ptchash);

		if (!get_tx_output_script(prev_hash, oidx, &script, &amount))
			continue;
		
		if (is_opfn_script(&script, PTR_NULL))
		{
			free_string(&script);
			continue;
		}

		if (get_out_script_address(&script, &pubk, out_addr))
		{
			if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
			{
				if (hasobjtxfr)
					send_obj(out_addr, tx_hash, iidx, ptchash, oidx, to_addr_list[0]);
			}
			else
				spend_tx_addr(out_addr, tx_hash, iidx, ptchash, oidx, to_addr_list, n_to_addrs);
		}
		free_string(&pubk);
		free_string(&script);
		
	}


	release_zone_ref(&txin_list);
	release_zone_ref(&txout_list);
	return 1;

}

OS_API_C_FUNC(int) store_wallet_txs(mem_zone_ref_ptr tx_list)
{
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref_ptr	tx = PTR_NULL;

	for (tree_manager_get_first_child(tx_list, &my_list, &tx); ((tx != NULL) && (tx->zone != NULL)); tree_manager_get_next_child(&my_list, &tx))
	{
		if (is_tx_null(tx))continue;
		if (is_app_root(tx))continue;
		if (!store_wallet_tx(tx))
		{
			dec_zone_ref(tx);
			release_zone_ref(&my_list);
			return 0;
		}
	}

	return 1;
}


OS_API_C_FUNC(int) create_signature_script(const struct string *signature, dh_key_t pubkey, struct string *script)
{
	struct string sign_seq = { PTR_NULL };
	mem_zone_ref script_node = { PTR_NULL };

	if (!tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node))
		return 0;

	encode_DER_sig(signature, &sign_seq, 1, 1);

	tree_manager_set_child_value_vstr(&script_node, "var1", &sign_seq);

	if (pubkey != PTR_NULL)
	{
		mem_zone_ref			pkvar = { PTR_NULL };

		if (tree_manager_add_child_node(&script_node, "pk", NODE_BITCORE_PUBKEY, &pkvar))
		{
			tree_manager_write_node_data(&pkvar, pubkey, 0, 33);
			release_zone_ref(&pkvar);
		}
	}
	serialize_script(&script_node, script);

	free_string(&sign_seq);
	release_zone_ref(&script_node);


	return 1;
}

int find_staking_block(unsigned int now,mem_zone_ref_ptr unspents, hash_t ptxHash,unsigned int * OutIdx,unsigned int *tx_time)
{
	mem_zone_ref my_ulist = { PTR_NULL };
	mem_zone_ref_ptr unspent = PTR_NULL;
	unsigned int timeStart, timeEnd;


	timeStart = now;
	timeEnd = timeStart + 16;

	for (tree_manager_get_first_child(unspents, &my_ulist, &unspent); ((unspent != NULL) && (unspent->zone != NULL)); tree_manager_get_next_child(&my_ulist, &unspent))
	{
		hash_t rdiff, diff;
		struct string hash_data = { 0 };
		unsigned int ct, n;

		tree_manager_get_child_value_istr(unspent, NODE_HASH("hash_data"), &hash_data, 0);
		tree_manager_get_child_value_hash(unspent, NODE_HASH("difficulty"), rdiff);

		for (n = 0; n < 32; n++)
		{
			diff[31 - n] = rdiff[n];
		}

		for (ct = timeStart; ct < timeEnd; ct += 16) {
			hash_t h1, h2;
			struct string			total_hash = { 0 };


			total_hash.len = hash_data.len / 2;
			total_hash.size = total_hash.len + 4 + 1;
			total_hash.str = malloc_c(total_hash.size);
			hex_2_bin(hash_data.str, total_hash.str, total_hash.len);

			strbuffer_append_bytes(&total_hash, (char *)&ct, 4);

			mbedtls_sha256(total_hash.str, total_hash.len, h1, 0);
			mbedtls_sha256(h1, 32, h2, 0);
			free_string(&total_hash);

			if (cmp_hashle(h2, diff) >= 0)
			{
				*tx_time = ct;
				tree_manager_get_child_value_hash(unspent, NODE_HASH("txid"), ptxHash);
				tree_manager_get_child_value_i32(unspent, NODE_HASH("vout"), OutIdx);
				dec_zone_ref(unspent);
				release_zone_ref(&my_ulist);
				free_string(&hash_data);
				return 1;
			}
		}

		free_string(&hash_data);
	}

	return 0;
}

OS_API_C_FUNC(int) generate_staking_block(const struct string *username, unsigned int iminconf, mem_zone_ref_ptr newBlock)
{
	hash_t			blkhash;
	mem_zone_ref_ptr addr = PTR_NULL;
	mem_zone_ref	 account_name = { PTR_NULL },addrs = { PTR_NULL };
	mem_zone_ref	my_list = { PTR_NULL };
	mem_zone_ref	last_blk = { PTR_NULL };
	struct string	pos_hash_data = { PTR_NULL }, null_str = { PTR_NULL };
	unsigned int 	now;
	uint64_t		lb, nb, rew;
	int				max = 2000;
	int				ret = 0;
	unsigned int 	block_time;
	unsigned int	target, staking, timeout;
	
	char			toto = 0;


	if (!wallet_infos(&staking,&timeout))
		return 0;

	if (!staking)
		return 0;
	
	now = get_time_c();

	if (now & 0xF)
		return 0;

	wallet_last_staking = now;
	
	null_str.str = &toto;
	null_str.len = 0;
	null_str.size = 1;

	if (iminconf < min_staking_depth)
		iminconf = min_staking_depth;

	if (!tree_manager_find_child_node(&my_node, NODE_HASH("last_block"), NODE_BITCORE_BLK_HDR, &last_blk))
		return 0;

	if (!tree_manager_get_child_value_i64(&last_blk, NODE_HASH("height"), &lb))
	{
		release_zone_ref(&last_blk);
		return 0;
	}
	if (!tree_manager_get_child_value_i32(&last_blk, NODE_HASH("time"), &block_time))
	{
		release_zone_ref(&last_blk);
		return 0;
	}
	if (!tree_manager_get_child_value_hash(&last_blk, NODE_HASH("blkHash"), blkhash))
	{
		compute_block_hash(&last_blk, blkhash);
		tree_manager_set_child_value_hash(&last_blk, "blkHash", blkhash);
	}

	if (now <= block_time)
	{
		release_zone_ref(&last_blk);
		return 0;
	}
	tree_manager_create_node		("addrs", NODE_BITCORE_WALLET_ADDR_LIST, &addrs);

	tree_manager_create_node		("username", NODE_BITCORE_VSTR, &account_name);
	tree_manager_write_node_vstr	(&account_name, 0, username);

	wallet_list_addrs				(&account_name, &addrs, 0,0);

	release_zone_ref				(&account_name);

	nb = lb + 1;
	get_stake_reward					(nb, &rew);
	get_target_spacing					(&target);
	ret = 0;

	for (tree_manager_get_first_child(&addrs, &my_list, &addr); ((addr != NULL) && (addr->zone != NULL)); tree_manager_get_next_child(&my_list, &addr))
	{
		hash_t ptxHash;
		btc_addr_t my_addr;
		mem_zone_ref unspents = { PTR_NULL };
		unsigned int new_time;
		unsigned int OutIdx;
		if (!tree_manager_get_child_value_btcaddr(addr, NODE_HASH("address"), my_addr))continue;

		tree_manager_create_node("staking", NODE_JSON_ARRAY, &unspents);
		if (!list_staking_unspent(&last_blk, my_addr, &unspents, iminconf, &max))
		{
			release_zone_ref(&unspents);
			continue;
		}
	
		if (find_staking_block(now, &unspents,ptxHash,&OutIdx, &new_time))
		{
			btc_addr_t				pubaddr;
			struct string			script = { 0 }, oscript = { 0 }, sPubk = { 0 };
			mem_zone_ref			newtx = { PTR_NULL };
			uint64_t				amount, half_am;
			
			ret = get_tx_output_script				(ptxHash, OutIdx, &script, &amount);
			if (ret) ret = get_out_script_address	(&script, &sPubk, pubaddr);
			if (ret) ret = new_transaction			(&newtx, new_time);

			if (ret)
			{
				if (sPubk.str == PTR_NULL)
				{
					dh_key_t		my_key, rpkey;
					mem_zone_ref	script_node = { PTR_NULL };
					int				cnt;

					ret = get_anon_key(pubaddr, my_key);
					if (ret)
					{
						dh_key_t mypub, mycpub;
						extract_pub(my_key, mypub);
						compress_key(mypub, mycpub);

						rpkey[0] = mycpub[0];
						for (cnt = 1; cnt < 33; cnt++)
						{
							rpkey[(32 - cnt) + 1] = mycpub[cnt];
						}
					}

					if (ret)ret = tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node);
					if (ret)
					{
						struct string	bpubkey = { PTR_NULL };

						bpubkey.str = rpkey;
						bpubkey.len = 33;
						bpubkey.size = 33;

						create_payment_script(&bpubkey, 0, &script_node);
						serialize_script(&script_node, &oscript);
						release_zone_ref(&script_node);
					}
				}
				else
				{
					clone_string(&oscript, &script);
				}
			}

			if (ret)
			{
				half_am = muldiv64(amount + rew, 1, 2);
				tx_add_input(&newtx, ptxHash, OutIdx, &script);
				tx_add_output(&newtx, 0, &null_str);
				tx_add_output(&newtx, half_am, &oscript);
				tx_add_output(&newtx, half_am, &oscript);
			}

			free_string(&oscript);
			free_string(&sPubk);
			free_string(&script);

			if (ret) ret = create_pos_block					(lb, &newtx, newBlock);
			if (ret) ret = tree_manager_set_child_value_i64	(newBlock, "reward", rew);
			if (ret)log_message								("found new staking block %blkHash%", newBlock);

			release_zone_ref(&newtx);
		}
	
		release_zone_ref(&unspents);
		if (ret)
		{
			dec_zone_ref(addr);
			release_zone_ref(&my_list);
			break;
		}
	}
	

	release_zone_ref(&addrs);
	release_zone_ref(&last_blk);
	
	return ret;

}


OS_API_C_FUNC(int) sign_staking_block(struct string *account_name, mem_zone_ref_ptr block)
{
	hash_t rh;
	dh_key_t privkey;
	hash_t blkMerkle, blkHash;
	struct string inPk = { PTR_NULL }, sign = { PTR_NULL }, sig_seq = { PTR_NULL };
	mem_zone_ref tx_list = { PTR_NULL }, tx = { PTR_NULL }, input = { 0 }, blkSig = { PTR_NULL };
	size_t blockSize;
	int ret;
	int n;

	

	if (!has_valid_pw())
		return 0;

	

	if (!tree_manager_find_child_node(block, NODE_HASH("txs"), NODE_BITCORE_TX_LIST, &tx_list))
		return 0;

	ret = tree_manager_get_child_at(&tx_list, 0, &tx);
	if (ret)ret = is_tx_null(&tx);
	if (ret)ret = tree_manager_get_child_at(&tx_list, 1, &tx);
	if (ret)ret = is_vout_null(&tx, 0);

	if (!ret)
	{
		release_zone_ref(&tx);
		release_zone_ref(&tx_list);
		return 0;
	}

	ret = get_tx_input(&tx, 0, &input);

	
	if (ret)
	{
		hash_t  signH, ptxHash;
		btc_addr_t	my_addr;
		uint64_t amount;
		struct string script = { PTR_NULL };
		unsigned int OutIdx;
		

		tree_manager_get_child_value_hash		(&input, NODE_HASH("txid"), ptxHash);
		tree_manager_get_child_value_i32		(&input, NODE_HASH("idx") , &OutIdx);

		ret = get_tx_output_script				(ptxHash, OutIdx, &script , &amount);
		if (ret) ret = get_out_script_address	(&script, &inPk, my_addr);
		if (ret) ret = compute_tx_sign_hash		(&tx, 0, &script, 1, signH);
		free_string								(&script);

		if (ret)ret = get_anon_key				(my_addr, privkey);
		if (ret)
		{
			for (n = 0; n < 32; n++)
			{
				rh[31 - n] = signH[n];
			}
			ret = sign_hash(rh, privkey, &sign);
		}
	}
	

	if (ret)
	{
		struct string script = { PTR_NULL };

		if (inPk.str == PTR_NULL)
		{
			dh_key_t	  mypub, mycpub, rpkey;
			

			ret = extract_pub(privkey, mypub);
			if (ret) {
				int		cnt;
				ret = compress_key(mypub, mycpub);
				rpkey[0] = mycpub[0];
				for (cnt = 1; cnt < 33; cnt++)
				{
					rpkey[(32 - cnt) + 1] = mycpub[cnt];
				}
			}
			if (ret)ret = create_signature_script(&sign, rpkey, &script);
		}
		else
		{
			ret = create_signature_script(&sign, inPk.str, &script);
		}

		if (ret)ret = tree_manager_set_child_value_vstr(&input, "script", &script);

		free_string(&script);

	}


	free_string(&inPk);
	free_string(&sign);
	
	release_zone_ref(&input);
	release_zone_ref(&tx);

	if (!ret)
	{
		release_zone_ref(&tx_list);
		return 0;
	}
	

	build_merkel_tree					(&tx_list, blkMerkle, PTR_NULL);
	tree_manager_set_child_value_hash	(block, "merkle_root", blkMerkle);
	compute_block_hash					(block, blkHash);
	tree_manager_set_child_value_hash	(block, "blkHash", blkHash);

	get_block_size						(block, &tx_list);

	release_zone_ref					(&tx_list);
	
	for (n = 0; n < 32; n++)
	{
		rh[31 - n] = blkHash[n];
	}

	sign_hash							(rh, privkey, &sign);
	encode_DER_sig						(&sign, &sig_seq,1, 1);
	   
	if (!tree_manager_find_child_node(block, NODE_HASH("signature"), NODE_BITCORE_ECDSA_SIG, &blkSig))
		tree_manager_add_child_node(block, "signature", NODE_BITCORE_ECDSA_SIG, &blkSig);

	tree_manager_write_node_sig(&blkSig, 0, sig_seq.str, sig_seq.len);

	if (!tree_manager_get_child_value_i32(block, NODE_HASH("size"), &blockSize))
		blockSize = 0;

	blockSize += sig_seq.len;

	tree_manager_set_child_value_i32(block, "size", blockSize);
	

	free_string(&sig_seq);
	free_string(&sign);
	release_zone_ref(&blkSig);


	memset_c(privkey, 0, sizeof(dh_key_t));
	return ret;
}

OS_API_C_FUNC(int) wallet_list_addrs(mem_zone_ref_ptr account_name, mem_zone_ref_ptr addr_list,unsigned int balance, unsigned int minconf)
{
	struct string username = { PTR_NULL };
	struct string user_key_file = { PTR_NULL };
	struct string user_pw_file = { PTR_NULL };
	struct string pw = { PTR_NULL };
	size_t		  keys_data_len = 0;
	size_t		  of;
	unsigned char *keys_data = PTR_NULL;


	tree_remove_children		(addr_list);

	tree_manager_get_node_istr	(account_name, 0, &username, 0);


	uname_cleanup(&username);

	of = strlpos_c(username.str, 0, ':');
	if (of != INVALID_SIZE)
	{
		make_string	(&pw, &username.str[of + 1]);
		username.str[of] = 0;
		username.len = of;
	}
	
	if (username.len < 3)
	{
		free_string(&username);
		free_string(&pw);
		return 0;
	}

	make_string		(&user_key_file, "keypairs");
	cat_cstring_p	(&user_key_file, username.str);
	free_string		(&username);
	
	
	
	if (get_file(user_key_file.str, &keys_data, &keys_data_len))
	{
		struct key_entry *keys_ptr = (struct key_entry *)keys_data;
		while (keys_data_len >= sizeof(struct key_entry))
		{
			mem_zone_ref new_addr = { PTR_NULL };

			if(tree_manager_create_node("addr", NODE_GFX_OBJECT, &new_addr))
			{
				tree_manager_set_child_value_str		(&new_addr, "label"  , keys_ptr->label);
				tree_manager_set_child_value_btcaddr	(&new_addr, "address", keys_ptr->addr);

				if (balance & 0xF)
				{
					uint64_t conf_amount = 0, unconf_amount = 0, addrOut = 0, addrIn = 0;
					int64_t	 total_unconf,total_amount;


					get_balance			(keys_ptr->addr, &conf_amount, &unconf_amount, minconf);
					get_addr_balance	(keys_ptr->addr, &addrOut, &addrIn);

					total_amount = conf_amount + unconf_amount;

					total_unconf = unconf_amount + addrIn - addrOut;

					if ( (balance & 2)||(total_amount>0))
					{
						tree_manager_set_child_value_i64	(&new_addr, "amount", conf_amount);
						tree_manager_set_child_value_si64	(&new_addr, "unconf_amount", total_unconf);
						tree_manager_node_add_child			(addr_list, &new_addr);
					}
				}
				else
					tree_manager_node_add_child(addr_list, &new_addr);
			
				release_zone_ref(&new_addr);
			}
			keys_ptr++;
			keys_data_len -= sizeof(struct key_entry);

			do_mark_sweep(get_tree_mem_area_id(), 250);
		}
		free_c(keys_data);

		
	}

	free_string(&user_key_file);

	return 1;
}



OS_API_C_FUNC(int) add_keypair(struct string *username, const char *clabel, btc_addr_t pubaddr, dh_key_t priv, unsigned int rescan, unsigned int *found)
{
	struct string user_key_file = { 0 }, adr_path = { 0 };
	size_t keys_data_len = 0;
	unsigned char *keys_data = PTR_NULL;

	init_string		(&user_key_file);
	make_string		(&user_key_file, "keypairs");
	cat_cstring_p	(&user_key_file, username->str);

	*found = 0;

	if (get_file(user_key_file.str, &keys_data, &keys_data_len) > 0)
	{
		struct key_entry	*keys_ptr = (struct key_entry *)keys_data;
		size_t				flen;

		flen = keys_data_len;
		while (keys_data_len > 0)
		{
			if (!memcmp_c(keys_ptr->addr, pubaddr, sizeof(btc_addr_t)))
			{
				if (strcmp_c(clabel, keys_ptr->label))
				{
					memcpy_c(keys_ptr->label, clabel, 32);
					put_file(user_key_file.str, keys_data, flen);
				}
				*found = 1;
				break;
			}
			keys_ptr++;
			keys_data_len -= sizeof(struct key_entry);
		}
		free_c(keys_data);
	}

	if (!(*found))
	{
		struct key_entry new_entry;

		strcpy_cs	(new_entry.label, 32, clabel);
		memcpy_c	(new_entry.addr	, pubaddr, sizeof(btc_addr_t));
		memcpy_c	(new_entry.key	, priv, sizeof(dh_key_t));

		append_file	(user_key_file.str, &new_entry, sizeof(struct key_entry));
	}
	free_string		(&user_key_file);

	init_string		(&adr_path);
	make_string		(&adr_path, "adrs");
	cat_ncstring_p	(&adr_path, pubaddr, sizeof(btc_addr_t));
	
	if ((!(*found)) || (rescan))
	{
		mem_zone_ref tx_list = { 0 };
		if (stat_file(adr_path.str) == 0)
		{
			struct string path = { 0 };
			clone_string(&path, &adr_path);
			cat_cstring_p(&path, "spent");
			rm_dir(path.str);
			free_string(&path);

			clone_string(&path, &adr_path);
			cat_cstring_p(&path, "unspent");
			rm_dir(path.str);
			free_string(&path);

			clone_string(&path, &adr_path);
			cat_cstring_p(&path, "objects");
			rm_dir(path.str);
			free_string(&path);

			clone_string(&path, &adr_path);
			cat_cstring_p(&path, "sentobjs");
			rm_dir(path.str);
			free_string(&path);

			clone_string(&path, &adr_path);
			cat_cstring_p(&path, "stakes");
			del_file(path.str);
			free_string(&path);
		}
		rm_dir(adr_path.str);
		create_dir(adr_path.str);
		if (tree_manager_create_node("txs", NODE_BITCORE_HASH_LIST, &tx_list))
		{
			mem_zone_ref my_list = { PTR_NULL };
			mem_zone_ref_ptr tx = PTR_NULL;

			size_t idx = 0;

			while (load_tx_addresses(pubaddr, &tx_list, idx, 1000))
			{
				for (tree_manager_get_first_child(&tx_list, &my_list, &tx); ((tx != NULL) && (tx->zone != NULL)); tree_manager_get_next_child(&my_list, &tx))
				{
					hash_t tx_hash;
					tree_manager_get_node_hash(tx, 0, tx_hash);
					store_tx_wallet(pubaddr, tx_hash);
				}

				if (!tree_manager_create_node("txs", NODE_BITCORE_HASH_LIST, &tx_list))
					break;

				idx += 1000;
			}
			release_zone_ref(&tx_list);
		}
	}

	free_string(&adr_path);

	return 1;
}


OS_API_C_FUNC(int) get_privkey(struct string *username, struct string *pubaddr,dh_key_t key)
{
	struct string user_key_file = { 0 }, adr_path = { 0 };
	size_t keys_data_len = 0;
	unsigned char *keys_data = PTR_NULL;
	int ret=0;

	make_string		(&user_key_file, "keypairs");
	cat_cstring_p	(&user_key_file, username->str);
	
	if (get_file(user_key_file.str, &keys_data, &keys_data_len)>0)
	{
		struct key_entry *keys_ptr = (struct key_entry *)keys_data;
		
		while (keys_data_len >= sizeof(struct key_entry))
		{
			if (!strncmp_c(keys_ptr->addr, pubaddr->str, sizeof(btc_addr_t)))
			{
				memcpy_c(key, keys_ptr->key, sizeof(dh_key_t));
				ret = 1;
				break;
			}
			keys_ptr++;
			keys_data_len -= sizeof(struct key_entry);
		}
		free_c(keys_data);
	}

	free_string(&user_key_file);


	return ret;
}

OS_API_C_FUNC(int) set_anon_pw(const char *pw, unsigned int staking,unsigned int timeout)
{
	hash_t				h1,pwh;
	struct string		user_pw_file={ 0 };
	unsigned char		*data;
	size_t				pw_len,len;
	int					ret=0;
	/*
	wallet_pw_set = 0;
	staking_enabled = 0;
	*/

	strcpy_cs			(anon_wallet_pw, 1024, pw);

	pw_len = strlen_c	(anon_wallet_pw);

	mbedtls_sha256		(anon_wallet_pw, pw_len, h1, 0);
	mbedtls_sha256		(h1, 32, pwh, 0);

	
	wallet_pw_timeout = get_time_c() + timeout;

	make_string	(&user_pw_file, "acpw/anonymous");
	
	if (get_file(user_pw_file.str,&data,&len)>0)
	{
		if ((len == 32) && (!memcmp_c(data, pwh, sizeof(hash_t))))
			ret = 1;

		free_c(data);
	}
	else
	{
		put_file(user_pw_file.str, pwh, 32);
		ret = 1;
	}

	if (ret)
	{
		staking_enabled = staking;
		wallet_pw_set = 1;
	}
		

	free_string(&user_pw_file);
	
	return ret;

}


OS_API_C_FUNC(int) privkey_to_addr(const dh_key_t privkey, struct string *privAddr)
{
	struct string	in_addr;
	hash_t			tmp_hash, fhash;
	unsigned char	taddr[38];
	unsigned int	n,i;
	

	if (!tree_manager_get_child_value_i32(&my_node, NODE_HASH("privKeyVersion"), &i))
		return 0;
	
	taddr[0] = i;

	for (n = 0; n < 32; n++)
	{
		taddr[1 + n] = privkey[31 - n];
	}
	taddr[33] = 0x01;

	mbedtls_sha256(taddr, 34, tmp_hash, 0);
	mbedtls_sha256(tmp_hash, 32, fhash, 0);

	memcpy_c(&taddr[34], fhash, 4);

	in_addr.str = taddr;
	in_addr.len = 38;
	in_addr.size = in_addr.len;

	return b58enc(&in_addr, privAddr);

}


OS_API_C_FUNC(int) generate_new_keypair(const char *clabel,btc_addr_t pubaddr)
{
	struct string		uname = { 0 };
	dh_key_t			privkey, pubkey, cpub, rcpub, enckey;
	unsigned int		found;
	int					ok = 0;
	int					n;

	if (!has_valid_pw())
		return 0;

	memset_c(privkey, 0, sizeof(dh_key_t));
	memset_c(pubkey, 0, sizeof(dh_key_t));
	memset_c(cpub, 0, sizeof(dh_key_t));
	memset_c(rcpub, 0, sizeof(dh_key_t));

	while (!ok)
	{
		ok=extract_key	(privkey,pubkey);
	}

	compress_key		(pubkey,cpub);
	rcpub[0] = cpub[0];
	for (n = 1; n < 33; n++)
	{
		rcpub[(32 - n) + 1] = cpub[n];
	}

	key_to_addr			(rcpub, pubaddr);

	RC4					(anon_wallet_pw, privkey, sizeof(dh_key_t), enckey);

	make_string			(&uname,"anonymous");
	add_keypair			(&uname, clabel, pubaddr, enckey, 0, &found);
	free_string			(&uname);

	return 1;
}

OS_API_C_FUNC(int) get_anon_key(btc_addr_t pubaddr,dh_key_t privkey)
{
	dh_key_t key;
	struct string uname = { 0 }, strKey = { 0 };
	int ret;

	if (!has_valid_pw())
		return 0;

	make_string(&uname, "anonymous");
	make_string(&strKey, pubaddr);

	memset_c(key, 0, sizeof(dh_key_t));
	memset_c(privkey, 0, sizeof(dh_key_t));
	
	ret = get_privkey(&uname, &strKey, key);

	free_string(&strKey);
	free_string(&uname);
	
		

	if(ret) RC4(anon_wallet_pw, key, sizeof(dh_key_t), privkey);
	return ret;
}

OS_API_C_FUNC(int) get_account_list(mem_zone_ref_ptr accnt_list,unsigned int page_idx)
{
	struct string	user_list = { PTR_NULL }, user_key_file = { PTR_NULL };
	size_t			nfiles;

	if ((nfiles = get_sub_files("keypairs", &user_list)) > 0)
	{
		size_t		dir_list_len;
		const char *ptr, *optr;
		size_t		cur;

		dir_list_len = user_list.len;
		optr = user_list.str;
		cur = 0;
		while (cur < nfiles)
		{
			struct string	user_name = { PTR_NULL };
			mem_zone_ref	accnt = { PTR_NULL };
			size_t			sz;

			ptr = memchr_c(optr, 10, dir_list_len);
			sz = mem_sub(optr, ptr);

			make_string_l(&user_name, optr, sz);
			make_string(&user_key_file, "keypairs");
			cat_cstring_p(&user_key_file, user_name.str);

			if (tree_manager_add_child_node(accnt_list, user_name.str, NODE_GFX_OBJECT, &accnt))
			{
				struct string user_pw_file = { PTR_NULL };

				tree_manager_set_child_value_vstr(&accnt, "name", &user_name);

				make_string(&user_pw_file, "acpw");
				cat_cstring_p(&user_pw_file, user_name.str);
				if (stat_file(user_pw_file.str) == 0)
					tree_manager_set_child_value_i32(&accnt, "pw", 1);
				else
					tree_manager_set_child_value_i32(&accnt, "pw", 0);

				free_string(&user_pw_file);
				release_zone_ref(&accnt);
			}

			free_string(&user_name);
			free_string(&user_key_file);
			cur++;

			optr = ptr + 1;
			dir_list_len -= sz;
		}
	}
	free_string(&user_list);

	return 1;
}


OS_API_C_FUNC(int) checkpassword(struct string *username, struct string *pw)
{
	struct string		user_pw_file = { PTR_NULL };
	int					ret = 0;
	unsigned char		*pwh_ptr;
	size_t				len;

	make_string(&user_pw_file, "acpw");
	cat_cstring_p(&user_pw_file, username->str);

	ret = get_file(user_pw_file.str, &pwh_ptr, &len) > 0 ? 1 : 0;
	if (ret)
	{
		if (len == 32)
		{
			hash_t		ipwh;
			mbedtls_sha256(pw->str, pw->len, ipwh, 0);
			ret = memcmp_c(ipwh, pwh_ptr, sizeof(hash_t)) == 0 ? 1 : 0;
		}
		else
			ret = 0;
		free_c(pwh_ptr);
	}
	return ret;

}

OS_API_C_FUNC(int) uname_cleanup(struct string *uname)
{
	size_t n;

	if (uname->len < 3)
		return 0;

	if (uname->len > 64)
	{
		uname->str[63] = 0;
		uname->len = 64;
	}

	n = uname->len;

	while (n--)
	{
		if ((uname->str[n] != '_') && (!isdigit_c(uname->str[n])) && (!isalpha_c(uname->str[n])))
			uname->str[n] = '-';
	}

	return 1;
}
OS_API_C_FUNC(int) get_sess_account(mem_zone_ref_ptr sessid, mem_zone_ref_ptr account_name)
{
	char sessionid[16];
	struct string sessionfile = { PTR_NULL };
	unsigned char *data;
	size_t len;

	tree_manager_get_node_str(sessid, 0, sessionid, 16, 16);

	if (strlen_c(sessionid) < 8)return 0;

	make_string(&sessionfile, "sess");
	cat_cstring_p(&sessionfile, sessionid);
	if (get_file(sessionfile.str, &data, &len) > 0)
	{
		struct string sacnt = { PTR_NULL };
		make_string_l(&sacnt, data, len);

		tree_manager_write_node_str(account_name, 0, sacnt.str);
		free_string(&sacnt);
	}
	free_string(&sessionfile);

	return 1;
}


OS_API_C_FUNC(int) setpassword(struct string *username, struct string *pw, struct string *newpw)
{
	struct string		user_pw_file = { PTR_NULL };
	int					ret=0;
	unsigned char		*pwh_ptr;
	size_t				len;

	make_string(&user_pw_file, "acpw");
	cat_cstring_p(&user_pw_file, username->str);

	if ((get_file(user_pw_file.str, &pwh_ptr, &len) > 0) && (len == sizeof(hash_t)))
	{
		hash_t		ipwh;
		mbedtls_sha256(pw->str, pw->len, ipwh, 0);
		if (!memcmp_c(ipwh, pwh_ptr, sizeof(hash_t)))
		{
			hash_t		npwh;
			mbedtls_sha256	(newpw->		str, newpw->len, npwh, 0);
			put_file		(user_pw_file.str, npwh, sizeof(hash_t));
			ret = 1;
		}
		free_c(pwh_ptr);
	}
	else
	{
		hash_t				pwh;

		mbedtls_sha256	(pw->str, pw->len, pwh, 0);
		put_file		(user_pw_file.str, pwh, 32);
		ret = 1;
	}

	free_string(&user_pw_file);
	return ret;
}


int lock_addr_scan(btc_addr_t addr)
{
	struct string lck_path = { 0 };

	make_string(&lck_path, "node");
	cat_cstring_p(&lck_path, addr);

	if (stat_file(lck_path.str) == 0)return 0;
	put_file(lck_path.str, "on", 2);

	free_string(&lck_path);

	return 1;
}

void rm_addr_scan(btc_addr_t addr)
{
	struct string lck_path = { 0 };

	make_string(&lck_path, "node");
	cat_cstring_p(&lck_path, addr);

	del_file(lck_path.str);

	free_string(&lck_path);
}


OS_API_C_FUNC(int) rescan_addr(btc_addr_t pubaddr)
{
	mem_zone_ref	 tx_list = { PTR_NULL }, txlist = { PTR_NULL };
	struct string	 adr_path = { PTR_NULL };
	mem_zone_ref_ptr tx = PTR_NULL;

	if (pubaddr == PTR_NULL)return 0;
	if (strlen_c(pubaddr) < 34)return 0;

	if (!lock_addr_scan(pubaddr))return 0;

	make_string(&adr_path, "adrs");
	cat_ncstring_p(&adr_path, pubaddr, 34);

	if (stat_file(adr_path.str) == 0)
	{
		struct string path = { 0 };
		clone_string(&path, &adr_path);
		cat_cstring_p(&path, "spent");
		rm_dir(path.str);
		create_dir(path.str);
		free_string(&path);

		clone_string(&path, &adr_path);
		cat_cstring_p(&path, "objects");
		rm_dir(path.str);
		create_dir(path.str);
		free_string(&path);

		clone_string(&path, &adr_path);
		cat_cstring_p(&path, "sentobjs");
		rm_dir(path.str);
		create_dir(path.str);
		free_string(&path);

		clone_string(&path, &adr_path);
		cat_cstring_p(&path, "unspent");
		rm_dir(path.str);
		create_dir(path.str);
		free_string(&path);

		clone_string(&path, &adr_path);
		cat_cstring_p(&path, "stakes");
		del_file(path.str);
		free_string(&path);
	}

	rm_dir(adr_path.str);
	create_dir(adr_path.str);

	if (tree_manager_create_node("txs", NODE_BITCORE_HASH_LIST, &tx_list))
	{
		size_t idx=0;

		while (load_tx_addresses(pubaddr, &tx_list, idx, 1000))
		{

			if (tree_manager_get_node_num_children(&tx_list) <= 0)
				break;

			for (tree_manager_get_first_child(&tx_list, &txlist, &tx); ((tx != NULL) && (tx->zone != NULL)); tree_manager_get_next_child(&txlist, &tx))
			{
				hash_t						tx_hash;
				tree_manager_get_node_hash(tx, 0, tx_hash);
				store_tx_wallet(pubaddr, tx_hash);
				do_mark_sweep(get_tree_mem_area_id(), 500);
			}

			if (!tree_manager_create_node("txs", NODE_BITCORE_HASH_LIST, &tx_list))
				break;

			idx += 1000;
		}
		release_zone_ref(&tx_list);
	}

	free_string(&adr_path);

	rm_addr_scan(pubaddr);

	return 1;
}

