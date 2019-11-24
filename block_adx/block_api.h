#ifndef BLOCK_API
#define BLOCK_API C_IMPORT
#endif

struct bin_tree;
typedef struct bin_tree node;

#define ONE_COIN		100000000ULL
#define ONE_CENT		1000000ULL


/*
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
block.c
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
*/

BLOCK_API  int	C_API_FUNC ripemd160(const void* in, unsigned long length, void* out);

/* init block module */


BLOCK_API  int	C_API_FUNC init_blocks(mem_zone_ref_ptr node_config, mem_zone_ref_ptr trustedApps);

BLOCK_API  int	C_API_FUNC is_trusted_app(const char *appName);

/* 128 bit mul of 32 bits compact with 64 bits operand to 256 bit */
BLOCK_API  void	C_API_FUNC mul_compact(unsigned int nBits, uint64_t op, hash_t hash);

/* compare 256 bits hash */
BLOCK_API  int	C_API_FUNC cmp_hashle(const hash_t hash1, const hash_t hash2);

/* compute difficulty retargeting */
BLOCK_API unsigned int	C_API_FUNC calc_new_target(unsigned int nActualSpacing, unsigned int TargetSpacing, unsigned int nTargetTimespan, unsigned int pBits);

/* get block reward at given block height */
BLOCK_API  int	C_API_FUNC get_blockreward(uint64_t block, uint64_t *block_reward);

/* get hash list from block header list */
BLOCK_API  int	C_API_FUNC get_hash_list(mem_zone_ref_ptr hdr_list, mem_zone_ref_ptr hash_list);

/* build merklee root */
BLOCK_API  int	C_API_FUNC build_merkel_tree(mem_zone_ref_ptr txs, hash_t merkleRoot, mem_zone_ref_ptr branches);

/* build genesis block */
BLOCK_API  int	C_API_FUNC make_genesis_block(mem_zone_ref_ptr genesis_conf, mem_zone_ref_ptr genesis_blk);

/* get output from tx at specified index */
BLOCK_API int	C_API_FUNC   get_tx_output(mem_zone_ref_const_ptr tx, unsigned int idx, mem_zone_ref_ptr vout);

/* parse infos from signature script */
BLOCK_API  int	C_API_FUNC   get_insig_info(const struct string *script, struct string *sign, struct string *pubk, unsigned char *hash_type);

/* parse signature */
BLOCK_API  int	C_API_FUNC   parse_sig_seq(const struct string *sign_seq, struct string *sign, unsigned char *hashtype, int rev);

/* serialize script */
BLOCK_API  int	C_API_FUNC	serialize_script(mem_zone_ref_const_ptr script_node, struct string *script);

/* create null tx */
BLOCK_API  int	C_API_FUNC	create_null_tx(mem_zone_ref_ptr tx, unsigned int time, unsigned int block_height);

/* check is tx is null */
BLOCK_API  int	C_API_FUNC	is_tx_null(mem_zone_ref_const_ptr tx);

/* check is tx vout is null */
BLOCK_API  int	C_API_FUNC	is_vout_null(mem_zone_ref_const_ptr tx, unsigned int idx);

/* create new transaction with the specified time */
BLOCK_API  int	C_API_FUNC	new_transaction(mem_zone_ref_ptr tx, ctime_t time);

/* add output to the transaction */
BLOCK_API  int	C_API_FUNC	 tx_add_output(mem_zone_ref_ptr tx, uint64_t value, const struct string *script);

/* add input to the transaction */
BLOCK_API  int	C_API_FUNC	tx_add_input(mem_zone_ref_ptr tx, const hash_t tx_hash, unsigned int index,const struct string *script);

/* create paiment script */
BLOCK_API  int	C_API_FUNC	create_payment_script(const struct string *pubk, unsigned int type, mem_zone_ref_ptr script_node);

/* create paiment script with return data */
BLOCK_API int	C_API_FUNC	create_payment_script_data(const struct string *pubk, unsigned int type, mem_zone_ref_ptr script_node, const unsigned char *data, size_t len);

/* create paiment script */
BLOCK_API  int	C_API_FUNC	create_p2sh_script			(btc_addr_t addr, mem_zone_ref_ptr script_node);

/* create paiment script with data */
BLOCK_API  int	C_API_FUNC	create_p2sh_script_byte		(btc_addr_t addr, mem_zone_ref_ptr script_node, unsigned char val);

BLOCK_API  int	C_API_FUNC	create_p2sh_script_data		(const btc_addr_t addr, mem_zone_ref_ptr script_node, const unsigned char *data, size_t len);

/* compute transaction signature hash */
BLOCK_API  int	C_API_FUNC	compute_tx_sign_hash		(mem_zone_ref_const_ptr tx, unsigned int nIn, const struct string *script, unsigned int hash_type, hash_t txh);

/* check tx input signature */
BLOCK_API  int	C_API_FUNC	check_tx_input_sig			(mem_zone_ref_const_ptr tx, unsigned int nIn, struct string *vpubK);

/* check tx inputs */
BLOCK_API  int	C_API_FUNC	check_tx_outputs			(mem_zone_ref_ptr tx,uint64_t *total, mem_zone_ref_ptr outobjs, unsigned int *is_staking, unsigned int flags);

/* check tx outputs */
BLOCK_API  int	C_API_FUNC	check_tx_inputs				(mem_zone_ref_ptr tx, uint64_t *total_in, mem_zone_ref_ptr inobjs, unsigned int *is_coinbase,unsigned int check_sig,mem_zone_ref_ptr mempool, node **blk_tree);

/*check block pow */
BLOCK_API  int	C_API_FUNC check_block_pow				(mem_zone_ref_ptr hdr, hash_t diff_hash);

/* get parent txid */
BLOCK_API  int	C_API_FUNC get_tx_input_hash			(mem_zone_ref_ptr tx, unsigned int idx, hash_t hash);

/* get amount from tx output */
BLOCK_API  int	C_API_FUNC get_tx_output_amount			(mem_zone_ref_ptr tx, unsigned int idx, uint64_t *amount);

/* get output addr from tx output idx */
BLOCK_API  int	C_API_FUNC get_tx_output_addr			(const hash_t tx_hash, unsigned int idx, btc_addr_t addr);

/* sign transaction input */
BLOCK_API  int	C_API_FUNC tx_sign						(mem_zone_ref_const_ptr tx, unsigned int nIn, unsigned int hashType, const struct string *sign, const struct string *inPubKey, mem_zone_ref_ptr mempool);

/* compute sha256d hash from block header */
BLOCK_API int	C_API_FUNC	compute_block_hash			(mem_zone_ref_ptr block, hash_t hash);

/* compute pow hash from block header */
BLOCK_API int	C_API_FUNC	compute_block_pow			(mem_zone_ref_ptr block, hash_t hash);

/* compute current pow target */
BLOCK_API  int	C_API_FUNC block_compute_pow_target		(mem_zone_ref_ptr ActualSpacing, mem_zone_ref_ptr diff);

/* compute txid from tx object */
BLOCK_API  int	C_API_FUNC compute_tx_hash				(mem_zone_ref_ptr tx, hash_t hash);

/* get block version */
BLOCK_API int	C_API_FUNC	get_block_version			(unsigned int *v);

/* check hash signature */
BLOCK_API int	C_API_FUNC	blk_check_sign				(const struct string *sign, const struct string *pubk, const hash_t hash);

/* check validity of input transactions */
BLOCK_API  int	C_API_FUNC check_tx_list				(mem_zone_ref_ptr tx_list, uint64_t block_reward,hash_t merkle, unsigned int block_time, unsigned int check_sig);

BLOCK_API  int	C_API_FUNC compute_txlist_size			(mem_zone_ref_const_ptr tx_list, size_t *totalSize);

BLOCK_API  int	C_API_FUNC is_opfn_script				(const struct string *script, struct string *fn_name);

/* find input in list */
BLOCK_API  int	C_API_FUNC find_inner_inputs			(mem_zone_ref_ptr txin_list, hash_t txid, unsigned int oidx);

/* get tx input idx */
BLOCK_API int	C_API_FUNC	get_tx_input				(mem_zone_ref_const_ptr tx, unsigned int idx, mem_zone_ref_ptr vin);


/* set hash from compact from */
BLOCK_API unsigned int	C_API_FUNC	SetCompact			(unsigned int bits, hash_t out);

BLOCK_API unsigned int	C_API_FUNC	GetCompact			(const hash_t in, unsigned int *bits);


/*check utxo */
BLOCK_API int C_API_FUNC check_utxo						(const char *tx, unsigned int oidx);


/* get in/out value for an addr in a transaction */
BLOCK_API int C_API_FUNC get_tx_value					(mem_zone_ref_const_ptr tx, btc_addr_t addr, uint64_t *recv, uint64_t *sent);

/* sdump infos about tx */
BLOCK_API int C_API_FUNC dump_tx_infos					(mem_zone_ref_ptr tx);






BLOCK_API int C_API_FUNC dump_txh_infos					(const char *hash);


BLOCK_API  int	C_API_FUNC get_block_tree				(node **blktree);

BLOCK_API  int	C_API_FUNC get_tx_data					(mem_zone_ref_ptr tx, mem_zone_ref_ptr txData);

BLOCK_API  int	C_API_FUNC blk_find_last_pow_block		(mem_zone_ref_ptr pindex, unsigned int *block_time);

BLOCK_API  int	C_API_FUNC blk_find_last_pos_block		(mem_zone_ref_ptr pindex, unsigned int *block_time);
BLOCK_API  int	C_API_FUNC get_pow_block_limit			(uint64_t *lastPowBlock);
BLOCK_API  int	C_API_FUNC get_pow_diff_limit			(unsigned int *min_diff);
BLOCK_API  int	C_API_FUNC extract_key					(dh_key_t priv, dh_key_t pub);
BLOCK_API  int	C_API_FUNC compress_key					(dh_key_t pub, dh_key_t cpub);
BLOCK_API  int	C_API_FUNC extract_pub					(const dh_key_t priv, dh_key_t pub);
BLOCK_API  int	C_API_FUNC derive_secret				(const dh_key_t pub, dh_key_t priv, hash_t secret);
BLOCK_API  int	C_API_FUNC sign_hash					(const hash_t hash, const dh_key_t privkey, struct string *signature);

BLOCK_API  int	C_API_FUNC remove_tx_index				(hash_t tx_hash);

BLOCK_API int C_API_FUNC set_root_app					(mem_zone_ref_ptr tx);
BLOCK_API int C_API_FUNC get_root_app					(mem_zone_ref_ptr  rootAppHash);
BLOCK_API int C_API_FUNC get_apps						(mem_zone_ref_ptr Apps);

BLOCK_API int C_API_FUNC get_root_app_addr				(mem_zone_ref_ptr rootAppAddr);

BLOCK_API int C_API_FUNC is_app_root					(mem_zone_ref_ptr tx);

BLOCK_API int C_API_FUNC blk_load_app_root				();

BLOCK_API int C_API_FUNC blk_load_apps					(mem_zone_ref_ptr apps);

BLOCK_API int C_API_FUNC make_approot_tx				(mem_zone_ref_ptr tx, ctime_t time, uint64_t appfees,btc_addr_t addr);

BLOCK_API int C_API_FUNC get_root_app_fee				(mem_zone_ref_ptr rootAppFees);

BLOCK_API int C_API_FUNC make_app_tx					(mem_zone_ref_ptr tx,const char *app_name, unsigned int flags,btc_addr_t appAddr);
BLOCK_API int C_API_FUNC make_app_item_tx				(mem_zone_ref_ptr tx, const struct string *app_name, unsigned int item_id);
BLOCK_API int C_API_FUNC make_obj_txfr_tx				(mem_zone_ref_ptr tx, const hash_t txh, unsigned int oidx, const btc_addr_t dstAddr,hash_t objHash);
BLOCK_API int C_API_FUNC make_op_tx						(mem_zone_ref_ptr tx, const struct string *op_name, mem_zone_ref_ptr arg1, mem_zone_ref_ptr arg2);
BLOCK_API int C_API_FUNC make_fn_tx						(mem_zone_ref_ptr tx, const struct string *op_name, mem_zone_ref_ptr arg1);


BLOCK_API int C_API_FUNC make_coinbase_script			(uint64_t block_height, size_t nonce_size, struct string *script, size_t *extranonce_offset);

BLOCK_API int C_API_FUNC parse_approot_tx				(mem_zone_ref_ptr tx);
BLOCK_API int C_API_FUNC find_obj_tx					(mem_zone_ref_const_ptr tx, hash_t objHash, mem_zone_ref_ptr inputs, mem_zone_ref_ptr outputs);
BLOCK_API int C_API_FUNC find_obj_ptxfr					(mem_zone_ref_const_ptr tx, hash_t objHash, mem_zone_ref_ptr prev_tx);
BLOCK_API int C_API_FUNC find_obj_txfr					(const char *app_name, const char *typeStr, const hash_t objHash, hash_t txh);
BLOCK_API int C_API_FUNC list_obj_txfr					(const char *app_name, const char *typeStr, const hash_t objHash, mem_zone_ref_ptr txfrs);

BLOCK_API int C_API_FUNC								future_drift(ctime_t time1,ctime_t time2);


BLOCK_API  int  C_API_FUNC get_app_name					(const struct string *script, struct string *app_name);
BLOCK_API  int  C_API_FUNC get_app_types				(mem_zone_ref_ptr app, mem_zone_ref_ptr types);
BLOCK_API  int  C_API_FUNC get_app_scripts				(mem_zone_ref_ptr app, mem_zone_ref_ptr scripts);

BLOCK_API  int	C_API_FUNC get_type_infos				(struct string *script, char *name, unsigned int *id, unsigned int *flags);

BLOCK_API  int	C_API_FUNC get_tx_file					(mem_zone_ref_ptr tx, mem_zone_ref_ptr hash_list);
BLOCK_API  int	C_API_FUNC tx_is_app_file				(mem_zone_ref_ptr tx, struct string *appName, mem_zone_ref_ptr file);
BLOCK_API  int	C_API_FUNC get_app_type_key				(const char *appName, unsigned int type_id, const char *kname, unsigned int *ktype, unsigned int *flags);
BLOCK_API  int	C_API_FUNC get_app_type_idxs			(const char *appName, unsigned int type_id, mem_zone_ref_ptr keys);
BLOCK_API  int	C_API_FUNC get_app_obj_addr				(const char *app_name, unsigned int type_id, btc_addr_t objAddr, mem_zone_ref_ptr obj_list);
BLOCK_API  int	C_API_FUNC load_obj_type				(const char *app_name, const char *objHash, unsigned int *type_id, btc_addr_t objAddr);
BLOCK_API  int	C_API_FUNC check_app_obj_unique			(const char *appName, unsigned int type_id, mem_zone_ref_ptr obj);



BLOCK_API  int	C_API_FUNC  get_block_hash_str(uint64_t height, char *hash, size_t len);
BLOCK_API  int	C_API_FUNC get_block_hash(uint64_t height, hash_t hash);
BLOCK_API  int	C_API_FUNC get_block_time(uint64_t height, unsigned int *time);
BLOCK_API  int	C_API_FUNC  block_add_index(hash_t hash, uint64_t nblks, unsigned int time);
BLOCK_API  int	C_API_FUNC find_block_hash(const hash_t h, uint64_t block_height, uint64_t *height);
BLOCK_API  int	C_API_FUNC block_load_index(uint64_t *block_height);
BLOCK_API  int	C_API_FUNC write_block_index(uint64_t height);
BLOCK_API  int	C_API_FUNC rebuild_time_index(uint64_t height);
BLOCK_API  int	C_API_FUNC create_sorted_block_index(uint64_t height);

BLOCK_API  int	C_API_FUNC load_utxo(const char *txh, unsigned int oidx, uint64_t *amount, btc_addr_t addr, hash_t objh);
BLOCK_API int C_API_FUNC  get_block_size(mem_zone_ref_ptr block, mem_zone_ref_ptr tx_list);

/*
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
script.c
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
*/

/* get public key / address from output script */
BLOCK_API int C_API_FUNC   get_out_script_address(const struct string *script, struct string *pubk, btc_addr_t addr);


BLOCK_API int C_API_FUNC   get_out_script_return_val(const struct string *script, struct string *data);

/* get tx ouput script */
BLOCK_API int C_API_FUNC   get_tx_output_script(const hash_t tx_hash, unsigned int idx, struct string *script, uint64_t *amount);

/* public key get to hash */
BLOCK_API void C_API_FUNC   key_to_hash(const unsigned char *pkey, unsigned char *keyHash);

/* public key  to addr */
BLOCK_API void C_API_FUNC  key_to_addr(const unsigned char *pkey, btc_addr_t addr);

BLOCK_API int C_API_FUNC   make_script_file(mem_zone_ref_ptr file, struct string *pKey, struct string *sign, mem_zone_ref_ptr script);

BLOCK_API int C_API_FUNC   make_script_layout(mem_zone_ref_ptr file, mem_zone_ref_ptr script);

BLOCK_API int C_API_FUNC   make_script_module(mem_zone_ref_ptr file, mem_zone_ref_ptr script);
/*
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
store.c
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
*/
/* get total money supply */
BLOCK_API int C_API_FUNC  get_moneysupply(uint64_t *amount);

/* get block index size */
BLOCK_API int C_API_FUNC get_last_block_height();

/* find block hash from tx hash */
BLOCK_API int C_API_FUNC  find_blk_hash(const hash_t tx_hash, hash_t blk_hash,uint64_t *height,unsigned int *ofset,unsigned int *tx_time);

/* load block header from local store */
BLOCK_API int C_API_FUNC  load_blk_hdr(mem_zone_ref_ptr hdr, const hash_t blk_hash);

/* load block header from local store */
BLOCK_API int C_API_FUNC  block_check_last(uint64_t num, uint64_t *last_good);

/* load block header from local store */
BLOCK_API int C_API_FUNC  load_blk_hdr_at(mem_zone_ref_ptr hdr, int64_t height);

/* load block header infos from tx hash */
BLOCK_API int C_API_FUNC  get_tx_blk_height(const hash_t tx_hash, uint64_t *height, uint64_t *block_time, unsigned int *tx_time);

/* check if block is pow */
BLOCK_API int C_API_FUNC  is_pow_block(const hash_t blk_hash);
BLOCK_API int C_API_FUNC  is_pow_block_at(uint64_t height);

/* store block in local storage */
BLOCK_API int C_API_FUNC  store_block(mem_zone_ref_ptr header, mem_zone_ref_ptr tx_list);

BLOCK_API int C_API_FUNC  get_obj_app(hash_t objHash, char *appName);

/* load tx from a block based on the tx ofset */
BLOCK_API int C_API_FUNC  blk_load_tx_ofset(unsigned int ofset, mem_zone_ref_ptr tx);

/* load tx from its hash */
BLOCK_API int C_API_FUNC  load_tx(mem_zone_ref_ptr tx, hash_t blk_hash, const hash_t tx_hash);

/* load tx input and parent tx */
BLOCK_API int C_API_FUNC  load_tx_input(mem_zone_ref_const_ptr tx, unsigned int idx, mem_zone_ref_ptr	vin, mem_zone_ref_ptr tx_out);

/* load input from block hash and tx hash + input id */
BLOCK_API int C_API_FUNC  load_blk_tx_input(const hash_t blk_hash, unsigned int tx_ofset, unsigned int vin_idx, mem_zone_ref_ptr vout);

/* get amount of tx output*/
BLOCK_API int C_API_FUNC  load_tx_output_amount(const hash_t tx_hash, unsigned int idx, uint64_t *amount);

/* load parent output from an input */
BLOCK_API int C_API_FUNC  load_tx_input_vout(mem_zone_ref_const_ptr tx, unsigned int vin_idx, mem_zone_ref_ptr vout);

/* load all tx hashes from an address */
BLOCK_API int C_API_FUNC  load_tx_addresses(btc_addr_t addr, mem_zone_ref_ptr tx_hashes,size_t first, size_t num);

/*load app object */
BLOCK_API int C_API_FUNC  load_obj(const char *app_name, const char *objHash, const char *objName, unsigned int opts, mem_zone_ref_ptr obj, btc_addr_t objAddr);

/*load app child objects */
BLOCK_API int C_API_FUNC  load_obj_childs(const char *app_name, const char *objHash, const char *KeyName, size_t first, size_t max, unsigned int opts, size_t *count, mem_zone_ref_ptr objs);

/* load operation tree from root tx */
BLOCK_API int C_API_FUNC  load_op_tx(const hash_t tx_hash, mem_zone_ref_ptr inputs, mem_zone_ref_ptr out, mem_zone_ref_const_ptr mempool);

/*add child obj tx */
BLOCK_API int C_API_FUNC  make_app_child_obj_tx(mem_zone_ref_ptr tx, const char *app_name, hash_t objHash, unsigned int objType, btc_addr_t objAddr,const char *keyName, unsigned int ktype,hash_t childHash);

/* get obj hashes list */
BLOCK_API int C_API_FUNC  get_app_obj_hashes(const char *app_name, mem_zone_ref_ptr hash_list);

/* get last obj hashes list */
OS_API_C_FUNC(int) get_app_type_last_objs_hashes(const char *app_name, unsigned int type_id, size_t first, size_t max, size_t *total, mem_zone_ref_ptr hash_list);

/* load block size from local store */
BLOCK_API int C_API_FUNC  load_block_size(const hash_t blk_hash, size_t *size);



/* load tx hashes from block hash */
BLOCK_API int C_API_FUNC  get_blk_txs(const hash_t blk_hash,uint64_t block_height, mem_zone_ref_ptr txs, size_t max);

/* load number of tx from block */
BLOCK_API unsigned int C_API_FUNC  get_blk_ntxs(const hash_t blk_hash,uint64_t	block_height);

/* load tx hashes from block hash */
BLOCK_API int C_API_FUNC  load_blk_txs(const hash_t blk_hash, mem_zone_ref_ptr txs);

/* load block time from local store */
BLOCK_API int C_API_FUNC  load_block_time(const hash_t blkHash, ctime_t *time);
BLOCK_API int C_API_FUNC  load_block_time_range(unsigned int start, unsigned int end, size_t first, size_t max, size_t *total, mem_zone_ref_ptr blocks);

/*store tx inputs */
BLOCK_API int C_API_FUNC  store_tx_inputs(mem_zone_ref_ptr tx);

/*store tx outputs */
BLOCK_API int C_API_FUNC  store_tx_outputs(mem_zone_ref_ptr tx);

/*store tx hash into address index */
BLOCK_API int C_API_FUNC  store_tx_addresses(btc_addr_t addr, hash_t tx_hash);

/* remove tx from address index */
BLOCK_API int C_API_FUNC  remove_tx_addresses(const btc_addr_t addr, const hash_t tx_hash);

/* remove tx from storage*/
BLOCK_API int C_API_FUNC  remove_tx(hash_t tx_hash);

/*store block hash/txid index */
BLOCK_API  int	C_API_FUNC store_tx_blk_index(const hash_t tx_hash, const hash_t blk_hash,uint64_t height,size_t tx_ofset,unsigned int time);

/*store block tx hashes */
BLOCK_API int  C_API_FUNC store_block_txs(mem_zone_ref_ptr header, mem_zone_ref_ptr tx_list);

/* clear all transaction indexes */
BLOCK_API  int	C_API_FUNC clear_tx_index();

/* load obj hash from type */
BLOCK_API  int	C_API_FUNC get_app_type_obj_hashes(const char *app_name, unsigned int type_id, size_t first, size_t max, mem_zone_ref_ptr hash_list);
BLOCK_API  int	C_API_FUNC find_objs_by_addr(const char *app_name, const char *typeStr, const char *keyname, const btc_addr_t val, mem_zone_ref_ptr hash_list);
BLOCK_API  int	C_API_FUNC get_app_type_nobjs(const char *app_name, unsigned int type_id);
BLOCK_API  int	C_API_FUNC get_app_file(mem_zone_ref_ptr file_tx, struct string *app_name, mem_zone_ref_ptr file);
BLOCK_API  int	C_API_FUNC get_appfile_tx(const char *app_name, hash_t fileHash, hash_t txHash);
BLOCK_API  int	C_API_FUNC has_app_file(struct string *app_name, hash_t fileHash);
BLOCK_API  int	C_API_FUNC get_app_files(struct string *app_name, size_t first, size_t num, mem_zone_ref_ptr files);
BLOCK_API  int	C_API_FUNC get_app_missing_files(struct string *app_name, mem_zone_ref_ptr pending, mem_zone_ref_ptr files);

BLOCK_API  int	C_API_FUNC tx_is_app_item(const hash_t txh, unsigned int oidx, mem_zone_ref_ptr app_tx, unsigned char *val);
BLOCK_API int C_API_FUNC encode_DER_sig(const struct string *sig, struct string *sigseq, unsigned int rev, unsigned char hash_type);
/* staking API definition */

typedef int C_API_FUNC get_blk_staking_infos_func	(mem_zone_ref_ptr blk, mem_zone_ref_ptr infos);
typedef int	C_API_FUNC store_tx_staking_func		(mem_zone_ref_ptr tx, hash_t tx_hash, btc_addr_t stake_addr, uint64_t	stake_in);
typedef int	C_API_FUNC get_tx_pos_hash_data_func	(mem_zone_ref_ptr hdr, const hash_t txHash, unsigned int OutIdx, struct string *hash_data, uint64_t *amount, hash_t out_diff);
typedef int	C_API_FUNC get_target_spacing_func		(unsigned int *target);
typedef int	C_API_FUNC get_stake_reward_func		(uint64_t height, uint64_t *reward);
typedef int	C_API_FUNC get_last_stake_modifier_func	(mem_zone_ref_ptr pindex, hash_t nStakeModifier, unsigned int *nModifierTime);
typedef int	C_API_FUNC compute_tx_pos_func			(mem_zone_ref_ptr tx, hash_t StakeModifier, unsigned int txTime, hash_t pos_hash, uint64_t *weight);
typedef int	C_API_FUNC create_pos_block_func		(uint64_t height, mem_zone_ref_ptr tx, mem_zone_ref_ptr newBlock);
typedef int	C_API_FUNC check_tx_pos_func			(mem_zone_ref_ptr blk, mem_zone_ref_ptr tx);
typedef int	C_API_FUNC get_min_stake_depth_func		(unsigned int *depth);
typedef unsigned int C_API_FUNC	get_current_pos_difficulty_func();

typedef get_blk_staking_infos_func		*get_blk_staking_infos_func_ptr;
typedef store_tx_staking_func			*store_tx_staking_func_ptr;
typedef get_tx_pos_hash_data_func		*get_tx_pos_hash_data_func_ptr;
typedef get_target_spacing_func			*get_target_spacing_func_ptr;
typedef get_stake_reward_func			*get_stake_reward_func_ptr;
typedef get_last_stake_modifier_func	*get_last_stake_modifier_func_ptr;
typedef compute_tx_pos_func				*compute_tx_pos_func_ptr;
typedef create_pos_block_func			*create_pos_block_func_ptr;
typedef check_tx_pos_func				*check_tx_pos_func_ptr;
typedef get_min_stake_depth_func		*get_min_stake_depth_func_ptr;
typedef get_current_pos_difficulty_func	*get_current_pos_difficulty_func_ptr;
