//copyright antoine bentue-ferrer 2016
#include <base/std_def.h>
#include <base/std_mem.h>
#include <base/mem_base.h>
#include <base/std_str.h>

#include <sha256.h>
#include <md5.h>
#include <crypto.h>
#include <strs.h>
#include <tree.h>
#include <fsio.h>

#include "../block_adx/block_api.h"

hash_t				nullhash = { 0xCD };
hash_t				Difflimit = { 0xCD };
unsigned int		Di = PTR_INVALID, last_diff = PTR_INVALID;
static int64_t		TargetSpacing = 0xABCDABCD;
static int64_t		nTargetTimespan = 0xABCDABCD;
static int64_t		nStakeReward = 0xABCDABCD;
static unsigned int	minStakeDepth = 10;
static unsigned int	log_level = 1;


#define ONE_COIN		100000000ULL
#define ONE_CENT		1000000ULL


OS_API_C_FUNC(int) init_pos(mem_zone_ref_ptr stake_conf)
{
	mem_zone_ref log = { PTR_NULL };
	char diff[16];

	memset_c(nullhash, 0, 32);

	if(!tree_manager_get_child_value_i64(stake_conf, NODE_HASH("targetspacing"), &TargetSpacing))
		TargetSpacing= 64;

	if (!tree_manager_get_child_value_i64(stake_conf, NODE_HASH("targettimespan"), &nTargetTimespan))
		nTargetTimespan = 16 * 60;  // 16 mins

	if (!tree_manager_get_child_value_i64(stake_conf, NODE_HASH("reward"), &nStakeReward))
		nStakeReward = 150 * ONE_CENT;  //

	if (!tree_manager_get_child_value_i32(stake_conf, NODE_HASH("limit"), &Di))
		Di = 0x1B00FFFF;

	if (!tree_manager_get_child_value_i32(stake_conf, NODE_HASH("minStakeDepth"), &minStakeDepth))
		minStakeDepth = 10;

	

	SetCompact(Di, Difflimit);

	last_diff = Di;

	uitoa_s(last_diff, diff, 16, 16);

	
	tree_manager_create_node("log", NODE_LOG_PARAMS, &log);
	tree_manager_set_child_value_i32 (&log, "target", TargetSpacing);
	tree_manager_set_child_value_i32 (&log, "timespan", nTargetTimespan);
	tree_manager_set_child_value_i32 (&log, "reward", nStakeReward);
	tree_manager_set_child_value_i32(&log, "last_diff", last_diff);
	tree_manager_set_child_value_str (&log, "last_diffs", diff);
	tree_manager_set_child_value_hash(&log, "limit", Difflimit);
	log_message("stake_pos3->init_pos target : %target% secs, time span %timespan% , reward %reward% , limit %limit%, last diff %last_diff% '%last_diffs%'\n", &log);
	release_zone_ref(&log);

	
	return 1;
}
OS_API_C_FUNC(int) get_stake_reward(uint64_t height,uint64_t *reward)
{
	*reward = nStakeReward;
	return 1;
}


OS_API_C_FUNC(int) stake_get_reward(mem_zone_ref_ptr nHeight, mem_zone_ref_ptr nReward)
{
	unsigned int height;
	tree_mamanger_get_node_dword	(nHeight, 0, &height);
	tree_manager_write_node_qword	(nReward, 0, nStakeReward);
	return 1;
}

OS_API_C_FUNC(int) get_min_stake_depth(unsigned int *depth)
{
	*depth = minStakeDepth;
	return 1;
}

OS_API_C_FUNC(int) get_target_spacing(unsigned int *target)
{
	*target = TargetSpacing;
	return 1;
}
OS_API_C_FUNC(int) generated_stake_modifier(const char *blk_hash,uint64_t *height, hash_t StakeMod)
{
	struct string		blk_path = { PTR_NULL };
	unsigned char		*data;
	size_t			len;
	int			ret = 0;

	make_string	(&blk_path, "blks");
	cat_ncstring_p	(&blk_path, blk_hash+0,2);
	cat_ncstring_p	(&blk_path, blk_hash+2, 2);
	cat_cstring_p	(&blk_path, blk_hash);
	cat_cstring	(&blk_path, "_blk");

	if(get_file_len(blk_path.str,238,&data,&len)>0)
	{
	  if(len>=88)
		*height = *((uint64_t *)(data+80));
	  if(len>=206)
	  {
		if(*((unsigned char *)(data+205))==32)
		{
			memcpy_c(StakeMod,data+206,32);
			ret		=1;
		}
	}
	free_c(data);
	}
	else
	  ret = -1;

	free_string(&blk_path);
	return ret;
}

// Get the last stake modifier and its generation time from a given block
OS_API_C_FUNC(int) find_last_stake_modifier(char * chash, hash_t nStakeModifier)
{
	uint64_t	height=0;

	while (generated_stake_modifier(chash,&height, nStakeModifier)==0)
	{
		hash_t		prevHash;
		
		if(height==1)return 0;
		if (!get_hash_idx("block_indexes", height-1, prevHash))return 0;

		bin_2_hex(prevHash, 32, chash);

		/*
		int n;
		n = 0;
		while (n<32)
		{
			chash[n * 2 + 0] = hex_chars[prevHash[n] >> 4];
			chash[n * 2 + 1] = hex_chars[prevHash[n] & 0x0F];
			n++;
		}
		chash[64] = 0;
		*/
                
	}
	return 1;
}
// Get the last stake modifier and its generation time from a given block
OS_API_C_FUNC(int) get_last_stake_modifier(mem_zone_ref_ptr pindex, hash_t nStakeModifier, unsigned int *nModifierTime)
{
	char			chash[65];
	int				ret=0;

    if (pindex==PTR_NULL)return 0;
    if (pindex->zone==PTR_NULL)return 0;
    if (!tree_manager_get_child_value_str(pindex, NODE_HASH("blkHash"), chash, 65, 16))return 0;

     ret=find_last_stake_modifier(chash, nStakeModifier);
    return ret;
}
/*
OS_API_C_FUNC(int) is_pos_block(const char *blk_hash)
{
	struct string file_path = { 0 };
	int stat, ret;

	make_string(&file_path, "blks");
	cat_ncstring_p(&file_path, blk_hash + 0, 2);
	cat_ncstring_p(&file_path, blk_hash + 2, 2);
	cat_cstring_p(&file_path, blk_hash);
	cat_cstring_p(&file_path, "pos");
	stat = stat_file(file_path.str);
	free_string(&file_path);
	ret = (stat == 0) ? 1 : 0;
	return ret;
}
*/

/* Get the last pos block and its generation time from a given block */
int get_last_pos_block(mem_zone_ref_ptr pindex, unsigned int *block_time)
{
	char			chash[65];
	int				ret = 0;
	tree_manager_get_child_value_str(pindex, NODE_HASH("blkHash"), chash, 65, 16);
	/*while (!is_pos_block(chash))*/
	
	while(!tree_manager_find_child_node(pindex,NODE_HASH("blk pos"),0xFFFFFFFF,PTR_NULL))
	{
		tree_manager_get_child_value_str(pindex, NODE_HASH("prev"), chash, 65, 16);
		if (!load_blk_hdr(pindex, chash))
			return 0;
	}
	/*if (is_pos_block(chash))*/

	if(tree_manager_find_child_node(pindex,NODE_HASH("blk pos"),0xFFFFFFFF,PTR_NULL))
	{
		tree_manager_get_child_value_i32(pindex, NODE_HASH("time"), block_time);
		return 1;
	}
	return 0;
}



OS_API_C_FUNC(int) load_last_pos_blk(mem_zone_ref_ptr header)
{
	unsigned char   *data;
	size_t			len;
	struct string	path = { PTR_NULL };
	int				ret = 0;

	make_string(&path, "node");
	cat_cstring_p(&path, "last_pos");
	if (get_file(path.str, &data, &len) > 0)
	{
		if (len >= sizeof(hash_t))
		{
			char chash[65];

			/*
			int	 n;
			n = 0;
			while (n<32)
			{
				chash[n * 2 + 0] = hex_chars[data[n] >> 4];
				chash[n * 2 + 1] = hex_chars[data[n] & 0x0F];
				n++;
			}
			chash[64] = 0;
			*/

			bin_2_hex(data, 32, chash);
			ret = load_blk_hdr(header, chash);

			/*
			if (ret)
			{
				uint64_t height;
				get_blk_height					(chash, &height);
				tree_manager_get_child_value_i64(header,NODE_HASH("height"),&height);
				tree_manager_set_child_value_i64(header, "height", height);
			}
			*/
		}
		free_c(data);
	}
	free_string(&path);

	

	return (ret>0);
}



OS_API_C_FUNC(int) store_tx_staking(mem_zone_ref_ptr tx, hash_t tx_hash, btc_addr_t stake_addr, uint64_t	stake_in)
{
	mem_zone_ref	 txout_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr out = PTR_NULL;
	uint64_t		 stake_out = 0, staked;
	struct string	 stake_path = { 0 };


	make_string(&stake_path, "adrs");
	cat_ncstring_p(&stake_path, stake_addr, 34);
	if (stat_file(stake_path.str) != 0)
	{
		free_string(&stake_path);
		return 0;
	}

	if (tree_manager_find_child_node(tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))
	{
		for (tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); tree_manager_get_next_child(&my_list, &out))
		{
			uint64_t amount = 0;
			if (!tree_manager_get_child_value_i64(out, NODE_HASH("value"), &amount))continue;
			stake_out += amount;
		}
		release_zone_ref(&txout_list);
	}
	staked = stake_out - stake_in;


	cat_cstring_p(&stake_path, "stakes");
	append_file	(stake_path.str, &staked, sizeof(uint64_t));
	append_file	(stake_path.str, tx_hash, sizeof(hash_t));
	free_string(&stake_path);
	return 1;
}
OS_API_C_FUNC(int) store_blk_tx_staking(mem_zone_ref_ptr tx_list)
{
	if (tx_list != PTR_NULL)
	{
		mem_zone_ref tx = { PTR_NULL };

		if (tree_manager_get_child_at(tx_list, 1, &tx))
		{
			if (is_vout_null(&tx, 0))
			{
				mem_zone_ref vin = { PTR_NULL };
				btc_addr_t	stake_addr;
				uint64_t	stake_in;
				if (get_tx_input(&tx, 0, &vin))
				{
					if (tree_manager_get_child_value_btcaddr(&vin, NODE_HASH("srcaddr"), stake_addr))
					{
						hash_t			tx_hash;

						tree_manager_get_child_value_i64(&vin, NODE_HASH("amount"), &stake_in);
						tree_manager_get_child_value_hash(&tx, NODE_HASH("txid"), tx_hash);

						store_tx_staking(&tx, tx_hash, stake_addr, stake_in);
					}
					release_zone_ref(&vin);
				}
			}
			release_zone_ref(&tx);
		}
	}

	return 1;
}

OS_API_C_FUNC(int) store_blk_staking(mem_zone_ref_ptr header)
{
	unsigned char buffer[128];
	char blk_hash[65];
	struct string blk_path = { 0 }, file_path = { 0 };
	int  ret;

	if (!tree_manager_get_child_value_str(header, NODE_HASH("blkHash"), blk_hash, 65, 16))return 0;

	make_string(&blk_path, "blks");
	cat_ncstring_p(&blk_path, blk_hash + 0, 2);
	cat_ncstring_p(&blk_path, blk_hash + 2, 2);
	cat_cstring_p(&blk_path, blk_hash);
	cat_cstring(&blk_path, "_blk");
	

	if(tree_manager_get_child_value_hash(header, NODE_HASH("blk pos")	, &buffer[1]))
	{
		buffer[0]	=	3;
		if(tree_manager_get_child_value_hash(header, NODE_HASH("StakeMod2"), &buffer[34]))
		{
			buffer[33]	=	32;
			ret=truncate_file		(blk_path.str,172,buffer,66);
		}
		else
		{
			buffer[33]	=	0;
                        memset_c(&buffer[34],0,32);
			ret=truncate_file		(blk_path.str,172,buffer,66);
		}
	}
	else
	{
		if(tree_manager_get_child_value_hash(header, NODE_HASH("StakeMod2"), &buffer[1]))
		{
			buffer[0]	=	32;
			ret=truncate_file		(blk_path.str,205,buffer,33);
		}
		else
		{
			buffer[0]	=	0;
			memset_c			(&buffer[1],0,32);
			ret=truncate_file		(blk_path.str,205,buffer,33);
		}
	}
	/*
	stat = stat_file(blk_path.str);
	if (stat != 0)
	{
		free_string(&blk_path);
		return 0;
	}
	ret = 1;
	if (tree_manager_get_child_value_hash(header, NODE_HASH("StakeMod2"), StakeMod))
	{
		clone_string(&file_path, &blk_path);
		cat_cstring_p(&file_path, "stakemodifier2");
		ret = put_file(file_path.str, StakeMod, sizeof(hash_t));
		free_string(&file_path);
	}
	if (tree_manager_get_child_value_hash(header, NODE_HASH("blk pos"), hashPos))
	{
		clone_string(&file_path, &blk_path);
		cat_cstring_p(&file_path, "pos");
		ret = put_file(file_path.str, hashPos, sizeof(hash_t));
		free_string(&file_path);
	}
	*/
	free_string(&blk_path);
	return ret;
}

/*
OS_API_C_FUNC(int) get_pos_block(const char *blk_hash, hash_t pos)
{
	unsigned char   *data;
	size_t			len;
	struct string file_path = { 0 };
	int ret = 0;

	make_string(&file_path, "blks");
	cat_ncstring_p(&file_path, blk_hash + 0, 2);
	cat_ncstring_p(&file_path, blk_hash + 2, 2);
	cat_cstring_p(&file_path, blk_hash);
	cat_cstring_p(&file_path, "pos");
	if (get_file(file_path.str, &data, &len) > 0)
	{
		if (len >= sizeof(hash_t))
		{
			memcpy_c(pos, data, sizeof(hash_t));
			ret = 1;
		}
		free_c(data);
	}
	free_string(&file_path);
	return ret;
}
*/

OS_API_C_FUNC(int) get_blk_staking_infos(mem_zone_ref_ptr blk, const char *blk_hash, mem_zone_ref_ptr infos)
{
	hash_t			stakemod, rdiff, diff,proof;
	mem_zone_ref	vout = { PTR_NULL };
	unsigned int	staketime, nBits;
	unsigned char	 n;
	uint64_t		weight;
	

	tree_manager_get_child_value_i32(blk, NODE_HASH("bits"), &nBits);

	tree_manager_set_child_value_i64(infos, "reward", nStakeReward);

	if (get_last_stake_modifier(blk, stakemod, &staketime))
		tree_manager_set_child_value_hash(infos, "stakemodifier2", stakemod);
	else
		tree_manager_set_child_value_hash(infos, "stakemodifier2", nullhash);

	if (load_blk_tx_input(blk_hash, 66, 0, &vout))
	{
		tree_manager_get_child_value_i64(&vout, NODE_HASH("value"), &weight);
		tree_manager_set_child_value_i64(infos, "stake weight", weight);
		release_zone_ref(&vout);
		mul_compact(nBits, weight, rdiff);
	}
	else
	{
		SetCompact(nBits, diff);
		n = 32;
		while (n--)
		{
			rdiff[n] = diff[31 - n];
		}

	}

	tree_manager_set_child_value_hash(infos, "hbits", rdiff);

	if(tree_manager_get_child_value_hash(blk,NODE_HASH("blk pos"),proof))
		tree_manager_set_child_value_hash(infos, "proofhash", proof);
	

	/*

	struct string blk_path = { 0 }, file_path = { 0 };
	make_string(&blk_path, "blks");
	cat_ncstring_p(&blk_path, blk_hash + 0, 2);
	cat_ncstring_p(&blk_path, blk_hash + 2, 2);
	cat_cstring_p(&blk_path, blk_hash);
	clone_string(&file_path, &blk_path);
	cat_cstring_p(&file_path, "pos");
	if (get_file(file_path.str, &data, &len) > 0)
	{
		if (len >= sizeof(hash_t))
			tree_manager_set_child_value_hash(infos, "proofhash", data);

		free_c(data);
	}
	free_string(&file_path);
	free_string(&blk_path);
	*/



	return 1;

}
OS_API_C_FUNC(int) get_tx_pos_hash_data(mem_zone_ref_ptr hdr,const hash_t txHash, unsigned int OutIdx, struct string *hash_data,uint64_t *amount,hash_t out_diff)
{
	unsigned char	buffer[128];
	hash_t			StakeModifier;
	size_t			sZ;
	unsigned int    StakeModifierTime;
	uint64_t		height, block_time, weight;
	unsigned int    tx_time,tt;
	
	hash_t			blkh;
	mem_zone_ref	mtx = { PTR_NULL };
	
	load_tx_output_amount(txHash, OutIdx, &weight);
	if (last_diff == 0xFFFFFFFF)
	{
		unsigned int					nBits;
		tree_manager_get_child_value_i32(hdr, NODE_HASH("bits"), &nBits);
		mul_compact(nBits, weight, out_diff);
	}
	else
		mul_compact(last_diff, weight, out_diff);

	load_tx(&mtx, blkh, txHash);
	tree_manager_get_child_value_i32(&mtx, NODE_HASH("time"), &tt);
	release_zone_ref(&mtx);

	if (!get_tx_blk_height	(txHash, &height, &block_time, &tx_time))
		return 0;



	if (!get_last_stake_modifier(hdr, StakeModifier, &StakeModifierTime))
		return 0;

	*amount = weight;
	memcpy_c(buffer															, StakeModifier	, sizeof(hash_t));
	memcpy_c(buffer + sizeof(hash_t)										, &tx_time		, sizeof(unsigned int));
	memcpy_c(buffer + sizeof(hash_t) + sizeof(unsigned int)					, txHash		, sizeof(hash_t));
	memcpy_c(buffer + sizeof(hash_t) + sizeof(hash_t) + sizeof(unsigned int), &OutIdx		, sizeof(unsigned int));

	sZ				= (sizeof(hash_t) + sizeof(hash_t) + sizeof(unsigned int) + sizeof(unsigned int));	
	hash_data->len  = sZ * 2;
	hash_data->size = hash_data->len + 1;
	hash_data->str	= malloc_c(hash_data->size);

	while (sZ--)
	{
		hash_data->str[sZ * 2 + 0] = hex_chars[buffer[sZ] >> 4];
		hash_data->str[sZ * 2 + 1] = hex_chars[buffer[sZ] & 0x0F];
	}
	hash_data->str[hash_data->len] = 0;



	return 1;
}

OS_API_C_FUNC(int) compute_tx_pos(mem_zone_ref_ptr tx, hash_t StakeModifier, unsigned int txTime, hash_t pos_hash, uint64_t *weight)
{
	
	hash_t					tmp;
	mbedtls_sha256_context	ctx;
	hash_t					prevOutHash;
	unsigned int			prevOutIdx;
	mem_zone_ref	vin = { PTR_NULL }, prev_tx = { PTR_NULL };
	unsigned int txPrevTime;
	
	if (!load_tx_input(tx, 0, &vin, &prev_tx))return 0;

	
	tree_manager_get_child_value_hash	(&vin, NODE_HASH("txid"), prevOutHash);
	tree_manager_get_child_value_i32	(&vin, NODE_HASH("idx"), &prevOutIdx);
	release_zone_ref					(&vin);
	
	tree_manager_get_child_value_i32	(&prev_tx, NODE_HASH("time"), &txPrevTime);
	get_tx_output_amount				(&prev_tx, prevOutIdx, weight);
	release_zone_ref					(&prev_tx);
	

	mbedtls_sha256_init(&ctx);
	mbedtls_sha256_starts(&ctx, 0);
	mbedtls_sha256_update(&ctx, StakeModifier, sizeof(hash_t));
	mbedtls_sha256_update(&ctx, (unsigned char *)&txPrevTime, sizeof(unsigned int));

	mbedtls_sha256_update(&ctx, prevOutHash, sizeof(hash_t));
	mbedtls_sha256_update(&ctx, (unsigned char *)&prevOutIdx, sizeof(unsigned int));
	mbedtls_sha256_update(&ctx, (unsigned char *)&txTime, sizeof(unsigned int));
	mbedtls_sha256_finish(&ctx, tmp);
	mbedtls_sha256_free(&ctx);
	mbedtls_sha256(tmp, 32, pos_hash, 0);
	return 1;
}


int compute_next_stake_modifier(mem_zone_ref_ptr blk,hash_t nStakeModifier, hash_t Kernel)
{
	hash_t					nStakeModifierNew;
	hash_t					tmp;
	mbedtls_sha256_context	ctx;
	mem_zone_ref			pindex = { PTR_NULL };
	mem_zone_ref			log = { PTR_NULL };
	
	/*
	tree_manager_create_node			("log", NODE_LOG_PARAMS, &log);
	tree_manager_set_child_value_hash	(&log, "StakeMod"	, nStakeModifier);
	tree_manager_set_child_value_hash	(&log, "kernel"		, Kernel);
	log_message							("stake_pos3: ComputeNextStakeModifier:\n\tprev modifier:\n\t%StakeMod%\n\tkernel:\n\t%kernel%\n", &log);
	release_zone_ref					(&log);
	*/

	mbedtls_sha256_init		(&ctx);
	mbedtls_sha256_starts	(&ctx, 0);
	mbedtls_sha256_update	(&ctx, Kernel, sizeof(hash_t));
	mbedtls_sha256_update	(&ctx, nStakeModifier, sizeof(hash_t));
	mbedtls_sha256_finish	(&ctx, tmp);
	mbedtls_sha256_free		(&ctx);
	mbedtls_sha256			(tmp, 32, nStakeModifierNew, 0);

	tree_manager_set_child_value_hash(blk, "StakeMod2", nStakeModifierNew);

	return 1;
}
#define tree_zone 2

OS_API_C_FUNC(unsigned int) get_current_pos_difficulty()
{
	return last_diff;
}

OS_API_C_FUNC(int) find_last_pos_block(mem_zone_ref_ptr pindex)
{
	char			chash[65];
	int				ret = 0;
	tree_manager_get_child_value_str(pindex, NODE_HASH("blkHash"), chash, 65, 16);
/*	while (!is_pos_block(chash))*/

	while(!tree_manager_find_child_node(pindex,NODE_HASH("blk pos"),0xFFFFFFFF,PTR_NULL))
	{
		tree_manager_get_child_value_str(pindex, NODE_HASH("prev"), chash, 65, 16);
		if (!load_blk_hdr(pindex, chash))
			return 0;
	}
	return 1;
}


OS_API_C_FUNC(int) compute_last_pos_diff(mem_zone_ref_ptr lastPOS, mem_zone_ref_ptr nBits)
{
	hash_t			od1;
	unsigned int	prevTime, pprevTime;
	unsigned int	Bits,pBits;
	int64_t			nActualSpacing;
	char			cpphash[65];
	mem_zone_ref	pprev = { PTR_NULL };

	if (!tree_manager_get_child_value_i32(lastPOS, NODE_HASH("bits"), &pBits))
		pBits = Di;

	tree_manager_get_child_value_i32	(lastPOS, NODE_HASH("time"), &prevTime);
	tree_manager_get_child_value_str	(lastPOS, NODE_HASH("prev"), cpphash, 65, 16);

	if (!load_blk_hdr(&pprev, cpphash))
	{
		last_diff	= pBits;
		tree_manager_write_node_dword(nBits, 0,pBits);
		return 1;
	}
	
	if (get_last_pos_block(&pprev, &pprevTime))
	{
		
		nActualSpacing = prevTime - pprevTime;

		if (nActualSpacing > TargetSpacing * 10)
			nActualSpacing = TargetSpacing * 10;

		Bits = calc_new_target(nActualSpacing, TargetSpacing, nTargetTimespan, pBits);

		SetCompact(Bits, od1);
		if (memcmp_c(od1, Difflimit, sizeof(hash_t)) > 0)
			Bits = Di;
	}
	else
		Bits = Di;

	last_diff = Bits;

	tree_manager_write_node_dword(nBits, 0, Bits);
	release_zone_ref(&pprev);
	return 1;
}

OS_API_C_FUNC(int) check_blk_sig(mem_zone_ref_ptr hdr, struct string *vpubK)
{
	struct string blksign = { PTR_NULL };
	struct string bsign = { PTR_NULL };
	unsigned char blk_hash_type;
	int ret=0;

	if (!tree_manager_get_child_value_istr(hdr, NODE_HASH("signature"), &blksign, 0))return 0;
	ret = parse_sig_seq(&blksign, &bsign, &blk_hash_type, 1);
	if (ret)
	{
		hash_t blk_hash = { 0 };
		tree_manager_get_child_value_hash(hdr, NODE_HASH("blkHash"), blk_hash);
		ret = blk_check_sign(&bsign, vpubK, blk_hash);
		free_string(&bsign);
	}
	free_string(&blksign);
	return ret;
}


OS_API_C_FUNC(int) find_blk_staking_tx(mem_zone_ref_ptr tx_list, mem_zone_ref_ptr tx)
{
	int ret;

	if (!tree_manager_get_child_at(tx_list, 0, tx))return 0;
	ret = is_tx_null(tx);
	release_zone_ref(tx);
	if(!ret)return 0;

	if (!tree_manager_get_child_at(tx_list, 1, tx))return 0;
	if (is_vout_null(tx, 0))return 1;
	release_zone_ref(tx);
	return 0;
	
}



OS_API_C_FUNC(int) check_tx_pos(mem_zone_ref_ptr blk,mem_zone_ref_ptr tx)
{
	char				prevHash[65];
	hash_t				pHash;
	hash_t				lastStakeModifier, StakeModKernel;
	hash_t				rpos, rdiff;
	hash_t				pos_hash, out_diff,blk_hash;
	mem_zone_ref		log = { PTR_NULL };
	mem_zone_ref		my_list = { PTR_NULL }, vpubK = { PTR_NULL };
	uint64_t			weight;
	unsigned int		txTime;
	int					ret=1;
	int					n;

	tree_manager_get_child_value_hash	(blk, NODE_HASH("prev"), pHash);

	bin_2_hex(pHash, 32, prevHash);
	/*
	n = 0;
	while (n<32)
	{
		prevHash[n * 2 + 0] = hex_chars[pHash[n] >> 4];
		prevHash[n * 2 + 1] = hex_chars[pHash[n] & 0x0F];
		n++;
	}
	prevHash[64] = 0;
	*/

	if (!find_last_stake_modifier(prevHash, lastStakeModifier))
	{
	    memset_c(lastStakeModifier, 0, sizeof(hash_t));
	}

	if (!tree_manager_get_child_value_hash(blk, NODE_HASH("blkHash"), blk_hash))
	{
		compute_block_hash(blk, blk_hash);
		tree_manager_set_child_value_hash(blk, "blkHash", blk_hash);
	}

	if (tx == PTR_NULL)
		return compute_next_stake_modifier(blk, lastStakeModifier, blk_hash);

	tree_manager_get_child_value_i32	(tx, NODE_HASH("time"), &txTime);
	compute_tx_pos						(tx, lastStakeModifier, txTime, pos_hash, &weight);

	if (last_diff == 0xFFFFFFFF)
		last_diff = Di;

	memset_c							(out_diff, 0, sizeof(hash_t));
	mul_compact							(last_diff, weight, out_diff);

	ret = (cmp_hashle(pos_hash, out_diff) >= 0) ? 1 : 0;

	n = 32;
	while (n--)
	{
		rpos[n] = pos_hash[31 - n];
		rdiff[n] = out_diff[31 - n];
	}

	if (ret)
	{
		if (log_level > 1)
		{
			tree_manager_create_node("log", NODE_LOG_PARAMS, &log);
			tree_manager_set_child_value_hash(&log, "diff", rdiff);
			tree_manager_set_child_value_hash(&log, "pos", rpos);
			tree_manager_set_child_value_hash(&log, "hash", blk_hash);
			log_message("----------------\nNEW POS BLOCK\n%diff%\n%pos%\n%hash%\n", &log);
			release_zone_ref(&log);
		}

		get_tx_input_hash				 (tx, 0, StakeModKernel);
		ret = compute_next_stake_modifier(blk, lastStakeModifier, StakeModKernel);

		tree_manager_set_child_value_hash(blk, "blk pos", pos_hash);
		tree_manager_set_child_value_i64 (blk, "weight", weight);
		
		if (log_level > 1)
		{
			log_message("new modifier=%StakeMod2% time=%time% hash=%blkHash%", blk);
		}

	}
	else
	{
		tree_manager_create_node("log", NODE_LOG_PARAMS, &log);
		tree_manager_set_child_value_hash(&log, "diff", rdiff);
		tree_manager_set_child_value_hash(&log, "pos", rpos);
		tree_manager_set_child_value_hash(&log, "hash", blk_hash);
		log_message("----------------\nBAD POS BLOCK\n%diff%\n%pos%\n%hash%\n", &log);
		release_zone_ref(&log);
	}

	
		

	return ret;
}



OS_API_C_FUNC(int) create_pos_block(hash_t pHash, mem_zone_ref_ptr tx, mem_zone_ref_ptr newBlock)
{
	char			chash[65];
	hash_t			block_hash, merkle;
	unsigned int	version, time,n;
	uint64_t		height;

	get_block_version				(&version);
	tree_manager_get_child_value_i32(tx, NODE_HASH("time"), &time);

	bin_2_hex(pHash, 32, chash);

	/*
	n = 0;
	while (n<32)
	{
		chash[n * 2 + 0] = hex_chars[pHash[n] >> 4];
		chash[n * 2 + 1] = hex_chars[pHash[n] & 0x0F];
		n++;
	}
	chash[64] = 0;
	*/

	get_blk_height	(chash, &height);

	if (tree_manager_create_node("block", NODE_BITCORE_BLK_HDR, newBlock))
	{
		char psig[80];
		struct string sigstr = { 0 };
		mem_zone_ref txs = { PTR_NULL };
		unsigned char vntx[2] = { 0 };

		


		tree_manager_set_child_value_hash	(newBlock, "prev", pHash);
		tree_manager_set_child_value_i32	(newBlock, "version", version);
		tree_manager_set_child_value_i32	(newBlock, "time", time);
		tree_manager_set_child_value_i32	(newBlock, "bits", last_diff);
		tree_manager_set_child_value_i32	(newBlock, "nonce", 0);
		tree_manager_set_child_value_vint	(newBlock, "ntx", vntx);
		tree_manager_add_child_node			(newBlock, "signature", NODE_BITCORE_ECDSA_SIG, PTR_NULL);

		tree_manager_allocate_child_data	(newBlock, "signature",128);
		

		if (tree_manager_add_child_node(newBlock, "txs", NODE_BITCORE_TX_LIST, &txs))
		{
			mem_zone_ref mtx = { PTR_NULL };
			if (tree_manager_add_child_node(&txs, "tx", NODE_BITCORE_TX, &mtx))
			{
				create_null_tx	(&mtx, time,height);
				release_zone_ref(&mtx);
			}

			tree_manager_node_dup	(&txs, tx, &mtx,0xFFFFFFFF);
			release_zone_ref		(&mtx);
			
			build_merkel_tree		(&txs, merkle);
			release_zone_ref		(&txs);
		}
		else
			memset_c(merkle, 0, sizeof(hash_t));

		tree_manager_set_child_value_hash	(newBlock, "merkle_root", merkle);
		compute_block_hash					(newBlock, block_hash);
		tree_manager_set_child_value_bhash	(newBlock, "blkHash", block_hash);

	}
	return 1;

}
