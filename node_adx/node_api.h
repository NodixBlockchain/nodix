#ifndef NODE_API
	#define NODE_API C_IMPORT 
#endif
//node module
NODE_API int			C_API_FUNC		node_init_self(mem_zone_ref_ptr self_node);
NODE_API int			C_API_FUNC		node_set_last_block(mem_zone_ref_ptr header);
NODE_API int			C_API_FUNC		node_load_last_blks();
NODE_API int			C_API_FUNC		node_log_addr_infos(mem_zone_ref_ptr addr);
NODE_API int			C_API_FUNC		update_peernodes();
NODE_API int			C_API_FUNC		is_node_addr(mem_zone_ref_ptr node, mem_zone_ref_ptr p2p_addr);

NODE_API int			C_API_FUNC		node_add_block_index(hash_t hash, unsigned int time);
NODE_API int			C_API_FUNC		node_add_tx_to_mempool(mem_zone_ref_ptr tx);
NODE_API int			C_API_FUNC		node_get_mempool_hashes(mem_zone_ref_ptr hash_list);
NODE_API int			C_API_FUNC		node_fill_block_from_mempool(mem_zone_ref_ptr block);
NODE_API int			C_API_FUNC		node_txsdata_from_mempool(mem_zone_ref_ptr tx_list, mem_zone_ref_ptr Fees);
NODE_API int			C_API_FUNC		node_del_txs_from_mempool(mem_zone_ref_ptr tx_list);
NODE_API int			C_API_FUNC		node_del_btree_from_mempool();
NODE_API int			C_API_FUNC		node_get_hash_idx(uint64_t block_idx, hash_t hash);
NODE_API int			C_API_FUNC		node_get_last_block_time(ctime_t *otime);
NODE_API int			C_API_FUNC		node_truncate_chain(uint64_t height);
NODE_API int			C_API_FUNC		node_remove_last_block();
NODE_API int			C_API_FUNC		node_rewrite_txs(uint64_t nums);
NODE_API int			C_API_FUNC		node_check_chain(mem_zone_ref_ptr node, mem_zone_ref_ptr hdr);
NODE_API int			C_API_FUNC		node_zip_self(struct string *out_data, mem_zone_ref_ptr opts);
NODE_API int			C_API_FUNC	    node_get_script_modules(mem_zone_ref_ptr modules);
NODE_API int			C_API_FUNC	    node_get_script_msg_handlers(mem_zone_ref_ptr handlers);
NODE_API int			C_API_FUNC		node_set_script(mem_zone_ref_ptr script);
NODE_API int			C_API_FUNC		remove_block(hash_t blk_hash);
NODE_API int			C_API_FUNC		node_list_accounts(mem_zone_ref_ptr account_list);
NODE_API int			C_API_FUNC		node_list_addrs(mem_zone_ref_ptr account_name, mem_zone_ref_ptr addr_list);
NODE_API int			C_API_FUNC		set_next_check(mem_zone_ref_ptr nSecs);
NODE_API int			C_API_FUNC		node_check_new_connections();
NODE_API int			C_API_FUNC		node_get_pow_spacing(mem_zone_ref_ptr lastPOW, mem_zone_ref_ptr Spacing);
NODE_API int			C_API_FUNC		compute_last_pow_diff(mem_zone_ref_ptr blk, mem_zone_ref_ptr nBits);
NODE_API int			C_API_FUNC		get_bitcore_addr(mem_zone_ref_ptr node, ipv4_t ip, unsigned short *port, uint64_t *services);
NODE_API int			C_API_FUNC		is_same_node(mem_zone_ref_ptr node1, mem_zone_ref_ptr node2);
NODE_API int			C_API_FUNC		is_node_addr(mem_zone_ref_ptr node, mem_zone_ref_ptr p2p_addr);
NODE_API int			C_API_FUNC		node_store_last_pow_hash(mem_zone_ref_ptr blk);
NODE_API int			C_API_FUNC		node_store_last_pos_hash(mem_zone_ref_ptr blk);
NODE_API int			C_API_FUNC		block_pow_limit();

NODE_API int			C_API_FUNC		node_dump_memory(unsigned int flags);
NODE_API int			C_API_FUNC		node_init_service(mem_zone_ref_ptr service, mem_zone_ref_ptr pos_mod_def);

NODE_API int			C_API_FUNC		get_file_mime(mem_zone_ref_const_ptr service, const char *filepath, struct string *mime);
NODE_API int			C_API_FUNC		check_http_request(mem_zone_ref_const_ptr service);
NODE_API int			C_API_FUNC		node_init_service(mem_zone_ref_ptr service, mem_zone_ref_ptr pos_mod_def);
NODE_API int			C_API_FUNC		node_process_event_handler(const char *msg_list_name, mem_zone_ref_ptr node, mem_zone_ref_ptr msg);

NODE_API int			C_API_FUNC		node_is_next_block(mem_zone_ref_const_ptr header);
NODE_API int			C_API_FUNC		connect_peer_node(mem_zone_ref_ptr node_def);
NODE_API int			C_API_FUNC		read_node_msg(mem_zone_ref_ptr node);
NODE_API int			C_API_FUNC		send_node_messages(mem_zone_ref_ptr node);
NODE_API int			C_API_FUNC		node_add_block_header(mem_zone_ref_ptr node, mem_zone_ref_ptr hdr);
NODE_API int			C_API_FUNC		queue_version_message(mem_zone_ref_ptr node);
NODE_API int			C_API_FUNC		queue_verack_message(mem_zone_ref_ptr node);
NODE_API int			C_API_FUNC		queue_ping_message(mem_zone_ref_ptr node);
NODE_API int			C_API_FUNC		queue_pong_message(mem_zone_ref_ptr node, mem_zone_ref_ptr nonce);
NODE_API int			C_API_FUNC		queue_getaddr_message(mem_zone_ref_ptr node);
NODE_API int			C_API_FUNC		queue_getdata_message(mem_zone_ref_ptr node, mem_zone_ref_ptr hash_list);
NODE_API int			C_API_FUNC		get_locator_next_blocks(mem_zone_ref_ptr locator, mem_zone_ref_ptr inv_pack);

NODE_API int			C_API_FUNC		queue_block_message(mem_zone_ref_ptr node, mem_zone_ref_ptr header, mem_zone_ref_ptr tx_list, struct string *signature);
NODE_API int			C_API_FUNC		queue_tx_message(mem_zone_ref_ptr node, mem_zone_ref_ptr tx);
NODE_API int			C_API_FUNC		queue_getblocks_message(mem_zone_ref_ptr node);
NODE_API int			C_API_FUNC		queue_getblock_hdrs_message(mem_zone_ref_ptr node);
NODE_API int			C_API_FUNC		queue_send_message(mem_zone_ref_ptr node, mem_zone_ref_ptr msg);
NODE_API int			C_API_FUNC		queue_emitted_element(mem_zone_ref_ptr node, mem_zone_ref_ptr element);
NODE_API int			C_API_FUNC		queue_emitted_message(mem_zone_ref_ptr node, mem_zone_ref_ptr msg);
NODE_API int			C_API_FUNC		queue_inv_message(mem_zone_ref_ptr node, mem_zone_ref_ptr hash_list);
NODE_API int			C_API_FUNC		queue_addr_message(mem_zone_ref_ptr node, mem_zone_ref_ptr addrs);

NODE_API int			C_API_FUNC		node_check_services();
NODE_API int			C_API_FUNC		reset_moneysupply();
NODE_API int			C_API_FUNC		add_moneysupply(uint64_t amount);
NODE_API int			C_API_FUNC		sub_moneysupply(uint64_t amount);
NODE_API int			C_API_FUNC		node_has_block(mem_zone_ref_ptr hash);
NODE_API int			C_API_FUNC		node_mempool_has_tx(mem_zone_ref_ptr tx_hash);
NODE_API int			C_API_FUNC		node_check_mempool_unique(mem_zone_ref_const_ptr node_txs, const char *appName, unsigned int typeID, mem_zone_ref_const_ptr obj);

NODE_API int			C_API_FUNC		node_create_pow_block(mem_zone_ref_ptr newBlock,btc_addr_t addr);

NODE_API int			C_API_FUNC		node_bookmark_check_addr(mem_zone_ref_ptr p2p_addr);
NODE_API int			C_API_FUNC		node_find_peers_node(mem_zone_ref_ptr addr);
NODE_API int			C_API_FUNC		node_set_bookmark(mem_zone_ref_ptr p2p_addr, mem_zone_ref_ptr state_node);


NODE_API int			C_API_FUNC		 node_get_root_app_fee(mem_zone_ref_ptr fee);
NODE_API int			C_API_FUNC		 node_get_apps(mem_zone_ref_ptr apps);
NODE_API int			C_API_FUNC		 node_get_ip(mem_zone_ref_const_ptr node, ipv4_t ip);
NODE_API int			C_API_FUNC		 node_get_app(mem_zone_ref_ptr appName, mem_zone_ref_ptr app);
NODE_API int			C_API_FUNC		 node_get_types_def(mem_zone_ref_ptr types);
NODE_API int			C_API_FUNC		 node_store_tmp_file(struct string *app_name, mem_zone_ref_ptr file);
NODE_API int			C_API_FUNC		 node_rm_tmp_file(struct string *app_name, mem_zone_ref_ptr file);
NODE_API int			C_API_FUNC		 node_release_mining_lock();
NODE_API int			C_API_FUNC		 node_aquire_mining_lock();

NODE_API int			C_API_FUNC		 node_aquire_mempool_lock(mem_zone_ref_ptr mempool);
NODE_API int			C_API_FUNC		 node_release_mempool_lock();

