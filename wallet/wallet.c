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

#ifdef _DEBUG
C_IMPORT int			C_API_FUNC		get_last_stake_modifier(mem_zone_ref_ptr pindex, hash_t nStakeModifier, unsigned int *nModifierTime);
C_IMPORT int			C_API_FUNC		get_tx_pos_hash_data(mem_zone_ref_ptr hdr, const hash_t txHash, unsigned int OutIdx, struct string *hash_data, uint64_t *amount, hash_t out_diff);
C_IMPORT int			C_API_FUNC		get_blk_staking_infos(mem_zone_ref_ptr blk, const char *blk_hash, mem_zone_ref_ptr infos);
C_IMPORT int			C_API_FUNC		store_tx_staking(mem_zone_ref_ptr tx, hash_t tx_hash, btc_addr_t stake_addr, uint64_t	stake_in);
C_IMPORT int			C_API_FUNC		get_target_spacing(unsigned int *target);
C_IMPORT unsigned int	C_API_FUNC		get_current_pos_difficulty();
C_IMPORT int			C_API_FUNC		get_stake_reward(uint64_t height, uint64_t *reward);
C_IMPORT int			C_API_FUNC		compute_tx_pos(mem_zone_ref_ptr tx, hash_t StakeModifier, unsigned int txTime, hash_t pos_hash, uint64_t *weight);
C_IMPORT int			C_API_FUNC		create_pos_block(hash_t pHash, mem_zone_ref_ptr tx, mem_zone_ref_ptr newBlock);
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
mem_zone_ref			my_node = { PTR_INVALID };
btc_addr_t				src_addr_list[1024] = { 0xCDFF };

char					anon_wallet_pw[1024] = { 0xFF };
unsigned int			wallet_pw_set = 0xFFFFFFFF;
unsigned int			wallet_pw_timeout = 0xFFFFFFFF;

OS_API_C_FUNC(int) reset_anon_pw()
{
	memset_c(anon_wallet_pw, 0, sizeof(anon_wallet_pw));
	wallet_pw_set = 0;
	wallet_pw_timeout = 0;

	return 1;
}


OS_API_C_FUNC(int) init_wallet(mem_zone_ref_ptr node, tpo_mod_file *pos_mod)
{
	my_node.zone = PTR_NULL;
	copy_zone_ref(&my_node, node);
	
	reset_anon_pw();
	

#ifndef _DEBUG
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


OS_API_C_FUNC(int)  get_tx_inputs_from_addr(btc_addr_t addr,mem_zone_ref_ptr mempool, uint64_t *total_unspent, uint64_t min_amount, size_t min_conf, size_t max_conf, mem_zone_ref_ptr tx)
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


	make_string(&unspent_path, "adrs");
	cat_ncstring_p(&unspent_path, addr, 34);
	cat_cstring_p(&unspent_path, "unspent");

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

		if (((mempool == PTR_NULL) || (!find_inputs(mempool, hash, output))) && (check_utxo(optr, output)))
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
						if (check_utxo(optr, output))
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
	}
	free_string(&dir_list);
	free_string(&unspent_path);

	return 1;
}


OS_API_C_FUNC(int)  list_unspent(btc_addr_t addr, mem_zone_ref_ptr unspents, mem_zone_ref_ptr mempool, size_t min_conf, size_t max_conf, uint64_t *total_unspent, size_t *ntx, size_t *max, size_t first)
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
		if ((mempool==PTR_NULL)||(!find_inputs(mempool, hash, output)))
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




OS_API_C_FUNC(int) list_staking_unspent(mem_zone_ref_ptr last_blk, btc_addr_t addr, mem_zone_ref_ptr mempool, mem_zone_ref_ptr unspents, unsigned int min_depth, int *max)
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

		if (!find_inputs(mempool, hash, output))
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
							tree_manager_set_child_value_str(&unspent, "hash_data", pos_hash_data.str);
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
	}
	free_string(&dir_list);
	free_string(&unspent_path);

	return 1;
}

OS_API_C_FUNC(int)  get_balance(btc_addr_t addr, uint64_t *conf_amount, uint64_t *amount, unsigned int minconf)
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
				free_c(data);
			}
		}
		free_string(&tx_path);
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
		if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)continue;
		if (amount == 0xFFFFFFFFFFFFFFFF)continue;

		tree_manager_get_child_value_istr	(output, NODE_HASH("script"), &script, 0);

		if (get_out_script_address(&script, &pubk, out_addr))
		{
			char chash[65];
			/*
			int	n = 0;
			while (n < 32)
			{
				chash[n * 2 + 0] = hex_chars[tx_hash[n] >> 4];
				chash[n * 2 + 1] = hex_chars[tx_hash[n] & 0x0F];
				n++;
			}
			chash[64] = 0;
			*/

			bin_2_hex(tx_hash, 32, chash);

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

				if ((amount != 0) && ((amount & 0xFFFFFFFF00000000) != 0xFFFFFFFF00000000) && (amount != 0xFFFFFFFFFFFFFFFF))
				{
					bin_2_hex(prev_hash, 32, pchash);

					/*
					int	n = 0;
					while (n < 32)
					{
						pchash[n * 2 + 0] = hex_chars[prev_hash[n] >> 4];
						pchash[n * 2 + 1] = hex_chars[prev_hash[n] & 0x0F];
						n++;
					}
					pchash[64] = 0;
					*/

					if (tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &script, 16))
					{
						if (get_out_script_address(&script, PTR_NULL, out_addr))
						{
							//cancel the spent in the wallet
							cancel_spend_tx_addr(out_addr, pchash, oidx);

							//remove tx from the address index
							remove_tx_addresses(out_addr, tx_hash);
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


OS_API_C_FUNC(int) store_tx_wallet(btc_addr_t addr, hash_t tx_hash)
{
	char			 tchash[65];
	hash_t			 blk_hash, null_hash;
	btc_addr_t	 	 to_addr_list[16];

	struct string	 tx_path = { 0 };
	mem_zone_ref	 txin_list = { PTR_NULL }, txout_list = { PTR_NULL }, my_list = { PTR_NULL }, tx = { PTR_NULL };
	mem_zone_ref_ptr input = PTR_NULL, out = PTR_NULL;
	unsigned int	 oidx, iidx;
	unsigned int	 n_to_addrs;
	unsigned int	 n_in_addr;


	memset_c(null_hash, 0, 32);

	if (!load_tx(&tx, blk_hash, tx_hash))return 0;
	if (is_tx_null(&tx))
	{
		release_zone_ref(&tx);
		return 0;
	}
	if (!tree_manager_find_child_node(&tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))return 0;
	if (!tree_manager_find_child_node(&tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list)){ release_zone_ref(&txin_list); return 0; }

	/*
	unsigned int n = 0;
	while (n < 32)
	{
		tchash[n * 2 + 0] = hex_chars[tx_hash[n] >> 4];
		tchash[n * 2 + 1] = hex_chars[tx_hash[n] & 0x0F];
		n++;
	}
	tchash[64] = 0;
	*/

	bin_2_hex(tx_hash, 32, tchash);


	n_to_addrs = 0;
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


	n_in_addr = 0;
	for (iidx = 0, tree_manager_get_first_child(&txin_list, &my_list, &input); ((input != NULL) && (input->zone != NULL)); tree_manager_get_next_child(&my_list, &input), iidx++)
	{
		char			ptchash[65];
		hash_t			prev_hash = { 0xFF };
		struct string	out_path = { 0 };
		int				my_addr;
		uint64_t		amount;

		tree_manager_get_child_value_hash(input, NODE_HASH("txid"), prev_hash);
		tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &oidx);

		my_addr = 0;

		//coin base
		if (!memcmp_c(prev_hash, null_hash, sizeof(hash_t)))
		{
			memset_c(src_addr_list[0], '0', sizeof(btc_addr_t));
			n_in_addr = 1;
			continue;
		}


		if (get_tx_output_addr(prev_hash, oidx, src_addr_list[n_in_addr]))
		{
			unsigned int nn, f;
			f = 0;
			for (nn = 0; nn < n_in_addr; nn++)
			{
				if (!memcmp_c(src_addr_list[n_in_addr], src_addr_list[nn], sizeof(btc_addr_t)))
				{
					f = 1;
					break;
				}
			}

			if (!memcmp_c(addr, src_addr_list[n_in_addr], sizeof(btc_addr_t)))
				my_addr = 1;

			if (f == 0)
				n_in_addr++;

		}

		if (my_addr)
		{
			/*
			n = 0;
			while (n < 32)
			{
				ptchash[n * 2 + 0] = hex_chars[prev_hash[n] >> 4];
				ptchash[n * 2 + 1] = hex_chars[prev_hash[n] & 0x0F];
				n++;
			}
			ptchash[64] = 0;
			*/

			bin_2_hex(prev_hash, 32, ptchash);


			spend_tx_addr(addr, tchash, iidx, ptchash, oidx, to_addr_list, n_to_addrs);

			if (load_tx_output_amount(prev_hash, oidx, &amount))
				tree_manager_set_child_value_i64(input, "amount", amount);
			tree_manager_set_child_value_btcaddr(input, "srcaddr", addr);
		}
	}


	out = PTR_NULL;
	release_zone_ref(&my_list);

	for (oidx = 0, tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); oidx++, tree_manager_get_next_child(&my_list, &out))
	{
		btc_addr_t		out_addr = { 0 };
		struct string	out_path = { 0 }, script = { 0 };
		uint64_t		amount = 0;
		int				ret;

		tree_manager_get_child_value_i64(out, NODE_HASH("value"), &amount);

		if (amount == 0)continue;
		if ((amount & 0xFFFFFFFF00000000)==0xFFFFFFFF00000000)continue;
		if (amount == 0xFFFFFFFFFFFFFFFF)continue;

		tree_manager_get_child_value_istr(out, NODE_HASH("script"), &script, 0);

		ret = get_out_script_address(&script, PTR_NULL, out_addr);
		free_string(&script);

		if (ret)
		{
			make_string(&out_path, "adrs");
			cat_ncstring_p(&out_path, out_addr, 34);

			/*
			if (is_coinbase(&tx))
			create_dir(out_path.str);
			*/
			if ((stat_file(out_path.str) == 0) && (!memcmp_c(addr, out_addr, sizeof(btc_addr_t))))
			{
				
				add_unspent(out_addr, tchash, oidx, amount, src_addr_list, n_in_addr);
			}
			free_string(&out_path);
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
	btc_addr_t			to_addr_list[16];
	char				tx_hash[65];
	mem_zone_ref		txout_list = { PTR_NULL }, txin_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr	out = PTR_NULL, input = PTR_NULL;
	unsigned int		oidx, iidx;
	unsigned int		n_to_addrs, n_in_addr;
	
	

	if (!tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))return 0;
	if (!tree_manager_find_child_node(tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list)){ release_zone_ref(&txin_list); return 0; }
	
	tree_manager_get_child_value_str (tx, NODE_HASH("txid"), tx_hash, 65, 0);

	n_to_addrs = 0;
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


	n_in_addr = 0;

	for (iidx = 0, tree_manager_get_first_child(&txin_list, &my_list, &input); ((input != NULL) && (input->zone != NULL)); tree_manager_get_next_child(&my_list, &input), iidx++)
	{
		char			 ptchash[65];
		hash_t			 prev_hash = { 0 };
		struct string	 script = { PTR_NULL };
		struct string	 out_path = { PTR_NULL };
		uint64_t		 amount;
		unsigned int	 oidx;
		int				 n;

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
		
		
		tree_manager_get_child_value_hash(input, NODE_HASH("txid"), prev_hash);
		tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &oidx);

		if (!memcmp_c(prev_hash, nullh, sizeof(hash_t)))
		{
			btc_addr_t coinbase;
			memset_c(coinbase, '0', sizeof(btc_addr_t));
			tree_manager_set_child_value_btcaddr(input, "srcaddr", coinbase);
			continue;
		}

		bin_2_hex(prev_hash, 32, ptchash);

		/*
		n = 0;
		while (n<32)
		{
			ptchash[n * 2 + 0] = hex_chars[prev_hash[n] >> 4];
			ptchash[n * 2 + 1] = hex_chars[prev_hash[n] & 0x0F];
			n++;
		}
		ptchash[64] = 0;
		*/

		if (get_tx_output_script(prev_hash, oidx, &script, &amount))
		{
			btc_addr_t		 out_addr;
			struct string	 pubk = { PTR_NULL };
			if (get_out_script_address(&script, &pubk, out_addr))
			{
				spend_tx_addr(out_addr, tx_hash, iidx, ptchash, oidx, to_addr_list, n_to_addrs);
			}
			free_string(&pubk);
			free_string(&script);
		}
	}

	for (oidx = 0, tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); oidx++, tree_manager_get_next_child(&my_list, &out))
	{
		btc_addr_t		out_addr = { 0 };
		struct string	out_path = { 0 };
		struct string   script = { PTR_NULL };
		uint64_t		amount = 0;
		int				addret = 0;

		tree_manager_get_child_value_i64(out, NODE_HASH("value"), &amount);

		if (amount == 0)continue;
		if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)continue;
		if (amount == 0xFFFFFFFFFFFFFFFF)continue;
		
		if(tree_manager_get_child_value_istr(out, NODE_HASH("script"), &script, 0))
		{
			struct string pk = { PTR_NULL };
			addret = get_out_script_address(&script, &pk, out_addr);
			free_string(&script);
			free_string(&pk);
		}
		if (!addret)continue;

		make_string		(&out_path, "adrs");
		cat_ncstring_p	(&out_path, out_addr, 34);

		if (stat_file(out_path.str) == 0)
			add_unspent(out_addr, tx_hash, oidx, amount, src_addr_list, n_in_addr);

		free_string(&out_path);
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



OS_API_C_FUNC(int) wallet_list_addrs(mem_zone_ref_ptr account_name, mem_zone_ref_ptr addr_list,unsigned int balance)
{
	struct string username = { PTR_NULL };
	struct string user_key_file = { PTR_NULL };
	struct string user_pw_file = { PTR_NULL };
	struct string   pw = { PTR_NULL };
	size_t		  keys_data_len = 0;
	size_t			of;
	unsigned int  minconf;
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
	
	minconf = 1;
	
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

				if (balance)
				{
					uint64_t conf_amount = 0, unconf_amount = 0;

					get_balance						(keys_ptr->addr, &conf_amount, &unconf_amount, minconf);

					if ((conf_amount > 0) || (unconf_amount > 0))
					{
						tree_manager_set_child_value_i64(&new_addr, "amount", conf_amount);
						tree_manager_set_child_value_i64(&new_addr, "unconf_amount", unconf_amount);
						tree_manager_node_add_child(addr_list, &new_addr);
					}
				}
				else
					tree_manager_node_add_child(addr_list, &new_addr);
			
				release_zone_ref(&new_addr);
			}
			keys_ptr++;
			keys_data_len -= sizeof(struct key_entry);
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

			load_tx_addresses(pubaddr, &tx_list, 0, 1000);
			for (tree_manager_get_first_child(&tx_list, &my_list, &tx); ((tx != NULL) && (tx->zone != NULL)); tree_manager_get_next_child(&my_list, &tx))
			{
				hash_t tx_hash;
				tree_manager_get_node_hash(tx, 0, tx_hash);
				store_tx_wallet(pubaddr, tx_hash);
			}
			release_zone_ref(&tx_list);
		}
	}

	free_string(&adr_path);

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

OS_API_C_FUNC(int) set_anon_pw(const char *pw,unsigned int timeout)
{
	hash_t				h1,pwh;
	struct string		user_pw_file={ 0 };
	unsigned char		*data;
	size_t				pw_len,len;
	int					ret=0;

	wallet_pw_set = 0;

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
		wallet_pw_set = 1;

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

	free_string(&uname);
	free_string(&strKey);
		

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
		free_string(&path);

		clone_string(&path, &adr_path);
		cat_cstring_p(&path, "unspent");
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
		load_tx_addresses(pubaddr, &tx_list,0, 1000);
		for (tree_manager_get_first_child(&tx_list, &txlist, &tx); ((tx != NULL) && (tx->zone != NULL)); tree_manager_get_next_child(&txlist, &tx))
		{
			hash_t						tx_hash;
			tree_manager_get_node_hash(tx, 0, tx_hash);
			store_tx_wallet(pubaddr, tx_hash);
			do_mark_sweep(get_tree_mem_area_id(), 500);
		}
		release_zone_ref(&tx_list);
	}

	free_string(&adr_path);

	rm_addr_scan(pubaddr);

	return 1;
}


OS_API_C_FUNC(int) encode_DER_sig(const struct string *sig, struct string *sigseq, unsigned int rev, unsigned char hash_type)
{
	unsigned char S[32], R[32];
	size_t		  slen, rlen, siglen;
	unsigned int	n;

	if (sig->len < 64)
		return 0;

	rlen = 32;
	slen = 32;

	if (rev)
	{
		for (n = 0; n < 32; n++)
			R[31 - n] = sig->str[n];

		for (n = 0; n < 32; n++)
			S[31 - n] = sig->str[n + 32];
	}
	else
	{
		memcpy_c(R, sig->str, 32);
		memcpy_c(S, sig->str + 32, 32);
	}

	while (R[0] == 0)
	{
		memcpy_c(R, R + 1, --rlen);
	}

	while (S[0] == 0)
	{
		memcpy_c(S, S + 1, --slen);
	}

	siglen = slen + rlen + 4;

	sigseq->len = siglen + 2+1;
	sigseq->size = sigseq->len + 1;
	sigseq->str = malloc_c(sigseq->size);

	sigseq->str[0] = 0x30;
	sigseq->str[1] = siglen;
	sigseq->str[2] = 0x02;
	sigseq->str[3] = rlen;

	memcpy_c(&sigseq->str[4], R, rlen);

	sigseq->str[4 + rlen] = 0x02;
	sigseq->str[4 + rlen + 1] = slen;

	memcpy_c(&sigseq->str[4 + rlen + 2], S, slen);

	sigseq->str[sigseq->len-1] = hash_type;

	return 1;


}