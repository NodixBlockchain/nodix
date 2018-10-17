#define NODE_API C_EXPORT

#include "base/std_def.h"
#include "base/std_mem.h"
#include "base/std_str.h"
#include "base/mem_base.h"

#include "strs.h"

#include "../node_adx/node_api.h"
//node module
NODE_API int			C_API_FUNC		node_init_self(mem_zone_ref_ptr self_node){return 0;}
NODE_API int			C_API_FUNC		node_set_last_block(mem_zone_ref_ptr header){return 0;}
NODE_API int			C_API_FUNC		node_find_last_pow_block(mem_zone_ref_ptr pindex, unsigned int *block_time){return 0;}
NODE_API int			C_API_FUNC		node_load_last_blks(){return 0;}
NODE_API int			C_API_FUNC		node_log_addr_infos(mem_zone_ref_ptr addr){return 0;}
NODE_API int			C_API_FUNC		update_peernodes(){return 0;}
NODE_API int			C_API_FUNC		node_add_block_index(hash_t hash, unsigned int time){return 0;}
NODE_API int			C_API_FUNC		node_add_tx_to_mempool(mem_zone_ref_ptr tx){return 0;}
NODE_API int			C_API_FUNC		node_fill_block_from_mempool(mem_zone_ref_ptr block){return 0;}
NODE_API int			C_API_FUNC		node_del_txs_from_mempool(mem_zone_ref_ptr tx_list){return 0;}

NODE_API int			C_API_FUNC		node_get_last_block_time(ctime_t *otime){return 0;}
NODE_API int			C_API_FUNC		node_truncate_chain(uint64_t height){return 0;}
NODE_API int			C_API_FUNC		node_remove_last_block(){return 0;}
NODE_API int			C_API_FUNC		node_rewrite_txs(uint64_t nums){return 0;}
NODE_API int			C_API_FUNC		node_check_chain(mem_zone_ref_ptr node, mem_zone_ref_ptr hdr){return 0;}
NODE_API int			C_API_FUNC		node_zip_self(struct string *out_data, mem_zone_ref_ptr opts){return 0;}
NODE_API int			C_API_FUNC	    node_get_script_modules(mem_zone_ref_ptr modules){return 0;}
NODE_API int			C_API_FUNC	    node_get_script_msg_handlers(mem_zone_ref_ptr handlers){return 0;}
NODE_API int			C_API_FUNC		node_set_script(mem_zone_ref_ptr script){return 0;}
NODE_API int			C_API_FUNC		remove_block(hash_t blk_hash){return 0;}
NODE_API int			C_API_FUNC		block_pow_limit() { return 0; }
NODE_API int			C_API_FUNC		node_inc_pow_blocks() { return 0; }
NODE_API int			C_API_FUNC		node_dec_pow_blocks() { return 0; }
NODE_API int			C_API_FUNC		node_init_service(mem_zone_ref_ptr service, mem_zone_ref_ptr pos_mod_def){ return 0; }
NODE_API int			C_API_FUNC		proccess_http_reqs(mem_zone_ref_ptr service){ return 0; }
NODE_API int			C_API_FUNC		get_file_mime(mem_zone_ref_const_ptr service, const char *filepath, struct string *mime){ return 0; }
NODE_API int			C_API_FUNC		check_http_request(mem_zone_ref_const_ptr service){ return 0; }
NODE_API int			C_API_FUNC		node_process_event_handler(const char *msg_list_name, mem_zone_ref_ptr node, mem_zone_ref_ptr msg){ return 0; }
NODE_API int			C_API_FUNC		has_peers() { return 0; }
NODE_API int			C_API_FUNC		node_is_next_block(mem_zone_ref_const_ptr header){return 0;}
NODE_API int			C_API_FUNC		new_peer_node(mem_zone_ref_ptr node_def){return 0;}
NODE_API int			C_API_FUNC		read_node_msg(mem_zone_ref_ptr node){return 0;}
NODE_API int			C_API_FUNC		send_node_messages(mem_zone_ref_ptr node){return 0;}
NODE_API int			C_API_FUNC		node_add_block_header(mem_zone_ref_ptr node, mem_zone_ref_ptr hdr){return 0;}
NODE_API int			C_API_FUNC		node_clear_block_headers(mem_zone_ref_ptr node) { return 0; }
NODE_API int			C_API_FUNC		broadcast_block_inv(mem_zone_ref_ptr from_node, mem_zone_ref_ptr block) { return 0; }
NODE_API int			C_API_FUNC		queue_version_message(mem_zone_ref_ptr node){return 0;}
NODE_API int			C_API_FUNC		queue_verack_message(mem_zone_ref_ptr node){return 0;}
NODE_API int			C_API_FUNC		queue_ping_message(mem_zone_ref_ptr node){return 0;}
NODE_API int			C_API_FUNC		queue_pong_message(mem_zone_ref_ptr node, mem_zone_ref_ptr nonce){return 0;}
NODE_API int			C_API_FUNC		queue_getaddr_message(mem_zone_ref_ptr node){return 0;}
NODE_API int			C_API_FUNC		queue_getdata_message(mem_zone_ref_ptr node, mem_zone_ref_ptr hash_list){return 0;}
NODE_API int			C_API_FUNC		queue_block_message(mem_zone_ref_ptr node, mem_zone_ref_ptr header, mem_zone_ref_ptr tx_list, struct string *signature){return 0;}
NODE_API int			C_API_FUNC		queue_tx_message(mem_zone_ref_ptr node, mem_zone_ref_ptr tx){return 0;}
NODE_API int			C_API_FUNC		queue_getblocks_message(mem_zone_ref_ptr node){return 0;}
NODE_API int			C_API_FUNC		queue_getblock_hdrs_message(mem_zone_ref_ptr node){return 0;}
NODE_API int			C_API_FUNC		queue_send_message(mem_zone_ref_ptr node, mem_zone_ref_ptr msg){return 0;}
NODE_API int			C_API_FUNC		queue_emitted_element(mem_zone_ref_ptr node, mem_zone_ref_ptr element){return 0;}
NODE_API int			C_API_FUNC		queue_emitted_message(mem_zone_ref_ptr node, mem_zone_ref_ptr msg){return 0;}
NODE_API int			C_API_FUNC		queue_inv_message(mem_zone_ref_ptr node, mem_zone_ref_ptr hash_list){return 0;}
NODE_API int			C_API_FUNC		node_check_services(){return 0;}
NODE_API int			C_API_FUNC		reset_moneysupply(){return 0;}
NODE_API int			C_API_FUNC		add_moneysupply(uint64_t amount){return 0;}
NODE_API int			C_API_FUNC		sub_moneysupply(uint64_t amount){return 0;}
NODE_API int			C_API_FUNC		get_locator_next_blocks(mem_zone_ref_ptr locator, mem_zone_ref_ptr inv_pack){ return 0; }
NODE_API int			C_API_FUNC		node_check_new_connections(){ return 0; }


NODE_API int			C_API_FUNC		node_release_mining_lock(){ return 0; }

NODE_API int			C_API_FUNC		queue_addr_message(mem_zone_ref_ptr node, mem_zone_ref_ptr addrs){ return 0; }
NODE_API int			C_API_FUNC		node_release_mempool_lock(){ return 0; }
NODE_API int			C_API_FUNC		node_aquire_mining_lock(){ return 0; }
NODE_API int			C_API_FUNC		node_del_btree_from_mempool(){ return 0; }
NODE_API int			C_API_FUNC		node_aquire_mempool_lock(mem_zone_ref_ptr mempool){ return 0; }
NODE_API int			C_API_FUNC		node_mempool_getobj(const char *appName, hash_t objHash, mem_zone_ref_ptr obj) { return 0; }
NODE_API int			C_API_FUNC		node_store_tmp_file(struct string *app_name, mem_zone_ref_ptr file){ return 0; }
NODE_API int			C_API_FUNC		node_create_pow_block(mem_zone_ref_ptr newBlock, btc_addr_t coinbaseAddr){ return 0; }
NODE_API int			C_API_FUNC		node_compute_pow_diff_after(mem_zone_ref_ptr newPOWBlock) { return 0; }
NODE_API int			C_API_FUNC		node_get_types_def(mem_zone_ref_ptr types){ return 0; }
NODE_API int			C_API_FUNC		node_rm_tmp_file(struct string *app_name, mem_zone_ref_ptr file){ return 0; }
NODE_API int			C_API_FUNC		get_bitcore_addr(mem_zone_ref_ptr node, ipv4_t ip, unsigned short *port, uint64_t *services) { return 0; }
NODE_API int			C_API_FUNC		is_same_node(mem_zone_ref_ptr node1, mem_zone_ref_ptr node2) { return 0; }
NODE_API int			C_API_FUNC		node_check_mempool_unique(mem_zone_ref_const_ptr node_txs, const char *appName, unsigned int typeID, mem_zone_ref_const_ptr obj) { return 0; }

NODE_API int			C_API_FUNC		store_last_pow_hash(hash_t hash) { return 0; }
NODE_API int			C_API_FUNC		store_last_pos_hash(hash_t hash) { return 0; }
NODE_API int			C_API_FUNC		node_get_hash_idx(uint64_t block_idx, hash_t hash) { return 0; }
