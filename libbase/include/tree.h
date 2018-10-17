/*copyright antoine bentue-ferrer 2016*/
#define		NODE_GFX_STR			 		0x00000001
#define		NODE_GFX_INT			 		0x00000002
#define		NODE_GFX_BOOL			 		0x00000004
#define		NODE_GFX_SIGNED_INT			 	0x00000008
#define		NODE_GFX_RECT			 		0x00000010
#define		NODE_GFX_DATA			 		0x00000020
#define		NODE_GFX_PTR			 		0x00000040
#define		NODE_GFX_4UC			 		0x00000080
#define		NODE_GFX_BINT			 		0x00000100
#define		NODE_GFX_NULL			 		0x00000200
#define		NODE_GFX_OBJECT			 		0x00000400
#define		NODE_GFX_SIGNED_BINT		 	0x00000800
#define		NODE_GFX_FLOAT					0x00001000
#define		NODE_GFX_SHORT			 		0x00002000
#define		NODE_GFX_DOUBLE					0x00004000

#define		NODE_GFX_SCENE			 		0x04000001
#define		NODE_GFX_TEXT			 		0x04000002
#define		NODE_GFX_IMAGE_OBJ		 		0x04000004
#define		NODE_GFX_CTRL_DATA_COLUMN 		0x04000008
#define		NODE_GFX_CTRL_DATA_COLUMN_LIST	0x04000010
#define		NODE_GFX_STYLE			 		0x04000020
#define		NODE_GFX_CTRL			 		0x04000040
#define		NODE_GFX_CTRL_ITEM		 		0x04000080
#define		NODE_GFX_TEXT_LIST		 		0x04000100
#define		NODE_GFX_TEXT_LIST_ENTRY 		0x04000200
#define		NODE_GFX_CTRL_ITEM_DATA			0x04000400
#define		NODE_GFX_CTRL_ITEM_LIST	 		0x04000800
#define		NODE_GFX_EVENT_LIST		 		0x04001000
#define		NODE_GFX_EVENT			 		0x04002000
#define		NODE_GFX_RECT_OBJ		 		0x04004000
#define		NODE_GFX_IMAGE					0x04008000
#define		NODE_GFX_IMAGE_LIST				0x04010000
#define		NODE_GFX_CUBEMAP				0x04020000


#define		NODE_REQUEST					0x07000001
#define		NODE_MEM_AREA_LIST				0x07000002
#define		NODE_MEM_AREA					0x07000004
#define		NODE_MEM_AREA_DESC				0x07000008
#define		NODE_TREE_AREA_DESC				0x07000010
#define		NODE_TREE_NODE_DESC				0x07000020

#define		NODE_TASK_LIST					0x08000001
#define		NODE_TASK						0x08000002
#define		NODE_TASK_DATA					0x08000003
#define		NODE_SEMAPHORE					0x08000004
#define		NODE_MODULE_DEF					0x08000008
#define		NODE_MODULE_PROC				0x08000010
#define		NODE_MODULE_RPROC				0x08000020
#define		NODE_MODULE_RWPROC				0x08000040
#define		NODE_HTTP_REQUEST				0x08000080
#define		NODE_SERVICE					0x08000100
#define		NODE_HTTP_REQUEST_LIST			0x08000200

#define		NODE_SCRIPT_HANDLER_PROC		0x08000080
#define		NODE_SCRIPT_PROC				0x08000100
#define		NODE_SCRIPT_ACUM_PROC			0x08000200
#define		NODE_SCRIPT						0x08000400
#define		NODE_MSG_HANDLER				0x08000800
#define		NODE_MSG_HANDLER_LIST			0x08001000
#define		NODE_SCRIPT_PAGE_PROC			0x08002000
#define		NODE_SCRIPT_PAGE_PARAM			0x08004000
#define		NODE_SCRIPT_LIST				0x08008000
#define		NODE_SCRIPT_JSON_PROC			0x08010000

#define		NODE_JSON_ARRAY					0x09000001
#define		NODE_PUBCHILDS_ARRAY			0x09000002

#define		NODE_TYPE_POOL_LIST				0x0A000001
#define		NODE_TYPE_POOL					0x0A000002
#define		NODE_TYPE_POOL_TUPPLE_LIST		0x0A000003
#define		NODE_POOL_JOB_LIST				0x0A000005
#define		NODE_POOL_JOB					0x0A000006
#define		NODE_CON_LIST					0x0A000007
#define		NODE_CON						0x0A000008
#define		NODE_LOG_PARAMS					0x0A000010
#define		NODE_MD5_HASH					0x0A000020
#define		NODE_BIN_DATA					0x0A000040
#define		NODE_FILE_HASH					0x0A000080
#define		NODE_BITCORE_PUBKEY				0x0A000100

#define		NODE_BITCORE_NODE_LIST			0x0B000001
#define		NODE_BITCORE_NODE				0x0B000002
#define		NODE_BITCORE_MSG_LIST			0x0B000004
#define		NODE_BITCORE_MSG				0x0B000008
#define		NODE_BITCORE_PAYLOAD			0x0B000010
#define		NODE_BITCORE_ADDR_LIST			0x0B000020
#define		NODE_BITCORE_ADDR				0x0B000040
#define		NODE_BITCORE_ADDRT				0x0B000080
#define		NODE_BITCORE_VSTR				0x0B000100
#define		NODE_BITCORE_VINT				0x0B000200
#define		NODE_BITCORE_BLK_HDR_LIST		0x0B000400
#define		NODE_BITCORE_BLK_HDR			0x0B000800
#define		NODE_BITCORE_HASH				0x0B001000
#define		NODE_BITCORE_HASH_LIST			0x0B003000
#define		NODE_BITCORE_BLOCK_HASH			0x0B005000
#define		NODE_BITCORE_TX_HASH			0x0B007000
#define		NODE_BITCORE_WALLET_ADDR_LIST	0x0B008000
#define		NODE_BITCORE_WALLET_ADDR		0x0B009000

#define		NODE_BITCORE_BLOCK_LIST			0x0B003000
#define		NODE_BITCORE_BLOCK				0x0B002000
#define		NODE_BITCORE_TX_LIST			0x0B004000
#define		NODE_BITCORE_TX					0x0B008000
#define		NODE_BITCORE_VINLIST			0x0B010000
#define		NODE_BITCORE_TXIN				0x0B020000
#define		NODE_BITCORE_SCRIPT_OPCODE		0x0B040000
#define		NODE_BITCORE_VOUTLIST			0x0B080000
#define		NODE_BITCORE_TXOUT				0x0B100000
#define		NODE_BITCORE_SCRIPT				0x0B200000	
#define		NODE_BITCORE_LOCATOR			0x0B400000	
#define		NODE_BITCORE_ECDSA_SIG			0x0B800000


#define		NODE_NET_IP						0x0C000000
#define		NODE_COOKIES					0x0C000001


#define		NODE_RT_VEC2					0x0D000001
#define		NODE_RT_VEC3					0x0D000002
#define		NODE_RT_VEC3_ARRAY				0x0D000003
#define		NODE_RT_MAT3x3					0x0D000004
#define		NODE_RT_SHADER_UNIFORM_LIST		0x0D000006
#define		NODE_RT_MATERIAL_LIST			0x0D000009
#define		NODE_RT_VEC2_ARRAY				0x0D000010
#define		NODE_RT_INT_ARRAY				0x0D000020

const enum op_type { CMP_E, CMP_L, CMP_G, CMPL_E, CMPL_L, CMPL_G, CMPL_N, CMP_N };

struct key_val
{
	char			key[64];
	unsigned int	kcrc;
	enum op_type	op;
	struct string	value;
};

struct node_hash_val_t
{
	unsigned int		crc;
	mem_ptr				data;
};
#ifndef LIBBASE_API
	#define LIBBASE_API C_IMPORT
#endif




LIBBASE_API  int			C_API_FUNC	is_node_ptr								(const mem_ptr node);
LIBBASE_API  int			C_API_FUNC	is_node_ref								(const mem_zone_ref *ref);
LIBBASE_API  void			C_API_FUNC	tree_manager_dump_mem					(unsigned int time);

LIBBASE_API  int  			C_API_FUNC	tree_manager_create_node				(const char *name,unsigned int type,mem_zone_ref_ptr ref_ptr);
LIBBASE_API  int			C_API_FUNC	tree_manager_create_node_childs			(const char *name,unsigned int type,mem_zone_ref_ptr ref_ptr,const char *params);
LIBBASE_API  int			C_API_FUNC	tree_manager_create_node_params			(mem_zone_ref_ptr ref_ptr,const char *params);
LIBBASE_API  int			C_API_FUNC	tree_manager_add_node_childs			(mem_zone_ref_ptr p_node_ref,const char *params,unsigned int merge);
LIBBASE_API  int			C_API_FUNC	tree_manager_cat_node_childs			(mem_zone_ref_ptr parent_ref_ptr, mem_zone_ref_ptr new_childs, unsigned int merge);
LIBBASE_API  void			C_API_FUNC	tree_manager_sort_childs				(mem_zone_ref_ptr parent_ref_ptr,const char *name,unsigned int dir);

LIBBASE_API  int			C_API_FUNC	tree_manager_get_first_child			(mem_zone_ref_const_ptr p_node_ref,	mem_zone_ref_ptr child_list, mem_zone_ref_ptr *p_node_out_ref);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_next_child				(									mem_zone_ref_ptr child_list, mem_zone_ref_ptr *p_node_out_ref);

LIBBASE_API  int			C_API_FUNC	tree_manager_get_first_child_shared		(mem_zone_ref_const_ptr p_node_ref, mem_zone_ref_ptr *child_list, mem_zone_ref_ptr *p_node_out_ref);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_next_child_shared		(mem_zone_ref_ptr *child_list, mem_zone_ref_ptr *p_node_out_ref);


LIBBASE_API  int			C_API_FUNC	tree_manager_get_last_child				(mem_zone_ref_const_ptr p_node_ref, mem_zone_ref_ptr child_list, mem_zone_ref_ptr *p_node_out_ref);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_prev_child				(mem_zone_ref_ptr child_list,mem_zone_ref_ptr *p_node_out_ref);

LIBBASE_API  int			C_API_FUNC	tree_release_node_child_lock			(mem_zone_ref_ptr p_node_ref, unsigned int key_crc);
LIBBASE_API  int			C_API_FUNC	tree_aquire_node_child_lock				(mem_zone_ref_ptr p_node_ref, unsigned int key_crc, mem_zone_ref	*out);

LIBBASE_API  unsigned int	C_API_FUNC	and_node_type							(unsigned int type1,unsigned int type2);
LIBBASE_API  const char *	C_API_FUNC	tree_mamanger_get_node_name				(mem_zone_ref_const_ptr node_ref);
LIBBASE_API  unsigned int	C_API_FUNC	tree_mamanger_get_node_type				(mem_zone_ref_const_ptr node_ref);
LIBBASE_API  unsigned int	C_API_FUNC	tree_mamanger_get_node_type_str			(const char *name);


LIBBASE_API  int			C_API_FUNC  tree_mamanger_get_parent				(mem_zone_ref_const_ptr node_ref,mem_zone_ref_ptr p_ref);
LIBBASE_API  int			C_API_FUNC  tree_manager_get_ancestor_by_type		(mem_zone_ref_const_ptr node_ref,unsigned int node_type,mem_zone_ref_ptr  p_ref);
LIBBASE_API  size_t			C_API_FUNC	tree_manager_get_node_num_children		(mem_zone_ref_const_ptr p_node_ref);
LIBBASE_API  void			C_API_FUNC	tree_manager_set_node_name				(mem_zone_ref_ptr node_ref,const char *name);

LIBBASE_API  int  			C_API_FUNC	tree_manager_add_child_node				(mem_zone_ref_ptr parent_ref_ptr,const char *name,unsigned int type,mem_zone_ref *ref_ptr);
LIBBASE_API  unsigned int	C_API_FUNC	tree_manager_node_add_child				(mem_zone_ref_ptr parent_ref_ptr,mem_zone_ref_const_ptr child_ref_ptr);
LIBBASE_API  unsigned int	C_API_FUNC	tree_manager_allocate_childs			(mem_zone_ref_ptr p_node_ref, unsigned int size);
LIBBASE_API  unsigned int	C_API_FUNC	tree_manager_node_add_unique_child		(mem_zone_ref_ptr p_node_ref, mem_zone_ref_const_ptr child_ref_ptr, const char *keys);
LIBBASE_API  int			C_API_FUNC	tree_manager_node_dup					(mem_zone_ref_ptr new_parent,mem_zone_ref_const_ptr src_ref_ptr,mem_zone_ref_ptr new_ref_ptr,unsigned int depth);
LIBBASE_API  int			C_API_FUNC	tree_manager_node_dup_one				(mem_zone_ref_ptr src_ref_ptr,mem_zone_ref_ptr new_ref_ptr);
LIBBASE_API  int			C_API_FUNC	tree_manager_copy_children				(mem_zone_ref_ptr dest_ref_ptr,mem_zone_ref_const_ptr src_ref_ptr,unsigned int depth);
LIBBASE_API  int			C_API_FUNC	tree_manager_copy_children_ref			(mem_zone_ref_ptr dest_ref_ptr, mem_zone_ref_const_ptr src_ref_ptr);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_child_at				(mem_zone_ref_const_ptr parent_ref_ptr	,unsigned int index,mem_zone_ref_ptr ref_ptr);
LIBBASE_API  int			C_API_FUNC	tree_manager_find_child_node			(mem_zone_ref_const_ptr parent_ref_ptr	,unsigned int crc_name,unsigned int type,mem_zone_ref_ptr ref_ptr);

LIBBASE_API  int			C_API_FUNC	 tree_node_read_childs					(mem_zone_ref_const_ptr p_node_ref	,struct node_hash_val_t *list);

LIBBASE_API  int			C_API_FUNC	tree_node_find_child_by_name			(mem_zone_ref_const_ptr p_node_ref	,const char *name						,mem_zone_ref_ptr p_node_out_ref);
LIBBASE_API  unsigned int	C_API_FUNC	tree_node_list_child_by_type			(mem_zone_ref_const_ptr p_node_ref	,unsigned int type						,mem_zone_ref_ptr p_node_out_ref,unsigned int index);
LIBBASE_API  int			C_API_FUNC	tree_manager_list_child_type			(mem_zone_ref_ptr child_list, unsigned int type, unsigned int *index, mem_zone_ref_ptr *p_node_out_ref);
LIBBASE_API  int			C_API_FUNC	tree_node_find_child_by_type_value		(mem_zone_ref_const_ptr node,unsigned int type,unsigned int value,mem_zone_ref_ptr p_node_out_ref);
LIBBASE_API  int			C_API_FUNC	tree_node_find_child_by_type			(mem_zone_ref_const_ptr p_node_ref		,unsigned int node_type					,mem_zone_ref_ptr p_node_out_ref,unsigned int index);
LIBBASE_API  int			C_API_FUNC	tree_node_find_child_by_id				(mem_zone_ref_const_ptr p_node_ref		,unsigned int node_id,mem_zone_ref_ptr p_node_out_ref);
LIBBASE_API  int			C_API_FUNC	tree_node_eval_i64						(mem_zone_ref_const_ptr p_node_ref, const char *key, enum op_type op,int64_t ivalue);
LIBBASE_API  int			C_API_FUNC	tree_node_keval_i64						(mem_zone_ref_const_ptr p_node_ref, const struct key_val *key);
LIBBASE_API  int			C_API_FUNC	tree_node_keval_hash					(mem_zone_ref_const_ptr p_node_ref, const struct key_val *key);
LIBBASE_API  int			C_API_FUNC	tree_node_keval_str						(mem_zone_ref_const_ptr p_node_ref, const struct key_val *key);
LIBBASE_API  int			C_API_FUNC	tree_node_keval							(mem_zone_ref_const_ptr p_node_ref, struct key_val *key);
LIBBASE_API  int			C_API_FUNC	tree_find_child_node_by_id_name			(mem_zone_ref_const_ptr  p_node_ref,unsigned int child_type,const char *id_name,unsigned int id_val,mem_zone_ref_ptr out_node);
LIBBASE_API  int			C_API_FUNC	tree_find_child_node_by_value_str		(mem_zone_ref_const_ptr p_node_ref, unsigned int child_type, const char *keyname, const char *val, mem_zone_ref *p_node_out_ref);
/*LIBBASE_API  int			C_API_FUNC	tree_find_child_node_by_value			(mem_zone_ref_const_ptr  p_node_ref,unsigned int child_type,const char *id_name,unsigned int id_val,mem_zone_ref_ptr out_node);*/
LIBBASE_API  int			C_API_FUNC  tree_find_child_node_idx_by_id			(mem_zone_ref *p_node_ref,unsigned int child_type,unsigned int child_id,unsigned int *out_idx);
LIBBASE_API  int			C_API_FUNC	tree_find_child_node_by_member_name		(mem_zone_ref_const_ptr p_node_ref,unsigned int child_type, unsigned int child_member_type,const char *child_member_name,mem_zone_ref_ptr out_node);
LIBBASE_API  int			C_API_FUNC	tree_find_child_node_by_member_name_hash(mem_zone_ref_const_ptr p_node_ref,unsigned int child_type, const char *child_member_name,const hash_t hash, mem_zone_ref_ptr out_node);

LIBBASE_API  int			C_API_FUNC	tree_find_child_node_by_hash			(mem_zone_ref_const_ptr p_node_ref, unsigned int child_type, hash_t bhash, mem_zone_ref_ptr out_node);
LIBBASE_API  int			C_API_FUNC	tree_swap_child_node_by_id				(mem_zone_ref_ptr p_node_ref,unsigned int id_val,mem_zone_ref_ptr node);
LIBBASE_API  int			C_API_FUNC	tree_manager_swap_child_ref				(mem_zone_ref_const_ptr parent_ref_ptr, unsigned int crc_name, unsigned int type, mem_zone_ref_ptr ref_ptr);

LIBBASE_API  int			C_API_FUNC	tree_remove_children					(mem_zone_ref_ptr p_node_ref);
LIBBASE_API  int			C_API_FUNC	tree_remove_child_by_type				(mem_zone_ref_ptr p_node_ref,unsigned int child_type);
LIBBASE_API  int			C_API_FUNC	tree_remove_child_by_name				(mem_zone_ref_ptr p_node_ref, unsigned int childkey);
LIBBASE_API  int			C_API_FUNC	tree_remove_child_by_id					(mem_zone_ref_ptr p_node_ref,unsigned int child_id);

LIBBASE_API  int			C_API_FUNC	tree_remove_child_by_value_dword		(mem_zone_ref_ptr p_node_ref, unsigned int type, unsigned int value);
LIBBASE_API  int			C_API_FUNC	tree_remove_child_by_value_str			(mem_zone_ref_ptr p_node_ref, unsigned int type, const char *value);

LIBBASE_API  int			C_API_FUNC	tree_remove_child_by_member_value_dword	(mem_zone_ref_ptr p_node_ref,unsigned int child_type,const char *member_name,unsigned int value);
LIBBASE_API  int			C_API_FUNC	tree_remove_child_by_member_value_hash	(mem_zone_ref_ptr p_node_ref, unsigned int child_type, const char *member_name, hash_t hash);

LIBBASE_API  int			C_API_FUNC	tree_remove_child_by_member_value_lt_dword(mem_zone_ref_ptr p_node_ref, unsigned int child_type, const char *member_name, unsigned int value);


LIBBASE_API  int  			C_API_FUNC	tree_manager_allocate_node_data			(mem_zone_ref_ptr node_ref,mem_size data_size);
LIBBASE_API  mem_ptr		C_API_FUNC	tree_manager_expand_node_data_ptr		(mem_zone_ref_ptr node_ref, mem_size ofset, mem_size size);
LIBBASE_API  int	 		C_API_FUNC	tree_manager_write_node_data			(mem_zone_ref_ptr node_ref,const_mem_ptr data,mem_size ofset,mem_size size);
LIBBASE_API  int			C_API_FUNC	tree_manager_copy_node_data				(mem_zone_ref_ptr dst_node,mem_zone_ref_const_ptr src_node);
LIBBASE_API  int	 		C_API_FUNC	tree_manager_write_node_dword			(mem_zone_ref_ptr node_ref,mem_size ofset,unsigned int value);
LIBBASE_API  int	 		C_API_FUNC	tree_manager_write_node_dword_pr		(mem_zone_ref_ptr node_ref, mem_size ofset, unsigned int value);
LIBBASE_API  int	 		C_API_FUNC	tree_manager_cmp_z_xchg_node_dword		(mem_zone_ref_ptr node_ref,mem_size ofset,unsigned int value);
LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_qword			(mem_zone_ref *node_ref,mem_size ofset,uint64_t value);
LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_ptr				(mem_zone_ref *node_ref, mem_size ofset, mem_ptr value);
LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_float			(mem_zone_ref *node_ref,mem_size ofset,float value);
LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_double			(mem_zone_ref *node_ref, mem_size ofset, double value);
LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_hash			(mem_zone_ref_ptr node_ref, mem_size ofset, const hash_t hash);
LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_btcaddr			(mem_zone_ref_ptr node_ref, mem_size ofset, const btc_addr_t addr);
LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_rhash			(mem_zone_ref_ptr node_ref, mem_size ofset, const hash_t hash);
LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_vstr			(mem_zone_ref_ptr node_ref, mem_size ofset, const struct string *str);
LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_vint			(mem_zone_ref_ptr node_ref, mem_size ofset, const_mem_ptr vint);
LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_sig				(mem_zone_ref_ptr node_ref, mem_size ofset, unsigned char *sign, size_t sign_len);
LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_4uc				(mem_zone_ref_ptr node_ref,mem_size ofset,const vec_4uc_t val);
LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_vec3f			(mem_zone_ref *node_ref, mem_size ofset, float x, float y, float z);
LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_vec2f			(mem_zone_ref *node_ref, mem_size ofset, float x, float y);

LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_word			(mem_zone_ref_ptr node_ref,mem_size ofset,unsigned short value);
LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_byte			(mem_zone_ref_ptr node_ref,mem_size ofset,unsigned char value);
LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_signed_qword	(mem_zone_ref_ptr node_ref, mem_size ofset, int64_t value);
LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_signed_dword    (mem_zone_ref_ptr node_ref, mem_size ofset, int value);
LIBBASE_API  int			C_API_FUNC	tree_manager_write_node_signed_word		(mem_zone_ref_ptr node_ref,mem_size ofset,short value);
LIBBASE_API  int		    C_API_FUNC	tree_manager_write_node_str				(mem_zone_ref_ptr node_ref,mem_size ofset,const char *str);
LIBBASE_API  void			C_API_FUNC	tree_manager_set_node_image_info		(mem_zone_ref_ptr node_ref,mem_size position,mem_size size);

/* LIBBASE_API  void			C_API_FUNC	tree_mamanger_get_node_data_ref			(mem_zone_ref_const_ptr node_ref,mem_zone_ref_ptr out); */

LIBBASE_API  size_t			C_API_FUNC	tree_manager_get_node_image_size		(mem_zone_ref_const_ptr node_ref);
LIBBASE_API  size_t			C_API_FUNC	tree_manager_get_node_image_pos			(mem_zone_ref_const_ptr node_ref);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_node_4uc				(mem_zone_ref_const_ptr node_ref,mem_size ofset,vec_4uc_t val);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_node_vec2f				(mem_zone_ref_const_ptr node_ref, mem_size ofset, float *x, float *y);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_node_vec3f				(mem_zone_ref_const_ptr node_ref, mem_size ofset, float *x, float *y, float *z);


LIBBASE_API  int			C_API_FUNC	tree_mamanger_get_node_ipv4				(mem_zone_ref_const_ptr node_ref, mem_size ofset, ipv4_t val);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_node_str				(mem_zone_ref_const_ptr node_ref,mem_size ofset,char *str,unsigned int str_len,unsigned int base);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_node_ptr(mem_zone_ref *node_ref, mem_size ofset, mem_ptr *value);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_node_istr				(mem_zone_ref_const_ptr node_ref,mem_size ofset,struct string *str,unsigned int base);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_node_vstr				(mem_zone_ref_const_ptr node_ref, mem_size ofset, mem_ptr vstr);
LIBBASE_API  int			C_API_FUNC	tree_mamanger_get_node_dword			(mem_zone_ref_const_ptr node_ref,mem_size ofset,unsigned int *val);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_node_hash				(mem_zone_ref_const_ptr node_ref, mem_size ofset, hash_t hash);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_node_btcaddr			(mem_zone_ref_ptr node_ref, mem_size ofset, btc_addr_t addr);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_node_rhash				(mem_zone_ref_const_ptr node_ref, mem_size ofset, hash_t hash);
LIBBASE_API  int			C_API_FUNC	tree_mamanger_get_node_qword			(mem_zone_ref_const_ptr node_ref, mem_size ofset, uint64_t *val);
LIBBASE_API  int			C_API_FUNC	tree_mamanger_get_node_signed_qword		(mem_zone_ref_const_ptr node_ref, mem_size ofset, int64_t *val);
LIBBASE_API  int			C_API_FUNC	tree_mamanger_get_node_float			(mem_zone_ref_const_ptr node_ref,mem_size ofset,float *val);
LIBBASE_API  int			C_API_FUNC	tree_mamanger_get_node_double			(mem_zone_ref_const_ptr node_ref, mem_size ofset, double *val);
LIBBASE_API  int			C_API_FUNC	tree_mamanger_get_node_signed_dword		(mem_zone_ref_const_ptr node_ref,mem_size ofset,int *val);
LIBBASE_API  int			C_API_FUNC	tree_mamanger_get_node_word				(mem_zone_ref_const_ptr node_ref,mem_size ofset,unsigned short *val);
LIBBASE_API  int			C_API_FUNC	tree_mamanger_get_node_signed_word		(mem_zone_ref_const_ptr node_ref,mem_size ofset,short *val);
LIBBASE_API  int			C_API_FUNC	tree_mamanger_get_node_byte				(mem_zone_ref_const_ptr node_ref,mem_size ofset,unsigned char *val);
LIBBASE_API  mem_ptr		C_API_FUNC	tree_mamanger_get_node_data_ptr			(mem_zone_ref_const_ptr node_ref,mem_size ofset);
LIBBASE_API  size_t			C_API_FUNC	tree_mamanger_get_node_data_size		(mem_zone_ref_const_ptr node_ref);

LIBBASE_API  unsigned int	C_API_FUNC	tree_manager_compare_node_crc			(mem_zone_ref_ptr node_ref,unsigned int crc);
LIBBASE_API  int			C_API_FUNC	tree_mamanger_compare_node_dword		(mem_zone_ref_ptr node_ref,mem_size ofset,unsigned int val);
LIBBASE_API  int			C_API_FUNC	tree_mamanger_compare_node_str			(mem_zone_ref_ptr node_ref, mem_size ofset, const char *val);

LIBBASE_API  int			C_API_FUNC	tree_manager_allocate_child_data		(mem_zone_ref_ptr p_node_ref, const char *name, unsigned int size);

LIBBASE_API  mem_ptr		C_API_FUNC	tree_manager_get_child_data				(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, unsigned int ofset);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_child_data_ptr			(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, mem_ptr *data_ptr);
LIBBASE_API  int		    C_API_FUNC	tree_manager_get_child_type				(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name);

LIBBASE_API  int			C_API_FUNC	tree_manager_get_child_value_i64		(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, uint64_t *value);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_child_value_i32		(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, unsigned int *value);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_child_value_i16		(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, unsigned short *value);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_child_value_si64		(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, int64_t *value);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_child_value_si32		(mem_zone_ref_const_ptr	p_node_ref, unsigned int crc_name, int *value);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_child_value_ptr		(mem_zone_ref_const_ptr	p_node_ref, unsigned int crc_name, unsigned int ofset,mem_ptr *value);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_child_value_float		(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, float *value);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_child_value_double		(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, double *value);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_child_value_hash		(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, hash_t hash);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_child_value_btcaddr	(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, btc_addr_t addr);
LIBBASE_API  int		    C_API_FUNC	tree_manager_get_child_value_str		(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, char *str, unsigned int str_len, unsigned int base);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_child_value_4uc		(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, vec_4uc_t value);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_child_value_rect		(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, struct gfx_rect *rect);
LIBBASE_API  int		    C_API_FUNC	tree_manager_get_child_value_istr		(mem_zone_ref_const_ptr p_node_ref, unsigned int crc_name, struct string *str, unsigned int base);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_child_value_vstr		(mem_zone_ref_const_ptr	p_node_ref, unsigned int crc_name, mem_ptr vstr);
LIBBASE_API  int			C_API_FUNC	tree_manager_get_child_value_ipv4		(mem_zone_ref_const_ptr	p_node_ref, unsigned int crc_name, ipv4_t value);


LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_i64		(mem_zone_ref_ptr p_node_ref, const char *name, uint64_t value);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_i32		(mem_zone_ref_ptr p_node_ref, const char *name, unsigned int value);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_i16		(mem_zone_ref_ptr p_node_ref, const char *name, unsigned short value);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_si64		(mem_zone_ref_ptr p_node_ref, const char *name, int64_t value);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_si32		(mem_zone_ref_ptr p_node_ref, const char *name, int value);
LIBBASE_API  int		    C_API_FUNC	tree_manager_set_child_value_ptr		(mem_zone_ref_ptr p_node_ref, const char *name, mem_ptr value);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_float		(mem_zone_ref_ptr p_node_ref, const char *name, float value);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_double		(mem_zone_ref_ptr p_node_ref, const char *name, double value);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_hash		(mem_zone_ref_ptr p_node_ref, const char *name, const hash_t str);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_btcaddr	(mem_zone_ref_ptr p_node_ref, const char *name, const btc_addr_t str);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_rhash		(mem_zone_ref_ptr p_node_ref, const char *name, const hash_t str);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_bhash		(mem_zone_ref_ptr p_node_ref, const char *name, const hash_t str);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_thash		(mem_zone_ref_ptr p_node_ref, const char *name, const hash_t str);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_vstr		(mem_zone_ref_ptr p_node_ref, const char *name, const struct string *str);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_vint		(mem_zone_ref_ptr p_node_ref, const char *name, const_mem_ptr vint);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_vint32		(mem_zone_ref_ptr p_node_ref, const char *name, unsigned int value);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_4uc		(mem_zone_ref_ptr p_node_ref, const char *name, const vec_4uc_t value);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_str		(mem_zone_ref_ptr p_node_ref, const char *name, const char *str);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_rect		(mem_zone_ref_ptr p_node_ref, const char *name, const struct gfx_rect *rect);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_bool		(mem_zone_ref_ptr p_node_ref, const char *name, unsigned int value);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_vec3		(mem_zone_ref_ptr p_node_ref, const char *name, float x, float y, float z);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_vec2		(mem_zone_ref	*p_node_ref, const char *name, float x, float y);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_vec3_arr   (mem_zone_ref_ptr p_node_ref, const char *name, size_t idx,float x, float y, float z);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_mat3x3		(mem_zone_ref_ptr p_node_ref, const char *name, float *mat3x3);
LIBBASE_API  int			C_API_FUNC	tree_manager_set_child_value_ipv4		(mem_zone_ref_ptr p_node_ref, const char *name, ipv4_t value);


LIBBASE_API  void			C_API_FUNC	tree_manager_set_output					(int output);
LIBBASE_API  void 			C_API_FUNC	tree_manager_dump_node_rec				(mem_zone_ref_const_ptr node_ref,unsigned int rec_level,unsigned int max_rec);

LIBBASE_API  unsigned int	C_API_FUNC	 node_array_pop							(mem_zone_ref_ptr node_array,mem_zone_ref_ptr	node);
LIBBASE_API  unsigned int	C_API_FUNC	 node_array_get_free_element			(mem_zone_ref_ptr node_array,mem_zone_ref_ptr	node);
LIBBASE_API  void			C_API_FUNC	 init_node_array						(mem_zone_ref_ptr node_array,unsigned int n_elems,const char *name,unsigned int type,unsigned int size_alloc);

LIBBASE_API  void			C_API_FUNC	qsort_ctx_c								(mem_ptr base, mem_size num, mem_size width, unsigned int context, int(*comp)(unsigned int, const_mem_ptr, const_mem_ptr));

LIBBASE_API	void			C_API_FUNC	tree_manager_sort_childs				(mem_zone_ref_ptr parent_ref_ptr, const char *name, unsigned int dir);

LIBBASE_API  void 	C_API_FUNC	tree_manager_init								(size_t size,size_t nzones,unsigned int flags);
LIBBASE_API  void	C_API_FUNC	tree_manager_free								();
LIBBASE_API  int	C_API_FUNC	tree_manager_json_loadb							(const char *buffer, size_t buflen, mem_zone_ref_ptr result);
LIBBASE_API  int	C_API_FUNC	tree_manager_json_loadb_as						(const char *buffer, size_t buflen, const char *name, unsigned int type, mem_zone_ref_ptr result);
//LIBBASE_API  int	C_API_FUNC	tree_manager_free_node_array					(mem_zone_ref_ptr childs_ref_ptr, unsigned int scan_id);
LIBBASE_API  void	C_API_FUNC	log_message										(const char *fmt, mem_zone_ref_const_ptr args);

LIBBASE_API  int	C_API_FUNC	tree_manager_get_first_child_type_lck			(mem_zone_ref_const_ptr p_node_ref, mem_zone_ref_ptr *child_list, unsigned int type, size_t lock_ofs, mem_zone_ref_ptr *p_node_out_ref);
LIBBASE_API  int	C_API_FUNC	tree_manager_get_next_child_type_lck			(mem_zone_ref_ptr child_list, unsigned int type, size_t lock_ofs, mem_zone_ref_ptr *p_node_out_ref);



#define NODE_HASH(name) calc_crc32_c(name,32)



/*

// the depth of the hashing
#define CONSTHASH_DEPTH 64

// randomly generated constants.  The bottom half has to be FFFF or
// else the entire hash loses some strength
static const size_t CONSTHASH_CONSTANTS[CONSTHASH_DEPTH + 1] =
{
0x6157FFFF, 0x5387FFFF, 0x8ECBFFFF, 0xB3DBFFFF, 0x1AFDFFFF, 0xD1FDFFFF, 0x19B3FFFF, 0xE6C7FFFF,
0x53BDFFFF, 0xCDAFFFFF, 0xE543FFFF, 0x369DFFFF, 0x8135FFFF, 0x50E1FFFF, 0x115BFFFF, 0x07D1FFFF,
0x9AA1FFFF, 0x4D87FFFF, 0x442BFFFF, 0xEAA5FFFF, 0xAEDBFFFF, 0xB6A5FFFF, 0xAFE9FFFF, 0xE895FFFF,
0x4E05FFFF, 0xF8BFFFFF, 0xCB5DFFFF, 0x2F85FFFF, 0xF1DFFFFF, 0xD5ADFFFF, 0x438DFFFF, 0x6073FFFF,
0xA99FFFFF, 0x2E0BFFFF, 0xF729FFFF, 0x5D01FFFF, 0x1ACDFFFF, 0xFAD1FFFF, 0xD86BFFFF, 0xE909FFFF,
0xD3BDFFFF, 0xF35BFFFF, 0xD53DFFFF, 0x4DC1FFFF, 0x6897FFFF, 0x6E4DFFFF, 0x305BFFFF, 0x6F0DFFFF,
0x33C9FFFF, 0xC955FFFF, 0xC1EDFFFF, 0x48D5FFFF, 0x0CF5FFFF, 0x356BFFFF, 0x5F65FFFF, 0x71C1FFFF,
0x3F13FFFF, 0x489DFFFF, 0xEBA3FFFF, 0x340DFFFF, 0xF537FFFF, 0xD5E7FFFF, 0x6D27FFFF, 0x89D7FFFF,
0xA93FFFFF,
};

// multiplication constants, this allows an abstract use
// of the string length
static const size_t CONSTHASH_MULTS[CONSTHASH_DEPTH + 1] =
{
33, 34, 35, 36, 37, 38, 39, 40,
41, 42, 43, 44, 45, 46, 47, 48,
49, 50, 51, 52, 53, 54, 55, 56,
57, 58, 59, 60, 61, 62, 63, 64,
65, 66, 67, 68, 69, 70, 71, 72,
73, 74, 75, 76, 77, 78, 79, 80,
81, 82, 83, 84, 85, 86, 87, 88,
89, 90, 91, 92, 93, 94, 95, 96,
97,
};

#define CONSTHASH_RECURSE_00(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[0] : CONSTHASH_MULTS[0] * CONSTHASH_RECURSE_01(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_01(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[1] : CONSTHASH_MULTS[1] * CONSTHASH_RECURSE_02(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_02(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[2] : CONSTHASH_MULTS[2] * CONSTHASH_RECURSE_03(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_03(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[3] : CONSTHASH_MULTS[3] * CONSTHASH_RECURSE_04(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_04(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[4] : CONSTHASH_MULTS[4] * CONSTHASH_RECURSE_05(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_05(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[5] : CONSTHASH_MULTS[5] * CONSTHASH_RECURSE_06(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_06(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[6] : CONSTHASH_MULTS[6] * CONSTHASH_RECURSE_07(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_07(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[7] : CONSTHASH_MULTS[7] * CONSTHASH_RECURSE_08(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_08(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[8] : CONSTHASH_MULTS[8] * CONSTHASH_RECURSE_09(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_09(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[9] : CONSTHASH_MULTS[9] * CONSTHASH_RECURSE_10(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_10(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[10] : CONSTHASH_MULTS[10] * CONSTHASH_RECURSE_11(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_11(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[11] : CONSTHASH_MULTS[11] * CONSTHASH_RECURSE_12(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_12(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[12] : CONSTHASH_MULTS[12] * CONSTHASH_RECURSE_13(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_13(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[13] : CONSTHASH_MULTS[13] * CONSTHASH_RECURSE_14(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_14(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[14] : CONSTHASH_MULTS[14] * CONSTHASH_RECURSE_15(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_15(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[15] : CONSTHASH_MULTS[15] * CONSTHASH_RECURSE_16(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_16(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[16] : CONSTHASH_MULTS[16] * CONSTHASH_RECURSE_17(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_17(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[17] : CONSTHASH_MULTS[17] * CONSTHASH_RECURSE_18(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_18(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[18] : CONSTHASH_MULTS[18] * CONSTHASH_RECURSE_19(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_19(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[19] : CONSTHASH_MULTS[19] * CONSTHASH_RECURSE_20(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_20(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[20] : CONSTHASH_MULTS[20] * CONSTHASH_RECURSE_21(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_21(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[21] : CONSTHASH_MULTS[21] * CONSTHASH_RECURSE_22(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_22(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[22] : CONSTHASH_MULTS[22] * CONSTHASH_RECURSE_23(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_23(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[23] : CONSTHASH_MULTS[23] * CONSTHASH_RECURSE_24(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_24(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[24] : CONSTHASH_MULTS[24] * CONSTHASH_RECURSE_25(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_25(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[25] : CONSTHASH_MULTS[25] * CONSTHASH_RECURSE_26(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_26(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[26] : CONSTHASH_MULTS[26] * CONSTHASH_RECURSE_27(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_27(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[27] : CONSTHASH_MULTS[27] * CONSTHASH_RECURSE_28(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_28(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[28] : CONSTHASH_MULTS[28] * CONSTHASH_RECURSE_29(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_29(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[29] : CONSTHASH_MULTS[29] * CONSTHASH_RECURSE_30(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_30(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[30] : CONSTHASH_MULTS[30] * CONSTHASH_RECURSE_31(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_31(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[31] : CONSTHASH_MULTS[31] * CONSTHASH_RECURSE_32(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_32(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[32] : CONSTHASH_MULTS[32] * CONSTHASH_RECURSE_33(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_33(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[33] : CONSTHASH_MULTS[33] * CONSTHASH_RECURSE_34(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_34(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[34] : CONSTHASH_MULTS[34] * CONSTHASH_RECURSE_35(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_35(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[35] : CONSTHASH_MULTS[35] * CONSTHASH_RECURSE_36(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_36(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[36] : CONSTHASH_MULTS[36] * CONSTHASH_RECURSE_37(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_37(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[37] : CONSTHASH_MULTS[37] * CONSTHASH_RECURSE_38(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_38(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[38] : CONSTHASH_MULTS[38] * CONSTHASH_RECURSE_39(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_39(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[39] : CONSTHASH_MULTS[39] * CONSTHASH_RECURSE_40(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_40(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[40] : CONSTHASH_MULTS[40] * CONSTHASH_RECURSE_41(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_41(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[41] : CONSTHASH_MULTS[41] * CONSTHASH_RECURSE_42(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_42(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[42] : CONSTHASH_MULTS[42] * CONSTHASH_RECURSE_43(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_43(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[43] : CONSTHASH_MULTS[43] * CONSTHASH_RECURSE_44(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_44(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[44] : CONSTHASH_MULTS[44] * CONSTHASH_RECURSE_45(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_45(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[45] : CONSTHASH_MULTS[45] * CONSTHASH_RECURSE_46(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_46(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[46] : CONSTHASH_MULTS[46] * CONSTHASH_RECURSE_47(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_47(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[47] : CONSTHASH_MULTS[47] * CONSTHASH_RECURSE_48(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_48(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[48] : CONSTHASH_MULTS[48] * CONSTHASH_RECURSE_49(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_49(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[49] : CONSTHASH_MULTS[49] * CONSTHASH_RECURSE_50(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_50(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[50] : CONSTHASH_MULTS[50] * CONSTHASH_RECURSE_51(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_51(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[51] : CONSTHASH_MULTS[51] * CONSTHASH_RECURSE_52(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_52(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[52] : CONSTHASH_MULTS[52] * CONSTHASH_RECURSE_53(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_53(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[53] : CONSTHASH_MULTS[53] * CONSTHASH_RECURSE_54(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_54(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[54] : CONSTHASH_MULTS[54] * CONSTHASH_RECURSE_55(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_55(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[55] : CONSTHASH_MULTS[55] * CONSTHASH_RECURSE_56(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_56(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[56] : CONSTHASH_MULTS[56] * CONSTHASH_RECURSE_57(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_57(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[57] : CONSTHASH_MULTS[57] * CONSTHASH_RECURSE_58(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_58(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[58] : CONSTHASH_MULTS[58] * CONSTHASH_RECURSE_59(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_59(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[59] : CONSTHASH_MULTS[59] * CONSTHASH_RECURSE_60(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_60(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[60] : CONSTHASH_MULTS[60] * CONSTHASH_RECURSE_61(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_61(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[61] : CONSTHASH_MULTS[61] * CONSTHASH_RECURSE_62(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_62(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[62] : CONSTHASH_MULTS[62] * CONSTHASH_RECURSE_63(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_63(string, value) CONSTHASH_FUNCTION((*(string+1) == 0 ? CONSTHASH_CONSTANTS[63] : CONSTHASH_MULTS[63] * CONSTHASH_RECURSE_64(string+1, *(string+1))), value)
#define CONSTHASH_RECURSE_64(string, value) CONSTHASH_CONSTANTS[64]

// The following is the function used for hashing
// Do NOT use NEXTHASH more than once, it will cause
// N-Squared expansion and make compilation very slow
// If not impossible
#define CONSTHASH_FUNCTION(next, value) ((value << 15) | value | 33) + ((11 * value * value * 257) ^ CONSTHASH_CONSTANTS[value & 0x3f] ^ (next))

// finally the macro used to generate the hash
//#define STATIC_HASH(string) CONSTHASH_RECURSE_00(string, *string)


#define H1(s,i,x)   (x*65599u+(unsigned char)s[(i)<sizeof(s)?sizeof(s)-1-(i):sizeof(s)])
#define H4(s,i,x)   H1(s,i,H1(s,i+1,H1(s,i+2,H1(s,i+3,x))))
#define H16(s,i,x)  H4(s,i,H4(s,i+4,H4(s,i+8,H4(s,i+12,x))))
#define H64(s,i,x)  H16(s,i,H16(s,i+16,H16(s,i+32,H16(s,i+48,x))))
#define H256(s,i,x) H64(s,i,H64(s,i+64,H64(s,i+128,H64(s,i+192,x))))

#define STATIC_HASH(s)    (H256(s,0,0)^(H256(s,0,0)>>16))


#define STR_TYPE(type) str_##type[]=#type;
#define CRC_TYPE(type) crc_##type=STATIC_HASH(str_##type);

#define HANDLE_TYPE(type) case crc_##type:ret=##type; break;
*/