//copyright antoine bentue-ferrer 2016
#include <base/std_def.h>
#include <base/std_mem.h>
#include <base/mem_base.h>
#include <base/std_str.h>

#include <sha256.h>
#define FORWARD_CRYPTO
#include <crypto.h>
#include <strs.h>
#include <tree.h>
#include <fsio.h>
#include <mem_stream.h>
#include <tpo_mod.h>


#define BLOCK_API C_EXPORT
#include "block_api.h"
#include <bin_tree.h>

C_IMPORT size_t			C_API_FUNC	compute_payload_size(mem_zone_ref_ptr payload_node);
C_IMPORT char*			C_API_FUNC	write_node			(mem_zone_ref_const_ptr key, unsigned char *payload);
C_IMPORT size_t			C_API_FUNC	get_node_size		(mem_zone_ref_ptr key);
C_IMPORT void			C_API_FUNC	serialize_children	(mem_zone_ref_ptr node, unsigned char *payload);
C_IMPORT size_t			C_API_FUNC	read_node			(mem_zone_ref_ptr key, const char *payload, size_t len);

C_IMPORT void			C_API_FUNC	unserialize_children(mem_zone_ref_ptr obj, const_mem_ptr payload,size_t len);


/* store txout */
extern int				store_tx_vout		(const char *txh,mem_zone_ref_ptr txout_list,unsigned int oidx, btc_addr_t out_addr);
/* check signature */
extern int				check_sign			(const struct string *sign, const struct string *pubK, const hash_t txh);
/* check public key from tx output */
extern int				check_txout_key		(mem_zone_ref_ptr output, unsigned char *pkey,btc_addr_t addr);
/* compute scrypt block hash */
extern int				scrypt_blockhash	(const void* input, hash_t hash);

extern	int				add_script_var		(mem_zone_ref_ptr script_node, const struct string *val);

extern  int				add_script_uivar	(mem_zone_ref_ptr script_node,uint64_t val);

extern  int				get_script_file		(struct string *script, mem_zone_ref_ptr file);

extern void				keyrh_to_addr		(unsigned char *pkeyh, btc_addr_t addr);

extern struct string	get_next_script_var	(const struct string *script,size_t *offset);

extern int				add_script_opcode	(mem_zone_ref_ptr script_node, unsigned char opcode);

extern int				add_script_push_data(mem_zone_ref_ptr script_node, const_mem_ptr data, size_t size);

extern int				get_script_data		(const struct string *script, size_t *offset, struct string *data);

extern int				get_script_layout	(struct string *script, mem_zone_ref_ptr file);
extern int				get_script_module	(struct string *script, mem_zone_ref_ptr file);
extern int				find_index_str		(const char *app_name, const char *typeStr, const char *keyname, const struct string *val, hash_t hash);


hash_t					null_hash			= { 0xCD };
hash_t					app_root_hash		= { 0xCD };
btc_addr_t				root_app_addr		= { 0xCD };
uint64_t				app_fee				=  0xFFFFFFFF;

mem_zone_ref			apps				= {PTR_INVALID},trusted_apps={PTR_INVALID};
mem_zone_ref			blkobjs				= { PTR_INVALID };

const char				*null_hash_str		= "0000000000000000000000000000000000000000000000000000000000000000";
unsigned char			pubKeyPrefix		= 0xFF;
static const uint64_t	one_coin			= ONE_COIN;
tpo_mod_file			sign_tpo_mod		= { 0xCD };

unsigned int			has_root_app		= 0xFFFFFFFF;
unsigned int			block_version		= 7;
unsigned int			diff_limit			= 0x1E0FFFFF;
unsigned int			TargetTimespan		= 960;
unsigned int			TargetSpacing		= 64;
unsigned int			MaxTargetSpacing	= 640;
unsigned int			reward_halving		= 0xFFFFFFFF;

uint64_t				last_pow_block		= 0xFFFFFFFF;
uint64_t				pow_reward			= 100000*ONE_COIN;
node					*blk_root			= PTR_INVALID;

//#undef _DEBUG
#ifdef _DEBUG
LIBEC_API int			C_API_FUNC crypto_extract_key	(dh_key_t pk, const dh_key_t sk);
LIBEC_API int			C_API_FUNC crypto_sign_open		(const struct string *sign, const struct string *msgh, const struct string *pk);
LIBEC_API struct string	C_API_FUNC crypto_sign			(struct string *msg, const dh_key_t sk);
LIBEC_API int			C_API_FUNC compress_pub			(dh_key_t pk, dh_key_t cpk);
LIBEC_API int			C_API_FUNC derive_key			(dh_key_t public_key, dh_key_t private_key, hash_t secret);
LIBEC_API int			C_API_FUNC crypto_get_pub		(const dh_key_t sk, dh_key_t pk);
#else

crypto_extract_key_func_ptr crypto_extract_key = PTR_INVALID;
crypto_sign_open_func_ptr	crypto_sign_open   = PTR_INVALID;
compress_pubkey_func_ptr	compress_pub	   = PTR_INVALID;
derive_key_func_ptr			derive_key		   = PTR_INVALID;
crypto_get_pub_func_ptr		crypto_get_pub	   = PTR_INVALID;

#ifdef FORWARD_CRYPTO
crypto_sign_func_ptr		crypto_sign		   = PTR_INVALID;
#endif

#endif



int load_sign_module(mem_zone_ref_ptr mod_def, tpo_mod_file *tpo_mod)
{
	char			file[256];
	char			name[64];
	int				ret=1;

	strcpy_cs							(name, 64, tree_mamanger_get_node_name(mod_def));
	tree_manager_get_child_value_str	(mod_def, NODE_HASH("file"), file, 256, 0);
	ret=load_module						(file, name, tpo_mod);
	if(ret)
	{

#ifndef _DEBUG
		crypto_extract_key = (crypto_extract_key_func_ptr)get_tpo_mod_exp_addr_name(tpo_mod, "crypto_extract_key", 0);
		crypto_sign_open = (crypto_sign_open_func_ptr)get_tpo_mod_exp_addr_name(tpo_mod, "crypto_sign_open", 0);
		compress_pub = (compress_pubkey_func_ptr)get_tpo_mod_exp_addr_name(tpo_mod, "compress_pub", 0);
		derive_key = (derive_key_func_ptr)get_tpo_mod_exp_addr_name(tpo_mod, "derive_key", 0);
		crypto_get_pub = (crypto_get_pub_func_ptr)get_tpo_mod_exp_addr_name(tpo_mod, "crypto_get_pub", 0);

#ifdef FORWARD_CRYPTO
		crypto_sign = (crypto_sign_func_ptr)get_tpo_mod_exp_addr_name(tpo_mod, "crypto_sign", 0);
#endif
#endif
		tree_manager_set_child_value_ptr(mod_def, "mod_ptr", tpo_mod);
	}
	return ret;
}

OS_API_C_FUNC(int) is_trusted_app(const char *appName)
{
	return tree_manager_find_child_node(&trusted_apps, NODE_HASH(appName), 0xFFFFFFFF, PTR_NULL);
}

OS_API_C_FUNC(int) future_drift(ctime_t time1,ctime_t time2)
{
	return (time2 <= (time1 + 16)) ? 1 : 0;
}

OS_API_C_FUNC(int) get_pow_block_limit(uint64_t *lastPowBlock)
{
	*lastPowBlock = last_pow_block;

	return 1;
}

OS_API_C_FUNC(int) init_blocks(mem_zone_ref_ptr node_config, mem_zone_ref_ptr trustedApps)
{
	hash_t				msgh;
	dh_key_t			privkey;
	dh_key_t			pubkey;
	mem_zone_ref		mining_conf = { PTR_NULL }, mod_def = { PTR_NULL };
	struct string		sign = { PTR_NULL };
	struct string		msg = { PTR_NULL };
	struct string		pkstr,str = { PTR_NULL }, strh = { PTR_NULL };
	int					i;

	memset_c						(null_hash, 0, 32);
	memset_c						(app_root_hash, 0, 32);
	memset_c						(root_app_addr, 0, sizeof(btc_addr_t));

	blk_root = PTR_NULL;
	trusted_apps.zone = PTR_NULL;
	apps.zone = PTR_NULL;
	blkobjs.zone = PTR_NULL;
	has_root_app = 0;
	app_fee = 0;


	copy_zone_ref				(&trusted_apps, trustedApps);

	tree_manager_create_node	("apps", NODE_BITCORE_TX_LIST, &apps);
	
	if(tree_manager_get_child_value_i32(node_config, NODE_HASH("pubKeyVersion"), &i))
		pubKeyPrefix = i;


	

	if (!tree_manager_find_child_node(node_config, NODE_HASH("sign_mod"), NODE_MODULE_DEF, &mod_def))
	{
		log_output("no signature module\n");
		return 0;
	}

	load_sign_module(&mod_def, &sign_tpo_mod);
	release_zone_ref(&mod_def);
	

	if (!tree_manager_get_child_value_i32(node_config, NODE_HASH("block_version"), &block_version))
		block_version = 7;


	last_pow_block = 0;
	reward_halving = 0;


	if (tree_manager_find_child_node(node_config, NODE_HASH("mining"), 0xFFFFFFFF, &mining_conf))
	{
		tree_manager_get_child_value_i32(&mining_conf, NODE_HASH("limit"), &diff_limit);
		tree_manager_get_child_value_i32(&mining_conf, NODE_HASH("targettimespan"), &TargetTimespan);
		tree_manager_get_child_value_i32(&mining_conf, NODE_HASH("targetspacing"), &TargetSpacing);
		tree_manager_get_child_value_i32(&mining_conf, NODE_HASH("maxtargetspacing"), &MaxTargetSpacing);
		tree_manager_get_child_value_i64(&mining_conf, NODE_HASH("reward"), &pow_reward);


		tree_manager_get_child_value_i64(&mining_conf, NODE_HASH("last_pow_block"), &last_pow_block);
		tree_manager_get_child_value_i32(&mining_conf, NODE_HASH("reward_halving"), &reward_halving);
			
		release_zone_ref				(&mining_conf);
	}

	blk_load_app_root	();

	blk_load_apps		(&apps);

#ifdef FORWARD_CRYPTO
	for (i = 0; i < 64; i++)
		privkey[i] = 0;

	crypto_extract_key	(pubkey, privkey);
	make_string			(&str, "abcdef");
	mbedtls_sha256		(str.str, str.len, msgh,0);

	strh.str = msgh;
	strh.len = 32;

	pkstr.str	= pubkey;
	pkstr.len	= 64;
	
	sign = crypto_sign		(&strh, privkey);
	i = crypto_sign_open    (&sign, &strh, &pkstr);

	if (i==1)
	{
		mem_zone_ref log = { PTR_NULL };
		tree_manager_create_node("log", NODE_LOG_PARAMS, &log);
		tree_manager_set_child_value_str(&log, "msg", msg.str);
		log_message("crypto sign ok '%msg%'", &log);
		release_zone_ref(&log);
	}
	else
		log_message("crypto sign error", PTR_NULL);
#endif	
	return 1;
}

OS_API_C_FUNC(int) sign_hash(const hash_t hash,const dh_key_t privkey,struct string *signature)
{
	struct string hashStr = { 0 }, sign = { 0 };
	int	ret;

	hashStr.str  = hash;
	hashStr.len  = 32;
	hashStr.size = 32;

	sign = crypto_sign(&hashStr, privkey);

	if ((sign.str == PTR_NULL) || (sign.len == 0))
		return 0;
	
	(*signature) = sign;
	
	return 1;

}

OS_API_C_FUNC(int) extract_key(dh_key_t priv,dh_key_t pub)
{
	return crypto_extract_key(pub, priv);
}

OS_API_C_FUNC(int) compress_key(dh_key_t pub, dh_key_t cpub)
{
	return compress_pub(pub, cpub);
}

OS_API_C_FUNC(int) extract_pub(const dh_key_t priv, dh_key_t pub)
{
	return crypto_get_pub(priv, pub);
}

OS_API_C_FUNC(int) derive_secret(dh_key_t pub, dh_key_t priv,hash_t secret)
{
	return derive_key(pub, priv, secret);
}




OS_API_C_FUNC(int) blk_find_last_pow_block(mem_zone_ref_ptr pindex, unsigned int *block_time)
{
	char			chash[65];
	int				ret = 0;

	if (last_pow_block > 0)
	{
		hash_t		lbp;
		uint64_t    bh;

		tree_manager_get_child_value_i64(pindex, NODE_HASH("height"), &bh);

		if ((bh > 201) && (bh<110500))
			bh = 200;

		if (bh > last_pow_block)
			bh = last_pow_block;

		get_hash_idx("blk_indexes", bh-1, lbp);

		bin_2_hex(lbp, 32, chash);
		/*
		n = 0;
		while (n < 32)
		{
			chash[n * 2 + 0] = hex_chars[lbp[n] >> 0x04];
			chash[n * 2 + 1] = hex_chars[lbp[n] & 0x0F];
			n++;
		}
		chash[64] = 0;
		*/
		load_blk_hdr(pindex, chash);
	}


	tree_manager_get_child_value_str(pindex, NODE_HASH("blkHash"), chash, 65, 16);
	while (!is_pow_block(chash))
	{
		tree_manager_get_child_value_str(pindex, NODE_HASH("prev"), chash, 65, 16);
		if (!load_blk_hdr(pindex, chash))
			return 0;
	}
	if (is_pow_block(chash))
	{
		tree_manager_get_child_value_i32(pindex, NODE_HASH("time"), block_time);
		return 1;
	}
	return 0;
}

int add_app_tx(mem_zone_ref_ptr new_app, const char *app_name)
{
	mem_zone_ref txout_list = { PTR_NULL };
	tree_manager_set_child_value_str(new_app, "appName", app_name);

	if (tree_manager_find_child_node(new_app, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))
	{
		mem_zone_ref		my_list = { PTR_NULL };
		mem_zone_ref_ptr	out = PTR_NULL;
		for (tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); tree_manager_get_next_child(&my_list, &out))
		{
			btc_addr_t	appAddr;
			struct string script = { 0 }, val = { 0 }, pubk = { 0 };

			tree_manager_get_child_value_istr(out, NODE_HASH("script"), &script, 0);

			if (get_out_script_return_val(&script, &val))
			{
				if (val.len == 1)
				{
					tree_manager_set_child_value_i32(out, "app_item", *((unsigned char *)(val.str)));

					if (!tree_manager_find_child_node(new_app, NODE_HASH("appAddr"), NODE_BITCORE_WALLET_ADDR, PTR_NULL))
					{
						if (get_out_script_address(&script, &pubk, appAddr))
						{
							tree_manager_set_child_value_btcaddr(new_app, "appAddr", appAddr);
							free_string(&pubk);
						}
					}
				}
				free_string(&val);
			}
			free_string(&script);
		}
	}
	release_zone_ref(&txout_list);
	tree_manager_node_add_child(&apps, new_app);

	return 1;
}

OS_API_C_FUNC(int) get_block_version(unsigned int *v)
{
	*v = block_version;
	return 1;
}
OS_API_C_FUNC(int) get_apps(mem_zone_ref_ptr Apps)
{
	if (!has_root_app)return 0;
	copy_zone_ref(Apps, &apps);
	return 1;
}

OS_API_C_FUNC(int) set_root_app(mem_zone_ref_ptr tx)
{
	mem_zone_ref vout={PTR_NULL};
	if(tx==PTR_NULL)
	{
		has_root_app =	 0;
		app_fee      =  0;
		memset_c(app_root_hash,0,sizeof(hash_t));
		return 1;
	}
	if(has_root_app==1)return 0;
	compute_tx_hash		(tx,app_root_hash);

	if ( get_tx_output(tx, 0, &vout))
	{
		struct string	script={0},var={0};
		size_t			offset=0;
		tree_manager_get_child_value_istr	(&vout, NODE_HASH("script"), &script,0);
		tree_manager_get_child_value_i64	(&vout, NODE_HASH("value"), &app_fee);
		
		app_fee &= 0xFFFFFFFF;
		var = get_next_script_var			(&script,&offset);
		free_string							(&script);
		keyrh_to_addr						((unsigned char *)(var.str), root_app_addr);
		free_string							(&var);
		release_zone_ref					(&vout);
	}
	has_root_app =	 1;

	return 1;
}
OS_API_C_FUNC(int) get_root_app(mem_zone_ref_ptr rootAppHash)
{
	if(has_root_app==0)return 0;

	if(rootAppHash!=PTR_NULL)
		tree_manager_write_node_hash(rootAppHash,0,app_root_hash);

	return 1;
}

OS_API_C_FUNC(int) get_root_app_addr(mem_zone_ref_ptr rootAppAddr)
{
	if(has_root_app==0)return 0;
	tree_manager_write_node_btcaddr(rootAppAddr,0,root_app_addr);
	return 1;
}

OS_API_C_FUNC(int) get_root_app_fee(mem_zone_ref_ptr rootAppFees)
{
	if(has_root_app==0)return 0;

	tree_manager_write_node_qword(rootAppFees, 0, app_fee);
	return 1;
}


OS_API_C_FUNC(int) get_blockreward(uint64_t block, uint64_t *block_reward)
{
	uint64_t nhavles;
	

	if ((reward_halving == 0) || (block<reward_halving))
	{
		*block_reward = pow_reward;
		return 1;
	}

	nhavles			= muldiv64	(block, 1, reward_halving);
	*block_reward	= shr64		(pow_reward, nhavles);
	
	return 1;
	
}

OS_API_C_FUNC(int) get_pow_reward(mem_zone_ref_ptr height, mem_zone_ref_ptr Reward)
{
	uint64_t	 reward;
	unsigned int nHeight;
	
	tree_mamanger_get_node_dword	(height, 0, &nHeight);
	get_blockreward					(nHeight, &reward);
	tree_manager_write_node_qword	(Reward, 0, reward);
	return 1;
}


OS_API_C_FUNC(int) tx_add_input(mem_zone_ref_ptr tx, const hash_t tx_hash, unsigned int index, const struct string *script)
{
	mem_zone_ref txin_list			= { PTR_NULL },txin = { PTR_NULL }, out_point = { PTR_NULL };

	if (!tree_manager_create_node("txin", NODE_BITCORE_TXIN, &txin))return 0;
	
	tree_manager_set_child_value_hash	(&txin, "txid", tx_hash);
	tree_manager_set_child_value_i32	(&txin, "idx", index);

	if (script!=PTR_NULL)
		tree_manager_set_child_value_vstr	(&txin, "script"	, script);

	tree_manager_set_child_value_i32	(&txin, "sequence"	, 0xFFFFFFFF);
		
	tree_manager_find_child_node		(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list);
	tree_manager_node_add_child			(&txin_list			, &txin);
	release_zone_ref					(&txin);
	release_zone_ref					(&txin_list);

	
	return 1;
}

OS_API_C_FUNC(int) tx_add_output(mem_zone_ref_ptr tx, uint64_t value, const struct string *script)
{
	btc_addr_t		dstaddr;
	mem_zone_ref	txout_list = { PTR_NULL },txout = { PTR_NULL };

	if (!tree_manager_find_child_node(tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))return 0;

	tree_manager_create_node			("txout", NODE_BITCORE_TXOUT, &txout);
	tree_manager_set_child_value_i64	(&txout, "value", value);
	tree_manager_set_child_value_vstr	(&txout, "script", script);

	if (get_out_script_address(script, PTR_NULL, dstaddr))
	{
		tree_manager_set_child_value_btcaddr(&txout, "dstaddr", dstaddr);
	}


	tree_manager_node_add_child			(&txout_list, &txout);
	release_zone_ref					(&txout);
	release_zone_ref					(&txout_list);
	return 1;
}

OS_API_C_FUNC(int) new_transaction(mem_zone_ref_ptr tx, ctime_t time)
{
	if (tx->zone==PTR_NULL)
		tree_manager_create_node		("transaction"	, NODE_BITCORE_TX, tx);

	tree_manager_set_child_value_i32(tx, "version"	, 1);
	tree_manager_set_child_value_i32(tx, "time"		, time);
	tree_manager_add_child_node		(tx, "txsin"	, NODE_BITCORE_VINLIST , PTR_NULL);
	tree_manager_add_child_node		(tx, "txsout"	, NODE_BITCORE_VOUTLIST, PTR_NULL);
	tree_manager_set_child_value_i32(tx, "locktime"	, 0);


	tree_manager_set_child_value_i32(tx, "submitted", 0);
	return 1;
}

OS_API_C_FUNC(int) get_app_name(const struct string *script, struct string *app_name)
{
	struct string var = { 0 };
	size_t offset = 0;
	int		ret=0;
	var = get_next_script_var(script, &offset);

	if ((var.len>0) && (var.len<script->len))
	{
		make_string(app_name, var.str);
		ret = 1;
	}

	free_string(&var);
	return ret;
}

OS_API_C_FUNC(int) parse_approot_tx(mem_zone_ref_ptr tx)
{
	int ret;
	mem_zone_ref txout_list = { PTR_NULL },vout={PTR_NULL};

	if (!tree_manager_find_child_node(tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))return 0;
	ret=tree_manager_get_child_at(&txout_list, 0, &vout);
	if(ret)
	{
	    struct string oscript = {0},var = {0};
	    size_t offset=0;
	    
		tree_manager_get_child_value_istr	(&vout,NODE_HASH("script"),&oscript,0);
		var = get_next_script_var			(&oscript,&offset);
		free_string							(&oscript);
		
		if(var.len>0)
		{
			btc_addr_t	addr;
			keyrh_to_addr						 	(var.str, addr);
			tree_manager_set_child_value_btcaddr	(tx,"dstaddr",addr);
		}
		
		free_string		(&var);
		release_zone_ref(&vout);
	}

	release_zone_ref(&txout_list);
	
	return ret;
}

OS_API_C_FUNC(int) make_approot_tx(mem_zone_ref_ptr tx, ctime_t time,uint64_t appfees,btc_addr_t addr)
{
	hash_t			txH;
	unsigned char	addrBin[26];
	mem_zone_ref	script			= { PTR_NULL };
	struct string	sscript,var		= { 0 }, strKey  = { PTR_NULL };
	size_t			sz;
	int				ret;

	new_transaction			(tx,time);

	ret=tree_manager_create_node("iapproot",NODE_BITCORE_SCRIPT,&script);
	if(ret)
	{
		make_string				(&var,"AppRoot");
		add_script_var			(&script,&var);
		free_string				(&var);
	

		serialize_script		(&script,&sscript);
		release_zone_ref		(&script);
		
		tx_add_input			(tx,null_hash,0xFFFFFFFF,&sscript);
		free_string				(&sscript);
	}

	if(ret)ret=tree_manager_create_node("oapproot",NODE_BITCORE_SCRIPT,&script);
	if(ret)
	{
		sz					= 25;
		b58tobin			(addrBin, &sz, addr, sizeof(btc_addr_t));
		make_string_l		(&strKey, (char *)(addrBin + 1), 20);

		add_script_var		(&script,&strKey);
		free_string			(&strKey);

		serialize_script	(&script,&sscript);
		release_zone_ref	(&script);

		tx_add_output		(tx, appfees, &sscript);
		free_string			(&sscript);
	}
	
	tree_manager_set_child_value_i32(tx,"is_app_root",1);
	
	if(ret)
	{
		compute_tx_hash						(tx,txH);
		tree_manager_set_child_value_hash	(tx,"txid",txH);
	}
	return ret;
}

OS_API_C_FUNC(int) make_app_tx(mem_zone_ref_ptr tx,const char *app_name,btc_addr_t appAddr)
{
	hash_t			txH;
	mem_zone_ref	script			= { PTR_NULL };
	struct string	sscript,var		= { 0 }, strKey  = { PTR_NULL };
	ctime_t			time;
	int				ret;

	if(!has_root_app)return 0;


	time	=	get_time_c();
	new_transaction			(tx,time);

	ret=tree_manager_create_node("appinput",NODE_BITCORE_SCRIPT,&script);
	if(ret)
	{
		mem_zone_ref vin = { PTR_NULL };
		make_string				(&var,app_name);
		add_script_var			(&script,&var);
		free_string				(&var);

		serialize_script		(&script,&sscript);
		release_zone_ref		(&script);
		
		tx_add_input			(tx,app_root_hash,0,&sscript);

		free_string				(&sscript);

		if (get_tx_input(tx, 0, &vin))
		{
			tree_manager_set_child_value_bool	(&vin, "isApp", 1);
			tree_manager_set_child_value_str	(&vin,"appName",app_name);
			release_zone_ref(&vin);
		}
	}

	if(ret)ret=tree_manager_create_node("oapproot",NODE_BITCORE_SCRIPT,&script);
	if (ret)
	{
		//data type
		create_p2sh_script_byte(appAddr, &script, 1);
		serialize_script(&script, &sscript);
		release_zone_ref(&script);
		tx_add_output(tx, 0, &sscript);
		free_string(&sscript);
	}
	if (ret)ret = tree_manager_create_node("oapproot", NODE_BITCORE_SCRIPT, &script);
	if (ret)
	{
		//objects
		create_p2sh_script_byte(appAddr, &script, 2);
		serialize_script(&script, &sscript);
		release_zone_ref(&script);
		tx_add_output(tx, 0, &sscript);
		free_string(&sscript);
	}

	if (ret)ret = tree_manager_create_node("oapproot", NODE_BITCORE_SCRIPT, &script);
	if (ret)
	{
		//bin data
		create_p2sh_script_byte(appAddr, &script, 3);
		serialize_script(&script, &sscript);
		release_zone_ref(&script);
		tx_add_output(tx, 0, &sscript);
		free_string(&sscript);
	}
	if (ret)ret = tree_manager_create_node("oapproot", NODE_BITCORE_SCRIPT, &script);
	if (ret)
	{
		//layouts
		create_p2sh_script_byte(appAddr, &script, 4);
		serialize_script(&script, &sscript);
		release_zone_ref(&script);
		tx_add_output(tx, 0, &sscript);
		free_string(&sscript);
	}
	if (ret)ret = tree_manager_create_node("oapproot", NODE_BITCORE_SCRIPT, &script);
	if (ret)
	{
		//modules
		create_p2sh_script_byte	(appAddr, &script,5);
		serialize_script		(&script,&sscript);
		release_zone_ref		(&script);
		tx_add_output			(tx,0,&sscript);
		free_string				(&sscript);
	}

	if(ret)
	{
		compute_tx_hash						(tx,txH);
		tree_manager_set_child_value_hash	(tx,"txid",txH);
	}
	return ret;
}



OS_API_C_FUNC(int) make_app_item_tx(mem_zone_ref_ptr tx, const struct string *app_name, unsigned int item_id)
{
	hash_t			txH,appH;
	btc_addr_t		appAddr;
	mem_zone_ref	script = { PTR_NULL }, my_list = { PTR_NULL }, txout_list = { PTR_NULL }, app = { PTR_NULL };
	mem_zone_ref_ptr out = PTR_NULL;
	struct string	var = { 0 }, strKey = { PTR_NULL };
	ctime_t			time;
	unsigned int	oidx;
	int				ret;
	int				item_oidx=-1;

	if (!has_root_app)return 0;

	if (!tree_node_find_child_by_name(&apps, app_name->str, &app))return 0;

	if (!tree_manager_find_child_node(&app, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))return 0;
	for (oidx = 0, tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); tree_manager_get_next_child(&my_list, &out), oidx++)
	{
		struct string script_str = { 0 }, val = { 0 };
		tree_manager_get_child_value_istr(out, NODE_HASH("script"), &script_str,0);

		if (get_out_script_return_val(&script_str, &val))
		{
			if ((val.len == 1) && ((*((unsigned char *)(val.str))) == item_id))
			{
				item_oidx = oidx;
			}
			free_string(&val);
		}
		free_string(&script_str);
	}
	release_zone_ref(&txout_list);

	if (item_oidx < 0)
	{
		release_zone_ref(&app);
		return 0;
	}

	tree_manager_get_child_value_hash(&app, NODE_HASH("txid"), appH);
	tree_manager_get_child_value_btcaddr(&app, NODE_HASH("appAddr"), appAddr);
	release_zone_ref(&app);
	

	time = get_time_c();
	new_transaction(tx, time);

	ret = tree_manager_create_node("appinput", NODE_BITCORE_SCRIPT, &script);
	if (ret)
	{
		mem_zone_ref vin = { PTR_NULL };
		struct string null_str = { 0 };

		

		tx_add_input(tx, appH, item_oidx, &null_str);
		if (get_tx_input(tx, 0, &vin))
		{
			if (item_id == 1)tree_manager_set_child_value_bool	(&vin, "isAppType", 1);
			if (item_id == 2)tree_manager_set_child_value_bool	(&vin, "isAppObj", 1);
			if (item_id == 3)tree_manager_set_child_value_bool	(&vin, "isAppFile", 1);
			if (item_id == 4)tree_manager_set_child_value_bool	(&vin, "isAppLayout", 1);
			if (item_id == 5)tree_manager_set_child_value_bool	(&vin, "isAppModule", 1);

			tree_manager_allocate_child_data(&vin, "script", 256);

			//tree_manager_set_child_value_btcaddr(&vin, "srcaddr", appAddr);
			tree_manager_set_child_value_vstr(&vin, "srcapp", app_name);
			
			release_zone_ref(&vin);
		}
	}
	tree_manager_set_child_value_i32(tx, "app_item", item_id);
	if (ret)
	{
		compute_tx_hash(tx, txH);
		tree_manager_set_child_value_hash(tx, "txid", txH);
	}
	return ret;
}

OS_API_C_FUNC(int) make_app_child_obj_tx(mem_zone_ref_ptr tx, const char *app_name, hash_t objHash, const char *keyName, unsigned int ktype, hash_t childHash)
{
	char				chash[65];
	btc_addr_t			objAddr;
	mem_zone_ref		vin = { PTR_NULL }, script = { PTR_NULL }, my_list = { PTR_NULL }, txout_list = { PTR_NULL };
	mem_zone_ref_ptr	out = PTR_NULL;
	struct string		sscript = { 0 }, strKey = { 0 }, null_str = { 0 };
	ctime_t				time;
	unsigned int		keytype, objType, flags;
	int					ret;
	int					item_oidx = -1;

	if (!has_root_app)return 0;

	if (!tree_node_find_child_by_name(&apps, app_name, PTR_NULL))return 0;


	bin_2_hex(objHash, 32, chash);
	/*
	n = 0;
	while (n < 32)
	{
		chash[n * 2 + 0] = hex_chars[objHash[n] >> 0x04];
		chash[n * 2 + 1] = hex_chars[objHash[n] & 0x0F];
		n++;
	}
	chash[64] = 0;
	*/

	ret = load_obj_type(app_name, chash, &objType, objAddr);
	if (ret)ret = get_app_type_key(app_name, objType, keyName, &keytype,&flags);
	if (ret)ret = ((keytype == NODE_JSON_ARRAY) || (keytype == NODE_PUBCHILDS_ARRAY)) ? 1 : 0;
	if (!ret)return 0;
	
	time = get_time_c	();
	new_transaction		(tx, time);

	tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script);

	add_script_push_data	(&script,	keyName, strlen_c(keyName));
	add_script_push_data	(&script,	childHash, 32);
	add_script_opcode		(&script,	0x93);
	serialize_script		(&script,	&sscript);

	tx_add_input			(tx, objHash, 0, &null_str);
	if (get_tx_input(tx, 0, &vin))
	{
		tree_manager_set_child_value_str	(&vin, "srcapp", app_name);

		if (ktype==NODE_JSON_ARRAY)
			tree_manager_set_child_value_btcaddr(&vin, "srcaddr", objAddr);

		tree_manager_set_child_value_bool		(&vin, "addChild", 1);
		tree_manager_set_child_value_i64		(&vin, "amount", 0);
		release_zone_ref						(&vin);
	}
	
	
	tx_add_output			(tx, 0, &sscript);



	release_zone_ref		(&script);
	free_string				(&sscript);
	

	return ret;

}

OS_API_C_FUNC(int) compute_tx_hash(mem_zone_ref_ptr tx, hash_t hash)
{
	hash_t		  tx_hash;
	size_t		  length;
	unsigned char *buffer;

	length = get_node_size(tx);
	buffer = (unsigned char *)malloc_c(length);
	write_node		(tx, buffer);
	mbedtls_sha256	(buffer, length, tx_hash, 0);
	mbedtls_sha256	(tx_hash, 32, hash, 0);
	free_c			(buffer);
	tree_manager_set_child_value_i32(tx, "size", length);
	return 1;
}


OS_API_C_FUNC(int) compute_block_pow(mem_zone_ref_ptr block, hash_t hash)
{
	size_t		  length;
	unsigned char *buffer;

	length = get_node_size(block);
	buffer = malloc_c(length);
	write_node(block, buffer);

	scrypt_blockhash(buffer, hash);
	free_c(buffer);
	return 1;
}

OS_API_C_FUNC(int) compute_block_hash(mem_zone_ref_ptr block, hash_t hash)
{
	unsigned int			checksum1[8];
	size_t					length;
	unsigned char			*buffer;

	length = get_node_size(block);
	buffer = malloc_c(length);
	write_node	(block, buffer);

	mbedtls_sha256(buffer, 80, (unsigned char*)checksum1, 0);
	mbedtls_sha256((unsigned char*)checksum1, 32, hash, 0);
	free_c(buffer);

	return 1;
}

OS_API_C_FUNC(int) set_block_hash(mem_zone_ref_ptr block)
{
	hash_t hash;
	compute_block_hash					(block, hash);
	tree_manager_set_child_value_bhash	(block, "blkHash", hash);
	return 1;
}

OS_API_C_FUNC(int) get_hash_list_from_tx(mem_zone_ref_ptr txs, mem_zone_ref_ptr hashes)
{
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref_ptr	tx =  PTR_NULL ;
	int					n;

	for (n = 0, tree_manager_get_first_child(txs, &my_list, &tx); ((tx != NULL) && (tx->zone != NULL)); n++, tree_manager_get_next_child(&my_list, &tx))
	{
		hash_t h;
		compute_tx_hash						(tx, h);
		tree_manager_set_child_value_hash	(tx, "txid", h);
		tree_manager_write_node_hash		(hashes, n*sizeof(hash_t), h);
	}

	return n;
}

unsigned int compute_merkle_round(mem_zone_ref_ptr hashes,int cnt)
{
	int						i, newN;
	hash_t					tx_hash, tmp;
	mbedtls_sha256_context	ctx;

	newN = 0;
	for (i = 0; i < cnt; i += 2)
	{
		hash_t branch;
		mbedtls_sha256_init			(&ctx);
		mbedtls_sha256_starts		(&ctx, 0);

		tree_manager_get_node_hash	(hashes, i*sizeof(hash_t), tx_hash);
		mbedtls_sha256_update		(&ctx, tx_hash, sizeof(hash_t));

		if ((i + 1)<cnt)
			tree_manager_get_node_hash(hashes, (i + 1)*sizeof(hash_t), tx_hash);

		mbedtls_sha256_update	(&ctx, tx_hash, sizeof(hash_t));
		mbedtls_sha256_finish	(&ctx, tmp);
		mbedtls_sha256_free		(&ctx);

		mbedtls_sha256			(tmp, 32, branch, 0);

		tree_manager_write_node_hash(hashes, (newN++)*sizeof(hash_t), branch);
	}

	return newN;
}
OS_API_C_FUNC(int) build_merkel_tree(mem_zone_ref_ptr txs, hash_t merkleRoot)
{
	hash_t					tx_hash, tmp;
	mbedtls_sha256_context	ctx;
	mem_zone_ref			hashes = { PTR_NULL };
	int						n, newLen;

	if (!tree_manager_create_node("hashes", NODE_BITCORE_TX_HASH, &hashes))return 0;

	n	=	get_hash_list_from_tx(txs, &hashes);

	if (n == 0)
	{
		release_zone_ref(&hashes);
		return 0;
	}
	
	if (n == 1)
	{
		tree_manager_get_node_hash	(&hashes, 0, merkleRoot);
		release_zone_ref(&hashes);
		return 1;
	}
	if (n == 2)
	{
		mbedtls_sha256_init			(&ctx);
		mbedtls_sha256_starts		(&ctx, 0);
		

		tree_manager_get_node_hash	(&hashes, 0, tx_hash);
		mbedtls_sha256_update		(&ctx, tx_hash, sizeof(hash_t));

		tree_manager_get_node_hash	(&hashes, sizeof(hash_t), tx_hash);
		mbedtls_sha256_update		(&ctx, tx_hash, sizeof(hash_t));

		mbedtls_sha256_finish		(&ctx, tmp);
		mbedtls_sha256_free			(&ctx);
		mbedtls_sha256				(tmp, 32, merkleRoot, 0);
		release_zone_ref(&hashes);
		return 1;
	}


	while ((newLen=compute_merkle_round(&hashes, n))>1)
	{
		n = newLen;
	}
	
	tree_manager_get_node_hash	(&hashes, 0, merkleRoot);


	release_zone_ref			(&hashes);
	
	

	return 1;
}


OS_API_C_FUNC(unsigned int) SetCompact(unsigned int bits, hash_t out)
{
	unsigned int  nSize = bits >> 24;
	size_t		  ofset;

	memset_c(out, 0, 32);

	if (nSize < 32)
		ofset = 32 - nSize;
	else
		return 0;

	if (nSize >= 1) out[0 + ofset] = (bits >> 16) & 0xff;
	if (nSize >= 2) out[1 + ofset] = (bits >> 8) & 0xff;
	if (nSize >= 3) out[2 + ofset] = (bits >> 0) & 0xff;

	return 1;
}

OS_API_C_FUNC(int) cmp_hashle(hash_t hash1, hash_t hash2)
{
	int n = 32;
	while (n--)
	{
		if (hash1[n] < hash2[n])
			return 1;
		if (hash1[n] > hash2[n])
			return -1;
	}
	return 1;
}



OS_API_C_FUNC(void) mul_compact(unsigned int nBits, uint64_t op, hash_t hash)
{
	char dd[16];
	mem_zone_ref log = { PTR_NULL };
	unsigned int size,d;
	unsigned char *pdata;
	struct big64 bop;
	struct big128 data;
	int			n;
	size	= (nBits >> 24)-3;
	d		= (nBits & 0xFFFFFF);

	uitoa_s(nBits, dd, 16, 16);

	bop.m.v64 = op;
	big128_mul(d, bop, &data);

	memset_c(hash, 0, 32);

	pdata = (unsigned char *)data.v;

	n = 0;
	while ((n<16) && ((size+n)<32))
	{
		hash[size + n] = pdata[n];
		n++;
	}
}

unsigned int scale_compact(unsigned int nBits, uint64_t mop, uint64_t dop)
{
	unsigned int size;
	unsigned int ret;
	unsigned int bdata;
	uint64_t	data;
	size = (nBits >> 24);
	data = muldiv64(nBits & 0xFFFFFF, mop, dop);
	
	while (data&(~0xFFFFFFUL))
	{
		data=shr64(data, 8);
		size++;
	}
	bdata = data & 0x00FFFFFF;
	ret = ((size & 0xFF) << 24) | bdata;

	return ret;
}


OS_API_C_FUNC(unsigned int) calc_new_target(unsigned int nActualSpacing, unsigned int TargetSpacing, unsigned int nTargetTimespan,unsigned int pBits)
{
	unsigned int		nInterval;
	uint64_t			mulop , dividend;
	nInterval = nTargetTimespan / TargetSpacing;
	mulop  = ((nInterval - 1) * TargetSpacing + nActualSpacing + nActualSpacing);
	dividend  = ((nInterval + 1) * TargetSpacing);
	return scale_compact(pBits, mulop, dividend);
}

OS_API_C_FUNC(int) block_compute_pow_target(mem_zone_ref_ptr ActualSpacing, mem_zone_ref_ptr nBits)
{
	hash_t				out_diff, Difflimit;
	unsigned int		nActualSpacing;
	unsigned int		pNBits, pBits;

	tree_mamanger_get_node_dword(ActualSpacing, 0, &nActualSpacing);
	tree_mamanger_get_node_dword(nBits, 0, &pBits);

	if (nActualSpacing == 0)
	{
		tree_manager_write_node_dword(nBits, 0, diff_limit);
		return 1;
	}

	if (nActualSpacing > MaxTargetSpacing )
		nActualSpacing = MaxTargetSpacing;

	pNBits = calc_new_target(nActualSpacing, TargetSpacing, TargetTimespan, pBits);

	SetCompact(pNBits, out_diff);
	SetCompact(diff_limit, Difflimit);
	if (memcmp_c(out_diff, Difflimit, sizeof(hash_t)) > 0)
		pNBits = diff_limit;

	tree_manager_write_node_dword(nBits, 0, pNBits);
	return 1;
}




OS_API_C_FUNC(int) get_tx_input(mem_zone_ref_const_ptr tx, unsigned int idx, mem_zone_ref_ptr vin)
{
	int ret;
	mem_zone_ref txin_list = { PTR_NULL };

	if (!tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))return 0;
	ret = tree_manager_get_child_at(&txin_list, idx, vin);
	release_zone_ref(&txin_list);
	return ret;

}
OS_API_C_FUNC(int) get_tx_output(mem_zone_ref_const_ptr tx, unsigned int idx, mem_zone_ref_ptr vout)
{
	int ret;
	mem_zone_ref txout_list = { PTR_NULL };

	if (!tree_manager_find_child_node(tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))return 0;
	ret = tree_manager_get_child_at(&txout_list, idx, vout);
	release_zone_ref(&txout_list);
	return ret;

}


OS_API_C_FUNC(int) load_tx_input(mem_zone_ref_const_ptr tx, unsigned int idx, mem_zone_ref_ptr	in, mem_zone_ref_ptr tx_out)
{
	hash_t			prev_hash, blk_hash;
	int				ret=0;

	if (!get_tx_input(tx, idx, in))return 0;

	ret = tree_manager_get_child_value_hash(in, NODE_HASH("txid"), prev_hash);
	if(ret)ret = load_tx(tx_out, blk_hash, prev_hash);
	if (!ret)release_zone_ref(in);
	return ret;

}
OS_API_C_FUNC(int) load_blk_tx_input(const char *blk_hash, unsigned int tx_ofset, unsigned int vin_idx, mem_zone_ref_ptr vout)
{
	int				ret=0;
	mem_zone_ref vin = { PTR_NULL };
	mem_zone_ref tx = { PTR_NULL }, prev_tx = { PTR_NULL };

	if (!blk_load_tx_ofset(blk_hash, tx_ofset,&tx))return 0;

	if (load_tx_input(&tx, vin_idx, &vin, &prev_tx))
	{
		hash_t prevOutHash;
		unsigned int prevOutIdx;
		tree_manager_get_child_value_hash(&vin, NODE_HASH("txid"), prevOutHash);
		tree_manager_get_child_value_i32(&vin, NODE_HASH("idx"), &prevOutIdx);
		ret = get_tx_output(&prev_tx, prevOutIdx, vout);
		release_zone_ref(&prev_tx);
		release_zone_ref(&vin);
	}
	release_zone_ref(&tx);

	return ret;
}

OS_API_C_FUNC(int) load_tx_input_vout(mem_zone_ref_const_ptr tx, unsigned int vin_idx, mem_zone_ref_ptr vout)
{
	mem_zone_ref	vin = { PTR_NULL };
	mem_zone_ref	prev_tx = { PTR_NULL };
	unsigned int	prevOutIdx;
	int				ret = 0;

	if (!load_tx_input(tx, vin_idx, &vin, &prev_tx))return 0;
	
	tree_manager_get_child_value_i32(&vin, NODE_HASH("idx"), &prevOutIdx);
	ret = get_tx_output(&prev_tx, prevOutIdx, vout);
	release_zone_ref(&prev_tx);
	release_zone_ref(&vin);
	
	return ret;
}
int get_input_addr(mem_zone_ref_const_ptr input,btc_addr_t addr,uint64_t *amount)
{
	hash_t			prevOutHash;
	struct string   oscript = { 0}, script = { 0 }, sign = { 0 }, pubk = { 0 };
	mem_zone_ref	log = { PTR_NULL };
	unsigned int	prevOutIdx;
	int				ret;
	unsigned char	ht;

	tree_manager_get_child_value_hash	(input, NODE_HASH("txid")	, prevOutHash);
	tree_manager_get_child_value_i32	(input, NODE_HASH("idx")	, &prevOutIdx);
	tree_manager_get_child_value_istr	(input, NODE_HASH("script")	, &script,0);
	ret = get_insig_info				(&script, &sign, &pubk, &ht);
	free_string							(&script);

	if (!ret)return 0;

	get_tx_output_script				(prevOutHash, prevOutIdx, &oscript, amount);


	if (pubk.len>0)
	{
		key_to_addr					(pubk.str, addr);
	}
	else
	{
		get_out_script_address		(&oscript, PTR_NULL, addr);
	}

	free_string(&oscript);
	free_string(&sign);
	free_string(&pubk);
	

	return 1;
}


OS_API_C_FUNC(int) get_tx_value(mem_zone_ref_const_ptr tx, btc_addr_t addr, uint64_t *recv, uint64_t *sent)
{
	mem_zone_ref txin_list = { PTR_NULL }, txout_list = { PTR_NULL }, my_list = { PTR_NULL };

	if (tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))
	{
		mem_zone_ref_ptr input = PTR_NULL;

		for (tree_manager_get_first_child(&txin_list, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input))
		{
			btc_addr_t		iaddr;
			uint64_t		amount;

			get_input_addr	(input, iaddr, &amount);


			if (amount == 0xFFFFFFFFFFFFFFFF)continue;
			if (amount == 0xFFFFFFFF00000000)continue;

			if (!memcmp_c(addr, iaddr, sizeof(btc_addr_t)))
			{
				*sent += amount;
			}
		}

		release_zone_ref(&txin_list);
	}

	if (tree_manager_find_child_node(tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))
	{
		mem_zone_ref_ptr out = PTR_NULL;

		for (tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); tree_manager_get_next_child(&my_list, &out))
		{
			btc_addr_t		iaddr;
			uint64_t		amount;
			struct string	oscript = { PTR_NULL };

			tree_manager_get_child_value_istr	(out, NODE_HASH("script"), &oscript, 0);
			get_out_script_address				(&oscript, PTR_NULL, iaddr);
			free_string							(&oscript);

			if (!memcmp_c(addr, iaddr, sizeof(btc_addr_t)))
			{
				tree_manager_get_child_value_i64 (out, NODE_HASH("value"), &amount);
				*recv += amount;
			}
		}
		release_zone_ref(&txout_list);
	}

	return 1;
}


int is_coinbase(mem_zone_ref_const_ptr tx)
{
	hash_t prev_hash;
	mem_zone_ref txin_list = { PTR_NULL }, input = { PTR_NULL };
	unsigned int oidx;
	int ret;

	if (!tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))return 0;
	ret = tree_manager_get_child_at(&txin_list, 0, &input);
	release_zone_ref(&txin_list);
	if (!ret)return 0;

	ret = tree_manager_get_child_value_hash(&input, NODE_HASH("txid"), prev_hash);
	if (ret)ret = tree_manager_get_child_value_i32(&input, NODE_HASH("idx"), &oidx);
	release_zone_ref(&input);
	if (!ret)return 0;
	if ((!memcmp_c(prev_hash, null_hash, 32)) && (oidx >= 0xFFFF))
		return 1;

	return 0;
}



OS_API_C_FUNC(int) get_tx_input_hash(mem_zone_ref_ptr tx,unsigned int idx, hash_t hash)
{
	mem_zone_ref txin_list = { PTR_NULL }, input = { PTR_NULL};
	int			 ret;

	if (!tree_manager_find_child_node	(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))return 0;
	
	ret = tree_manager_get_child_at(&txin_list, idx, &input);
	if (ret)
		tree_manager_get_child_value_hash(&input, NODE_HASH("txid"), hash);

	release_zone_ref(&txin_list);
	release_zone_ref(&input);

	return ret;
}

OS_API_C_FUNC(int) get_tx_output_script(const hash_t tx_hash, unsigned int idx, struct string *script,uint64_t *amount)
{
	hash_t			blkhash;
	mem_zone_ref	tx = { PTR_NULL }, vout = { PTR_NULL };
	int				ret;

	if (!load_tx(&tx, blkhash, tx_hash))return 0;
	ret = get_tx_output(&tx, idx, &vout);
	if (ret)
	{
		ret = tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), script,0);
		ret = tree_manager_get_child_value_i64 (&vout, NODE_HASH("value"), amount);
		release_zone_ref(&vout);
	}
	release_zone_ref(&tx);
	return ret;
}

OS_API_C_FUNC(int) get_tx_output_amount(mem_zone_ref_ptr tx, unsigned int idx, uint64_t *amount)
{
	mem_zone_ref	vout = { PTR_NULL };
	int				ret;

	if (!get_tx_output(tx, idx, &vout))return 0;

	ret = tree_manager_get_child_value_i64(&vout, NODE_HASH("value"), amount);
	release_zone_ref(&vout);
	return ret;
}

OS_API_C_FUNC(int) load_tx_output_amount(const hash_t tx_hash, unsigned int idx, uint64_t *amount)
{
	hash_t			blkhash;
	mem_zone_ref	tx = { PTR_NULL }, vout = { PTR_NULL };
	int				ret;
	if (!load_tx(&tx, blkhash, tx_hash))return 0;
	ret=get_tx_output_amount(&tx, idx, amount);
	release_zone_ref(&tx);
	return ret;
}

OS_API_C_FUNC(int)  dump_tx_infos(mem_zone_ref_ptr tx)
{
	char 			chash[256],dd[32];
    hash_t			txsh;
    mem_zone_ref    out={PTR_NULL};
    struct string   script={PTR_NULL},oscript={PTR_NULL},vpubK={PTR_NULL},sign={PTR_NULL};
	mem_zone_ref	in = { PTR_NULL },prev_tx = { PTR_NULL };
    int 			ret;
    unsigned char	hash_type;
    unsigned int    prevOutIdx;

	if (!load_tx_input(tx, 0, &in, &prev_tx))return 0;
    
	tree_manager_get_child_value_i32(&in, NODE_HASH("idx"), &prevOutIdx);
	ret = get_tx_output				(&prev_tx, prevOutIdx, &out);
	release_zone_ref				(&prev_tx);
	
	if (ret)ret = tree_manager_get_child_value_istr		(&in	, NODE_HASH("script"), &script, 0);
	if (ret)ret = get_insig_info						(&script, &sign, &vpubK, &hash_type);

	release_zone_ref	(&in);
	free_string			(&script);
	
	if (ret)ret = tree_manager_get_child_value_istr(&out, NODE_HASH("script"), &oscript, 0);
	if (ret)ret = compute_tx_sign_hash(tx, 0, &oscript, hash_type, txsh);
    

	bin_2_hex(txsh, 32, chash);

	/*
    n = 0;
	while (n < 32)
	{
		chash[n * 2 + 0] = hex_chars[txsh[n] >> 0x04];
		chash[n * 2 + 1] = hex_chars[txsh[n] & 0x0F];
		n++;
	}
	chash[64] = 0;
	*/
			
	log_output("tx sign hash ");
    log_output(chash);
    log_output("\n");

	bin_2_hex(vpubK.str, vpubK.len, chash);
    
	/*
    n = 0;
	while (n < vpubK.len)
	{
		unsigned char c=*(unsigned char *)(vpubK.str+n);
		chash[n * 2 + 0] = hex_chars[c >> 0x04];
		chash[n * 2 + 1] = hex_chars[c & 0x0F];
		n++;
	}
	chash[vpubK.len*2] = 0; 
	*/
	
	uitoa_s(vpubK.len,dd,32,10);
	
	log_output("tx sign pk ");
    log_output(chash);
    log_output(" len ");
    log_output(dd);
    log_output("\n");
      

	bin_2_hex(sign.str, sign.len, chash);
	/*
    n = 0;
	while (n < sign.len)
	{
		unsigned char c=*(unsigned char *)(sign.str+n);
		chash[n * 2 + 0] = hex_chars[c >> 0x04];
		chash[n * 2 + 1] = hex_chars[c & 0x0F];
		n++;
	}
	chash[sign.len*2] = 0; 
	*/
	
	uitoa_s(sign.len,dd,32,10);
	
	log_output("tx sign ");
    log_output(chash);
    log_output(" len ");
    log_output(dd);
    log_output("\n");
    
    if(blk_check_sign(&sign, &vpubK, txsh))
    {
    	log_output("tx sign ok \n");
    }
    else
    {
    	log_output("tx sign fail \n");
    }
    
	free_string(&script);
	free_string(&oscript);
	free_string(&sign);
    

	return 1;
}

OS_API_C_FUNC(int)  dump_txh_infos(const char *hash)
{
	mem_zone_ref	tx = { PTR_NULL };
	hash_t 			blk_hash,tx_hash;
    int 			n=0;
    
  	
  	while (n<32)
	{
		char    hex[3];
		hex[0] = hash[(31-n) * 2 + 0];
		hex[1] = hash[(31-n) * 2 + 1];
		hex[2] = 0;
		tx_hash[n] = strtoul_c(hex, PTR_NULL, 16);
		n++;
	}
  
    if(!load_tx(&tx,blk_hash,tx_hash))
    {
        log_output("unable to load tx ");
        log_output(hash);
        log_output("\n");
        return 0;
    }
    
    dump_tx_infos(&tx);
	release_zone_ref(&tx);
	
	return 1;

}

OS_API_C_FUNC(int) get_tx_output_addr(const hash_t tx_hash, unsigned int idx, btc_addr_t addr)
{
	hash_t			blkhash;
	mem_zone_ref	tx = { PTR_NULL }, vout = { PTR_NULL };
	int				ret;

	if (!load_tx(&tx, blkhash, tx_hash))return 0;
	ret = get_tx_output(&tx, idx, &vout);
	if (ret)
	{
		struct string  script;
		ret = tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &script,0);
		if (ret)
		{
			get_out_script_address(&script, PTR_NULL,addr);
			free_string(&script);
		}
		release_zone_ref(&vout);
	}
	release_zone_ref(&tx);
	return ret;
}


OS_API_C_FUNC(int) is_vout_null(mem_zone_ref_const_ptr tx, unsigned int idx)
{
	uint64_t		amount;
	struct string	script = { PTR_NULL };
	mem_zone_ref vout = { PTR_NULL };
	int			ret;
	if (!get_tx_output(tx, idx, &vout))return 0;

	ret = tree_manager_get_child_value_i64(&vout, NODE_HASH("value"), &amount);
	if ((ret) && (amount > 0))
		ret = 0;

	if (ret)
		ret = tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &script, 0);

	if ((ret) && (script.str[0] != 0))
		ret = 0;

	free_string(&script);
	release_zone_ref(&vout);

	return ret;
}

OS_API_C_FUNC(int) create_null_tx(mem_zone_ref_ptr tx,unsigned int time,unsigned int block_height)
{
	mem_zone_ref	script_node = { PTR_NULL };
	struct string	nullscript = { PTR_NULL };
	struct string	coinbasescript = { PTR_NULL };
	char			null = 0;
	char			script[8];

	
	nullscript.str = &null;
	nullscript.len = 0;
	nullscript.size = 1;

	coinbasescript.str  = script;
	coinbasescript.len  = 4;
	coinbasescript.size = 4;

	script[0]							= 3;
	*((unsigned int *)(script + 1))		= block_height;

	new_transaction (tx, time);
	tx_add_input(tx, null_hash, 0xFFFFFFFF, &coinbasescript);
	tx_add_output	(tx,0, &nullscript);
	return 1;
}

OS_API_C_FUNC(int) is_tx_null(mem_zone_ref_const_ptr tx)
{
	struct string	script = { 0 };
	mem_zone_ref	vout = { PTR_NULL };
	mem_zone_ref	txout_list = { PTR_NULL };
	uint64_t		amount;
	int				ret, nc;


	if (!tree_manager_find_child_node(tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))return -1;
	nc = tree_manager_get_node_num_children(&txout_list);
	if (nc == 0)
	{
		release_zone_ref(&txout_list);
		return -1;
	}
	ret = tree_manager_get_child_at(&txout_list, 0, &vout);
	release_zone_ref(&txout_list);
	if (!ret)return -1;
	ret = tree_manager_get_child_value_i64(&vout, NODE_HASH("value"), &amount);
	if (ret)ret = tree_manager_get_child_value_istr(&vout, NODE_HASH("script"), &script, 0);
	release_zone_ref(&vout);
	if (!ret)return -1;
	if ((nc == 1) && (amount == 0) && (script.str[0] == 0))
		ret = 1;
	else
		ret = 0;

	free_string(&script);
	return ret;
}
OS_API_C_FUNC(int) hash_equal(hash_t hash, const char *shash)
{
	int n = 0;
	while (n < 32)
	{
		char hex[3] = { shash[n * 2], shash[n * 2 + 1], 0 };
		unsigned char uc;
		uc = strtoul_c(hex, PTR_NULL, 16);
		if (hash[31 - n] != uc)
			return 0;

		n++;
	}
	return 1;
}


OS_API_C_FUNC(int) get_hash_list(mem_zone_ref_ptr hdr_list, mem_zone_ref_ptr hash_list)
{
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref_ptr	hdr = PTR_NULL;
	int					n = 0;

	tree_manager_create_node("hash list", NODE_BITCORE_HASH_LIST, hash_list);

	for (n = 0, tree_manager_get_first_child(hdr_list, &my_list, &hdr); ((hdr != NULL) && (hdr->zone != NULL)); n++, tree_manager_get_next_child(&my_list, &hdr))
	{
		hash_t blk_hash;

		tree_manager_get_child_value_hash(hdr, NODE_HASH("blkHash"), blk_hash);
		tree_manager_set_child_value_bhash(hash_list, "hash", blk_hash);
	}
	return n;
}





OS_API_C_FUNC(int) compute_tx_sign_hash(mem_zone_ref_const_ptr tx, unsigned int nIn, const struct string *script, unsigned int hash_type, hash_t txh)
{
	hash_t				tx_hash;
	mem_zone_ref		txin_list = { PTR_NULL }, my_list = { PTR_NULL }, txTmp = { PTR_NULL };
	mem_zone_ref_ptr	input = PTR_NULL;
	size_t				length;
	unsigned char		*buffer;
	unsigned int		iidx;

	tree_manager_node_dup(PTR_NULL, tx, &txTmp,0xFFFFFFFF);
	if (!tree_manager_find_child_node(&txTmp, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))
	{
		log_output("sign hash no txin\n");
		return 0;
	}

	for (iidx = 0, tree_manager_get_first_child(&txin_list, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input), iidx++)
	{
		if (nIn == iidx)
			tree_manager_set_child_value_vstr(input, "script", script);
		else
			tree_manager_set_child_value_str(input, "script", "");
	}

	release_zone_ref(&txin_list);

	length									= get_node_size(&txTmp);
	buffer									= (unsigned char *)malloc_c(length + 4);
	*((unsigned int *)(buffer + length))	= hash_type;

	write_node				(&txTmp, buffer);
	mbedtls_sha256			(buffer, length + 4, tx_hash, 0);
	mbedtls_sha256			(tx_hash, 32, txh, 0);
	free_c					(buffer);
	release_zone_ref		(&txTmp);
	return 1;

}
OS_API_C_FUNC(int) blk_check_sign(const struct string *sign, const struct string *pubk, const hash_t hash)
{
	return check_sign(sign, pubk, hash);
}

OS_API_C_FUNC(int) check_tx_input_sig(mem_zone_ref_const_ptr tx, unsigned int nIn, struct string *vpubK)
{
	hash_t			txsh = { 0x0 };
	struct string	oscript = { PTR_NULL }, script = { PTR_NULL }, sign = { PTR_NULL }, blksign = { PTR_NULL };
	mem_zone_ref	prev_tx = { PTR_NULL };
	mem_zone_ref	out = { PTR_NULL }, in = { PTR_NULL };
	unsigned int	prevOutIdx=0xFFFFFFFF;
	unsigned char	hash_type;
	int				ret = 0;

	if (!load_tx_input(tx, nIn, &in, &prev_tx))
	{
		log_message("could not load tx input '%txid%'", tx);
		return 0;
	}
	if (!tree_manager_get_child_value_i32(&in, NODE_HASH("idx"), &prevOutIdx))
	{
		log_output("invalid tx input");
		return 0;
	}
	ret = get_tx_output(&prev_tx, prevOutIdx, &out);

	if (!ret)log_output("could not load tx output \n");

	release_zone_ref(&prev_tx);
	
	if (ret){
		ret = tree_manager_get_child_value_istr(&in, NODE_HASH("script"), &script, 0);
		if (!ret)log_output("could input script\n");
	}
	if (ret)
	{
		ret = get_insig_info(&script, &sign, vpubK, &hash_type);
		if (!ret)log_output("no sig infos\n");
	}

	release_zone_ref	(&in);
	free_string			(&script);
	

	if (ret)
	{
		ret = tree_manager_get_child_value_istr	(&out, NODE_HASH("script"), &oscript, 0);
		if (!ret)log_output						("no output script\n");
	}
	if (ret)
	{
		ret = compute_tx_sign_hash(tx, nIn, &oscript, hash_type, txsh);
		if (!ret)log_output("compute sign hash failed\n");
	}
	if (ret)
	{
		if (vpubK->len < 31)
		{
			btc_addr_t addr;
			free_string(vpubK);
			ret = get_out_script_address(&oscript, vpubK, addr);
			if (!ret)log_output("output address failed\n");
		}
		if (ret)
		{
			ret = blk_check_sign(&sign, vpubK, txsh);
			if (!ret)
			{
				char th[65] = { 0 };
				char txh[65] = { 0 };

				tree_manager_get_child_value_str(tx, NODE_HASH("txid"), txh, 65, 0);

				bin_2_hex(txsh, 32, th);

				/*
				n = 0;
				while (n < 32)
				{
					th[n * 2 + 0] = hex_chars[txsh[n] >> 0x04];
					th[n * 2 + 1] = hex_chars[txsh[n] & 0x0F];
					n++;
				}
				th[64] = 0;
				*/


				log_output("signature check failed '");
				log_output(th);
				log_output("' '");
				log_output(txh);
				log_output("'\n");
			}
		}
	}

	free_string			(&oscript);
	release_zone_ref	(&out);
	free_string			(&sign);
	release_zone_ref	(&out);

	return ret;
}

OS_API_C_FUNC(int) tx_sign(mem_zone_ref_const_ptr tx, unsigned int nIn, unsigned int hashType, const struct string *sign_seq, const struct string *inPubKey)
{
	hash_t				tx_hash;
	struct				string oscript = { PTR_NULL };
	mem_zone_ref		vin = { PTR_NULL }, vout = { PTR_NULL };
	int					isObj =0,ret = 0;

	get_tx_input(tx, nIn, &vin);

	if (!tree_manager_get_child_value_i32(&vin, NODE_HASH("isAppObj"), &isObj))
		isObj = 0;

	if (isObj)
	{
		btc_addr_t			addr;
		struct string		pubk = { PTR_NULL };
		struct string		sign = { 0 };
		unsigned char		htype;

		get_tx_output						(tx, 0, &vout);
		tree_manager_get_child_value_istr	(&vout, NODE_HASH("script"), &oscript, 0);
		get_out_script_address				(&oscript, &pubk, addr);


		ret = (pubk.len == 33) ? 1 : 0;
		if (ret)ret = compute_tx_sign_hash	(tx, nIn, &oscript, hashType, tx_hash);
		if (ret)ret = parse_sig_seq			(sign_seq, &sign, &htype, 1);
		if (ret)ret = check_sign			(&sign, &pubk, tx_hash);
		if (ret)
		{
			struct string sscript		= { PTR_NULL };
			mem_zone_ref script_node	= { PTR_NULL };

			tree_manager_create_node		 ("script", NODE_BITCORE_SCRIPT, &script_node);
			tree_manager_set_child_value_vstr(&script_node, "var1", sign_seq);
			serialize_script				 (&script_node, &sscript);
			release_zone_ref				 (&script_node);

			tree_manager_set_child_value_vstr(&vin, "script", &sscript);
			free_string						 (&sscript);
		}
		free_string		(&sign);
		free_string		(&pubk);
		release_zone_ref(&vout);
		free_string		(&oscript);
	}
	else if (load_tx_input_vout(tx, nIn, &vout))
	{
		tree_manager_get_child_value_istr	(&vout	, NODE_HASH("script"), &oscript , 0);
		if (compute_tx_sign_hash(tx, nIn, &oscript, hashType, tx_hash))
		{
			btc_addr_t		addr;
			struct string	pubk = { PTR_NULL };
			get_out_script_address(&oscript, &pubk, addr);
			if (pubk.len > 0)
			{
				struct string			sign = { 0 };
				unsigned char			htype;
				if (parse_sig_seq(sign_seq, &sign, &htype, 1))
				{
					ret = check_sign	(&sign, &pubk, tx_hash);
					free_string			(&sign);
					if (ret)
					{
						struct string sscript = { PTR_NULL };
						mem_zone_ref script_node = { PTR_NULL };
						tree_manager_create_node			("script", NODE_BITCORE_SCRIPT, &script_node);
						tree_manager_set_child_value_vstr	(&script_node, "var1", sign_seq);
						serialize_script					(&script_node, &sscript);
						release_zone_ref					(&script_node);
						tree_manager_set_child_value_vstr	(&vin, "script", &sscript);
						free_string							(&sscript);
					}
				}
				free_string(&pubk);
			}
			else if ((inPubKey != PTR_NULL) && (inPubKey->str!=PTR_NULL))
			{
				btc_addr_t				inAddr;
				struct string			sign = { PTR_NULL };
				unsigned char			htype;

				key_to_addr						(inPubKey->str, inAddr);
				ret = (memcmp_c(inAddr, addr, sizeof(btc_addr_t)) == 0) ? 1 : 0;
				if (ret)ret = parse_sig_seq		(sign_seq, &sign, &htype, 1);
				if (ret)ret = check_sign		(&sign, inPubKey, tx_hash);
				if (ret)
				{
					mem_zone_ref script_node = { PTR_NULL };
						
					if(tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node))
					{
						struct string sscript = { PTR_NULL };

						tree_manager_set_child_value_vstr	(&script_node, "var1", sign_seq);
						tree_manager_set_child_value_vstr	(&script_node, "var2", inPubKey);
						serialize_script					(&script_node, &sscript);
						tree_manager_set_child_value_vstr	(&vin, "script", &sscript);
						release_zone_ref					(&script_node);
						free_string							(&sscript);
					}
				}
				free_string						(&sign);
			}
			else
				ret=0;

			free_string(&oscript);
		}
		release_zone_ref(&vout);
	}

	release_zone_ref(&vin);
	return ret;
}

OS_API_C_FUNC(int) get_type_infos(struct string *script, char *name, unsigned int *id, unsigned int *flags)
{
	struct string ktype = { PTR_NULL }, kname = { PTR_NULL }, kflags = { PTR_NULL };
	size_t offset = 0;
	int ret = 0;
	
	kname = get_next_script_var(script, &offset);
	if ((kname.len < 3) || (kname.len>32))
	{
		free_string(&kname);
		return 0;
	}
	strcpy_cs(name, 32, kname.str);

	ktype = get_next_script_var(script, &offset);
	if (ktype.len == 4)
	{
		*id = *((unsigned int *)(ktype.str));
		ret = 1;
	}
	else if (ktype.len == 2)
	{
		*id = *((unsigned short *)(ktype.str));
		ret = 1;
	}
	else if (ktype.len == 1)
	{
		*id = *((unsigned char *)(ktype.str));
		ret = 1;
	}

	kflags = get_next_script_var(script, &offset);

	if (kflags.len == 0)
		*flags = 0;
	else if (kflags.len == 1)
		*flags = *((unsigned char *)(kflags.str));

	free_string(&kflags);
	free_string(&kname);
	free_string(&ktype);

	return ret;
}

OS_API_C_FUNC(int) get_app_types(mem_zone_ref_ptr app, mem_zone_ref_ptr types)
{
	mem_zone_ref	 txout_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr out = PTR_NULL;
	int				 ret = 0;

	if (!tree_manager_find_child_node(app, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))return 0;
	
	for (tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); tree_manager_get_next_child(&my_list, &out))
	{
		unsigned int app_item;
		if (!tree_manager_get_child_value_i32(out, NODE_HASH("app_item"), &app_item))continue;
		if (app_item == 1)
		{
			mem_zone_ref app_types = { PTR_NULL };

			tree_manager_find_child_node(out, NODE_HASH("types"), NODE_BITCORE_TX_LIST, types);
			dec_zone_ref				(out);
			release_zone_ref			(&my_list);
			release_zone_ref			(&txout_list);
			ret = 1;
			break;
		}
	}
	release_zone_ref(&txout_list);
	return ret;
}

OS_API_C_FUNC(int) get_app_scripts(mem_zone_ref_ptr app, mem_zone_ref_ptr scripts)
{
	mem_zone_ref	 txout_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr out = PTR_NULL;
	int				 ret = 0;

	if (!tree_manager_find_child_node(app, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))return 0;

	for (tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); tree_manager_get_next_child(&my_list, &out))
	{
		unsigned int app_item;
		if (!tree_manager_get_child_value_i32(out, NODE_HASH("app_item"), &app_item))continue;
		if (app_item == 5)
		{
			ret=tree_manager_find_child_node(out, NODE_HASH("scripts"), NODE_SCRIPT_LIST, scripts);
			dec_zone_ref				(out);
			release_zone_ref			(&my_list);
			break;
		}
	}
	release_zone_ref(&txout_list);
	return ret;
}
int add_app_tx_type(mem_zone_ref_ptr app, mem_zone_ref_ptr typetx)
{
	mem_zone_ref txout_list = { PTR_NULL }, my_list = { PTR_NULL }, type_def = { PTR_NULL };
	struct string typeStr = { 0 }, typeName = { 0 }, typeId = { 0 }, Flags = { 0 };
	mem_zone_ref_ptr out = PTR_NULL;
	size_t offset = 0;
	unsigned int type_id;
	int ret = 0;

	if (!get_tx_output(typetx, 0, &type_def))return 0;

	if (!tree_manager_find_child_node(app, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list)){
		release_zone_ref(&type_def);
		return 0;
	}

	tree_manager_get_child_value_istr(&type_def, NODE_HASH("script"), &typeStr, 0);

	typeName = get_next_script_var(&typeStr, &offset);
	typeId	 = get_next_script_var(&typeStr, &offset);
	

	free_string	(&typeStr);

	if (typeId.len != 4)
	{
		free_string		(&typeName);
		free_string		(&typeId);
		release_zone_ref(&type_def);
		release_zone_ref(&txout_list);
		return 0;
	}

	
	type_id = *((unsigned int *)(typeId.str));
	
	tree_manager_set_child_value_vstr(typetx, "typeName", &typeName);
	tree_manager_set_child_value_i32 (typetx , "typeId", type_id);



	for (tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); tree_manager_get_next_child(&my_list, &out))
	{
		unsigned int app_item;
		if (!tree_manager_get_child_value_i32(out, NODE_HASH("app_item"), &app_item))continue;
		if (app_item == 1)
		{
			mem_zone_ref types = { PTR_NULL };

			if (!tree_manager_find_child_node(out, NODE_HASH("types"), NODE_BITCORE_TX_LIST, &types))
				tree_manager_add_child_node(out, "types", NODE_BITCORE_TX_LIST, &types);

			tree_manager_node_add_child	(&types, typetx);

			release_zone_ref			(&types);

			dec_zone_ref				(out);
			release_zone_ref			(&my_list);
			ret = 1;
			break;
		}
	}

	free_string		(&Flags);
	free_string		(&typeName);
	free_string		(&typeId);
	release_zone_ref(&txout_list);
	release_zone_ref(&type_def);
	return ret;
}

int add_app_script(mem_zone_ref_ptr app, mem_zone_ref_ptr script_vars)
{
	mem_zone_ref txout_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr out = PTR_NULL;

	int ret = 0;

	if (!tree_manager_find_child_node(app, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))return 0;
	

	for (tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); tree_manager_get_next_child(&my_list, &out))
	{
		unsigned int app_item;
		if (!tree_manager_get_child_value_i32(out, NODE_HASH("app_item"), &app_item))continue;
		if (app_item == 5)
		{
			mem_zone_ref scripts = { PTR_NULL };

			if (!tree_manager_find_child_node(out, NODE_HASH("scripts"), NODE_SCRIPT_LIST, &scripts))
				tree_manager_add_child_node(out, "scripts", NODE_SCRIPT_LIST, &scripts);

			tree_manager_node_add_child(&scripts, script_vars);

			release_zone_ref(&scripts);

			dec_zone_ref(out);
			release_zone_ref(&my_list);
			ret = 1;
			break;
		}
	}
	release_zone_ref(&txout_list);

	return ret;
}


OS_API_C_FUNC(int) is_app_root(mem_zone_ref_ptr tx)
{
	mem_zone_ref		txin_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr	input = PTR_NULL;
	unsigned int		iidx,app_root;


	if (!tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))
		return 0;

	app_root=0;

	for (iidx = 0, tree_manager_get_first_child(&txin_list, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input), iidx++)
	{
		hash_t				prev_hash;
		unsigned int		oidx = 0;
		int					n = 0;

		memset_c(prev_hash, 0, sizeof(hash_t));

		tree_manager_get_child_value_hash	(input, NODE_HASH("txid"), prev_hash);
		tree_manager_get_child_value_i32	(input, NODE_HASH("idx") , &oidx);

		if ((!memcmp_c(prev_hash, null_hash, 32)) && (oidx >= 0xFFFF))
		{
			struct string script={0},var={0};
			size_t offset=0;

			tree_manager_get_child_value_istr	(input, NODE_HASH("script"), &script,16);
			var = get_next_script_var			(&script,&offset);
			free_string							(&script);

			if(var.len>0)
			{
				if(!strcmp_c(var.str,"AppRoot"))
					app_root=1;
			}
			free_string(&var);
		}
	}

	release_zone_ref(&txin_list);

	return app_root;
}



int tx_is_app_item(const hash_t txh,unsigned int oidx,mem_zone_ref_ptr app_tx,unsigned char *val)
{
	struct string	oscript = { 0 }, my_val = { PTR_NULL };
	mem_zone_ref	prevout = { PTR_NULL };
	int				ret = 0;

	if (!tree_find_child_node_by_member_name_hash(&apps, NODE_BITCORE_TX, "txid", txh, app_tx))return 0;
	
	get_tx_output						(app_tx, oidx, &prevout);
	if (!tree_manager_get_child_value_istr(&prevout, NODE_HASH("script"), &oscript, 0))
	{
		release_zone_ref(app_tx);
		return 0;
	}

	if (get_out_script_return_val(&oscript, &my_val))
	{
		if ((my_val.len == 1) && (*((unsigned char*)(my_val.str)) > 0) && (*((unsigned char*)(my_val.str)) < 6))
		{
			*val = *((unsigned char*)(my_val.str));
			ret = 1;
		}

		free_string(&my_val);
	}
	free_string(&oscript);
	release_zone_ref(&prevout);

	if (!ret)
		release_zone_ref(app_tx);

	return ret;
}

int tx_is_app_child(hash_t txh, unsigned int oidx,struct string *appname)
{
	hash_t bh;
	mem_zone_ref tx = { PTR_NULL }, vin = { PTR_NULL }, app_tx = { PTR_NULL };
	int ret = 0;

	if (oidx > 0)return 0;
	if (!load_tx(&tx, bh, txh))return 0;

	if (get_tx_input(&tx, 0, &vin))
	{
		hash_t	prev_hash;
		unsigned int oidx;
		unsigned char app_item;
		tree_manager_get_child_value_hash	(&vin, NODE_HASH("txid"), prev_hash);
		tree_manager_get_child_value_i32	(&vin, NODE_HASH("idx"), &oidx);

		if (tx_is_app_item(prev_hash, oidx, &app_tx, &app_item))
		{
			tree_manager_get_child_value_istr(&app_tx, NODE_HASH("appName"), appname,0);
			ret = 1;
			release_zone_ref(&app_tx);
		}
		release_zone_ref(&vin);
	}

	release_zone_ref(&tx);

	return ret;
}

OS_API_C_FUNC(int) tx_is_app_file(mem_zone_ref_ptr tx, struct string *appName,mem_zone_ref_ptr file)
{
	hash_t			txh;
	struct string	oscript = { 0 }, my_val = { PTR_NULL };
	mem_zone_ref	input = { PTR_NULL }, prevout = { PTR_NULL }, app_tx = { PTR_NULL };
	unsigned int	oidx;
	int				ret = 0;

	if (!get_tx_input(tx, 0, &input))return 0;
	tree_manager_get_child_value_hash(&input, NODE_HASH("txid"), txh);
	tree_manager_get_child_value_i32(&input, NODE_HASH("idx"), &oidx);
	release_zone_ref(&input);

	if (!tree_find_child_node_by_member_name_hash(&apps, NODE_BITCORE_TX, "txid", txh, &app_tx))return 0;

	get_tx_output						(&app_tx, oidx, &prevout);
	

	tree_manager_get_child_value_istr	(&prevout, NODE_HASH("script"), &oscript, 0);
	release_zone_ref			(&prevout);
	if (get_out_script_return_val(&oscript, &my_val))
	{
		if ((my_val.len == 1) && ( (*((unsigned char*)(my_val.str))) ==3))
		{
			struct string fscript = { 0 };

			get_tx_output						(tx, 0, &prevout);
			tree_manager_get_child_value_istr	(&prevout, NODE_HASH("script"), &fscript, 0);
			ret	=	get_script_file				(&fscript, file);

			if (ret)tree_manager_get_child_value_istr(&app_tx, NODE_HASH("appName"), appName, 0);

			release_zone_ref					(&prevout);
			free_string							(&fscript);
		}
		free_string(&my_val);
	}
	free_string(&oscript);
	
	release_zone_ref(&app_tx);

	return ret;
}

OS_API_C_FUNC(int) get_tx_file(mem_zone_ref_ptr tx,mem_zone_ref_ptr hash_list)
{
	hash_t			tx_hash;
	mem_zone_ref	new_file = { PTR_NULL };

	if (!tree_manager_find_child_node(tx, NODE_HASH("fileDef"), NODE_GFX_OBJECT, PTR_NULL))
	{ 
		return 0; 
	
	}
	tree_manager_get_child_value_hash(tx, NODE_HASH("txid"), tx_hash);

	if (tree_manager_add_child_node(hash_list, "file", NODE_FILE_HASH, &new_file))
	{
		tree_manager_write_node_hash(&new_file, 0, tx_hash);
		release_zone_ref(&new_file);
	}

	return 1;
}

int obj_new(mem_zone_ref_ptr type, const char *objName, struct string *script, mem_zone_ref_ptr obj)
{
	mem_zone_ref		type_outs = { PTR_NULL }, my_list = { PTR_NULL };
	struct string		objData = { 0 };
	mem_zone_ref_ptr	key = PTR_NULL;
	unsigned int		type_id, oidx;

	tree_manager_get_child_value_i32(type, NODE_HASH("typeId"), &type_id);

	tree_manager_find_child_node(type, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &type_outs);
	tree_manager_create_node(objName, type_id, obj);

	for (oidx = 0, tree_manager_get_first_child(&type_outs, &my_list, &key); ((key != NULL) && (key->zone != NULL)); oidx++, tree_manager_get_next_child(&my_list, &key))
	{
		char			KeyName[32];
		struct string	KeyStr = { 0 };
		unsigned int	KeyId, flags;
		uint64_t		amount;

		if (oidx == 0)continue;
		tree_manager_get_child_value_i64(key, NODE_HASH("value"), &amount);
		if (amount != 0)continue;

		tree_manager_get_child_value_istr(key, NODE_HASH("script"), &KeyStr, 0);

		if (get_type_infos(&KeyStr, KeyName, &KeyId, &flags))
		{
			if (KeyId == NODE_GFX_STR)
				KeyId = NODE_BITCORE_VSTR;
			tree_manager_add_child_node(obj, KeyName, KeyId, PTR_NULL);
		}
		free_string(&KeyStr);
	}
	release_zone_ref(&type_outs);

	if (get_out_script_return_val(script, &objData))
	{
		unserialize_children(obj, objData.str, objData.len);
		free_string(&objData);
		return 1;
	}

	return 0;
}


int is_obj_child(const hash_t ph,unsigned int pidx,mem_zone_ref_ptr prev_tx,struct string *appName)
{
	hash_t				pBlock, pid;
	mem_zone_ref		prevout = { PTR_NULL }, prev_input = { PTR_NULL }, app = { PTR_NULL };
	struct string		oscript = { 0 };
	int					ret=0;


	if (!tree_find_child_node_by_member_name_hash(&blkobjs, NODE_BITCORE_TX, "txid", ph, prev_tx))
	{
		if (!load_tx(prev_tx, pBlock, ph))return -1;
	}

	get_tx_input(prev_tx, 0, &prev_input);

	tree_manager_get_child_value_hash(&prev_input, NODE_HASH("txid"), pid);

	if ((pidx == 0) && tree_find_child_node_by_member_name_hash(&apps, NODE_BITCORE_TX, "txid", pid, &app))
	{
		mem_zone_ref		app_out = { PTR_NULL };
		unsigned int		pidx;

		tree_manager_get_child_value_i32(&prev_input, NODE_HASH("idx"), &pidx);

		if (get_tx_output(&app, pidx, &app_out))
		{
			struct string app_script = { PTR_NULL }, val = { PTR_NULL };

			tree_manager_get_child_value_istr(&app_out, NODE_HASH("script"), &app_script, 0);

			if (get_out_script_return_val(&app_script, &val))
			{
				if ((val.len == 1) && (*((unsigned char *)(val.str)) == 2))
				{
					tree_manager_get_child_value_istr(&app, NODE_HASH("appName"), appName, 0);
					ret = 1;
				}
				free_string(&val);
			}
			free_string		(&app_script);
			release_zone_ref(&app_out);
		}
		release_zone_ref(&app);
	}
	release_zone_ref(&prev_input);

	

	return ret;
}

OS_API_C_FUNC(int) get_app_type_idxs(const char *appName, unsigned int type_id, mem_zone_ref_ptr keys)
{
	mem_zone_ref app = { PTR_NULL }, types = { PTR_NULL }, type = { PTR_NULL }, type_outs = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr key;
	int ret = 0;
	
	if (!tree_manager_find_child_node(&apps, NODE_HASH(appName), NODE_BITCORE_TX, &app))return 0;

	get_app_types(&app, &types);

	if (tree_find_child_node_by_id_name(&types, NODE_BITCORE_TX, "typeId", type_id, &type))
	{
		unsigned int oidx;
		tree_manager_find_child_node(&type, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &type_outs);

		for (oidx = 0, tree_manager_get_first_child(&type_outs, &my_list, &key); ((key != NULL) && (key->zone != NULL)); oidx++, tree_manager_get_next_child(&my_list, &key))
		{
			char			KeyName[32];
			struct string	KeyStr = { 0 };
			unsigned int	KeyId, flags;
			uint64_t		amount;

			if (oidx == 0)continue;
			tree_manager_get_child_value_i64(key, NODE_HASH("value"), &amount);
			if (amount != 0)continue;
			tree_manager_get_child_value_istr(key, NODE_HASH("script"), &KeyStr, 0);

			if (get_type_infos(&KeyStr, KeyName, &KeyId, &flags))
			{
				if ((flags & 1) || (flags & 2))
				{
					mem_zone_ref nk = { 0 };
					tree_manager_add_child_node		(keys, KeyName, KeyId, &nk);
					tree_manager_write_node_dword	(&nk, 0, flags);
					release_zone_ref				(&nk);
				}
			}
			free_string(&KeyStr);
		}
		release_zone_ref(&type_outs);
	}

	release_zone_ref(&type);
	release_zone_ref(&types);
	release_zone_ref(&app);
	return ret;
}

OS_API_C_FUNC(int) check_app_obj_unique(const char *appName, unsigned int type_id, mem_zone_ref_ptr obj)
{
	mem_zone_ref app = { PTR_NULL }, types = { PTR_NULL }, type = { PTR_NULL }, type_outs = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr key;
	unsigned int unique;
	int ret = 0;

	if (!tree_manager_find_child_node(&apps, NODE_HASH(appName), NODE_BITCORE_TX, &app))return 0;

	unique = 1;

	get_app_types(&app, &types);

	if (tree_find_child_node_by_id_name(&types, NODE_BITCORE_TX, "typeId", type_id, &type))
	{
		unsigned int oidx;
		tree_manager_find_child_node(&type, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &type_outs);

		for (oidx = 0, tree_manager_get_first_child(&type_outs, &my_list, &key); ((key != NULL) && (key->zone != NULL)); oidx++, tree_manager_get_next_child(&my_list, &key))
		{
			char			KeyName[32];
			struct string	KeyStr = { 0 };
			unsigned int	KeyId, flags;
			uint64_t		amount;

			if (oidx == 0)continue;
			tree_manager_get_child_value_i64(key, NODE_HASH("value"), &amount);
			if (amount != 0)continue;
			tree_manager_get_child_value_istr(key, NODE_HASH("script"), &KeyStr, 0);

			if (get_type_infos(&KeyStr, KeyName, &KeyId, &flags))
			{
				if (flags & 1)
				{
					char typestr[16];

					uitoa_s			(type_id, typestr, 16, 16);

					switch (KeyId)
					{
						case NODE_BITCORE_VSTR:
						{
							struct string	val = { 0 };
							hash_t			h;
							tree_manager_get_child_value_istr(obj, NODE_HASH(KeyName), &val, 0);

							if (find_index_str(appName, typestr, KeyName, &val, h))
								unique = 0;
						}
						break;
					}
				}
			}
			free_string(&KeyStr);

			if (!unique)
			{
				dec_zone_ref(key);
				release_zone_ref(&my_list);
				break;
			}
		}
		release_zone_ref(&type_outs);
	}

	release_zone_ref(&type);
	release_zone_ref(&types);
	release_zone_ref(&app);
	return unique;
}



OS_API_C_FUNC(int) get_app_type_key(const char *appName, unsigned int type_id, const char *kname, unsigned int *ktype, unsigned int *Flags)
{
	mem_zone_ref app = { PTR_NULL }, types = { PTR_NULL }, type = { PTR_NULL }, type_outs = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr key;
	int ret=0;


	if (!tree_manager_find_child_node(&apps, NODE_HASH(appName), NODE_BITCORE_TX, &app))return 0;

	get_app_types					(&app, &types);

	if (tree_find_child_node_by_id_name(&types, NODE_BITCORE_TX, "typeId", type_id, &type))
	{
		unsigned int oidx;
		tree_manager_find_child_node(&type, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &type_outs);

		for (oidx = 0, tree_manager_get_first_child(&type_outs, &my_list, &key); ((key != NULL) && (key->zone != NULL)); oidx ++, tree_manager_get_next_child(&my_list, &key))
		{
			char			KeyName[32];
			struct string	KeyStr = { 0 };
			unsigned int	KeyId, flags;
			uint64_t		amount;

			if (oidx == 0)continue;
			tree_manager_get_child_value_i64(key, NODE_HASH("value"), &amount);
			if (amount != 0)continue;
			tree_manager_get_child_value_istr(key, NODE_HASH("script"), &KeyStr, 0);

			if (get_type_infos(&KeyStr, KeyName, &KeyId, &flags))
			{
				if (!strcmp_c(KeyName, kname))
				{
					*ktype = KeyId;
					*Flags = flags;

					free_string		(&KeyStr);
					dec_zone_ref	(key);
					release_zone_ref(&my_list);
					ret = 1;
					break;
				}
			}
			free_string(&KeyStr);
		}
		release_zone_ref(&type_outs);
	}

	release_zone_ref(&type);
	release_zone_ref(&types);
	release_zone_ref(&app);

	return ret;
}

OS_API_C_FUNC(int) get_block_tree(node **blktree)
{
	*blktree = blk_root;

	return 1;
}

OS_API_C_FUNC(int) check_tx_inputs(mem_zone_ref_ptr tx, uint64_t *total_in, unsigned int *is_coinbase,unsigned int check_sig)
{
	mem_zone_ref		txin_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr	input = PTR_NULL;
	unsigned int		iidx, has_app, is_app_item;
	int ret;

	if (!tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))
		return 0;

	is_app_item = 0;
	has_app		= 0;
	*total_in	= 0;

	for (iidx = 0, tree_manager_get_first_child(&txin_list, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input), iidx++)
	{
		tree_entry			et;
		btc_addr_t			addr;
		hash_t				pBlock;
		mem_zone_ref		prev_tx = { PTR_NULL };
		struct string		appName = { 0 };
		uint64_t			amount = 0;
		unsigned char		app_item;
		int					n= 0;

		memset_c(et			, 0, sizeof(tree_entry));
		memset_c(pBlock		, 0, sizeof(hash_t));

		if (!tree_manager_get_child_value_hash(input, NODE_HASH("txid"), (unsigned char *)et))
		{
			log_output("chk tx bad txin txid\n");
			release_zone_ref(&my_list);
			dec_zone_ref(input);
			release_zone_ref(&txin_list);
			return 0;
		}
		if (!tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &et[8]))
		{
			log_output("chk tx bad txin idx\n");
			release_zone_ref(&my_list);
			dec_zone_ref(input);
			release_zone_ref(&txin_list);
			return 0;

		}

		ret = 0;


		if ((!memcmp_c(et, null_hash, 32)) && (et[8] >= 0xFFFF))
		{
			if ((*is_coinbase) == 0)
			{
				*is_coinbase = 1;
				continue;
			}
			release_zone_ref	(&my_list);
			dec_zone_ref		(input);
			release_zone_ref	(&txin_list);
			return 0;
		}
		else if ((has_root_app == 1) && (!memcmp_c(et, app_root_hash, sizeof(hash_t))))
		{
			struct string script = { 0 };

			ret = (has_app == 0) ? 1 : 0;
			if (ret)ret = tree_manager_get_child_value_istr(input, NODE_HASH("script"), &script, 0);
			if (ret)ret = get_app_name(&script, &appName);
			if (ret)ret = (appName.len >= 3) ? 1 : 0;
			if (ret)ret = (appName.len < 32) ? 1 : 0;
			if (ret)ret = tree_manager_set_child_value_str(tx, "AppName", appName.str);
			if (ret)ret = tree_manager_set_child_value_bool(input, "isApp", 1);
			if (ret)ret = tree_manager_set_child_value_str(input, "appName", appName.str);
			free_string(&appName);
			free_string(&script);
			if(ret)has_app = 1;
		}
		else if (tx_is_app_item((unsigned char *)et, et[8], &prev_tx, &app_item))
		{
			unsigned char	hash_type;
			
			if (is_app_item)
			{
				log_output("app item already found \n");
				ret = 0;
			}
			else
			{
			
				struct string	oscript = { 0 };
				mem_zone_ref	prevout = { PTR_NULL };

				tree_manager_get_child_value_istr	(&prev_tx, NODE_HASH("appName"), &appName,0);
				log_output							("new app item for '");
				log_output							(appName.str);
				log_output							("'\n");

				ret = tree_manager_find_child_node(&apps, NODE_HASH(appName.str), NODE_BITCORE_TX, PTR_NULL);

				if (ret)
				{
					get_tx_output					 (&prev_tx, et[8], &prevout);
					tree_manager_get_child_value_istr(&prevout, NODE_HASH("script"), &oscript, 0);

					switch (app_item)
					{
						case 1:
						case 4:
						case 5:
						{
							hash_t			txh;
							struct string	script = { PTR_NULL }, sign = { PTR_NULL }, vpubK = { PTR_NULL };

							tree_manager_get_child_value_istr	(input, NODE_HASH("script"), &script, 0);
							ret = get_insig_info				(&script, &sign, &vpubK, &hash_type);
							free_string							(&script);

							if (ret)
							{
								if (vpubK.len == 0)
								{
									ret = get_out_script_address(&oscript, &vpubK, addr);
									if (!ret)log_output("unable to parse input addr \n");
								}
								else
								{
									ret = check_txout_key(&prevout, (unsigned char *)vpubK.str, addr);
									if (!ret)log_output("check input pkey hash failed\n");
								}
							}

							if (ret)ret = compute_tx_sign_hash(tx, iidx, &oscript, hash_type, txh);
							if (ret)ret = check_sign(&sign, &vpubK, txh);

							free_string(&sign);
							free_string(&vpubK);

							if (ret)ret = tree_manager_set_child_value_i32(tx, "app_item", app_item);
							if (ret)is_app_item = 1;
							if (ret)
							{
								if (app_item == 1)
									tree_manager_set_child_value_vstr(tx, "appType", &appName);

								if (app_item == 4)
									tree_manager_set_child_value_vstr(tx, "appLayout", &appName);

								if (app_item == 5)
									tree_manager_set_child_value_vstr(tx, "appModule", &appName);
							}
					
						}
						break;
						case 2:
						{
							struct string script = { 0 }, pkey = { 0 };
							struct string sign = { 0 }, bsign = { 0 };

							tree_manager_get_child_value_istr(input, NODE_HASH("script"), &script, 0);
							ret = get_insig_info(&script, &sign, &pkey, &hash_type);
							if(ret) ret = (pkey.len == 0) ? 1 : 0;
							if (ret)ret = tree_manager_set_child_value_vstr(tx, "appObj", &appName);
							if (ret)ret = tree_manager_set_child_value_vstr(tx, "ObjSign", &sign);
							if (ret)ret = tree_manager_set_child_value_i32(tx, "app_item", app_item);
							if (ret)is_app_item = 1;
							if (ret)tree_manager_node_add_child(&blkobjs, tx);

							free_string(&script);
							free_string(&sign);
							free_string(&pkey);

						}
						break;
						case 3:
							tree_manager_set_child_value_vstr(tx, "appFile", &appName);
							ret = tree_manager_set_child_value_i32(tx, "app_item", app_item);
							if (ret)is_app_item = 1;
						break;
					}
					free_string(&oscript);
					release_zone_ref(&prevout);
				}
				if(ret)tree_manager_set_child_value_vstr(input, "srcapp", &appName);

				free_string(&appName);
			}
			release_zone_ref(&prev_tx);
			if (!ret)
			{
				log_output("new app failed \n");
				release_zone_ref(&my_list);
				dec_zone_ref(input);
				release_zone_ref(&txin_list);
			}
		}
		else if (is_obj_child((unsigned char *)et, et[8], &prev_tx, &appName) == 1)
		{
			mem_zone_ref		prevout = { PTR_NULL };
			struct string		script = { PTR_NULL }, sign = { PTR_NULL }, vpubK = { PTR_NULL };
			struct string		oscript = { 0 };
			int					is_signed;
			unsigned char		hash_type;

			ret=get_tx_output					(&prev_tx, et[8], &prevout);
		
			tree_manager_get_child_value_istr	(&prevout, NODE_HASH("script"), &oscript, 0);
			tree_manager_get_child_value_istr	(input, NODE_HASH("script"), &script, 0);
			
			if (get_insig_info(&script, &sign, &vpubK, &hash_type))
			{
				if (vpubK.len == 0)
				{
					free_string(&vpubK);
					is_signed = get_out_script_address(&oscript, &vpubK, addr);

					if (!is_signed)log_output("unable to parse input addr \n");
				}
				else
				{
					is_signed = check_txout_key(&prevout, (unsigned char *)vpubK.str, addr);
					if (!is_signed)log_output("check input pkey hash failed\n");
				}

				if (is_signed)
				{
					hash_t txh;

					is_signed = compute_tx_sign_hash(tx, iidx, &oscript, hash_type, txh);
					if (is_signed)is_signed = check_sign(&sign, &vpubK, txh);
				}
				tree_manager_set_child_value_btcaddr(input, "srcaddr", addr);
			}
			else
			{
				is_signed = 0;

				if (get_out_script_address(&oscript, PTR_NULL, addr))
					tree_manager_set_child_value_btcaddr(input, "srcaddr", addr);
			}

			tree_manager_set_child_value_bool(tx   , "pObjSigned", is_signed);
			tree_manager_set_child_value_i64 (input, "value", amount);

						
			free_string(&vpubK);
			free_string(&sign);
			free_string(&script);
			free_string(&oscript);

			if (ret)
			{
				unsigned int		type_id;
				uint64_t			val;

				tree_manager_get_child_value_i64(&prevout, NODE_HASH("value"), &val);
				type_id = val & 0xFFFFFFFF;


				tree_manager_set_child_value_i32	(tx, "pObjType"		, type_id);
				tree_manager_set_child_value_vstr	(tx, "appChild"		, &appName);
				tree_manager_set_child_value_hash	(tx, "appChildOf"	, (unsigned char *)et);
				tree_manager_set_child_value_vstr	(input, "srcapp"	, &appName);
			}
					
			release_zone_ref					(&prevout);
			release_zone_ref					(&prev_tx);
			free_string							(&appName);
		}
		else if (prev_tx.zone != PTR_NULL)
		{
			char			phash[65];
			mem_zone_ref	prevout = { PTR_NULL }, txouts = { PTR_NULL };
			unsigned char	*pet = (unsigned char *)et;


			bin_2_hex(pet, 32, phash);
			
			/*
			n = 0;
			while (n < 32)
			{
				phash[n * 2 + 0] = hex_chars[pet[n] >> 0x04];
				phash[n * 2 + 1] = hex_chars[pet[n] & 0x0F];
				n++;
			}
			phash[64]	= 0;
			*/

			ret			= check_utxo(phash, et[8]);

			if (!ret)
			{
				char	rphash[65];
				char	iStr[16];

				bin_2_hex_r(pet, 32, rphash);

				/*
				n = 0;
				while (n < 32)
				{
					rphash[n * 2 + 0] = hex_chars[pet[31 - n] >> 0x04];
					rphash[n * 2 + 1] = hex_chars[pet[31 - n] & 0x0F];
					n++;
				}
				rphash[64] = 0;
				*/
				
				uitoa_s		(et[8], iStr, 16, 10);

				log_output	("bad utxo '");
				log_output	(rphash);
				log_output	("' - ");
				log_output	(iStr);
				log_output	("\n");

				/*
				if(tree_manager_find_child_node(&prev_tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txouts))
				{
					store_tx_vout	(phash, &txouts, et[8], addr);
					release_zone_ref(&txouts);
				}
				ret = check_utxo(phash, et[8]);
				*/
			}
		
			if ((ret)&&(get_tx_output(&prev_tx, et[8], &prevout)))
			{
				struct string	oscript = { PTR_NULL }, script = { PTR_NULL }, sign = { PTR_NULL }, sigseq = { PTR_NULL }, vpubK = { PTR_NULL };
				mem_zone_ref	txTmp = { PTR_NULL };
				size_t			offset = 0;
				unsigned char	hash_type;

				tree_manager_get_child_value_i64(&prevout, NODE_HASH("value"), &amount);
				
				if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
					amount = 0;


				tree_manager_get_child_value_istr	(&prevout, NODE_HASH("script"), &oscript, 0);
				tree_manager_get_child_value_istr	(input, NODE_HASH("script"), &script, 0);
				ret = get_insig_info				(&script, &sign, &vpubK, &hash_type);
				if (ret)
				{
					btc_addr_t addr;

					if (vpubK.len == 0)
					{
						free_string						(&vpubK);
						ret = get_out_script_address	(&oscript, &vpubK, addr);

						if (!ret)log_output("unable to parse input addr \n");
					}
					else
					{
						ret = check_txout_key(&prevout, (unsigned char *)vpubK.str, addr);
						if (!ret)log_output("check input pkey hash failed\n");
					}
					
					if (ret)
					{
						if (check_sig & 1)
						{
							hash_t txh;

							ret = compute_tx_sign_hash(tx, iidx, &oscript, hash_type, txh);
							if (ret)ret = check_sign(&sign, &vpubK, txh);
						}
					}

					if (ret)tree_manager_set_child_value_btcaddr(input, "srcaddr", addr);
					if (ret)tree_manager_set_child_value_i64	(input, "value", amount);
					
					free_string	(&vpubK);
					free_string	(&sign);
				}

				free_string		(&script);
				free_string		(&oscript);
				release_zone_ref(&prevout);

				if ((ret) && (check_sig & 2))
				{
					if (!bt_insert(&blk_root, et))
					{
						release_zone_ref(&prev_tx);
						release_zone_ref(&my_list);
						dec_zone_ref(input);
						release_zone_ref(&txin_list);
						log_output("double spent found \n");
						return 0;
					}
				}
			}
			release_zone_ref(&prev_tx);
		}

		if (!ret)
		{
			char prevTx[65];
			char iStr[16];
			unsigned char	*pet = (unsigned char *)et;


			bin_2_hex_r(pet, 32, prevTx);

			/*
			n = 0;
			while (n < 32)
			{
				prevTx[n * 2 + 0] = hex_chars[pet[31 - n] >> 0x04];
				prevTx[n * 2 + 1] = hex_chars[pet[31 - n] & 0x0F];
				n++;
			}
			prevTx[64] = 0;
			*/

			itoa_s			(iidx, iStr, 16, 10);
			log_output		("check input failed at input #");
			log_output		(iStr);
			log_output		(" tx :'");
			log_output		(prevTx);
			log_output		("'\n");

			release_zone_ref	(&my_list);
			dec_zone_ref		(input);
			release_zone_ref	(&txin_list);
			return 0;
		}
		(*total_in) += amount;
	}
	release_zone_ref(&txin_list);
	return 1;
}




OS_API_C_FUNC(int) check_tx_outputs(mem_zone_ref_ptr tx, uint64_t *total, unsigned int *is_staking)
{
	mem_zone_ref		txout_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr	out = PTR_NULL;
	unsigned int		idx, app_flags, ret;

	*is_staking = 0;
	app_flags = 0;
	if (!tree_manager_find_child_node(tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))
	{
		log_output("no utxo\n");
		return 0;
	}
	for (idx = 0, tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); idx++, tree_manager_get_next_child(&my_list, &out))
	{
		hash_t			pObjH;
		struct			string script = { 0 };
		uint64_t		amount = 0;
		unsigned int	app_item;

		ret = 1;

		if (!tree_manager_get_child_value_i64(out, NODE_HASH("value"), &amount))continue;
		if (!tree_manager_get_child_value_istr(out, NODE_HASH("script"), &script, 16))continue;

		if ((idx == 0) && (amount == 0) && (script.str[0] == 0))
		{
			*is_staking = 1;
		}
		else
		{
			btc_addr_t		addr;
			struct string	pubk = { PTR_NULL };

			if (get_out_script_address(&script, &pubk, addr))
			{
				tree_manager_set_child_value_btcaddr(out, "dstaddr", addr);
				free_string(&pubk);
			}

			if (tree_manager_find_child_node(tx, NODE_HASH("AppName"), NODE_GFX_STR, PTR_NULL))
			{
				struct string my_val = { PTR_NULL };
				if (get_out_script_return_val(&script, &my_val))
				{
					ret = (my_val.len == 1) ? 1 : 0;
					if (ret)
					{
						unsigned char app_code = *((unsigned char*)(my_val.str));
						switch (app_code)
						{
						case 1:app_flags |= 1; break;
						case 2:app_flags |= 2; break;
						case 3:app_flags |= 4; break;
						case 4:app_flags |= 8; break;
						case 5:app_flags |= 16; break;
						default:ret = 0; break;
						}
					}
					free_string(&my_val);
				}
				else if (!memcmp_c(addr, root_app_addr, sizeof(btc_addr_t)))
				{
					uint64_t root_amnt;
					if (!tree_manager_get_child_value_i64(tx, NODE_HASH("app_root_amnt"), &root_amnt))root_amnt = 0;
					root_amnt += amount;
					tree_manager_set_child_value_i64(tx, "app_root_amnt", root_amnt);
				}
			}
			else if (tree_manager_get_child_value_i32(tx, NODE_HASH("app_item"), &app_item))
			{
				switch (app_item)
				{
					case 1:
						if (amount == 0)
						{
							char			typeName[32];
							unsigned int	TypeId, flags;
							ret = get_type_infos(&script, typeName, &TypeId,&flags);
						}
					break;
					case 2:
						if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
						{
							char			app_name[32];
							hash_t			sh;
							struct string	bsign = { 0 }, pkey = { 0 };
							unsigned int	type_id;

							type_id = amount & 0xFFFFFFFF;

							tree_manager_get_child_value_str (tx , NODE_HASH("appObj"), app_name, 32, 0);
							tree_manager_get_child_value_istr(tx , NODE_HASH("ObjSign"), &bsign,0);
							tree_manager_get_child_value_istr(out, NODE_HASH("script"), &script, 0);

							get_out_script_address(&script, &pkey, addr);

							ret = (pkey.len == 33) ? 1 : 0;

							if (ret)ret = compute_tx_sign_hash(tx, 0, &script, 1, sh);
							if (ret)ret = check_sign(&bsign, &pkey, sh);

							free_string(&bsign);
							free_string(&pkey);
							
							if (ret)
							{
								mem_zone_ref types = { PTR_NULL }, type = { PTR_NULL }, myobj = { PTR_NULL }, app = { PTR_NULL };

								ret=tree_manager_find_child_node(&apps, NODE_HASH(app_name), NODE_BITCORE_TX, &app);

								if (ret)ret = get_app_types						(&app, &types);
								if (ret)ret = tree_find_child_node_by_id_name	(&types, NODE_BITCORE_TX, "typeId", type_id, &type);
								if (ret)ret = obj_new							(&type, "objDef", &script, &myobj);
								if (ret)ret = check_app_obj_unique				(app_name,type_id,&myobj);
								if (ret)ret = tree_manager_node_add_child		(tx, &myobj);
								if (ret)ret = tree_manager_set_child_value_i32	(tx, "objType", type_id);

								release_zone_ref	(&myobj);
								release_zone_ref	(&type);
								release_zone_ref	(&types);
								release_zone_ref	(&app);

							}
							free_string(&script);
							amount = 0;
						}

					break;
					case 3:
						if (amount == 0xFFFFFFFFFFFFFFFF)
						{
							mem_zone_ref file = { PTR_NULL };
							if (tree_manager_create_node("fileDef", NODE_GFX_OBJECT, &file))
							{
								ret = get_script_file(&script, &file);
								if (ret)
								{
									hash_t h;
									tree_manager_get_child_value_hash	(&file, NODE_HASH("dataHash"), h);
									tree_manager_set_child_value_hash	(tx, "fileHash", h);
									tree_manager_node_add_child			(tx, &file);
								}
								
								release_zone_ref(&file);
							}
							amount = 0;
						}
					break;
					case 4:
						if (amount == 0xFFFFFFFFFFFFFFFF)
						{
							mem_zone_ref file = { PTR_NULL };
							if (tree_manager_create_node("layoutDef", NODE_GFX_OBJECT, &file))
							{
								ret = get_script_layout(&script, &file);
								if (ret)tree_manager_node_add_child(tx, &file);
								release_zone_ref(&file);
							}
							amount = 0;
						}
					break;
					case 5:
						if (amount == 0xFFFFFFFFFFFFFFFF)
						{
							mem_zone_ref file = { PTR_NULL };
							if (tree_manager_create_node("moduleDef", NODE_GFX_OBJECT, &file))
							{
								ret = get_script_module(&script, &file);
								if (ret)tree_manager_node_add_child(tx, &file);
								release_zone_ref(&file);
							}
							amount = 0;
						}
					break;
				}
			}
			else if ((idx == 0) && (tree_manager_get_child_value_hash(tx, NODE_HASH("appChildOf"), pObjH)))
			{
				size_t offset = 0;
				struct string key = { 0 }, cHash = { 0 };
				ret = get_script_data(&script, &offset, &key);
				if (ret)ret = get_script_data(&script, &offset, &cHash);
				if (ret)ret = (cHash.len == 32) ? 1 : 0;
				if (ret)
				{
					char			chash[65];
					struct string	appName;
					unsigned int	ptype, ktype, is_signed, flags;
					unsigned char	*hd = (unsigned char *)cHash.str;


					bin_2_hex(hd, 32, chash);
					/*
					n = 0;
					while (n < 32)
					{
						chash[n * 2 + 0] = hex_chars[hd[n] >> 0x04];
						chash[n * 2 + 1] = hex_chars[hd[n] & 0x0F];
						n++;
					}
					chash[64] = 0;
					*/

					log_output("new obj child '");
					log_output(chash);
					log_output("':'");
					log_output(key.str);
					log_output("'\n");

					if (!tree_manager_get_child_value_i32(tx, NODE_HASH("pObjSigned"), &is_signed))
						is_signed = 0;


					tree_manager_get_child_value_i32 (tx, NODE_HASH("pObjType"), &ptype);
					tree_manager_get_child_value_istr(tx, NODE_HASH("appChild"), &appName,0);

					ret=get_app_type_key				(appName.str,ptype, key.str, &ktype,&flags);
					
					if (ret)ret = ((ktype == NODE_JSON_ARRAY) || (ktype == NODE_PUBCHILDS_ARRAY)) ? 1 : 0;
					if ((ret)&&((ktype == NODE_JSON_ARRAY) && (!is_signed)))ret = 0;
					if (ret)
					{
						tree_manager_set_child_value_vstr(tx, "appChildKey", &key);
						tree_manager_set_child_value_hash(tx, "newChild", hd);
					}

					free_string(&appName);
				}
				free_string(&key);
				free_string(&cHash);
			}

			if (ret)*total += amount;
		}
		free_string(&script);
		if (!ret)
		{
			char iStr[16];
			itoa_s(idx, iStr, 16, 10);
			log_output("check output failed at output #");
			log_output(iStr);
			log_output("\n");
			dec_zone_ref(out);
			release_zone_ref(&my_list);
			break;
		}
	}
	release_zone_ref(&txout_list);

	if (!ret)return 0;

	if (tree_manager_find_child_node(tx, NODE_HASH("AppName"), NODE_GFX_STR, PTR_NULL))
	{
		if (app_flags != 31){
			log_output("invalid app tx\n");
			return 0;
		}
	}
	return 1;
}


OS_API_C_FUNC(int) find_inputs(mem_zone_ref_ptr tx_list, hash_t txid,unsigned int oidx)
{
	mem_zone_ref my_txlist = { PTR_NULL };
	mem_zone_ref_ptr tx = PTR_NULL;
	int ret=0;

	if (tx_list == PTR_NULL)return 0;
	if (tx_list->zone == PTR_NULL)return 0;

	for (tree_manager_get_first_child(tx_list, &my_txlist, &tx); ((tx != PTR_NULL) && (tx->zone != PTR_NULL)); tree_manager_get_next_child(&my_txlist, &tx))
	{
		mem_zone_ref		txin_list = { PTR_NULL }, my_ilist = { PTR_NULL };
		mem_zone_ref_ptr	input = PTR_NULL;

		if (!tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))
			continue;

		for (tree_manager_get_first_child(&txin_list, &my_ilist, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_ilist, &input))
		{
			hash_t			th;
			unsigned int	idx;
			tree_manager_get_child_value_hash	(input, NODE_HASH("txid"), th);
			tree_manager_get_child_value_i32	(input, NODE_HASH("idx"), &idx);

			if (!memcmp_c(txid, th, sizeof(hash_t)) && (idx == oidx))
			{
				dec_zone_ref(input);
				release_zone_ref(&my_ilist);
				ret = 1;
				break;
			}
		}
		release_zone_ref(&txin_list);
		if (ret == 1)
		{
			dec_zone_ref(tx);
			release_zone_ref(&my_txlist);
			break;
		}
	}
	return ret;
}


OS_API_C_FUNC(int) check_tx_list(mem_zone_ref_ptr tx_list,uint64_t block_reward,hash_t merkle, unsigned int block_time,unsigned int check_sig)
{
	hash_t				merkleRoot;
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref_ptr	tx = PTR_NULL;
	uint64_t			list_reward;
	int					ret;
	unsigned int		coinbase, coinstaking, is_staking, is_coinbase;
	uint64_t			txFee, fees;

	build_merkel_tree	(tx_list, merkleRoot);

	if (memcmp_c(merkleRoot, merkle,sizeof(hash_t)))
	{
		log_message("bad merkle root ",PTR_NULL);
		return 0;
	}
	tree_manager_get_first_child	(tx_list, &my_list, &tx);

	if (is_tx_null(tx))
	{
		tree_manager_get_next_child(&my_list, &tx);
		coinbase = 0;
		coinstaking = 1;
	}
	else
	{
		coinbase = 1;
		coinstaking = 0;
	}

	list_reward = 0;
	fees = 0;


	tree_manager_create_node("blkobjs", NODE_BITCORE_TX_LIST, &blkobjs);
	
	bt_deltree(blk_root);

	blk_root = PTR_NULL;

	ret = 1;
	for (; ((tx != PTR_NULL) && (tx->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &tx))
	{
		struct string		tx_path = { 0 };
		uint64_t			total_in, total_out;
		mem_zone_ref		txin_list = { PTR_NULL }, my_llist = { PTR_NULL };
		mem_zone_ref_ptr	input = PTR_NULL;
		unsigned int		tx_time;
		

		is_staking = 0;
		is_coinbase = 0;
		total_out = 0;
		total_in = 0;

		ret = tree_manager_get_child_value_i32(tx, NODE_HASH("time"), &tx_time);

		if (ret)
		{
			ret = (tx_time <= (block_time+20)) ? 1 : 0;
			if (!ret)
				log_output("tx time after block time \n");
		}

		if (ret)
		{
			if (is_app_root(tx))
			{
				if (has_root_app)
				{
					ret = 0;
					log_output("app root already set \n");
				}
				else
					continue;
			}
		}

		if (ret)
		{
			ret = check_tx_inputs(tx, &total_in, &is_coinbase, check_sig | 2);
			if(!ret)log_output("invalid inputs \n");
		}
		if (ret)
		{
			ret = check_tx_outputs(tx, &total_out, &is_staking);
			if(!ret)log_output("invalid outputs \n");
		}

		if (ret)
		{
			if (is_staking)
			{
				if (coinstaking == 0)
				{
					ret = 0;
					log_output("invalid coin stake\n");
				}
				else
				{
					coinstaking = 0;
					list_reward = total_out - total_in;
				}
			}
			else if (is_coinbase)
			{
				if (coinbase == 0)
				{
					ret = 0;
					log_output("invalid coin base\n");
				}
				else
				{
					coinbase = 0;
					list_reward = total_out - total_in;
				}
			}
			else
			{
				if (total_out > total_in)
				{
					ret = 0;
					log_output("insufficient input \n");
				}
				else
				{
					txFee = total_in - total_out;
					fees += txFee;
				}
			}
		}

		if (ret)
		{
			if (tree_manager_find_child_node(tx, NODE_HASH("AppName"), NODE_GFX_STR, PTR_NULL))
			{
				uint64_t root_amnt;
				if (!tree_manager_get_child_value_i64(tx, NODE_HASH("app_root_amnt"), &root_amnt))root_amnt = 0;
				if (root_amnt < app_fee)
				{
					ret = 0;
					log_output("insufficient root amount\n");
				}
			}
		}

		if (!ret)
		{
			dec_zone_ref		(tx);
			release_zone_ref	(&my_list);
			break;
		}
	}
		
	release_zone_ref(&blkobjs);
	if (!ret)
	{
		bt_deltree	(blk_root);
		blk_root	= PTR_NULL;
		log_message("error tx",PTR_NULL);
		return 0;
	}
	if (list_reward > (block_reward + fees))
	{
		mem_zone_ref log = { PTR_NULL };

		bt_deltree(blk_root);
		blk_root = PTR_NULL;

		tree_manager_create_node		 ("log", NODE_LOG_PARAMS, &log);
		tree_manager_set_child_value_i64 (&log, "list", list_reward);
		tree_manager_set_child_value_i64 (&log, "block", block_reward);
		tree_manager_set_child_value_i64 (&log, "fees", fees);
		log_message						 ("bad tx reward %list% %block% %fees% ",&log);
		release_zone_ref				 (&log);
		return 0;
	}
	
	return 1;
}



OS_API_C_FUNC(int) check_block_pow(mem_zone_ref_ptr hdr,hash_t diff_hash)
{
	hash_t				blk_pow, rdiff;
	mem_zone_ref		log={PTR_NULL};
	char				rpow[32];
	hash_t				bhash;
	int					n= 32;
	
	//pow block

	if (!tree_manager_get_child_value_hash(hdr, NODE_HASH("blkHash"), bhash))
	{
		compute_block_hash					(hdr, bhash);
		tree_manager_set_child_value_bhash	(hdr, "blkHash", bhash);
	}
	if (!tree_manager_get_child_value_hash(hdr, NODE_HASH("blk pow"), blk_pow))
	{
		compute_block_pow					(hdr, blk_pow);
		tree_manager_set_child_value_hash	(hdr, "blk pow", blk_pow);
	}
	n = 32;
	while (n--)
	{
		rdiff[n] = diff_hash[31 - n];
		rpow[n]  = blk_pow[31 - n];
	}
	//compare pow & diff
	if (cmp_hashle(blk_pow, rdiff) == 1)
	{
		tree_manager_create_node("log", NODE_LOG_PARAMS, &log);
		tree_manager_set_child_value_hash(&log, "diff", diff_hash);
		tree_manager_set_child_value_hash(&log, "pow", rpow);
		tree_manager_set_child_value_hash(&log, "hash", bhash);
		log_message("----------------\nNEW POW BLOCK\n%diff%\n%pow%\n%hash%\n", &log);
		release_zone_ref(&log);
		return 1;
	}
	else
	{
		tree_manager_create_node("log", NODE_LOG_PARAMS, &log);
		tree_manager_set_child_value_hash(&log, "diff", diff_hash);
		tree_manager_set_child_value_hash(&log, "pow" , rpow);
		tree_manager_set_child_value_hash(&log, "hash", bhash);
		log_message("----------------\nBAD POW BLOCK\n%diff%\n%pow%\n%hash%\n", &log);
		release_zone_ref(&log);
		return 0;
	}
	
}


OS_API_C_FUNC(int)  get_prev_block_time(mem_zone_ref_ptr header, ctime_t *time)
{
	char prevHash[65];
	struct string blk_path = { 0 };
	
	if (!tree_manager_get_child_value_str(header, NODE_HASH("prev"), prevHash,65,16))return 0;
	return get_block_time(prevHash, time);
}






OS_API_C_FUNC(int) block_has_pow(mem_zone_ref_ptr blockHash)
{
	char blk_hash[65];
	if (!tree_manager_get_node_str(blockHash, 0, blk_hash, 65, 16))return 0;
	return is_pow_block(blk_hash);
}
int make_iadix_merkle(mem_zone_ref_ptr genesis,mem_zone_ref_ptr txs,hash_t merkle)
{
	mem_zone_ref	newtx = { PTR_NULL };
	mem_zone_ref	script_node = { PTR_NULL };
	struct string	out_script = { PTR_NULL }, script = { PTR_NULL };
	struct string	timeproof = { PTR_NULL };

	make_string(&timeproof, "1 Sep 2016 Iadix coin");
	tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node);
	tree_manager_set_child_value_vint32(&script_node, "0", 0);
	tree_manager_set_child_value_vint32(&script_node, "1", 42);
	tree_manager_set_child_value_vstr(&script_node, "2", &timeproof);
	serialize_script		(&script_node, &script);
	release_zone_ref		(&script_node);

	new_transaction				(&newtx, 1466419086);
	tx_add_input				(&newtx, null_hash, 0xFFFFFFFF, &script);
	tx_add_output				(&newtx, 0, &out_script);
	free_string					(&script);
	free_string					(&timeproof);
	tree_manager_node_add_child (txs, &newtx);
	release_zone_ref			(&newtx);
	build_merkel_tree			(txs, merkle);
	
	return 0;
}


OS_API_C_FUNC(int) make_genesis_block(mem_zone_ref_ptr genesis_conf,mem_zone_ref_ptr genesis)
{
	hash_t								blk_pow, merkle;
	mem_zone_ref						txs = { PTR_NULL };
	uint64_t							StakeMod;
	unsigned int						version, time, bits, nonce;
	hash_t								hmod;
	

	if (genesis->zone == PTR_NULL)
	{
		if (!tree_manager_create_node("genesis", NODE_BITCORE_BLK_HDR, genesis))
			return 0;
	}
	
	if (!tree_manager_create_node("txs", NODE_BITCORE_TX_LIST, &txs))
		return 0;
	
	if (!tree_manager_get_child_value_hash(genesis_conf, NODE_HASH("merkle_root"), merkle))
	{
		make_iadix_merkle					(genesis, &txs, merkle);
	}
	
	tree_manager_set_child_value_hash	(genesis, "merkle_root"			, merkle);
	tree_manager_set_child_value_hash	(genesis, "prev"					, null_hash);

	tree_manager_get_child_value_i32	(genesis_conf, NODE_HASH("version")	, &version);
	tree_manager_get_child_value_i32	(genesis_conf, NODE_HASH("time")	, &time);
	tree_manager_get_child_value_i32	(genesis_conf, NODE_HASH("bits")	, &bits);
	tree_manager_get_child_value_i32	(genesis_conf, NODE_HASH("nonce")	, &nonce);


	tree_manager_set_child_value_i32	(genesis, "version"			, version);
	tree_manager_set_child_value_i32	(genesis, "time"			, time);
	tree_manager_set_child_value_i32	(genesis, "bits"			, bits);
	tree_manager_set_child_value_i32	(genesis, "nonce"			, nonce);
	
	tree_manager_node_add_child			(genesis, &txs);

	compute_block_pow					(genesis, blk_pow);
	tree_manager_set_child_value_bhash	(genesis, "blkHash", blk_pow);
	tree_manager_set_child_value_hash	(genesis, "blk pow" , blk_pow);
	
	if (tree_manager_get_child_value_i64(genesis_conf, NODE_HASH("InitialStakeModifier"), &StakeMod))
		tree_manager_set_child_value_i64(genesis, "StakeMod", StakeMod);

	if (tree_manager_get_child_value_hash(genesis_conf, NODE_HASH("InitialStakeModifier2"), hmod))
		tree_manager_set_child_value_hash(genesis, "StakeMod2", hmod);

	if (!find_hash(blk_pow))
	{
		store_block		(genesis, &txs);
	}
	release_zone_ref(&txs);
	return 1;

}


OS_API_C_FUNC(int) get_tx_data(mem_zone_ref_ptr tx, mem_zone_ref_ptr txData)
{
	struct string	txdata = { 0 };
	size_t			size;
	uint64_t		fee;
	unsigned char	*buffer;
	hash_t txh;

	tree_manager_get_child_value_i32(tx, NODE_HASH("size"), &size);

	buffer = malloc_c(size);
	write_node(tx, buffer);

	txdata.len = size * 2;
	txdata.size = txdata.len + 1;
	txdata.str = malloc_c(txdata.size);


	bin_2_hex(buffer, size, txdata.str);
	/*
	for (n = 0; n < size; n++)
	{
		txdata.str[n * 2 + 0] = hex_chars[buffer[n] >> 4];
		txdata.str[n * 2 + 1] = hex_chars[buffer[n] & 0x0F];
	}
	txdata.str[txdata.len] = 0;
	*/

	free_c(buffer);

	if (!tree_manager_get_child_value_i64(tx, NODE_HASH("fee"), &fee))
		fee = 0;
	tree_manager_get_child_value_hash	(tx, NODE_HASH("txid"), txh);

	tree_manager_set_child_value_vstr	(txData, "data", &txdata);
	tree_manager_set_child_value_i64	(txData, "fee", fee);
	tree_manager_set_child_value_hash	(txData, "hash", txh);
	tree_manager_set_child_value_bool	(txData, "required", 1);

	free_string(&txdata);

	return 1;
}