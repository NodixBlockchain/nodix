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


mem_zone_ref			apps = { PTR_INVALID }, trusted_apps = { PTR_INVALID };
mem_zone_ref			blkobjs = { PTR_INVALID };

/* #include "lang.h" */

C_IMPORT size_t		C_API_FUNC	compute_payload_size(mem_zone_ref_ptr payload_node);
C_IMPORT char*		C_API_FUNC	write_node			(mem_zone_ref_const_ptr key, unsigned char *payload);
C_IMPORT size_t		C_API_FUNC	get_node_size		(mem_zone_ref_ptr key);
C_IMPORT void		C_API_FUNC	serialize_children	(mem_zone_ref_ptr node, unsigned char *payload);
C_IMPORT size_t		C_API_FUNC read_node(mem_zone_ref_ptr key, const unsigned char *payload, size_t len);
C_IMPORT size_t		C_API_FUNC init_node(mem_zone_ref_ptr key);

C_IMPORT void		C_API_FUNC	unserialize_children(mem_zone_ref_ptr obj, const_mem_ptr payload,size_t len);


/* check signature */
extern int				check_sign			(const struct string *sign, const struct string *pubK, const hash_t txh);
/* check public key from tx output */
extern int				check_txout_key		(mem_zone_ref_ptr output, unsigned char *pkey,btc_addr_t addr);
/* compute scrypt block hash */
extern int				scrypt_blockhash	(const void* input, hash_t hash);

extern	int				add_tx_script_var	(mem_zone_ref_ptr script_node, const struct string *val);

extern  int				add_script_uivar	(mem_zone_ref_ptr script_node,uint64_t val);

extern  int				get_script_file		(struct string *script, mem_zone_ref_ptr file);

extern void				keyrh_to_addr		(unsigned char *pkeyh, btc_addr_t addr);

extern struct string	get_next_script_var	(const struct string *script,size_t *offset);

extern int				add_script_opcode	(mem_zone_ref_ptr script_node, unsigned char opcode);

extern int				add_script_push_data(mem_zone_ref_ptr script_node, const_mem_ptr data, size_t size);
 
extern int				get_script_data		(const struct string *script, size_t *offset, struct string *data);

extern int				add_script_float	(mem_zone_ref_ptr script_node, float val);
extern int				add_script_ivar		(mem_zone_ref_ptr script_node, int64_t val);

extern int				get_script_layout	(struct string *script, mem_zone_ref_ptr file);
extern int				get_script_module	(struct string *script, mem_zone_ref_ptr file);
extern int				find_index_str		(const char *app_name, const char *typeStr, const char *keyname, const struct string *val, hash_t hash);


hash_t					null_hash			= { 0xCD };
hash_t					ff_hash				= { 0xCD };
hash_t					app_root_hash		= { 0xCD };
btc_addr_t				root_app_addr		= { 0xCD };
uint64_t				app_fee				=  0xFFFFFFFF;

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
volatile unsigned int	sort_lock			=0xFFFFFFFF;

uint64_t				last_pow_block		= 0xFFFFFFFF;
uint64_t				pow_reward			= 100000*ONE_COIN;
node					*blk_root			= PTR_INVALID;

extern mem_zone_ref block_index_node;
extern mem_zone_ref time_index_node;
extern mem_zone_ref block_rindex_node;

//#undef _DEBUG
#ifdef _NATIVE_LINK_
LIBEC_API int			C_API_FUNC crypto_extract_key	(dh_key_t pk, const dh_key_t sk);
LIBEC_API int			C_API_FUNC crypto_sign_open		(const struct string *sign, const hash_t msgh, const struct string *pk);
LIBEC_API struct string	C_API_FUNC crypto_sign			(const hash_t msg, const dh_key_t sk);
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
	ret=load_module						(file, name, tpo_mod,0);
	if(ret)
	{

#ifndef _NATIVE_LINK_
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

OS_API_C_FUNC(int) get_pow_diff_limit(unsigned int *min_diff)
{
	*min_diff = diff_limit;

	return 1;
}



OS_API_C_FUNC(int) init_blocks(mem_zone_ref_ptr node_config, mem_zone_ref_ptr trustedApps)
{
	hash_t				msgh;
	dh_key_t			privkey;
	dh_key_t			pubkey;
	mem_zone_ref		mining_conf = { PTR_NULL }, mod_def = { PTR_NULL };
	struct string		smsgh = { 0 };
	struct string		sign = { 0 };
	struct string		msg = { 0 };
	struct string		pkstr = { 0 },str = { 0 };
	int					i;

	memset_c						(null_hash, 0, 32);
	memset_c						(ff_hash, 0xFF, 32);
	memset_c						(app_root_hash, 0, 32);
	memset_c						(root_app_addr, 0, sizeof(btc_addr_t));

	//parseDict	("block_adx", dict);

	blk_root = PTR_NULL;
	trusted_apps.zone = PTR_NULL;
	apps.zone = PTR_NULL;
	blkobjs.zone = PTR_NULL;
	has_root_app = 0;
	app_fee = 0;
	sort_lock = 0;

	block_index_node.zone = PTR_NULL;
	time_index_node.zone = PTR_NULL;
	block_rindex_node.zone = PTR_NULL;


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


	smsgh.str = msgh;
	smsgh.len = sizeof(hash_t);
	smsgh.size = smsgh.len;


	pkstr.str	= pubkey;
	pkstr.len	= 64;
	
	sign = crypto_sign		(smsgh.str, privkey);
	i = crypto_sign_open    (&sign, smsgh.str, &pkstr);

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
	struct string  sign = { 0 };

	sign = crypto_sign(hash, privkey);

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

OS_API_C_FUNC(int) derive_secret(const dh_key_t pub, dh_key_t priv,hash_t secret)
{
	return derive_key(pub, priv, secret);
}




OS_API_C_FUNC(int) blk_find_last_pow_block(mem_zone_ref_ptr pindex, unsigned int *block_time)
{
	hash_t		hash;
	uint64_t    myh;
	int			ret = 0;

	

	tree_manager_get_child_value_i64	(pindex, NODE_HASH("height"), &myh);
	tree_manager_get_child_value_hash	(pindex, NODE_HASH("blkHash"), hash);

	if (last_pow_block > 0)
	{
		if (myh >= last_pow_block)
			myh = last_pow_block;
	}
	
	get_block_hash(myh, hash);

	while (!is_pow_block(hash))
	{
		if (!get_block_hash(myh--, hash))
			break;
	}
	if (is_pow_block(hash))
	{
		if (load_blk_hdr(pindex, hash))
		{
			if(block_time!=PTR_NULL)
				tree_manager_get_child_value_i32(pindex, NODE_HASH("time"), block_time);
			return 1;
		}
		else
			return -1;
	}
	return 0;
}

OS_API_C_FUNC(int) blk_find_last_pos_block(mem_zone_ref_ptr pindex, unsigned int *block_time)
{
	hash_t		hash;
	uint64_t    myh;
	int			ret = 0;
	

	tree_manager_get_child_value_i64(pindex, NODE_HASH("height"), &myh);
	tree_manager_get_child_value_hash(pindex, NODE_HASH("blkHash"), hash);

	if (last_pow_block > 0)
	{
		if (myh >= last_pow_block)
			myh = last_pow_block;
	}

	get_block_hash(myh, hash);

	while (is_pow_block(hash))
	{
		if (!get_block_hash(myh--, hash))
			break;
	}
	if (!is_pow_block(hash))
	{
		if (load_blk_hdr(pindex, hash))
		{
			if(block_time!=PTR_NULL)
				tree_manager_get_child_value_i32(pindex, NODE_HASH("time"), block_time);
			return 1;
		}
		else
			return -1;
	}
	return 0;
}

int add_app_tx(mem_zone_ref_ptr new_app, const char *app_name)
{
	mem_zone_ref txout_list = { PTR_NULL }, input = { PTR_NULL };
	int ret,appLocked;
	
	tree_manager_set_child_value_str(new_app, "appName", app_name);

	appLocked = 0;
	ret = get_tx_input(new_app, 0, &input);
	if(ret)
	{
		struct string script = { PTR_NULL };
		size_t offset=0;

		ret = tree_manager_get_child_value_istr(&input, NODE_HASH("script"), &script, 0);
		release_zone_ref(&input);

		if (ret) {
			struct string appName = { PTR_NULL };
			appName = get_next_script_var(&script, &offset);
			free_string(&appName);
		}
		if (ret)
		{
			struct string locked;
			locked = get_next_script_var(&script, &offset);
			if (locked.len > 0)
			{
				unsigned char *data = locked.str;

				if (*data == 0x00)
					appLocked = 0;
				else if (*data < 0xFD)
					appLocked = *data;
			}

			free_string(&locked);

		}
	}

	tree_manager_set_child_value_bool(new_app, "locked", appLocked);


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

	if(script!=PTR_NULL)
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
		add_tx_script_var			(&script,&var);
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

		add_tx_script_var		(&script,&strKey);
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

OS_API_C_FUNC(int) make_obj_txfr_tx(mem_zone_ref_ptr tx, const hash_t txh, unsigned int oidx, const btc_addr_t dstAddr,hash_t objHash)
{
	hash_t			txH;
	btc_addr_t		srcAddr;
	char			tx_hash[65];
	mem_zone_ref	script = { PTR_NULL };
	struct string	sscript;
	uint64_t		amount;
	ctime_t			time;

	bin_2_hex(txh, 32, tx_hash);

	if (load_utxo(tx_hash, oidx, &amount, srcAddr, objHash) != 2)
		return 0;

	time = get_time_c();
	new_transaction(tx, time);
	tx_add_input(tx, txh, oidx, PTR_NULL);
	if (tree_manager_create_node("oapproot", NODE_BITCORE_SCRIPT, &script))
	{
		create_p2sh_script_data(dstAddr, &script, objHash, 32);
		serialize_script(&script, &sscript);
		release_zone_ref(&script);
		tx_add_output(tx, amount, &sscript);
		free_string(&sscript);
	}

	compute_tx_hash(tx, txH);
	tree_manager_set_child_value_hash(tx, "txid", txH);

	return 1;
}

int add_txop_arg(mem_zone_ref_ptr tx, mem_zone_ref_ptr arg)
{
	hash_t			ihash = { 0xFF };
	struct string	sscript = { 0 };
	mem_zone_ref	script = { PTR_NULL };
	
	unsigned int	t1, tt1;

	
	t1 = tree_mamanger_get_node_type(arg);

	tree_manager_create_node("opinput", NODE_BITCORE_SCRIPT, &script);

	switch (t1)
	{
		case NODE_GFX_OBJECT:
		{	
			struct string objKey = { 0 };

			if (tree_manager_get_child_value_istr(arg, NODE_HASH("objKey"), &objKey ,0))
			{
				int ret;

				ret = tree_manager_get_child_value_hash(arg, NODE_HASH("objId"), ihash);
				if (ret) {
					add_tx_script_var(&script, &objKey);
					tt1 = NODE_GFX_OBJECT;
				}
				free_string							(&objKey);
			}
			else if (tree_manager_get_child_value_hash(arg, NODE_HASH("txid"), ihash))
			{
				//op reference
				tt1 = 0;
			}
			else
			{
				release_zone_ref(&script);
				return 0;
			}
		}
	break;
	case NODE_GFX_STR:
	case NODE_BITCORE_VSTR:
	{
		struct string var_name = { 0 };

		memset_c(ihash, 0xFF, sizeof(hash_t));
		tt1 = 0;

		tree_manager_get_node_istr(arg, 0, &var_name, 0);
		add_tx_script_var(&script, &var_name);
		free_string(&var_name);

		
	}
	break;
	case NODE_GFX_SIGNED_INT:
	case NODE_GFX_SIGNED_BINT:
	{
		int64_t value;

		memset_c(ihash, 0xFF, sizeof(hash_t));
		tt1 = t1;
		
		tree_mamanger_get_node_signed_qword(arg, 0, &value);
		add_script_ivar(&script, value);
	}

	break;
	case NODE_GFX_FLOAT:
	{
		float value;

		memset_c(ihash, 0xFF, sizeof(hash_t));
		tt1 = t1;

		tree_mamanger_get_node_float(arg, 0, &value);
		add_script_float(&script, value);
	}
	break;

	}

	serialize_script(&script, &sscript);
	release_zone_ref(&script);

	tx_add_input(tx, ihash, tt1, &sscript);
	free_string(&sscript);


	return 1;
}

int add_tx_op(mem_zone_ref_ptr tx, const struct string *op_name)
{
	hash_t			ihash = { 0xFF };
	struct string	sscript = { 0 };
	mem_zone_ref	script = { PTR_NULL };


	if (!tree_manager_create_node("oapproot", NODE_BITCORE_SCRIPT, &script))
		return 0;

	add_tx_script_var		(&script, op_name);
	add_script_opcode	(&script, 0xFF);

	serialize_script	(&script, &sscript);
	release_zone_ref	(&script);
	tx_add_output		(tx, 0, &sscript);
	free_string			(&sscript);

	return 1;
}

int add_tx_fn(mem_zone_ref_ptr tx, const struct string *op_name)
{
	hash_t			ihash = { 0xFF };
	struct string	sscript = { 0 };
	mem_zone_ref	script = { PTR_NULL };


	if (!tree_manager_create_node("oapproot", NODE_BITCORE_SCRIPT, &script))
		return 0;

	add_tx_script_var		(&script, op_name);
	add_script_opcode	(&script, 0xFE);


	serialize_script(&script, &sscript);
	release_zone_ref(&script);
	tx_add_output(tx, 0, &sscript);
	free_string(&sscript);

	return 1;
}

OS_API_C_FUNC(int) is_opfn_script(const struct string *script,struct string *fn_name)
{
	struct string name = { 0 };
	unsigned char op_code;
	size_t offset = 0;
	int ret;

	name = get_next_script_var(script, &offset);
	ret = ((name.str != PTR_NULL) && (name.len >0 )) ? 1 : 0;
	if (ret) {
		op_code = *((unsigned char *)(script->str + offset));
		ret = ((op_code == 0xFF) | (op_code == 0xFE)) ? op_code : 0;
	}

	if ((ret)&&(fn_name!=PTR_NULL))
		clone_string(fn_name, &name);

	free_string(&name);
	return ret;

}


OS_API_C_FUNC(int) make_op_tx(mem_zone_ref_ptr tx, const struct string *op_name, mem_zone_ref_ptr arg1, mem_zone_ref_ptr arg2)
{
	ctime_t			time;
	
	time = get_time_c();
	new_transaction(tx, time);	
	
	add_txop_arg(tx, arg1);
	add_txop_arg(tx, arg2);
	add_tx_op	(tx, op_name);

	return 1;
}
OS_API_C_FUNC(int) make_fn_tx(mem_zone_ref_ptr tx, const struct string *op_name, mem_zone_ref_ptr arg1)
{
	ctime_t			time;

	time = get_time_c();
	new_transaction(tx, time);

	add_txop_arg(tx, arg1);
	add_tx_fn(tx, op_name);

	return 1;
}


OS_API_C_FUNC(int) make_app_tx(mem_zone_ref_ptr tx,const char *app_name,unsigned int flags,btc_addr_t appAddr)
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
		add_tx_script_var		(&script,&var);
		free_string				(&var);
		add_script_uivar		(&script,flags);

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

OS_API_C_FUNC(int) make_app_child_obj_tx(mem_zone_ref_ptr tx, const char *app_name, hash_t objHash,unsigned int objType,btc_addr_t objAddr, const char *keyName, unsigned int ktype, hash_t childHash)
{
	char				chash[65];
	mem_zone_ref		vin = { PTR_NULL }, script = { PTR_NULL }, my_list = { PTR_NULL }, txout_list = { PTR_NULL };
	mem_zone_ref_ptr	obj = { PTR_NULL },out = PTR_NULL;
	struct string		sscript = { 0 }, strKey = { 0 }, null_str = { 0 };
	ctime_t				time;
	unsigned int		keytype, flags;
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


	ret = get_app_type_key(app_name, objType, keyName, &keytype,&flags);
	if (ret)ret = ((keytype == NODE_JSON_ARRAY) || (keytype == NODE_PUBCHILDS_ARRAY)) ? 1 : 0;
	if (!ret)return 0;
	
	time = get_time_c	();
	new_transaction		(tx, time);

	tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script);

	add_script_push_data	(&script,	keyName, strlen_c(keyName));
	add_script_push_data	(&script,	childHash, 32);
	add_script_opcode		(&script,	0x93);
	serialize_script		(&script,	&sscript);

	null_str.str = malloc_c(128);
	null_str.len = 128;
	tx_add_input			(tx, objHash, 0, &null_str);
	free_string(&null_str);

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
OS_API_C_FUNC(int) build_merkel_tree(mem_zone_ref_ptr txs, hash_t merkleRoot, mem_zone_ref_ptr branches)
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

		if (branches != PTR_NULL)
		{
			mem_zone_ref	newbranch = { PTR_NULL };
			if (tree_manager_add_child_node(branches, "branch", NODE_BITCORE_HASH, &newbranch))
			{
				tree_manager_write_node_hash(&newbranch, 0, tx_hash);
				release_zone_ref(&newbranch);
			}
		}


		mbedtls_sha256_update		(&ctx, tx_hash, sizeof(hash_t));

		mbedtls_sha256_finish		(&ctx, tmp);
		mbedtls_sha256_free			(&ctx);
		mbedtls_sha256				(tmp, 32, merkleRoot, 0);
		release_zone_ref(&hashes);
		return 1;
	}

	

	if (branches != PTR_NULL)
	{
		mem_zone_ref	newbranch = { PTR_NULL };

		tree_manager_get_node_hash(&hashes, sizeof(hash_t), tx_hash);

		if (tree_manager_add_child_node(branches, "branch", NODE_BITCORE_HASH, &newbranch))
		{
			tree_manager_write_node_hash(&newbranch, 0, tx_hash);
			release_zone_ref(&newbranch);
		}
	}


	while ((newLen=compute_merkle_round(&hashes, n))>1)
	{
		if (branches != PTR_NULL)
		{
			hash_t			merkleBranch;
			mem_zone_ref	newbranch = { PTR_NULL };

			tree_manager_get_node_hash		(&hashes, sizeof(hash_t) * 1, merkleBranch);
			
			if (tree_manager_add_child_node(branches, "branch", NODE_BITCORE_HASH, &newbranch))
			{
				tree_manager_write_node_hash	(&newbranch, 0, merkleBranch);
				release_zone_ref				(&newbranch);
			}
		}
		
		n = newLen;	
	}
	
	tree_manager_get_node_hash	(&hashes, 0, merkleRoot);


	release_zone_ref			(&hashes);
	
	

	return 1;
}

/*
#define BN_MASK2    (0xffffffffffffffffL)
#define BN_BITS2    64
#define BN_ULONG    unsigned long

int BN_num_bits_word(BN_ULONG l)
{
	BN_ULONG x, mask;
	int bits = (l != 0);

	x = l >> 16;
	mask = (0 - x) & BN_MASK2;
	mask = (0 - (mask >> (BN_BITS2 - 1)));
	bits += 16 & mask;
	l ^= (x ^ l) & mask;

	x = l >> 8;
	mask = (0 - x) & BN_MASK2;
	mask = (0 - (mask >> (BN_BITS2 - 1)));
	bits += 8 & mask;
	l ^= (x ^ l) & mask;

	x = l >> 4;
	mask = (0 - x) & BN_MASK2;
	mask = (0 - (mask >> (BN_BITS2 - 1)));
	bits += 4 & mask;
	l ^= (x ^ l) & mask;

	x = l >> 2;
	mask = (0 - x) & BN_MASK2;
	mask = (0 - (mask >> (BN_BITS2 - 1)));
	bits += 2 & mask;
	l ^= (x ^ l) & mask;

	x = l >> 1;
	mask = (0 - x) & BN_MASK2;
	mask = (0 - (mask >> (BN_BITS2 - 1)));
	bits += 1 & mask;

	return bits;
}


int BN_bn2mpi(const BIGNUM *a, unsigned char *d)
{	
    int bits;
    int num = 0;
    int ext = 0;
    long l;	

    bits = BN_num_bits(a);	
    num = (bits + 7) / 8;	
	if (bits > 0) {
		ext = ((bits & 0x07) == 0);
	}

    if (d == NULL)
        return (num + 4 + ext);


    l = num + ext;
    d[0] = (unsigned char)(l >> 24) & 0xff;	
    d[1] = (unsigned char)(l >> 16) & 0xff;	
    d[2] = (unsigned char)(l >> 8) & 0xff;	
    d[3] = (unsigned char)(l) & 0xff;
    if (ext)
        d[4] = 0;

    num = BN_bn2bin(a, &(d[4 + ext]));	
    if (a->neg)
        d[4] |= 0x80;

    return (num + 4 + ext);

}
*/

OS_API_C_FUNC(unsigned int) GetCompact(const hash_t in,unsigned int *bits)
{
	int n;
	unsigned int pos, val;
	
	for (n = 0; n <32; n++)
	{
		if (in[n] != 0)
			break;
	}

	if (n < 2)
		return 0;

	if (n > 29)
		return 0;

	pos		= (31-n) + 2;
	(*bits)	= in[n+1] + (in[n]<<8);
	(*bits) |= (pos << 24);


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

OS_API_C_FUNC(int) cmp_hashle(const hash_t hash1, const hash_t hash2)
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

int get_block_tx_offset(mem_zone_ref_ptr hdr,uint64_t *offset)
{
	uint64_t		txofs;
	unsigned int	ntx;

	if (!tree_manager_get_child_value_i64(hdr, NODE_HASH("txoffset"), &txofs))return 0;
	if (!tree_manager_get_child_value_i32(hdr, NODE_HASH("ntx"), &ntx))return 0;
	(*offset) = txofs + (ntx * 32 + 4);

	return 1;
}

OS_API_C_FUNC(int) load_blk_tx_input(const hash_t blk_hash, unsigned int tx_ofset, unsigned int vin_idx, mem_zone_ref_ptr vout)
{
	uint64_t		txofs=0xFFFFFFFF;
	int				ret=0;
	mem_zone_ref vin = { PTR_NULL }, hdr = { PTR_NULL };
	mem_zone_ref tx = { PTR_NULL }, prev_tx = { PTR_NULL };

	load_blk_hdr(&hdr, blk_hash);
	get_block_tx_offset(&hdr, &txofs);

	if (!blk_load_tx_ofset(txofs + tx_ofset, &tx))
	{
		release_zone_ref(&hdr);
		return 0;
	}

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
	release_zone_ref(&hdr);

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

			
	log_output("tx sign hash ");
    log_output(chash);
    log_output("\n");

	bin_2_hex(vpubK.str, vpubK.len, chash);
    

	
	uitoa_s(vpubK.len,dd,32,10);
	
	log_output("tx sign pk ");
    log_output(chash);
    log_output(" len ");
    log_output(dd);
    log_output("\n");
      

	bin_2_hex(sign.str, sign.len, chash);
	
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
	hash_t					tx_hash;
	mbedtls_sha256_context  ctx;
	mem_zone_ref			txout_list = { PTR_NULL }, txin_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr		output = PTR_NULL, input = PTR_NULL;
	size_t					len;
	int						nc;
	mem_ptr					data;
	unsigned int			iidx;
	const unsigned char		z = 0;
	if (!tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))
	{
		log_output("sign hash no txin\n");
		return 0;
	}


	mbedtls_sha256_init(&ctx);
	mbedtls_sha256_starts(&ctx, 0);

	tree_manager_get_child_data_ptr (tx, NODE_HASH("version"), &data);
	mbedtls_sha256_update			(&ctx, data, sizeof(unsigned int));

	tree_manager_get_child_data_ptr (tx, NODE_HASH("time"), &data);
	mbedtls_sha256_update			(&ctx, data, sizeof(unsigned int));


	nc = tree_manager_get_node_num_children(&txin_list);

	if (nc < 0xFD)
	{
		mbedtls_sha256_update(&ctx, (mem_ptr)&nc, sizeof(unsigned char));
	}
	else if (nc < 0xFFFF)
	{
		unsigned char hdr = 0xFD;
		mbedtls_sha256_update(&ctx, &hdr, 1);
		mbedtls_sha256_update(&ctx, (mem_ptr)&nc, sizeof(unsigned short));
	}
	else
	{
		unsigned char hdr = 0xFE;
		mbedtls_sha256_update(&ctx, &hdr, 1);
		mbedtls_sha256_update(&ctx, (mem_ptr)&nc, sizeof(unsigned int));
	}


	for (iidx = 0, tree_manager_get_first_child(&txin_list, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input), iidx++)
	{
		tree_manager_get_child_data_ptr	(input, NODE_HASH("txid"), &data);
		mbedtls_sha256_update			(&ctx, data, sizeof(hash_t));

		tree_manager_get_child_data_ptr	(input, NODE_HASH("idx"), &data);
		mbedtls_sha256_update			(&ctx, data, sizeof(unsigned int));

		if (nIn == iidx)
		{
			size_t szSz;
			unsigned char hdr;


			if (script->len < 0xFD)
				szSz = 1;
			else  if (script->len < 0xFFFF)
				szSz = 3;
			else  if (script->len < 0xFFFFFFFF)
				szSz = 5;
			else
				szSz = 9;

			switch (szSz)
			{
				case 1:mbedtls_sha256_update (&ctx, (mem_ptr)&script->len, 1); break;
				case 3:		
					hdr = 0xFD;
					mbedtls_sha256_update(&ctx, &hdr, 1); 
					mbedtls_sha256_update(&ctx, (mem_ptr)&script->len, sizeof(unsigned short));
				break;
				case 5:					
					hdr = 0xFE;
					mbedtls_sha256_update(&ctx, &hdr, 1);
					mbedtls_sha256_update(&ctx, (mem_ptr)&script->len, sizeof(unsigned int));
					
				break;
				case 9:	
					hdr = 0xFF;
					mbedtls_sha256_update(&ctx, &hdr, 1);
					mbedtls_sha256_update(&ctx, (mem_ptr)&script->len, sizeof(uint64_t));
				break;
			}

			mbedtls_sha256_update(&ctx, script->str, script->len);

		}
		else
			mbedtls_sha256_update(&ctx, &z, 1);
		
		tree_manager_get_child_data_ptr	(input, NODE_HASH("sequence"), &data);
		mbedtls_sha256_update			(&ctx, data, sizeof(unsigned int));
	}

	release_zone_ref(&txin_list);


	if (tree_manager_find_child_node(tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txout_list))
	{
		nc = tree_manager_get_node_num_children(&txout_list);

		if (nc < 0xFD)
		{
			mbedtls_sha256_update(&ctx, (mem_ptr)&nc, sizeof(unsigned char));
		}
		else if (nc < 0xFFFF)
		{
			unsigned char hdr = 0xFD;
			mbedtls_sha256_update(&ctx, &hdr, 1);
			mbedtls_sha256_update(&ctx, (mem_ptr)&nc, sizeof(unsigned short));
		}
		else
		{
			unsigned char hdr = 0xFE;
			mbedtls_sha256_update(&ctx, &hdr, 1);
			mbedtls_sha256_update(&ctx, (mem_ptr)&nc, sizeof(unsigned int));
		}


		for (tree_manager_get_first_child(&txout_list, &my_list, &output); ((output != PTR_NULL) && (output->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &output))
		{
			tree_manager_get_child_data_ptr (output, NODE_HASH("value") , &data);
			mbedtls_sha256_update			(&ctx, data, sizeof(uint64_t));

			tree_manager_get_child_data_ptr (output, NODE_HASH("script"), &data);

			if (*((unsigned char *)(data)) < 0xFD)
				len = 1 + (*((unsigned char *)(data)));
			else if (*((unsigned char *)(data)) == 0xFD)
				len = 3 + (*((unsigned short *)(mem_add(data, 1))));
			else if (*((unsigned char *)(data)) == 0xFE)
				len = 5 + (*((unsigned int *)(mem_add(data, 1))));

			mbedtls_sha256_update(&ctx, data, len);
		}

		release_zone_ref(&txout_list);
	}



	tree_manager_get_child_data_ptr	(tx, NODE_HASH("locktime"), &data);
	mbedtls_sha256_update			(&ctx, data, sizeof(unsigned int));

	mbedtls_sha256_update			(&ctx, (mem_ptr)&hash_type, sizeof(unsigned int));

	mbedtls_sha256_finish			(&ctx, tx_hash);
	mbedtls_sha256_free				(&ctx);

	mbedtls_sha256					(tx_hash, 32, txh, 0);
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
		if (!ret)
		{
			char hex[1024];

			bin_2_hex(script.str, script.len, hex);
			log_output("script no sig infos\n");
			log_output(hex);
			log_output("\n");
		}
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

OS_API_C_FUNC(int) tx_sign(mem_zone_ref_const_ptr tx, unsigned int nIn, unsigned int hashType, const struct string *sign_seq, const struct string *inPubKey,mem_zone_ref_ptr mempool)
{
	hash_t				ph,tx_hash;
	struct				string oscript = { PTR_NULL };
	mem_zone_ref		vin = { PTR_NULL }, vout = { PTR_NULL }, ptx = { PTR_NULL };
	unsigned int		oIdx;
	int					isObj =0, isObjChild=0,ret = 0;

	get_tx_input(tx, nIn, &vin);

	if (!tree_manager_get_child_value_i32(&vin, NODE_HASH("isAppObj"), &isObj))
		isObj = 0;

	if (!tree_manager_get_child_value_i32(&vin, NODE_HASH("addChild"), &isObjChild))
		isObjChild = 0;

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
		release_zone_ref(&vin);
		return 1;
	}

	tree_manager_get_child_value_hash(&vin, NODE_HASH("txid"), ph);
	tree_manager_get_child_value_i32 (&vin, NODE_HASH("idx") , &oIdx);

	ret = tree_find_child_node_by_member_name_hash(mempool, NODE_BITCORE_TX, "txid", ph, &ptx);
	if (ret)
	{
		ret = get_tx_output(&ptx, oIdx, &vout);
		release_zone_ref(&ptx);
	}
	else
		ret = load_tx_input_vout(tx, nIn, &vout);
	
	if (ret)
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
	/*
	else if (isObjChild)
	{
		if ((inPubKey != PTR_l::::::::::::::::::::::::::::::::::::çymàççççççççççççççççççççççççççççççççççççççççççççççççççççççççççççççççççççççççççççççNULL) && (inPubKey->str != PTR_NULL))
		{
			btc_addr_t				inAddr;
			struct string			sign = { PTR_NULL };
			unsigned char			htype;

			ret = compute_tx_sign_hash(tx, nIn, &oscript, hashType, tx_hash);
			if (ret)key_to_addr(inPubKey->str, inAddr);
			if (ret)ret = parse_sig_seq(sign_seq, &sign, &htype, 1);
			if (ret)ret = check_sign(&sign, inPubKey, tx_hash);
			if (ret)
			{
				mem_zone_ref script_node = { PTR_NULL };

				if (tree_manager_create_node("script", NODE_BITCORE_SCRIPT, &script_node))
				{
					struct string sscript = { PTR_NULL };

					tree_manager_set_child_value_vstr(&script_node, "var1", sign_seq);
					tree_manager_set_child_value_vstr(&script_node, "var2", inPubKey);
					serialize_script(&script_node, &sscript);
					tree_manager_set_child_value_vstr(&vin, "script", &sscript);
					release_zone_ref(&script_node);
					free_string(&sscript);
				}
			}
			free_string(&sign);
		}
	}
	*/

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



OS_API_C_FUNC(int) tx_is_app_item(const hash_t txh,unsigned int oidx,mem_zone_ref_ptr app_tx,unsigned char *val)
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

int is_obj_child(const hash_t ph, unsigned int pIdx, mem_zone_ref_ptr prev_tx, struct string *appName, mem_zone_ref_const_ptr mempool)
{
	hash_t				pBlock, pid;
	mem_zone_ref		prevout = { PTR_NULL }, prev_input = { PTR_NULL }, app = { PTR_NULL };
	struct string		oscript = { 0 };
	int					ld;
	int					ret = 0;

	/* load parent object transaction */

	/* block validation mode */
	if (mempool == PTR_NULL)
		ld = tree_find_child_node_by_member_name_hash(&blkobjs, NODE_BITCORE_TX, "txid", ph, prev_tx);
	/* memory pool mode */
	else
		ld = tree_find_child_node_by_member_name_hash(mempool, NODE_BITCORE_TX, "txid", ph, prev_tx);
	
	if (!ld) ld = load_tx(prev_tx, pBlock, ph);

	if(!ld)
		return -1;


	/* object can only be in first output, NODE_GFX_OBJECT indicate object operand in a parse tree */
	if ((pIdx != 0) && (pIdx != NODE_GFX_OBJECT))
		return 0;

	/* first input of the parent object is an application */
	get_tx_input						(prev_tx, 0, &prev_input);
	tree_manager_get_child_value_hash	(&prev_input, NODE_HASH("txid"), pid);

	if (tree_find_child_node_by_member_name_hash(&apps, NODE_BITCORE_TX, "txid", pid, &app))
	{
		mem_zone_ref		app_out = { PTR_NULL };
		unsigned int		pidx;

		/* get application root from the parent object's transaction */
		tree_manager_get_child_value_i32(&prev_input, NODE_HASH("idx"), &pidx);

		if (get_tx_output(&app, pidx, &app_out))
		{
			struct string app_script = { PTR_NULL }, val = { PTR_NULL };

			/* check application root type must be object's root */
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
			free_string(&app_script);
			release_zone_ref(&app_out);
		}
		release_zone_ref(&app);
	}
	release_zone_ref(&prev_input);
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

OS_API_C_FUNC(int) get_block_size(mem_zone_ref_ptr block,mem_zone_ref_ptr tx_list)
{
	mem_zone_ref my_list = { PTR_NULL };
	mem_zone_ref_ptr	ptx = PTR_NULL;
	size_t	blockSize, nTxs;

	blockSize = 80;
	nTxs = tree_manager_get_node_num_children(tx_list);

	if (nTxs < 0xFD)
		blockSize += 1;
	else
		blockSize += 3;

	for (tree_manager_get_first_child(tx_list, &my_list, &ptx); ((ptx != NULL) && (ptx->zone != NULL)); tree_manager_get_next_child(&my_list, &ptx))
	{
		size_t size;
		if (tree_manager_get_child_value_i32(ptx, NODE_HASH("size"), &size))
			blockSize += size;
	}
	tree_manager_set_child_value_i32(block, "nTx" , nTxs);
	tree_manager_set_child_value_i32(block, "size", blockSize);

	return 1;
}
OS_API_C_FUNC(int) find_obj_tx(mem_zone_ref_const_ptr tx, hash_t objHash,mem_zone_ref_ptr inputs, mem_zone_ref_ptr outputs)
{
	hash_t				txh;
	mem_zone_ref		txin_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr	input = PTR_NULL,output=PTR_NULL;
	unsigned int		iidx, oidx;

	tree_manager_get_child_value_hash(tx, NODE_HASH("txid"), txh);

	if (inputs != PTR_NULL)
	{
		if (!tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))
			return 0;

		for (iidx = 0, tree_manager_get_first_child(&txin_list, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input), iidx++)
		{
			hash_t ptxh;
			mem_zone_ref prevout = { 0 };
			uint64_t	amount;
			unsigned int poidx;

			if (!tree_manager_get_child_value_hash(input, NODE_HASH("txid"), ptxh))
				continue;

			if (!tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &poidx))
				continue;

			if (!load_tx_input_vout(tx, iidx, &prevout))
				continue;


			tree_manager_get_child_value_i64(&prevout, NODE_HASH("value"), &amount);

			if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
			{
				btc_addr_t	oaddr;
				struct string oscript = { 0 }, objHashStr = { 0 };

				tree_manager_get_child_value_istr(&prevout, NODE_HASH("script"), &oscript, 0);
				get_out_script_address(&oscript, PTR_NULL, oaddr);
				get_out_script_return_val(&oscript, &objHashStr);
				free_string(&oscript);

				if ((objHashStr.len == 32) && (!memcmp_c(objHash, objHashStr.str, sizeof(hash_t))))
				{
					mem_zone_ref newinput = { 0 };
					if (tree_manager_add_child_node(inputs, "input", NODE_BITCORE_WALLET_ADDR, &newinput))
					{
						tree_manager_write_node_btcaddr(&newinput, 0, oaddr);
						release_zone_ref(&newinput);
					}
				}
				else if (!memcmp_c(objHash, ptxh, sizeof(hash_t)))
				{
					mem_zone_ref newinput = { 0 };
					if (tree_manager_add_child_node(inputs, "input", NODE_BITCORE_WALLET_ADDR, &newinput))
					{
						tree_manager_write_node_btcaddr(&newinput, 0, oaddr);
						release_zone_ref(&newinput);
					}
				}
				free_string(&objHashStr);
			}


			release_zone_ref(&prevout);
		}

		release_zone_ref(&txin_list);
	}
	if (outputs != PTR_NULL)
	{
		if (!tree_manager_find_child_node(tx, NODE_HASH("txsout"), NODE_BITCORE_VOUTLIST, &txin_list))
			return 0;


		for (oidx = 0, tree_manager_get_first_child(&txin_list, &my_list, &output); ((output != PTR_NULL) && (output->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &output), oidx++)
		{
			uint64_t	amount;

			tree_manager_get_child_value_i64(output, NODE_HASH("value"), &amount);

			if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
			{
				btc_addr_t oaddr;
				struct string oscript = { 0 }, objHashStr = { 0 };
				int ret;

				tree_manager_get_child_value_istr(output, NODE_HASH("script"), &oscript, 0);

				get_out_script_address(&oscript, PTR_NULL,oaddr);
				ret = get_out_script_return_val(&oscript, &objHashStr);
				free_string(&oscript);

				if (ret)
				{
					mem_zone_ref newoutput = { 0 };

					if (objHashStr.len != 32)
					{
						free_string(&objHashStr);
						objHashStr.str = malloc_c(32);
						objHashStr.len = 32;
						memcpy_c(objHashStr.str, txh, 32);
					}

					if (!memcmp_c(objHashStr.str, objHash, sizeof(hash_t)))
					{
						if (tree_manager_add_child_node(outputs, "output", NODE_BITCORE_WALLET_ADDR, &newoutput))
						{
							tree_manager_write_node_btcaddr(&newoutput, 0, oaddr);
							release_zone_ref(&newoutput);
						}
					}
				}
				free_string(&objHashStr);
			}
		}

		release_zone_ref(&txin_list);
	}

	return 1;
}

OS_API_C_FUNC(int) find_obj_ptxfr(mem_zone_ref_const_ptr tx, hash_t objHash, mem_zone_ref_ptr prev_tx)
{
	hash_t				txh;
	mem_zone_ref		txin_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr	input = PTR_NULL, output = PTR_NULL;
	unsigned int		iidx;
	int					ret=0;

	tree_manager_get_child_value_hash(tx, NODE_HASH("txid"), txh);

	if (!tree_manager_find_child_node(tx, NODE_HASH("txsin"), NODE_BITCORE_VINLIST, &txin_list))
		return 0;

	for (iidx = 0, tree_manager_get_first_child(&txin_list, &my_list, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &input), iidx++)
	{
		hash_t pbh,ptxh;
		mem_zone_ref prevout = { 0 };
		uint64_t	amount;
		unsigned int poidx;

		if (!tree_manager_get_child_value_hash(input, NODE_HASH("txid"), ptxh))
			continue;

		if (!tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &poidx))
			continue;

		if (!load_tx(prev_tx, pbh, ptxh))
			continue;

		get_tx_output(prev_tx, poidx, &prevout);

		tree_manager_get_child_value_i64(&prevout, NODE_HASH("value"), &amount);

		if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
		{
			btc_addr_t	oaddr;
			struct string oscript = { 0 }, objHashStr = { 0 };

			tree_manager_get_child_value_istr(&prevout, NODE_HASH("script"), &oscript, 0);
			get_out_script_address(&oscript, PTR_NULL, oaddr);
			get_out_script_return_val(&oscript, &objHashStr);
			free_string(&oscript);

			if ((objHashStr.len == 32) && (!memcmp_c(objHash, objHashStr.str, sizeof(hash_t))))
			{
				ret = 1;
			}
			free_string(&objHashStr);
		}

		release_zone_ref(&prevout);

		if (ret)
		{
			dec_zone_ref(input);
			release_zone_ref(&my_list);
			break;
		}
		release_zone_ref(prev_tx);
	}

	release_zone_ref(&txin_list);
	

	return ret;
}

OS_API_C_FUNC(int) check_tx_inputs(mem_zone_ref_ptr tx, uint64_t *total_in, mem_zone_ref_ptr inobjs,unsigned int *is_coinbase,unsigned int check_sig, mem_zone_ref_ptr mempool)
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

		/* coinbase transaction */
		if ((!memcmp_c(et, null_hash, 32)) && (et[8] >= 0xFFFF))
		{
			if ((*is_coinbase) == 0)
			{
				*is_coinbase = 1;
				ret = 1;
				continue;
			}
			release_zone_ref	(&my_list);
			dec_zone_ref		(input);
			release_zone_ref	(&txin_list);
			return 0;
		}

		/* operands */
		else if (!memcmp_c(et, ff_hash, 32))
		{
			struct string script = { 0 };

			if (!tree_manager_get_child_value_istr(input, NODE_HASH("script"), &script, 0))
			{
				release_zone_ref(&my_list);
				dec_zone_ref(input);
				release_zone_ref(&txin_list);
				return 0;
			}

			/* var operand */
			if (et[8] == 0)
			{
				struct string var_name = { 0 };
				size_t offset = 0;

				var_name = get_next_script_var(&script, &offset);

				ret = ((var_name.str != PTR_NULL) && (var_name.len > 0)) ? 1 : 0;
				if(ret)ret = tree_manager_set_child_value_vstr(input, "var_name", &var_name);
				free_string(&var_name);
			}
			/* const operand */
			else
			{
				mem_zone_ref value = { PTR_NULL }, tmp = { PTR_NULL };

				ret = tree_manager_create_node("imvalue", et[8], &value);
				if (ret)
				{
					switch (et[8])
					{
						case NODE_GFX_BINT:
						case NODE_GFX_SIGNED_BINT:
						case NODE_GFX_INT:
						case NODE_GFX_SIGNED_INT:
						{
							mem_zone_ref tmp = { PTR_NULL };

							ret = tree_manager_create_node("value", NODE_BITCORE_VINT, &tmp);
							if (ret)ret = (read_node(&tmp, script.str, script.len) != INVALID_SIZE) ? 1 : 0;
							if (ret)ret = tree_manager_write_node_vint(&value, 0, (const_mem_ptr)script.str);
							release_zone_ref(&tmp);
						}
						break;
						case NODE_GFX_FLOAT:
							ret = (read_node(&value, script.str, script.len) != INVALID_SIZE) ? 1 : 0;
						break;
						default:
							ret = 0;
						break;
					}
					if (ret)ret = tree_manager_node_add_child(input, &value);
					release_zone_ref(&value);
				}
			}
			free_string(&script);
		}
		/* new application */
		else if ((has_root_app == 1) && (!memcmp_c(et, app_root_hash, sizeof(hash_t))))
		{
			struct string script = { 0 };

			ret = (has_app == 0) ? 1 : 0;

			/* get application name from input script */
			if (ret)ret = tree_manager_get_child_value_istr(input, NODE_HASH("script"), &script, 0);
			if (ret)ret = get_app_name(&script, &appName);
			if (ret)ret = (appName.len >= 3) ? 1 : 0;
			if (ret)ret = (appName.len < 32) ? 1 : 0;
			
			/* set transaction as application */
			if (ret)ret = tree_manager_set_child_value_str(tx, "AppName", appName.str);

			/* set application information in the input */
			if (ret)ret = tree_manager_set_child_value_bool(input, "isApp", 1);
			if (ret)ret = tree_manager_set_child_value_str(input, "appName", appName.str);

			free_string(&appName);
			free_string(&script);

			if(ret)has_app = 1;
		}
		/* new application item */
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
				mem_zone_ref	prevout = { PTR_NULL }, app = { PTR_NULL };
			
				/* get app name from parent transaction */
				tree_manager_get_child_value_istr	(&prev_tx, NODE_HASH("appName"), &appName,0);

				log_output							("new app item for '");
				log_output							(appName.str);
				log_output							("'\n");

				/* check application configuration */
				ret = tree_manager_find_child_node(&apps, NODE_HASH(appName.str), NODE_BITCORE_TX, &app);
				if (ret)
				{
					unsigned int locked;

					if (!tree_manager_get_child_value_i32(&app, NODE_HASH("locked"), &locked))
						locked = 0;

					/* check application roots */
					get_tx_output					 (&prev_tx, et[8], &prevout);
					tree_manager_get_child_value_istr(&prevout, NODE_HASH("script"), &oscript, 0);

					switch (app_item)
					{
						/* type, layouts, modules */
						case 1:
						case 4:
						case 5:
						{
							/* check that signature correpond to app master */

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
						/* object */
						case 2:
						{
							struct string script = { 0 }, pkey = { 0 };
							struct string sign = { 0 }, bsign = { 0 };

							/* check object's signature */

							tree_manager_get_child_value_istr(input, NODE_HASH("script"), &script, 0);
							ret = get_insig_info(&script, &sign, &pkey, &hash_type);
							if (ret) ret = (pkey.len == 0) ? 1 : 0;
							if (ret) ret = get_out_script_address(&oscript, PTR_NULL, addr);

							/* check application permission */
							if ((ret)&&(locked))
							{
								btc_addr_t appAddr;
								ret = tree_manager_get_child_value_btcaddr(&app, NODE_HASH("appAddr"), appAddr);
								if (ret)ret = (memcmp_c(addr, appAddr, sizeof(btc_addr_t)) == 0) ? 1 : 0;
							}


							/* set object informations in the transaction */
							if (ret) ret = tree_manager_set_child_value_vstr(tx, "appObj", &appName);
							if (ret) ret = tree_manager_set_child_value_vstr(tx, "ObjSign", &sign);
							if (ret) ret = tree_manager_set_child_value_i32(tx, "app_item", app_item);

							/* set application address in the input */
							if (ret) ret = tree_manager_set_child_value_btcaddr(input, "srcaddr", addr);
							if (ret) is_app_item = 1;

							/* add object to the transaction balance */
							if ((ret)&&(inobjs!=PTR_NULL))
							{
								hash_t			txh;
								mem_zone_ref	objHash = { PTR_NULL };

								ret = tree_manager_get_child_value_hash(tx, NODE_HASH("txid"), txh);

								if (ret)ret = (tree_find_child_node_by_hash(inobjs, NODE_BITCORE_HASH, txh, PTR_NULL)==0)?1:0;
								if (ret)
								{
									if (tree_manager_add_child_node(inobjs, "obj", NODE_BITCORE_HASH, &objHash))
									{
										tree_manager_write_node_hash(&objHash, 0, txh);
										release_zone_ref(&objHash);
									}
								}
							}

							free_string(&script);
							free_string(&sign);
							free_string(&pkey);

						}
						break;
						/* file */
						case 3:
							tree_manager_set_child_value_vstr(tx, "appFile", &appName);
							ret = tree_manager_set_child_value_i32(tx, "app_item", app_item);
							if (ret)is_app_item = 1;
						break;
					}
					free_string(&oscript);
					release_zone_ref(&prevout);
					release_zone_ref(&app);


					
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
		/* new application object child */
		else if (is_obj_child((unsigned char *)et, et[8], &prev_tx, &appName, mempool) == 1)
		{
		
			struct string		script = { PTR_NULL };

			tree_manager_get_child_value_istr(input, NODE_HASH("script"), &script, 0);

			/* object key reference operand */
			if (et[8] == NODE_GFX_OBJECT)
			{
				struct string keyName = { 0 };
				size_t offset = 0;

				keyName = get_next_script_var(&script, &offset);
				ret = (keyName.len >0) ? 1 : 0;

				/* set object reference in the input */
				if (ret)ret = tree_manager_set_child_value_vstr(input, "appName", &appName);
				if (ret)ret = tree_manager_set_child_value_vstr(input, "keyName", &keyName);
				if (ret)ret = tree_manager_set_child_value_hash(input, "objHash", (unsigned char *)et);

				if (ret)
				{
					char			oh[65];
					mem_zone_ref	obj = { PTR_NULL }, objtx = { PTR_NULL };

					/* load object */
					bin_2_hex((unsigned char *)et, sizeof(hash_t), oh);

					ret = tree_find_child_node_by_member_name_hash(mempool, NODE_BITCORE_TX, "txid", (unsigned char *)et, &objtx);
					if (ret)
					{
						ret = tree_manager_find_child_node(&objtx, NODE_HASH("objDef"), 0xFFFFFFFF, &obj);
						release_zone_ref(&objtx);
					}
					else
					{
						ret = load_obj(appName.str, oh, "obj", 0, &obj, PTR_NULL);
						if (!ret)
						{
							log_output("obj ref missing '");
							log_output(oh);
							log_output("\n'");
						}
					}

					/* check that object contains the key */
					if (ret)ret = tree_manager_find_child_node(&obj, NODE_HASH(keyName.str), 0xFFFFFFFF, PTR_NULL);
					release_zone_ref(&obj);
				}

				free_string(&keyName);
			}
			/* object child */
			else
			{
				char				phash[65];	
				struct string		sign = { PTR_NULL }, vpubK = { PTR_NULL };
				struct string		oscript = { 0 };
				mem_zone_ref		prevout = { PTR_NULL };
				uint64_t			val;
				unsigned int		type_id;
				int					is_signed, utxo;
				unsigned char		hash_type;

				/* check object's signature */

				ret = get_tx_output(&prev_tx, et[8], &prevout);
				if(ret) ret = tree_manager_get_child_value_istr(&prevout, NODE_HASH("script"), &oscript, 0);

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

						struct string oss = { PTR_NULL };

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

				tree_manager_get_child_value_i64(&prevout, NODE_HASH("value"), &val);
				type_id = val & 0xFFFFFFFF;

				/* set object signature flag in the transaction */
				tree_manager_set_child_value_bool(tx, "pObjSigned", is_signed);
				tree_manager_set_child_value_i32 (tx, "pObjType", type_id);
				tree_manager_set_child_value_vstr(tx, "appChild", &appName);
				tree_manager_set_child_value_hash(tx, "appChildOf", (unsigned char *)et);

				/* check parent's utxo */
				bin_2_hex((unsigned char *)et, 32, phash);
				utxo = check_utxo(phash, et[8]);

				/* set object's information in the input */
				tree_manager_set_child_value_i32 (input, "isObjChild", 1);
				tree_manager_set_child_value_i32 (input, "hasUTXO", utxo);
				tree_manager_set_child_value_i64 (input, "value", amount);
				tree_manager_set_child_value_hash(input, "objHash", (unsigned char *)et);
				tree_manager_set_child_value_vstr(input, "srcapp", &appName);

				/* add object in the transaction balance */
				if (inobjs != PTR_NULL)
				{
					mem_zone_ref objHash = { PTR_NULL };

					ret = (tree_find_child_node_by_hash(inobjs, NODE_BITCORE_HASH, (unsigned char *)et, PTR_NULL) == 0) ? 1 : 0;
					if (ret)
					{
						if (tree_manager_add_child_node(inobjs, "obj", NODE_BITCORE_HASH, &objHash))
						{
							tree_manager_write_node_hash(&objHash, 0, (unsigned char *)et);
							release_zone_ref(&objHash);
						}
					}
				}
				
				release_zone_ref(&prevout);
				free_string(&vpubK);
				free_string(&sign);
				free_string(&oscript);
			}

			free_string							(&script);
			release_zone_ref					(&prev_tx);
			free_string							(&appName);
		}
		else if (prev_tx.zone != PTR_NULL)
		{
			char			phash[65];
			struct string	oscript = { PTR_NULL }, script = { PTR_NULL }, sym_name = { PTR_NULL };
			mem_zone_ref	prevout = { PTR_NULL }, txouts = { PTR_NULL };
			unsigned char	*pet = (unsigned char *)et;
			
			
			ret = get_tx_output								(&prev_tx, et[8], &prevout);
			if(ret)ret = tree_manager_get_child_value_istr	(&prevout, NODE_HASH("script"), &oscript, 0);
			
			/* check is previous tx is opcode reference */
			if (ret)
			{
				int opcode;

				opcode = is_opfn_script(&oscript, &sym_name);
				
				if ( opcode != 0 )
				{
					switch (opcode)
					{
						case 0xFF: ret = tree_manager_set_child_value_vstr(input, "op_name", &sym_name); break;	
						case 0xFE: ret = tree_manager_set_child_value_vstr(input, "fn_name", &sym_name); break;
					}

					free_string	(&sym_name);

					if (ret)
					{
						free_string		(&oscript);
						release_zone_ref(&prevout);
						release_zone_ref(&prev_tx);
						continue;
					}
				}
			}


			/* check valid utxo */
			if (ret)
			{
				bin_2_hex(pet, 32, phash);
				ret = check_utxo(phash, et[8]);
			}

			if (!ret)
			{
				char	rphash[65];
				char	iStr[16];

				bin_2_hex_r(pet, 32, rphash);
				
				uitoa_s		(et[8], iStr, 16, 10);

				log_output	("bad utxo '");
				log_output	(rphash);
				log_output	("' - ");
				log_output	(iStr);
				log_output	("\n");
			}

			if (ret)ret = tree_manager_get_child_value_istr(input, NODE_HASH("script"), &script, 0);

			if (ret)
			{
				struct string	sign = { PTR_NULL }, sigseq = { PTR_NULL }, vpubK = { PTR_NULL };
				mem_zone_ref	txTmp = { PTR_NULL };
				size_t			offset = 0;
				unsigned char	hash_type;

			
				/* check input signature */
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
						if (!ret)
						{
							char hex[128];

							log_output("check input pkey hash failed\n");
							bin_2_hex(vpubK.str,33,hex);
							log_output(hex);
							log_output("\n");

							log_output(addr);
							log_output("\n");
						}
					}
					if ( (ret)&& (check_sig & 1))
					{
						hash_t txh;
						ret = compute_tx_sign_hash(tx, iidx, &oscript, hash_type, txh);
						if (ret)ret = check_sign(&sign, &vpubK, txh);
					
					}

					/* check object second hand transfer */
					if (ret) ret = tree_manager_get_child_value_i64(&prevout, NODE_HASH("value"), &amount);
					if (ret && ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000))
					{
						struct string objHashStr = { 0 };

						ret = get_out_script_return_val(&oscript, &objHashStr);

						if (ret) ret = (objHashStr.len == 32) ? 1 : 0;
						if (ret) ret = tree_manager_set_child_value_hash(input, "srcObj", objHashStr.str);
						
						/* add object to the transaction balance */

						if ( ret && (inobjs != PTR_NULL) )
						{
							mem_zone_ref objHash = { PTR_NULL };

							ret = (tree_find_child_node_by_hash(inobjs, NODE_BITCORE_HASH, objHashStr.str, PTR_NULL) == 0) ? 1 : 0;
							if (ret)
							{
								if (tree_manager_add_child_node(inobjs, "obj", NODE_BITCORE_HASH, &objHash))
								{
									tree_manager_write_node_hash(&objHash, 0, objHashStr.str);
									release_zone_ref(&objHash);
								}
							}
						}
						
						free_string(&objHashStr);
						
						amount = 0;
					}

					/* coin transfer */
					if (ret)ret = tree_manager_set_child_value_btcaddr	(input, "srcaddr", addr);
					if (ret)ret = tree_manager_set_child_value_i64		(input, "value", amount);
					
					free_string	(&vpubK);
					free_string	(&sign);
				}

				if ((ret) && (check_sig & 2))
				{
					ret = bt_insert(&blk_root, et);
					if(!ret) log_output("double spent found \n");
				}
			}

			free_string		(&oscript);
			release_zone_ref(&prevout);
			free_string		(&script);
			release_zone_ref(&prev_tx);
		}

		if (!ret)
		{
			char myTx[65];
			char prevTx[65];
			char iStr[16];
			hash_t mh;
			unsigned char	*pet = (unsigned char *)et;


			if (tree_manager_get_child_value_hash(tx, NODE_HASH("txid"), mh))
				bin_2_hex_r(mh, 32, myTx);
			else
			{
				memset_c(myTx, '?', 64);
				myTx[64] = 0;
			}

			bin_2_hex_r		(pet, 32, prevTx);
			

			itoa_s			(iidx, iStr, 16, 10);
			log_output		("check input failed at input #");
			log_output		(iStr);
			log_output		(" tx : '");
			log_output		(prevTx);
			log_output		("' in '");
			log_output		(myTx);
			log_output		("'\n");

			release_zone_ref	(&my_list);
			dec_zone_ref		(input);
			break;
		}
		(*total_in) += amount;
	}
	release_zone_ref(&txin_list);
	return ret;
}




OS_API_C_FUNC(int) check_tx_outputs(mem_zone_ref_ptr tx,uint64_t *total, mem_zone_ref_ptr outobjs, unsigned int *is_staking,unsigned int flags)
{
	hash_t				txh;
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

	tree_manager_get_child_value_hash(tx, NODE_HASH("txid"), txh);

	for (idx = 0, tree_manager_get_first_child(&txout_list, &my_list, &out); ((out != NULL) && (out->zone != NULL)); idx++, tree_manager_get_next_child(&my_list, &out))
	{
		btc_addr_t		addr;
		hash_t			pObjH;
		struct	string	script = { 0 }, pubk = { 0 }, sym_name = { 0 };
		uint64_t		amount = 0;
		int				opcode;
		unsigned int	app_item;

		ret = 1;

		if (!tree_manager_get_child_value_i64 (out, NODE_HASH("value"), &amount))continue;
		if (!tree_manager_get_child_value_istr(out, NODE_HASH("script"), &script, 16))continue;

		/* null output for staking transaction */
		if ((idx == 0) && (amount == 0) && (script.str[0] == 0))
		{
			*is_staking = 1;
			free_string(&script);
			continue;
		}

		/* operation / function  */
		opcode = is_opfn_script(&script, &sym_name);
		if (opcode != 0)
		{
			switch (opcode)
			{
				case 0xFF: ret = tree_manager_set_child_value_vstr(out, "op_name", &sym_name); break;
				case 0xFE: ret = tree_manager_set_child_value_vstr(out, "fn_name", &sym_name); break;
			}

#ifdef _DEBUG
			log_output("new op '");
			log_output(sym_name.str);
			log_output("'\n");
#endif

			if(flags & 0x01)
				tree_manager_node_add_child(&blkobjs, tx);

			free_string(&sym_name);
			continue;
		}
		

		/* get output address  */
		if (get_out_script_address(&script, &pubk, addr))
		{
			tree_manager_set_child_value_btcaddr(out, "dstaddr", addr);
			free_string(&pubk);
		}

		/* new application roots */
		if (tree_manager_find_child_node(tx, NODE_HASH("AppName"), NODE_GFX_STR, PTR_NULL))
		{
			struct string my_val = { PTR_NULL };

			/* check application root's */
			if (get_out_script_return_val(&script, &my_val))
			{
				ret = (my_val.len == 1) ? 1 : 0;
				if (ret)
				{
					unsigned char app_code = *((unsigned char*)(my_val.str));
					switch (app_code)
					{
						/* application type definition root */
						case 1:app_flags |= 1; break;
						/* application object root */
						case 2:app_flags |= 2;break;
						/* application files root */
						case 3:app_flags |= 4; break;
						/* application layouts root */
						case 4:app_flags |= 8; break;
						/* application modules root */
						case 5:app_flags |= 16; break;
						/* other fail */
						default:ret = 0; break;
					}
				}
				free_string(&my_val);
			}
			/* application creation fees to the root addr */
			else if (!memcmp_c(addr, root_app_addr, sizeof(btc_addr_t)))
			{
				uint64_t root_amnt;
				if (!tree_manager_get_child_value_i64(tx, NODE_HASH("app_root_amnt"), &root_amnt))root_amnt = 0;
				root_amnt += amount;
				tree_manager_set_child_value_i64(tx, "app_root_amnt", root_amnt);
			}
		}
		/* application item creation */
		else if ((idx == 0)&&(tree_manager_get_child_value_i32(tx, NODE_HASH("app_item"), &app_item)))
		{
			switch (app_item)
			{
				case 1:
					/* new type definition */
					if (amount == 0)
					{
						char			typeName[32];
						unsigned int	TypeId, flags;
						ret = get_type_infos(&script, typeName, &TypeId,&flags);
					}
				break;
				case 2:
					/* new object */

					/*
					if (idx != 0)
					{
						log_output("object definition must be at output #0 \n");
						ret = 0;
						break;
					}
					*/

					ret = ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000) ? 1 : 0;

					if (ret)
					{
						char			app_name[32];
						hash_t			sh;
						struct string	bsign = { 0 }, pkey = { 0 };
						unsigned int	type_id;

						type_id = amount & 0xFFFFFFFF;

						/* get object information from the tx's input */
						tree_manager_get_child_value_str	(tx , NODE_HASH("appObj"), app_name, 32, 0);
						tree_manager_get_child_value_istr	(tx , NODE_HASH("ObjSign"), &bsign,0);


						/* check object signature */
						tree_manager_get_child_value_istr	(out, NODE_HASH("script"), &script, 0);
						get_out_script_address				(&script, &pkey, addr);
						ret = (pkey.len == 33) ? 1 : 0;
						if (ret)ret = compute_tx_sign_hash	(tx, 0, &script, 1, sh);
						if (ret)ret = check_sign			(&bsign, &pkey, sh);

						free_string(&bsign);
						free_string(&pkey);
						
						if (ret)
						{
							mem_zone_ref types = { PTR_NULL }, type = { PTR_NULL }, myobj = { PTR_NULL }, app = { PTR_NULL };

							/* get type definition from application */
							ret=tree_manager_find_child_node(&apps, NODE_HASH(app_name), NODE_BITCORE_TX, &app);
							if (ret)ret = get_app_types						(&app, &types);
							if (ret)ret = tree_find_child_node_by_id_name	(&types, NODE_BITCORE_TX, "typeId", type_id, &type);

							/* unserialize script data to object using type definition */
							if (ret)ret = obj_new							(&type, "objDef", &script, &myobj);

							/* set owner address in the object */
							if (ret)ret = tree_manager_set_child_value_btcaddr(&myobj, "objAddr", addr);

							/* check unique key in the object data */
							if (ret)ret = check_app_obj_unique				(app_name,type_id,&myobj);

							/* add object definition to the transaction */
							if (ret)ret = tree_manager_node_add_child		(tx, &myobj);
							if (ret)ret = tree_manager_set_child_value_i32	(tx, "objType", type_id);

							/* set object object infos in the utxo */
							if (ret)ret = tree_manager_set_child_value_hash (out, "objHash", txh);
							if (ret)ret = tree_manager_set_child_value_hash (out, "appName", app_name);

							/* add object in the block's list */
							if ((ret)&&(flags & 0x01)) ret = tree_manager_node_add_child(&blkobjs, tx);

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
					/* new file */
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
					/* new layout */
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
					/* new module */
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
			char app_name[32];
			struct string DataStr = { 0 };

			amount = 0;
			
			/* genesis object transfer script, return value contain object hash */
			if (get_out_script_return_val(&script, &DataStr))
			{
				btc_addr_t		dstAddr;
				unsigned int	is_signed;

				ret = (DataStr.len == 32) ? 1 : 0;
				/* get object destination address */
				if (ret)ret = get_out_script_address				(&script, PTR_NULL, dstAddr);
				/* check signature for object transfer transactions */
				if (ret)ret = tree_manager_get_child_value_i32		(tx, NODE_HASH("pObjSigned"), &is_signed);
				if (ret)ret = (is_signed != 0) ? 1 : 0;

				/* set the obj transfer flag in the transaction */
				if (ret)ret = tree_manager_set_child_value_bool		(tx, "objTxfr", 1);

				/* get object's application name from input parsing */
				if (ret)ret = tree_manager_get_child_value_str		(tx, NODE_HASH("appChild"), app_name, 32, 0);

				/* set the application name in the utxo object */
				if (ret)ret = tree_manager_set_child_value_hash		(out, "appName", app_name);

				/* set the object hash from the script in the utxo object */
				if (ret)ret = tree_manager_set_child_value_hash		(out, "objHash", DataStr.str);

				/* set the destination address in the utxo object */
				if (ret)ret = tree_manager_set_child_value_btcaddr	(out, "dstAddr", dstAddr);

				/* add the object in the transaction balance */
				if ((ret) && (outobjs != PTR_NULL))
				{
					mem_zone_ref objHash = { PTR_NULL };

					/* check if the object is not already transfered in the current block */
					ret = (tree_find_child_node_by_hash(outobjs, NODE_BITCORE_HASH, DataStr.str, PTR_NULL) == 0) ? 1 : 0;
					if (ret)
					{
						/* add the object in the current block */
						if (tree_manager_add_child_node(outobjs, "obj", NODE_BITCORE_HASH, &objHash))
						{
							tree_manager_write_node_hash(&objHash, 0, DataStr.str);
							release_zone_ref(&objHash);
						}
					}
				}

				/* debug informations */
#ifdef _DEBUG
				if (ret)
				{
					char objHex[65];
					bin_2_hex(DataStr.str, 32, objHex);
					log_output("new obj transfer '");
					log_output(objHex);
					log_output("' to '");
					log_output(dstAddr);
					log_output("'\n");
				}
#endif
			}
			/*  add child object transaction, script contain parent object's key and child object hash */
			else
			{
				struct string key = { 0 }, cHash = { 0 };
				size_t offset = 0;

				/* get parent object's key from the script */
				ret = get_script_data(&script, &offset, &key);

				/* get new child object hash from the script */
				if (ret)ret = get_script_data(&script, &offset, &cHash);
				if (ret)ret = (cHash.len == 32) ? 1 : 0;

				if (ret)
				{

					struct string	appName;
					unsigned int	ptype, ktype, is_signed, flags;
					unsigned char	*hd = (unsigned char *)cHash.str;

#ifdef _DEBUG
					char			chash[65];
					bin_2_hex(hd, 32, chash);

					log_output("new obj child '");
					log_output(chash);
					log_output("':'");
					log_output(key.str);
					log_output("'\n");
#endif

					/* get object information from set in the tx from the inputs */

					if (!tree_manager_get_child_value_i32(tx, NODE_HASH("pObjSigned"), &is_signed))
						is_signed = 0;

					tree_manager_get_child_value_i32(tx, NODE_HASH("pObjType"), &ptype);
					tree_manager_get_child_value_istr(tx, NODE_HASH("appChild"), &appName, 0);

					/* check parent's key type */
					ret = get_app_type_key(appName.str, ptype, key.str, &ktype, &flags);

					if (ret)ret = ((ktype == NODE_JSON_ARRAY) || (ktype == NODE_PUBCHILDS_ARRAY)) ? 1 : 0;

					/* private key type require signature */
					if ((ret) && ((ktype == NODE_JSON_ARRAY) && (!is_signed)))ret = 0;

					/* set new child parenting informations in the transaction */
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

		}
		/* second hand object transfer output */
		else if ((amount & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000)
		{
			struct string DataStr = { 0 };

			if (get_out_script_return_val(&script, &DataStr))
			{
				if (DataStr.len == 32)
				{
					tree_manager_set_child_value_hash(out, "dstObj", DataStr.str);
				}
				free_string(&DataStr);
			}
			amount = 0;
		}

		if (ret) 
			*total += amount;

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
OS_API_C_FUNC(int) find_inner_inputs(mem_zone_ref_ptr txin_list, hash_t txid, unsigned int oidx)
{
	mem_zone_ref		my_ilist = { PTR_NULL };
	mem_zone_ref_ptr	input = PTR_NULL;
	int ret = 0;

	for (tree_manager_get_first_child(txin_list, &my_ilist, &input); ((input != PTR_NULL) && (input->zone != PTR_NULL)); tree_manager_get_next_child(&my_ilist, &input))
	{
		hash_t			th;
		unsigned int	idx;
		tree_manager_get_child_value_hash(input, NODE_HASH("txid"), th);
		tree_manager_get_child_value_i32(input, NODE_HASH("idx"), &idx);

		if (!memcmp_c(txid, th, sizeof(hash_t)) && (idx == oidx))
		{
			dec_zone_ref(input);
			release_zone_ref(&my_ilist);
			ret = 1;
			break;
		}
	}

	return ret;
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

		ret = find_inner_inputs(&txin_list, txid, oidx);
		
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

int check_object_balance(mem_zone_ref_const_ptr inobjs, mem_zone_ref_const_ptr outobjs)
{
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref_ptr	outobj = PTR_NULL;
	int ret;

	for (tree_manager_get_first_child(outobjs, &my_list, &outobj); ((outobj != NULL) && (outobj->zone != NULL)); tree_manager_get_next_child(&my_list, &outobj))
	{
		hash_t outhash;
		ret = tree_manager_get_node_hash(outobj, 0, outhash);

		if (!tree_find_child_node_by_hash(inobjs, NODE_BITCORE_HASH, outhash, PTR_NULL))
		{
			dec_zone_ref(outobj);
			release_zone_ref(&my_list);
			return 0;
		}
	}

	return 1;

}


OS_API_C_FUNC(int) compute_txlist_size(mem_zone_ref_const_ptr tx_list,size_t *totalSize)
{
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref_ptr	tx = PTR_NULL;
	

	*totalSize = 0;
	for (tree_manager_get_first_child(tx_list, &my_list, &tx); ((tx != PTR_NULL) && (tx->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &tx))
	{
		size_t				txSz;

		if (!tree_manager_get_child_value_i32(tx, NODE_HASH("size"), &txSz))
			continue;

		*totalSize += txSz;
	}
	return 1;
}

OS_API_C_FUNC(int) check_tx_list(mem_zone_ref_ptr tx_list,uint64_t block_reward,hash_t merkle, unsigned int block_time,unsigned int check_sig)
{
	hash_t				merkleRoot;
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref_ptr	tx = PTR_NULL;
	uint64_t			list_reward;
	uint64_t			txFee, fees;
	unsigned int		coinbase, coinstaking, is_staking, is_coinbase;
	int					ret;


	build_merkel_tree	(tx_list, merkleRoot,PTR_NULL);

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
		mem_zone_ref		inobjs = { PTR_NULL }, outobjs = { PTR_NULL };
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
			tree_manager_create_node("inobjs", NODE_BITCORE_HASH_LIST, &inobjs);
			ret = check_tx_inputs(tx, &total_in, &inobjs, &is_coinbase, check_sig | 2,PTR_NULL);
			if(!ret)log_output("invalid inputs \n");

		}
		if (ret)
		{
			tree_manager_create_node("inobjs", NODE_BITCORE_HASH_LIST, &outobjs);
			ret = check_tx_outputs(tx, &total_out, &outobjs, &is_staking,1);
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
			ret=check_object_balance(&inobjs, &outobjs);
			if(!ret)
				log_output("wrong object balance\n");
		}

		release_zone_ref(&inobjs);
		release_zone_ref(&outobjs);


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
//	if (!tree_manager_get_child_value_hash(hdr, NODE_HASH("blk pow"), blk_pow))
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
	hash_t prevHash;
	
	if (!tree_manager_get_child_value_hash(header, NODE_HASH("prev"), prevHash))return 0;
	return load_block_time(prevHash, time);
}






OS_API_C_FUNC(int) block_has_pow(mem_zone_ref_ptr blockHash)
{
	hash_t hash;
	if (!tree_manager_get_node_hash(blockHash, 0, hash))return 0;
	return is_pow_block(hash);
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
	build_merkel_tree			(txs, merkle, PTR_NULL);
	
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

	if (!find_block_hash(blk_pow, 1, PTR_NULL))
	{
		store_block		(genesis, &txs);
		block_add_index(blk_pow, 0, time);
	}
		

	release_zone_ref(&txs);
	return 1;

}


OS_API_C_FUNC(int) get_tx_data(mem_zone_ref_ptr tx, mem_zone_ref_ptr txData)
{
	struct string	txdata = { 0 };
	size_t			size,sz;
	uint64_t		fee;
	unsigned char	*buffer,*end;
	hash_t txh;

	size	= get_node_size(tx);
	buffer	= malloc_c(size);
	end		= write_node(tx, buffer);

	sz		= mem_sub(buffer, end);

	txdata.len	= size * 2;
	txdata.size = txdata.len + 1;
	txdata.str	= malloc_c(txdata.size);
	
	bin_2_hex	(buffer, size, txdata.str);

	txdata.str[txdata.len] = 0;

	free_c(buffer);

	if (!tree_manager_get_child_value_i64(tx, NODE_HASH("fee"), &fee))
		fee = 0;
	tree_manager_get_child_value_hash	(tx, NODE_HASH("txid"), txh);

	tree_manager_set_child_value_vstr	(txData, "data", &txdata);
	tree_manager_set_child_value_i64	(txData, "fee", fee);
	tree_manager_set_child_value_hash	(txData, "hash", txh);
	tree_manager_set_child_value_hash	(txData, "txid", txh);
	tree_manager_set_child_value_bool	(txData, "required", 1);

	free_string(&txdata);

	return 1;
}





OS_API_C_FUNC(int) get_block_hash(uint64_t height, hash_t hash)
{
	return tree_manager_get_node_hash(&block_index_node, mul64(height,sizeof(hash_t)), hash);
}

OS_API_C_FUNC(int) get_block_time(uint64_t height, unsigned int *time)
{
	return tree_mamanger_get_node_dword(&time_index_node, mul64(height, sizeof(unsigned int )), time);
}

OS_API_C_FUNC(int) get_block_hash_str(uint64_t height, char *hash,size_t len)
{
	return tree_manager_get_node_str(&block_index_node, mul64(height, sizeof(hash_t)), hash, len, 16);
}

int cmph(const unsigned int *h1, const unsigned int *h2)
{
	int	n, ret = 0;
	n = 8;
	while (n--)
	{
		if (h1[n] < h2[n])
		{
			ret = -1;
			break;
		}
		if (h1[n] > h2[n])
		{
			ret = 1;
			break;
		}
	}

	return ret;
}


// A recursive binary search function. It returns 
// location of x in given array arr[l..r] is present, 
// otherwise -1
int binarySearch(mem_zone_ref_const_ptr indexes, const unsigned char *hashes, int l, int r, const hash_t x)
{
	if (r >= l)
	{
		uint64_t	index;
		int mid = l + (r - l) / 2;
		int ret;

		if (!tree_mamanger_get_node_qword(indexes, mul64(mid, sizeof(uint64_t)), &index))
			return -1;

		ret = cmph((unsigned int *)&hashes[mul64(index,32)], (unsigned int *)x);

		// If the element is present at the middle 
		// itself
		if (ret == 0)
			return mid;

		// If element is smaller than mid, then 
		// it can only be present in left subarray
		if (ret>0)
			return binarySearch(indexes, hashes, l, mid - 1, x);

		// Else the element can only be present
		// in right subarray
		return binarySearch(indexes, hashes,  mid + 1, r, x);
	}

	// We reach here when element is not 
	// present in array
	return -1;
}

OS_API_C_FUNC(int) find_block_hash(const hash_t h, uint64_t block_height, uint64_t *height)
{
	unsigned char   *hashes;
	int				ret = 0;

	aquire_lock_excl(&sort_lock, 1);

	hashes = tree_mamanger_get_node_data_ptr(&block_index_node, 0);
	ret = binarySearch(&block_rindex_node, hashes, 0, block_height-1, h);

	release_lock_excl(&sort_lock);

	if (ret < 0)
		return 0;
	
	if (height != PTR_NULL)
	{
		uint64_t *indexes; 
		indexes = tree_mamanger_get_node_data_ptr(&block_rindex_node, 0);
		*height = indexes[ret];
	}
		
	
	return 1;
}


// A recursive binary search function. It returns 
// location of x in given array arr[l..r] is present, 
// otherwise -1
int binarySearchg(mem_zone_ref_ptr indexes, unsigned char *hashes, int l, int r, hash_t x, uint64_t *next_block)
{
	if (r >= l)
	{
		uint64_t	index;
		int mid = l + (r - l) / 2;
		int ret;

		if (!tree_mamanger_get_node_qword(indexes, mul64(mid, sizeof(uint64_t)), &index))
			return -1;

		ret = cmph((unsigned int *)&hashes[mul64(index, 32)], (unsigned int *)x);

		// If the element is present at the middle 
		// itself
		if (ret == 0)
			return mid;

		// If element is smaller than mid, then 
		// it can only be present in left subarray
		if (ret>0)
			return binarySearchg(indexes, hashes, l, mid - 1, x, next_block);

		// Else the element can only be present
		// in right subarray
		return binarySearchg(indexes, hashes, mid + 1, r, x, next_block);
	}

	*next_block = l;

	// We reach here when element is not 
	// present in array
	return -1;
}

int add_sorted_block(uint64_t src_h, uint64_t block_height)
{
	uint64_t n, index;
	unsigned char *hashes;
	int			 ret;


	if (block_height == 0)
	{
		tree_manager_write_node_qword(&block_rindex_node, 0, src_h);
		return 1;
	}

	
	hashes = tree_mamanger_get_node_data_ptr(&block_index_node, 0);
	ret = binarySearchg(&block_rindex_node, hashes, 0, block_height-1, &hashes[mul64(src_h, 32)], &index);
	if (ret >= 0)
		return 1;

	while (!compare_z_exchange_c(&sort_lock, 1)) {}

	if (!tree_manager_expand_node_data_ptr(&block_rindex_node, mul64(block_height, sizeof(uint64_t)), sizeof(uint64_t)))
	{
		sort_lock = 0;
		return 0;
	}
	
	for (n = block_height; n > index; n--)
	{
		uint64_t	idx;
		tree_mamanger_get_node_qword(&block_rindex_node, mul64(n-1, sizeof(uint64_t)), &idx);
		tree_manager_write_node_qword(&block_rindex_node, mul64(n, sizeof(uint64_t)), idx);
	}
	
	tree_manager_write_node_qword(&block_rindex_node, mul64(index, sizeof(uint64_t)), src_h);

	
	sort_lock = 0;

	return 1;
}


int comp_bh(const_mem_ptr *a, const_mem_ptr *b)
{
	uint64_t		i1, i2;
	unsigned int	*hash, *hash2;
	int				ret = 0;


	i1 = mul64(*((uint64_t *)a), sizeof(hash_t));
	i2 = mul64(*((uint64_t *)b), sizeof(hash_t));

	hash = tree_mamanger_get_node_data_ptr(&block_index_node, i1);
	hash2 = tree_mamanger_get_node_data_ptr(&block_index_node, i2);

	return cmph(hash, hash2);
}

int check_sorted_block_index(uint64_t height)
{
	uint64_t n;
	int ret = 1;

	for (n = 0; n < height; n++)
	{
		hash_t		h1;
		uint64_t	bhght;
		tree_manager_get_node_hash(&block_index_node, mul64(n, sizeof(hash_t)), h1);

		if (!find_block_hash(h1, height, &bhght))
		{
			ret = 0;
			log_output("error blk index \n");
			break;
		}

		if (bhght != n)
		{
			ret = 0;
			log_output("error blk height\n");
			break;
		}

		if (n > 0)
		{
			hash_t h2;
			uint64_t pi;

			tree_mamanger_get_node_qword(&block_rindex_node, mul64(n, sizeof(uint64_t)), &pi);
			tree_manager_get_node_hash(&block_index_node, mul64(pi, sizeof(hash_t)), h1);

			tree_mamanger_get_node_qword(&block_rindex_node, mul64(n - 1, sizeof(uint64_t)), &pi);
			tree_manager_get_node_hash(&block_index_node, mul64(pi, sizeof(hash_t)), h2);
			if (cmph((unsigned int *)h1, (unsigned int *)h2) < 0)
			{
				ret = 0;
				log_output("error blk hash position\n");
				break;
			}
		}
	}

	return ret;

}


OS_API_C_FUNC(int) dump_sorted_hashes(uint64_t block_height)
{
	char hex[65];
	uint64_t n, index;

	for (n = 0; n < block_height; n++)
	{
		unsigned char *hash2;

		if (!tree_mamanger_get_node_qword(&block_rindex_node, mul64(n, sizeof(uint64_t)), &index))
			return 0;

		hash2 = tree_mamanger_get_node_data_ptr(&block_index_node, mul64(index, sizeof(hash_t)));

		bin_2_hex(hash2, 32, hex);
		log_output(hex);
		log_output("\n");
	}

	return 1;
}


OS_API_C_FUNC(int) create_sorted_block_index(uint64_t height)
{
	uint64_t		n;
	uint64_t		*indexes;

	if (block_rindex_node.zone == PTR_NULL)
		return 0;

	if (block_index_node.zone == PTR_NULL)
		return 0;

	for (n = 0; n <= height; n++)
	{
		tree_manager_write_node_qword(&block_rindex_node, mul64(n , sizeof(uint64_t)), n);
	}

	indexes = tree_mamanger_get_node_data_ptr(&block_rindex_node, 0);

	qsort_c(indexes, height, sizeof(uint64_t), comp_bh);


	return check_sorted_block_index(height);
}


OS_API_C_FUNC(int) rebuild_time_index(uint64_t height)
{
	uint64_t		cur, size;
	int				ret = 1;

	size = mul64(height, sizeof(unsigned int));

	log_message("creating block time index ... ", PTR_NULL);
	del_file("blk_times");

	if (tree_manager_expand_node_data_ptr(&time_index_node, size, 4)<=0)
	{
		log_message("could not expqnd time index memory.", PTR_NULL);
		return 0;
	}
	
	cur = 0;
	while (cur < height)
	{
		hash_t hash;
		struct string blk_path = { PTR_NULL };
		ctime_t ctime;

		if (!get_block_hash(cur, hash))
		{
			ret = 0;
			break;
		}

		if (load_block_time(hash, &ctime))
			append_file("blk_times", &ctime, sizeof(unsigned int));
		else
		{
			ret = 0;
			break;
		}
		cur++;
	}

	return ret;
}


