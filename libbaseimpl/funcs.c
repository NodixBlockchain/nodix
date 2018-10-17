#include "base/std_def.h"
#include "base/std_mem.h"
#include "base/mem_base.h"
#include "base/std_str.h"


#define LIBBASE_API C_EXPORT

#include "strs.h"
#include "tree.h"

#include "sha256.h"

OS_API_C_FUNC(  void)				tree_manager_dump_mem(unsigned int time){return ;}

OS_API_C_FUNC(  int  )				tree_manager_create_node(const char *name, unsigned int type, mem_zone_ref *ref_ptr){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_create_node_childs(const char *name, unsigned int type, mem_zone_ref *ref_ptr, const char *params){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_create_node_params(mem_zone_ref_ptr ref_ptr, const char *params){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_add_node_childs(mem_zone_ref_ptr p_node_ref, const char *params, unsigned int merge){return 0;}
OS_API_C_FUNC(  void	)			tree_manager_sort_childs(mem_zone_ref_ptr parent_ref_ptr, const char *name, unsigned int dir){return ;}

OS_API_C_FUNC(  int	)			tree_manager_get_first_child(mem_zone_ref_const_ptr p_node_ref, mem_zone_ref_ptr child_list, mem_zone_ref_ptr *p_node_out_ref){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_get_next_child(mem_zone_ref_ptr child_list, mem_zone_ref_ptr *p_node_out_ref){return 0;}

OS_API_C_FUNC(int)				tree_manager_get_next_child_shared(mem_zone_ref_ptr *child_list, mem_zone_ref_ptr *p_node_out_ref) { return 0; }
OS_API_C_FUNC(int)				tree_manager_get_first_child_shared(mem_zone_ref_const_ptr p_node_ref, mem_zone_ref_ptr *child_list, mem_zone_ref_ptr *p_node_out_ref) { return 0; }

OS_API_C_FUNC(  int	)			tree_manager_get_last_child(mem_zone_ref_const_ptr p_node_ref, mem_zone_ref_ptr child_list, mem_zone_ref_ptr *p_node_out_ref){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_get_prev_child(mem_zone_ref_ptr child_list, mem_zone_ref_ptr *p_node_out_ref){return 0;}
OS_API_C_FUNC(int)			ripemd160(const void* in, unsigned long length, void* out){ return 0; }
OS_API_C_FUNC(int)	tree_manager_get_child_value_btcaddr(const mem_zone_ref	*p_node_ref, unsigned int crc_name, btc_addr_t addr){ return 0; }
OS_API_C_FUNC(  unsigned int	)	and_node_type(unsigned int type1, unsigned int type2){return 0;}
OS_API_C_FUNC(  const char *	)	tree_mamanger_get_node_name(mem_zone_ref_const_ptr node_ref){return 0;}
OS_API_C_FUNC(  unsigned int)		tree_mamanger_get_node_type(mem_zone_ref_const_ptr node_ref){return 0;}
OS_API_C_FUNC(  int		)	  tree_mamanger_get_parent(mem_zone_ref_const_ptr node_ref, mem_zone_ref_ptr p_ref){return 0;}
OS_API_C_FUNC(  int	)		  tree_manager_get_ancestor_by_type(mem_zone_ref_const_ptr node_ref, unsigned int node_type, mem_zone_ref_ptr  p_ref){return 0;}
OS_API_C_FUNC(  size_t	)		tree_manager_get_node_num_children(mem_zone_ref_const_ptr p_node_ref){return 0;}

OS_API_C_FUNC(  void	)			tree_manager_set_node_name(mem_zone_ref_ptr node_ref, const char *name){return ;}

OS_API_C_FUNC(  int  		)		tree_manager_add_child_node(mem_zone_ref_ptr parent_ref_ptr, const char *name, unsigned int type, mem_zone_ref *ref_ptr){return 0;}
OS_API_C_FUNC(  unsigned int)		tree_manager_node_add_child(mem_zone_ref_ptr parent_ref_ptr, mem_zone_ref_const_ptr child_ref_ptr){return 0;}

OS_API_C_FUNC(  int	)			tree_manager_node_dup_one(mem_zone_ref_ptr src_ref_ptr, mem_zone_ref_ptr new_ref_ptr){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_copy_children(mem_zone_ref_ptr dest_ref_ptr, mem_zone_ref_const_ptr src_ref_ptr,unsigned int depth){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_get_child_at(mem_zone_ref_const_ptr parent_ref_ptr, unsigned int index, mem_zone_ref_ptr ref_ptr){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_find_child_node(mem_zone_ref_const_ptr parent_ref_ptr, unsigned int crc_name, unsigned int type, mem_zone_ref_ptr ref_ptr){return 0;}

OS_API_C_FUNC(  int	)			 tree_node_read_childs(mem_zone_ref_const_ptr p_node_ref, struct node_hash_val_t *list){return 0;}
OS_API_C_FUNC(  int	)			tree_node_find_child_by_name(mem_zone_ref_const_ptr p_node_ref, const char *name, mem_zone_ref_ptr p_node_out_ref){return 0;}
OS_API_C_FUNC(  unsigned int)		tree_node_list_child_by_type(mem_zone_ref_const_ptr p_node_ref, unsigned int type, mem_zone_ref_ptr p_node_out_ref, unsigned int index){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_list_child_type(mem_zone_ref_ptr child_list, unsigned int type, unsigned int *index, mem_zone_ref_ptr *p_node_out_ref){return 0;}
OS_API_C_FUNC(  int	)			tree_node_find_child_by_type_value(mem_zone_ref_const_ptr node, unsigned int type, unsigned int value, mem_zone_ref_ptr p_node_out_ref){return 0;}
OS_API_C_FUNC(  int	)			tree_node_find_child_by_type(mem_zone_ref_const_ptr p_node_ref, unsigned int node_type, mem_zone_ref_ptr p_node_out_ref, unsigned int index){return 0;}
OS_API_C_FUNC(  int	)			tree_node_find_child_by_id(mem_zone_ref_const_ptr p_node_ref, unsigned int node_id, mem_zone_ref_ptr p_node_out_ref){return 0;}
OS_API_C_FUNC(  int	)			tree_find_child_node_by_id_name(mem_zone_ref_const_ptr  p_node_ref, unsigned int child_type, const char *id_name, unsigned int id_val, mem_zone_ref_ptr out_node){return 0;}
OS_API_C_FUNC(int)				tree_find_child_node_by_hash(mem_zone_ref_const_ptr p_node_ref, unsigned int child_type, hash_t bhash, mem_zone_ref_ptr out_node) { return 0; }
//OS_API_C_FUNC(  int				tree_find_child_node_by_value			(mem_zone_ref_const_ptr  p_node_ref,unsigned int child_type,const char *id_name,unsigned int id_val,mem_zone_ref_ptr out_node){return 0;}
OS_API_C_FUNC(  int	)		  tree_find_child_node_idx_by_id(mem_zone_ref *p_node_ref, unsigned int child_type, unsigned int child_id, unsigned int *out_idx){return 0;}
OS_API_C_FUNC(  int	)			tree_find_child_node_by_member_name(mem_zone_ref_const_ptr p_node_ref, unsigned int child_type, unsigned int child_member_type, const char *child_member_name, mem_zone_ref_ptr out_node){return 0;}
OS_API_C_FUNC(int) tree_find_child_node_by_member_name_hash(mem_zone_ref_const_ptr p_node_ref, unsigned int child_type, const char *child_member_name, const hash_t bhash, mem_zone_ref_ptr out_node){ return 0; }
OS_API_C_FUNC(  int	)			tree_swap_child_node_by_id(mem_zone_ref_ptr p_node_ref, unsigned int id_val, mem_zone_ref_ptr node){return 0;}

OS_API_C_FUNC(  int	)			tree_remove_children(mem_zone_ref_ptr p_node_ref){return 0;}
OS_API_C_FUNC(  int	)			tree_remove_child_by_type(mem_zone_ref_ptr p_node_ref, unsigned int child_type){return 0;}
OS_API_C_FUNC(  int	)			tree_remove_child_by_id(mem_zone_ref_ptr p_node_ref, unsigned int child_id){return 0;}

OS_API_C_FUNC(  int	)			tree_remove_child_by_value_dword(mem_zone_ref_ptr p_node_ref, unsigned int type, unsigned int value){return 0;}
OS_API_C_FUNC(  int	)			tree_remove_child_by_member_value_dword(mem_zone_ref_ptr p_node_ref, unsigned int child_type, const char *member_name, unsigned int value){return 0;}

OS_API_C_FUNC(int)			tree_manager_write_node_sig(mem_zone_ref_ptr node_ref, mem_size ofset, unsigned char *sign, size_t sign_len){ return 0; }



OS_API_C_FUNC(  int  )				tree_manager_allocate_node_data(mem_zone_ref_ptr node_ref, mem_size data_size){return 0;}
OS_API_C_FUNC(  int	 )			tree_manager_write_node_data(mem_zone_ref_ptr node_ref, const_mem_ptr data, mem_size ofset, mem_size size){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_copy_node_data(mem_zone_ref_ptr dst_node, mem_zone_ref_const_ptr src_node){return 0;}
OS_API_C_FUNC(  int	 )			tree_manager_write_node_dword(mem_zone_ref_ptr node_ref, mem_size ofset, unsigned int value){return 0;}
OS_API_C_FUNC(  int	 )			tree_manager_cmp_z_xchg_node_dword(mem_zone_ref_ptr node_ref, mem_size ofset, unsigned int value){return 0;}
OS_API_C_FUNC(  int		)		tree_manager_write_node_qword(mem_zone_ref *node_ref, mem_size ofset, uint64_t value){return 0;}
OS_API_C_FUNC(  int		)		tree_manager_write_node_float(mem_zone_ref *node_ref, mem_size ofset, float value){return 0;}

OS_API_C_FUNC(int)				tree_manager_write_node_double(mem_zone_ref *node_ref, mem_size ofset, double value){ return 0; }

OS_API_C_FUNC(  int	)			tree_manager_write_node_hash(mem_zone_ref_ptr node_ref, mem_size ofset, const hash_t hash){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_write_node_rhash(mem_zone_ref_ptr node_ref, mem_size ofset, const hash_t hash){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_write_node_vstr(mem_zone_ref_ptr node_ref, mem_size ofset, const struct string *str){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_write_node_vint(mem_zone_ref_ptr node_ref, mem_size ofset, const_mem_ptr vint){return 0;}
OS_API_C_FUNC(int)			tree_manager_write_node_btcaddr(mem_zone_ref_ptr node_ref, mem_size ofset, const btc_addr_t addr){ return 0; }
OS_API_C_FUNC(  int	)			tree_manager_write_node_4uc(mem_zone_ref_ptr node_ref, mem_size ofset, const vec_4uc_t val){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_write_node_word(mem_zone_ref_ptr node_ref, mem_size ofset, unsigned short value){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_write_node_byte(mem_zone_ref_ptr node_ref, mem_size ofset, unsigned char value){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_write_node_signed_dword(mem_zone_ref_ptr node_ref, mem_size ofset, int value){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_write_node_signed_word(mem_zone_ref_ptr node_ref, mem_size ofset, short value){return 0;}
OS_API_C_FUNC(  int	)	    	tree_manager_write_node_str(mem_zone_ref_ptr node_ref, mem_size ofset, const char *str){return 0;}
OS_API_C_FUNC(  void)				tree_manager_set_node_image_info(mem_zone_ref_ptr node_ref, mem_size position, mem_size size){return ;}



OS_API_C_FUNC(  int		)    	tree_manager_get_child_value_str(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, char *str, unsigned int str_len, unsigned int base){return 0;}
OS_API_C_FUNC(  int	)	    	tree_manager_get_child_value_istr(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, struct string *str, unsigned int base){return 0;}

OS_API_C_FUNC(  size_t	)		tree_manager_get_node_image_size(mem_zone_ref_const_ptr node_ref){return 0;}
OS_API_C_FUNC(  size_t	)		tree_manager_get_node_image_pos(mem_zone_ref_const_ptr node_ref){return 0;}
OS_API_C_FUNC(  int		)		tree_manager_get_node_4uc(mem_zone_ref_const_ptr node_ref, mem_size ofset, vec_4uc_t val){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_get_node_str(mem_zone_ref_const_ptr node_ref, mem_size ofset, char *str, unsigned int str_len, unsigned int base){return 0;}
OS_API_C_FUNC(  int		)		tree_manager_get_node_istr(mem_zone_ref_const_ptr node_ref, mem_size ofset, struct string *str, unsigned int base){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_get_node_vstr(mem_zone_ref_const_ptr node_ref, mem_size ofset, mem_ptr vstr){return 0;}
OS_API_C_FUNC(  int	)			tree_mamanger_get_node_dword(mem_zone_ref_const_ptr node_ref, mem_size ofset, unsigned int *val){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_get_node_hash(mem_zone_ref_const_ptr node_ref, mem_size ofset, hash_t hash){return 0;}
OS_API_C_FUNC(  int		)		tree_manager_get_node_rhash(mem_zone_ref_const_ptr node_ref, mem_size ofset, hash_t hash){return 0;}
OS_API_C_FUNC( int	)		    tree_manager_get_node_btcaddr(mem_zone_ref_ptr node_ref, mem_size ofset, btc_addr_t addr){return 0;}
OS_API_C_FUNC(  int	)			tree_mamanger_get_node_qword(mem_zone_ref_const_ptr node_ref, mem_size ofset, uint64_t *val){return 0;}
OS_API_C_FUNC(  int	)			tree_mamanger_get_node_float(mem_zone_ref_const_ptr node_ref, mem_size ofset, float *val){return 0;}
OS_API_C_FUNC(int)				tree_mamanger_get_node_double(mem_zone_ref_const_ptr node_ref, mem_size ofset, double *val){ return 0; }
OS_API_C_FUNC(  int		)		tree_mamanger_get_node_signed_dword(mem_zone_ref_const_ptr node_ref, mem_size ofset, int *val){return 0;}
OS_API_C_FUNC(  int		)		tree_mamanger_get_node_word(mem_zone_ref_const_ptr node_ref, mem_size ofset, unsigned short *val){return 0;}
OS_API_C_FUNC(  int		)		tree_mamanger_get_node_signed_word(mem_zone_ref_const_ptr node_ref, mem_size ofset, short *val){return 0;}
OS_API_C_FUNC(  int		)		tree_mamanger_get_node_byte(mem_zone_ref_const_ptr node_ref, mem_size ofset, unsigned char *val){return 0;}
OS_API_C_FUNC(  mem_ptr	)		tree_mamanger_get_node_data_ptr(mem_zone_ref_const_ptr node_ref, mem_size ofset){return 0;}
OS_API_C_FUNC(size_t) tree_mamanger_get_node_data_size(mem_zone_ref_const_ptr node_ref) { return 0; }
OS_API_C_FUNC(int) tree_manager_set_child_value_mat3x3(mem_zone_ref	*p_node_ref, const char *name, float *mat3x3) { return 0; }
OS_API_C_FUNC(  unsigned int)		tree_manager_compare_node_crc(mem_zone_ref_ptr node_ref, unsigned int crc){return 0;}
OS_API_C_FUNC(  int		)		tree_mamanger_compare_node_dword(mem_zone_ref_ptr node_ref, mem_size ofset, unsigned int val){return 0;}
OS_API_C_FUNC(int)				tree_node_keval_i64(mem_zone_ref_const_ptr p_node_ref, const struct key_val *key){ return 0; }
OS_API_C_FUNC(int)				tree_node_eval_i64(mem_zone_ref_const_ptr p_node_ref, const char *key, enum op_type op, int64_t ivalue){ return 0; }
OS_API_C_FUNC(int)				tree_remove_child_by_member_value_lt_dword(mem_zone_ref_ptr p_node_ref, unsigned int child_type, const char *member_name, unsigned int value){ return 0; }

OS_API_C_FUNC(  int	)			tree_manager_get_child_value_i16(mem_zone_ref_const_ptr parent_ref_ptr, unsigned int crc_name, unsigned short *value){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_get_child_value_i32(mem_zone_ref_const_ptr parent_ref_ptr, unsigned int crc_name, unsigned int *value){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_get_child_value_i64(const mem_zone_ref	*p_node_ref, unsigned int crc_name, uint64_t *value){return 0;}
OS_API_C_FUNC(int)				tree_manager_get_child_value_si64(const mem_zone_ref	*p_node_ref, unsigned int crc_name, int64_t *value){ return 0; }
OS_API_C_FUNC(  int	)			tree_manager_get_child_data_ptr(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, mem_ptr *data_ptr){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_get_child_value_4uc(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, vec_4uc_t value){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_get_child_value_rect(const mem_zone_ref	*p_node_ref, unsigned int crc_name, struct gfx_rect *rect){return 0;}
OS_API_C_FUNC(  int	)	    	tree_manager_get_child_type(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_get_child_value_si32(mem_zone_ref_const_ptr	p_node_ref, unsigned int crc_name, int *value){return 0;}
OS_API_C_FUNC(  mem_ptr)			tree_manager_get_child_data(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, unsigned int ofset){return 0;}
OS_API_C_FUNC(  int		)		tree_manager_get_child_value_ptr(mem_zone_ref_const_ptr	p_node_ref, unsigned int node_hash, unsigned int ofset, mem_ptr *value){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_get_child_value_ipv4(mem_zone_ref_const_ptr	p_node_ref, unsigned int node_hash, ipv4_t value){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_get_child_value_vstr(mem_zone_ref_const_ptr	p_node_ref, unsigned int crc_name, mem_ptr vstr){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_get_child_value_hash(const mem_zone_ref	*p_node_ref, unsigned int crc_name, hash_t hash){return 0;}
OS_API_C_FUNC(int) tree_manager_get_child_value_float(const mem_zone_ref	*p_node_ref, unsigned int crc_name, float *value){ return 0; }

OS_API_C_FUNC(int)			tree_manager_allocate_child_data(mem_zone_ref_ptr parent_ref_ptr, const char *name, unsigned int size){ return 0; }
OS_API_C_FUNC(  int	)			tree_manager_set_child_value_i16(mem_zone_ref_ptr	p_node_ref, const char *name, unsigned short value){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_set_child_value_i32(mem_zone_ref_ptr	p_node_ref, const char *name, unsigned int value){return 0;}
OS_API_C_FUNC(int)			tree_manager_set_child_value_i64(mem_zone_ref_ptr	p_node_ref, const char *name, uint64_t value){ return 0; }
OS_API_C_FUNC(  int	)			tree_manager_set_child_value_float(mem_zone_ref_ptr	p_node_ref, const char *name, float value){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_set_child_value_double(mem_zone_ref_ptr	p_node_ref, const char *name, double value){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_set_child_value_4uc(mem_zone_ref_ptr	p_node_ref, const char *name, const vec_4uc_t value){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_set_child_value_rect(mem_zone_ref	*p_node_ref, const char *name, const struct gfx_rect *rect){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_set_child_value_bool(mem_zone_ref_ptr	p_node_ref, const char *name, unsigned int value){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_set_child_value_si32(mem_zone_ref_ptr	p_node_ref, const char *name, int value){return 0;}
OS_API_C_FUNC(  int	)	    	tree_manager_set_child_value_ptr(mem_zone_ref_ptr	p_node_ref, const char *name, mem_ptr value){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_set_child_value_str(mem_zone_ref_ptr	p_node_ref, const char *name, const char *str){return 0;}
OS_API_C_FUNC(int)				tree_manager_set_child_value_si64(mem_zone_ref_ptr p_node_ref, const char *name, int64_t value){ return 0; }
OS_API_C_FUNC(int)				tree_manager_set_child_value_ipv4(mem_zone_ref_ptr p_node_ref, const char *name, ipv4_t value){ return 0; }
OS_API_C_FUNC(  int	)			tree_manager_set_child_value_hash(mem_zone_ref_ptr p_node_ref, const char *name, const hash_t str){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_set_child_value_btcaddr(mem_zone_ref_ptr p_node_ref, const char *name, const btc_addr_t str){return 0;}


OS_API_C_FUNC(  int	)			tree_manager_set_child_value_rhash(mem_zone_ref_ptr p_node_ref, const char *name, const hash_t str){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_set_child_value_bhash(mem_zone_ref_ptr p_node_ref, const char *name, const hash_t str){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_set_child_value_vstr(mem_zone_ref_ptr p_node_ref, const char *name, const struct string *str){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_set_child_value_vint(mem_zone_ref_ptr p_node_ref, const char *name, const_mem_ptr vint){return 0;}
OS_API_C_FUNC(  int	)			tree_manager_set_child_value_vint32(mem_zone_ref_ptr p_node_ref, const char *name, unsigned int value){return 0;}

OS_API_C_FUNC(  void)				tree_manager_set_output(int output){return ;}
OS_API_C_FUNC(  void )				tree_manager_dump_node_rec(mem_zone_ref_const_ptr node_ref, unsigned int rec_level, unsigned int max_rec){return ;}

OS_API_C_FUNC(  unsigned int)		 node_array_pop(mem_zone_ref_ptr node_array, mem_zone_ref_ptr	node){return 0;}
OS_API_C_FUNC(  unsigned int)		 node_array_get_free_element(mem_zone_ref_ptr node_array, mem_zone_ref_ptr	node){return 0;}
OS_API_C_FUNC(  void	)			 init_node_array(mem_zone_ref_ptr node_array, unsigned int n_elems, const char *name, unsigned int type, unsigned int size_alloc){return ;}
OS_API_C_FUNC(const struct http_hdr *)	find_key(const struct http_hdr *hdrs, const char *key){ return PTR_NULL; }
OS_API_C_FUNC(struct http_hdr *) add_key(struct http_hdr *hdrs, const char *key, size_t key_len, const char *data, size_t data_len){return PTR_NULL;}
OS_API_C_FUNC(int)			tree_manager_copy_children_ref(mem_zone_ref_ptr dest_ref_ptr, mem_zone_ref_const_ptr src_ref_ptr){ return 0; }


OS_API_C_FUNC(void) free_http_infos(struct http_infos *infos){}
OS_API_C_FUNC(  void )		tree_manager_init(size_t size, size_t nzones, unsigned int flags) {return ;}
OS_API_C_FUNC(  int	)	tree_manager_json_loadb(const char *buffer, size_t buflen, mem_zone_ref_ptr result){return 0;}
OS_API_C_FUNC(  int	)	tree_manager_free_node_array(mem_zone_ref_ptr childs_ref_ptr){return 0;}
OS_API_C_FUNC(  void)		log_message(const char *fmt, mem_zone_ref_const_ptr args){return ;}

OS_API_C_FUNC(void) mbedtls_sha256_init(mbedtls_sha256_context *ctx){ return ; }
OS_API_C_FUNC(void) mbedtls_sha256_starts(mbedtls_sha256_context *ctx, int is224){ return; }
OS_API_C_FUNC(void) mbedtls_sha256_update(mbedtls_sha256_context *ctx, const unsigned char *input, size_t ilen){ return; }
OS_API_C_FUNC(void) mbedtls_sha256_free(mbedtls_sha256_context *ctx){ return; }
OS_API_C_FUNC(void) mbedtls_sha256_finish(mbedtls_sha256_context *ctx, unsigned char output[32]){ return; }
OS_API_C_FUNC(void) mbedtls_sha256(const unsigned char *input, size_t ilen, unsigned char output[32], int is224){ return; }
OS_API_C_FUNC(int) tree_manager_cat_node_childs(mem_zone_ref_ptr parent_ref_ptr, mem_zone_ref_ptr new_childs, unsigned int merge){ return 0; }
OS_API_C_FUNC(int)	crypto_hash_sha512(unsigned char *out, const unsigned char *in, size_t inlen){ return 0; }
OS_API_C_FUNC(int)	tree_remove_child_by_member_value_hash(mem_zone_ref_ptr p_node_ref, unsigned int child_type, const char *member_name, hash_t hash){ return 0; }

OS_API_C_FUNC(int) tree_manager_write_node_vec3f(mem_zone_ref *node_ref, mem_size ofset, float x, float y, float z){ return 0; }
OS_API_C_FUNC(int) tree_manager_write_node_vec2f(mem_zone_ref *node_ref, mem_size ofset, float x, float y) { return 0; }
OS_API_C_FUNC(int) load_script(const char *file, const char *name, mem_zone_ref_ptr script_vars, unsigned int opts){ return 0; }
OS_API_C_FUNC(int) tree_manager_node_dup(mem_zone_ref *new_parent, mem_zone_ref_const_ptr src_ref_ptr, mem_zone_ref *new_ref_ptr, unsigned int depth){ return 0; }
OS_API_C_FUNC(int) tree_remove_child_by_name(mem_zone_ref_ptr p_node_ref, unsigned int childkey){ return 0; }
OS_API_C_FUNC(void) init_upnp(){ return ; }

OS_API_C_FUNC(int) resolve_script_var(mem_zone_ref_ptr global_vars, mem_zone_ref_ptr script_proc, const char *var_path, unsigned int var_type, mem_zone_ref_ptr out_var, mem_zone_ref_ptr pout_var){ return 0; }
OS_API_C_FUNC(int) tree_manager_set_child_value_vec3(mem_zone_ref	*p_node_ref, const char *name, float x, float y, float z){ return 0; }
OS_API_C_FUNC(int) tree_manager_set_child_value_vec2(mem_zone_ref	*p_node_ref, const char *name, float x, float y) { return 0; }
OS_API_C_FUNC(int) tree_manager_get_child_value_double(const mem_zone_ref	*p_node_ref, unsigned int crc_name, double *value){ return 0; }
OS_API_C_FUNC(int) RC4(char *key, char *plaintext, size_t len, unsigned char *ciphertext){ return 0; }
OS_API_C_FUNC(void) qsort_c(mem_ptr base, mem_size num, mem_size width, int(*comp)(const_mem_ptr, const_mem_ptr)) { return 0; }
OS_API_C_FUNC(mem_ptr) tree_manager_expand_node_data_ptr(mem_zone_ref_ptr node_ref, mem_size ofset, mem_size size) { return 0; }
#ifdef _WIN32
C_EXPORT int _fltused = 0;
C_EXPORT mod_name_decoration_t	 mod_name_deco_type = MOD_NAME_DECO;
unsigned int C_API_FUNC _DllMainCRTStartup(unsigned int *prev, unsigned int *cur, unsigned int *xx)
{

	return 1;
}
#endif