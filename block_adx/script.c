//copyright antoine bentue-ferrer 2016
#include <base/std_def.h>
#include <base/std_mem.h>
#include <base/mem_base.h>
#include <base/std_str.h>

#include <sha256.h>
#include <crypto.h>
#include <strs.h>
#include <tree.h>
#include <fsio.h>

#include "../zlib-1.2.8/zlib.h"

#define BLOCK_API C_EXPORT
#include "block_api.h"





#ifdef _DEBUG
	LIBEC_API int	C_API_FUNC			crypto_sign_open(const struct string *sign, const hash_t msgh, struct string *pk);
#else
	extern	crypto_sign_open_func_ptr	crypto_sign_open;
#endif

extern unsigned char	pubKeyPrefix;
extern unsigned char	privKeyPrefix;

char* base58(unsigned char *s, char *out) {
	static const char *tmpl = "123456789"
		"ABCDEFGHJKLMNPQRSTUVWXYZ"
		"abcdefghijkmnopqrstuvwxyz";
	static char buf[40];

	int c, i, n;
	if (!out) out = buf;

	n = 34;
	while (n--) {
		for (c = i = 0; i < 25; i++) {
			c = c * 256 + s[i];
			s[i] = c / 58;
			c %= 58;
		}
		out[n] = tmpl[c];
	}

	for (n = 0; out[n] == '1'; n++);
	memmove_c(out, out + n, 33 - n);

	return out;
}


int compute_script_size(mem_zone_ref_const_ptr script_node)
{
	mem_zone_ref_ptr	key = PTR_NULL;
	mem_zone_ref		my_list = { PTR_NULL };
	size_t				length;
	unsigned char		byte;

	length = 0;

	for (tree_manager_get_first_child(script_node, &my_list, &key); ((key != PTR_NULL) && (key->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &key))
	{
		unsigned char	*data;
		uint64_t		i64val;
		unsigned int	ival;

		switch (tree_mamanger_get_node_type(key))
		{
			case NODE_BITCORE_PUBKEY:
				length+=34;
			break;

			case NODE_BITCORE_SCRIPT_OPCODE:
				tree_mamanger_get_node_byte(key, 0, &byte);
				length++;

				if(byte==0x4c)
				{
					unsigned char datasz;
					tree_mamanger_get_node_byte(key, 1, &datasz);
					length += (datasz+1);
				}
				if (byte == 0x4d)
				{
					unsigned short datasz;
					tree_mamanger_get_node_word(key, 1, &datasz);
					length += (datasz + 1);
				}
					
			break;
			case NODE_BITCORE_VSTR:
				data = (unsigned char	*)tree_mamanger_get_node_data_ptr(key, 0);
				if (*data == 0x00)
					length++;
				else if (*data < 0xFD)
					length += 1 + (*data);
				else if (*data == 0xFD)
					length += 3 + (*((unsigned short *)(data + 1)));
				else if (*data == 0xFE)
					length += 5 + (*((unsigned int *)(data + 1)));
				else if (*data == 0xFF)
					length += 9 + (*((uint64_t *)(data + 1)));
			break;
			case NODE_BITCORE_VINT:
				data = (unsigned char	*)tree_mamanger_get_node_data_ptr(key, 0);
				if (*data == 0x00)
				{
					length++;
				}
				else if (*data < 0xFD)
				{
					length += 2;
				}
				else if (*data == 0xFD)
				{
					length += 3;
				}
				else if (*data == 0xFE)
				{
					length += 5;
				}
				else if (*data == 0xFF)
				{
					length += 9;
				}
				break;
			case NODE_GFX_INT:
				tree_mamanger_get_node_dword(key, 0, &ival);

				if (ival == 0)
					length++;
				else if (ival < 256)
					length+=2;
				else if (ival < ((1 << 16) - 1))
					length+=3;
				else
					length += 5;
				break;
			case NODE_GFX_BINT:
				tree_mamanger_get_node_qword(key, 0, &i64val);
				if (ival == 0)
					length++;
				else if (ival < 256)
					length += 2;
				else if (ival < ((1 << 16) - 1))
					length += 3;
				else if (ival < ((1 << 32) - 1))
					length += 5;
				else 
					length += 9;

				break;
		}
	}
	return length;
}

OS_API_C_FUNC(int) serialize_script(mem_zone_ref_const_ptr script_node, struct string *script)
{
	mem_zone_ref_ptr	key=PTR_NULL;
	mem_zone_ref		my_list = { PTR_NULL };
	size_t				length;
	unsigned char		*script_data;

	length			= compute_script_size(script_node);
	if (length == 0)return 1;
	
	script->len		= length;
	script->size	= length + 1;
	script->str		= (char	*)calloc_c(script->size, 1);

	script_data		= (unsigned char *)script->str;

	for (tree_manager_get_first_child(script_node, &my_list, &key); ((key != PTR_NULL) && (key->zone != PTR_NULL)); tree_manager_get_next_child(&my_list, &key))
	{
		unsigned char	*data;
		unsigned char	byte;
		uint64_t		i64val;
		unsigned int	ival;

		switch (tree_mamanger_get_node_type(key))
		{
		case NODE_BITCORE_SCRIPT_OPCODE:
			tree_mamanger_get_node_byte(key, 0, &byte);
			*(script_data++) = byte;

			if(byte==0x4c)
			{
				unsigned char datasz;
				mem_ptr		  data_ptr;

				tree_mamanger_get_node_byte(key, 1, &datasz);
				*(script_data++) = datasz;
				if(datasz>0)
				{
					data_ptr = tree_mamanger_get_node_data_ptr(key,2);
					memcpy_c(script_data,data_ptr,datasz);
					script_data+=datasz;
				}
			}
			if (byte == 0x4d)
			{
				unsigned short datasz;
				mem_ptr		  data_ptr;
				tree_mamanger_get_node_word(key, 1, &datasz);
				*((unsigned short*)(script_data)) = datasz;
				script_data += 2;
				if (datasz>0)
				{
					data_ptr = tree_mamanger_get_node_data_ptr(key, 3);
					memcpy_c(script_data, data_ptr, datasz);
					script_data += datasz;
				}
			}
		break;
		case NODE_BITCORE_VSTR:
			data = (unsigned char *)tree_mamanger_get_node_data_ptr(key, 0);
			if (*data < 0xFD)
			{
				*(script_data++) = *data;
				memcpy_c(script_data, &data[1], *data);
				script_data += (*data);
			}
			else if (*data == 0xFD)
			{
				*(script_data++) = 0xFD;
				(*((unsigned short *)(script_data))) = (*((unsigned short *)(data + 1)));
				script_data += 2;
				memcpy_c(script_data, &data[3], (*((unsigned short *)(data + 1))));
				script_data += (*((unsigned short *)(data + 1)));
			}
			else if (*data == 0xFE)
			{
				*(script_data++) = 0xFE;
				(*((unsigned int *)(script_data))) = (*((unsigned int *)(data + 1)));
				script_data += 4;
				memcpy_c(script_data, &data[5], (*((unsigned int *)(data + 1))));
				script_data += (*((unsigned int *)(data + 1)));
			}
			else if (*data == 0xFF)
			{
				*(script_data++) = 0xFF;
				(*((uint64_t *)(script_data))) = (*((uint64_t *)(data + 1)));
				script_data += 8;
				memcpy_c(script_data, &data[9], (*((uint64_t *)(data + 1))));
				script_data += (*((uint64_t *)(data + 1)));
			}
		break;
		case  NODE_BITCORE_PUBKEY:

			data = (unsigned char *)tree_mamanger_get_node_data_ptr(key, 0);
			*(script_data++) = 33;
			memcpy_c(script_data, data, 33);

			script_data += 34;

		break;
		case NODE_BITCORE_VINT:
			data = (unsigned char *)tree_mamanger_get_node_data_ptr(key, 0);
			if (*data == 0x00)
			{
				*(script_data++) = *data;
			}
			else if (*data < 0xFD)
			{
				*(script_data++) = 1;
				*(script_data++) = *data;
			}
			else if (*data == 0xFD)
			{
				*(script_data++) = 2;
				(*((unsigned short *)(script_data))) = (*((unsigned short *)(data + 1)));
				script_data += 2;
			}
			else if (*data == 0xFE)
			{
				*(script_data++) = 4;
				(*((unsigned int *)(script_data))) = (*((unsigned int *)(data + 1)));
				script_data += 4;
			}
			else if (*data == 0xFF)
			{
				*(script_data++) = 8;
				(*((uint64_t *)(script_data))) = (*((uint64_t *)(data + 1)));
				script_data += 8;
			}
			break;
			case NODE_GFX_INT:
				tree_mamanger_get_node_dword(key, 0, &ival);
				
				if (ival == 0)
				{
					*(script_data++) = 0;
				}
				else if (ival < 256)
				{
					*(script_data++) = 1;
					*(script_data++) = ival;
				}
				else if (ival < ((1 << 16) - 1))
				{
					*(script_data++) = 2;
					(*((unsigned short *)(script_data))) = ival;
					script_data += 2;
				}
				else 
				{
					*(script_data++) = 4;
					(*((unsigned int *)(script_data))) = ival;
					script_data += 4;
				}
			break;
			case NODE_GFX_BINT:
				tree_mamanger_get_node_qword(key, 0, &i64val);
				if (i64val == 0)
				{
					*(script_data++) = 0;
				}
				else if (i64val < 256)
				{
					*(script_data++) = 1;
					*(script_data++) = i64val;
				}
				else if (i64val < ((1 << 16) - 1))
				{
					*(script_data++) = 2;
					(*((unsigned short *)(script_data))) = i64val;
					script_data += 2;
				}
				else if (i64val < ((1 << 32) - 1))
				{
					*(script_data++) = 4;
					(*((unsigned int *)(script_data))) = i64val;
					script_data += 4;
				}
				else
				{
					*(script_data++) = 8;
					(*((uint64_t *)(script_data))) = i64val;
					script_data += 8;
				}
				break;
		}
	}

	*script_data = 0;
	return 1;
}
void keyrh_to_addr(unsigned char *pkeyh, btc_addr_t addr)
{
	hash_t			tmp_hash, fhash;
	unsigned char	hin[32];

	hin[0] = pubKeyPrefix;
	memcpy_c		(&hin[1],pkeyh, 20);
	mbedtls_sha256	(hin, 21, tmp_hash, 0);
	mbedtls_sha256	(tmp_hash, 32, fhash, 0);
	memcpy_c		(&hin[21], fhash, 4);
	base58			(hin, addr);
}
void keyh_to_addr(unsigned char *pkeyh, btc_addr_t addr)
{
	hash_t			tmp_hash, fhash;
	unsigned char	hin[32];

	hin[0] = pubKeyPrefix;
	ripemd160(pkeyh, 32, &hin[1]);

	mbedtls_sha256(hin, 21, tmp_hash, 0);
	mbedtls_sha256(tmp_hash, 32, fhash, 0);
	memcpy_c(&hin[21], fhash, 4);
	base58(hin, addr);
}




OS_API_C_FUNC(void) key_to_hash(const unsigned char *pkey, unsigned char *keyHash)
{
	hash_t			tmp_hash;
	mbedtls_sha256	(pkey, 33, tmp_hash, 0);
	ripemd160		(tmp_hash, 32, keyHash);
}

OS_API_C_FUNC(void) key_to_addr(const unsigned char *pkey,btc_addr_t addr)
{
	hash_t			tmp_hash;
	mbedtls_sha256	(pkey, 33, tmp_hash, 0);
	keyh_to_addr	(tmp_hash, addr);
}



struct string get_next_script_var(const struct string *script,size_t *offset)
{
	struct string var = { PTR_NULL };
	unsigned char *p = (unsigned char *)(&script->str[*offset]);

	if ((*offset) >= script->len)return var;

	if ((*p) < 0xFD)
	{
		var.len		= (*p);

		if (((*offset) + var.len) > script->len)return var;

		var.size	= var.len + 1;
		var.str		= malloc_c(var.size);
		memcpy_c(var.str, p + 1, var.len);
		var.str[var.len] = 0;
		(*offset) += var.len + 1;
	}
	else if ((*p) == 0xFD)
	{
		(*offset)++;
		if (((*offset) + var.len) > script->len)return var;

		var.len = *(unsigned short *)(p+1);
		var.size = var.len + 1;
		var.str = malloc_c(var.size);
		memcpy_c(var.str, p + 3, var.len);
		var.str[var.len] = 0;
		(*offset) += var.len + 3;
	}
	else if (((*offset) + 1) <= script->len)
		(*offset)++;

	return var;
}


int add_script_var(mem_zone_ref_ptr script_node, const struct string *val)
{
	mem_zone_ref new_var = { PTR_NULL };
	int ret;
	if (!tree_manager_add_child_node(script_node, "var", NODE_BITCORE_VSTR, &new_var))return 0;
	ret=tree_manager_write_node_vstr(&new_var, 0, val);
	release_zone_ref(&new_var);
	return ret;
}

int add_script_uivar(mem_zone_ref_ptr script_node,uint64_t val)
{
	mem_zone_ref new_var = { PTR_NULL };
	int ret;
	if (!tree_manager_add_child_node(script_node, "var", NODE_BITCORE_VINT, &new_var))return 0;
	ret=tree_manager_write_node_qword(&new_var, 0, val);
	release_zone_ref(&new_var);
	return ret;
}

int add_script_opcode(mem_zone_ref_ptr script_node, unsigned char opcode)
{
	mem_zone_ref new_var = { PTR_NULL };
	int ret;
	if (!tree_manager_add_child_node(script_node, "op", NODE_BITCORE_SCRIPT_OPCODE, &new_var))return 0;
	ret = tree_manager_write_node_byte(&new_var, 0, opcode);
	release_zone_ref(&new_var);
	return ret;
}

int add_script_push_data(mem_zone_ref_ptr script_node,const_mem_ptr data, size_t size)
{
	mem_zone_ref	new_var = { PTR_NULL };
	int				ret;
	if (!tree_manager_add_child_node(script_node, "op", NODE_BITCORE_SCRIPT_OPCODE, &new_var))return 0;
	
	if(size<256)
	{
		ret = tree_manager_write_node_byte(&new_var, 0, 0x4c);
		if(ret)ret = tree_manager_write_node_byte(&new_var, 1, size);
		if(ret)ret = tree_manager_write_node_data(&new_var, data,2, size);
	}
	else if (size<65536)
	{
		ret = tree_manager_write_node_byte(&new_var, 0, 0x4D);
		if (ret)ret = tree_manager_write_node_word(&new_var, 1, size);
		if (ret)ret = tree_manager_write_node_data(&new_var, data, 3, size);
	}

	release_zone_ref(&new_var);
	return ret;
}

OS_API_C_FUNC(int) make_script_file(mem_zone_ref_ptr file,struct string *pKey,struct string *sign, mem_zone_ref_ptr script)
{
	hash_t		  hash;
	struct string mime = { 0 }, fileName = { 0 }, hashStr = { 0 };
	size_t		  start,end,size;

	tree_manager_get_child_value_i32(file, NODE_HASH("start"), &start);
	tree_manager_get_child_value_i32(file, NODE_HASH("end"), &end);

	size = end - start;

	tree_manager_get_child_value_istr(file, NODE_HASH("filename"), &fileName, 0);
	tree_manager_get_child_value_istr(file, NODE_HASH("mime"), &mime, 0);
	tree_manager_get_child_value_hash(file, NODE_HASH("dataHash"), hash);
	
	hashStr.str = hash;
	hashStr.len = 32;

	add_script_var	(script, &hashStr);
	add_script_var	(script, pKey);
	add_script_var	(script, sign);
	add_script_var	(script, &fileName);
	add_script_var	(script, &mime);
	add_script_uivar(script, size);

	free_string		(&fileName);
	free_string		(&mime);
	return 1;
}
int get_script_file(struct string *script,mem_zone_ref_ptr file)
{
	hash_t		  hash;
	struct string pKey = { 0 }, sizeStr = { 0 }, sign = { 0 }, mime = { 0 }, fileName = { 0 }, hashStr = { 0 };
	size_t		  offset,size;
	int			  ret;
	
	offset  = 0;

	hashStr = get_next_script_var(script, &offset);
	ret = (hashStr.len == 32) ? 1 : 0;
	if (ret)
	{
		int n = 32;
		while (n--)
		{
			hash[n] = hashStr.str[31 - n];
		}
	}

	if(ret)pKey = get_next_script_var(script, &offset);
	if(ret)ret  = (pKey.len == 33) ? 1 : 0;

	if(ret)sign	= get_next_script_var(script, &offset);
	if (ret)ret = blk_check_sign(&sign, &pKey, hash);
	if(ret)fileName = get_next_script_var(script, &offset);
	if(ret)mime = get_next_script_var(script, &offset);
	if(ret)ret = (mime.len > 0) ? 1 : 0;
	if(ret)sizeStr = get_next_script_var(script, &offset);
	if(ret)ret = (sizeStr.len > 0) ? 1 : 0;

	if (ret)
	{
		if (sizeStr.len == 0)
			ret = 0;
		else if (sizeStr.len == 1)
			size = *((unsigned char *)(sizeStr.str));
		else if (sizeStr.len == 2)
			size = *((unsigned short *)(sizeStr.str));
		else if (sizeStr.len == 4)
			size = *((unsigned int *)(sizeStr.str));
		else
			size = *((uint64_t *)(sizeStr.str));
	}


	if (ret)
	{
		mem_zone_ref sig = { PTR_NULL };

		tree_manager_set_child_value_vstr(file, "filename", &fileName);
		tree_manager_set_child_value_vstr(file, "mime", &mime);
		tree_manager_set_child_value_i32(file, "size", size);
		tree_manager_set_child_value_hash(file, "dataHash", (unsigned char *)hashStr.str);
	}
	
	free_string(&hashStr);
	free_string(&pKey);
	free_string(&sign);
	free_string(&fileName);
	free_string(&mime);
	free_string(&sizeStr);

	return ret;
}




OS_API_C_FUNC(int) make_script_layout(mem_zone_ref_ptr file, mem_zone_ref_ptr script)
{
	char			chash[65];
	struct string	mime = { 0 }, fileName = { 0 }, fileData = { 0 }, tmp_file = { 0 };
	int				ret = 0;

	tree_manager_get_child_value_istr(file, NODE_HASH("filename"), &fileName, 0);
	tree_manager_get_child_value_str(file, NODE_HASH("dataHash"), chash, 65, 0);


	make_string		(&tmp_file, "tmp");
	cat_cstring_p	(&tmp_file, chash);
	if (get_file(tmp_file.str, &fileData.str, &fileData.len)>0)
	{
		size_t			ll;
		struct string	cbuff;

		ll			= fileData.len;
		cbuff.size  = ll;
		cbuff.str	= malloc_c(cbuff.size);

		compress2		(cbuff.str, &ll, fileData.str, fileData.len,9);

		cbuff.len = ll;

		add_script_var	(script, &fileName);
		add_script_uivar(script, fileData.len);
		add_script_var	(script, &cbuff);
		
		free_string		(&fileData);
		free_string		(&cbuff);
		ret = 1;
	}

	free_string(&tmp_file);
	free_string(&fileName);
	free_string(&mime);
	return ret;
}

OS_API_C_FUNC(int) make_script_module(mem_zone_ref_ptr file, mem_zone_ref_ptr script)
{
	char			chash[65];
	struct string	mime = { 0 }, fileName = { 0 }, fileData = { 0 }, tmp_file = { 0 };
	int				ret = 0;

	tree_manager_get_child_value_istr(file, NODE_HASH("filename"), &fileName, 0);
	tree_manager_get_child_value_istr(file, NODE_HASH("mime"), &mime, 0);
	tree_manager_get_child_value_str(file, NODE_HASH("dataHash"), chash, 65, 0);


	make_string(&tmp_file, "tmp");
	cat_cstring_p(&tmp_file, chash);
	if (get_file(tmp_file.str, &fileData.str, &fileData.len)>0)
	{
		size_t			ll;
		struct string	cbuff;

		ll = fileData.len;
		cbuff.size = ll;
		cbuff.str = malloc_c(cbuff.size);

		compress2(cbuff.str, &ll, fileData.str, fileData.len, 9);

		cbuff.len = ll;

		add_script_var(script, &fileName);
		add_script_var(script, &mime);
		
		add_script_uivar(script, fileData.len);
		add_script_var(script, &cbuff);

		free_string(&fileData);
		free_string(&cbuff);
		ret = 1;
	}

	free_string(&tmp_file);
	free_string(&mime);
	free_string(&fileName);
	free_string(&mime);
	return ret;
}

int get_script_layout(struct string *script, mem_zone_ref_ptr file)
{
	struct string sizeStr = { 0 }, fileName = { 0 }, cbuff = { 0 }, fileData = { 0 };
	size_t		  offset,size;
	int			  ret;

	offset = 0;

	fileName		= get_next_script_var(script, &offset);
	ret				= (fileName.len > 0) ? 1 : 0;
	if (ret)sizeStr	= get_next_script_var(script, &offset);
	if (ret)ret		= (sizeStr.len > 0) ? 1 : 0;

	if (sizeStr.len == 1)
		size = *((unsigned char *)(sizeStr.str));
	else if (sizeStr.len == 2)
		size = *((unsigned short *)(sizeStr.str));
	else if (sizeStr.len == 4)
		size = *((unsigned int *)(sizeStr.str));
	else
		size = *((uint64_t *)(sizeStr.str));

	if (ret)cbuff = get_next_script_var(script, &offset);

	if (ret)
	{
		hash_t		 hash;

		fileData.size = size;
		fileData.str  = malloc_c(fileData.size);

		ret = (uncompress		(fileData.str, &size, cbuff.str, cbuff.len) == Z_OK)?1:0;

		if (ret)
		{
			fileData.len = size;

			mbedtls_sha256					 (fileData.str, fileData.len, hash, 0);
			tree_manager_set_child_value_vstr(file, "filename", &fileName);
			tree_manager_set_child_value_vstr(file, "filedata", &fileData);
			tree_manager_set_child_value_hash(file, "dataHash", hash);
		}

		free_string(&fileData);
	}

	free_string(&fileName);
	free_string(&cbuff);
	free_string(&sizeStr);

	return ret;
}

int get_script_module(struct string *script, mem_zone_ref_ptr file)
{
	struct string sizeStr = { 0 }, fileName = { 0 }, mime = { 0 }, cbuff = { 0 }, fileData = { 0 };
	size_t		  offset, size;
	int			  ret;

	offset = 0;

	fileName = get_next_script_var(script, &offset);
	ret = (fileName.len > 0) ? 1 : 0;
	if(ret)mime = get_next_script_var(script, &offset);
	if(ret)ret  = ((!strcmp_c(mime.str, "application/module")) || (!strcmp_c(mime.str, "application/script"))) ? 1 : 0;
	if (ret)sizeStr = get_next_script_var(script, &offset);
	if (ret)ret = (sizeStr.len > 0) ? 1 : 0;

	if (sizeStr.len == 1)
		size = *((unsigned char *)(sizeStr.str));
	else if (sizeStr.len == 2)
		size = *((unsigned short *)(sizeStr.str));
	else if (sizeStr.len == 4)
		size = *((unsigned int *)(sizeStr.str));
	else
		size = *((uint64_t *)(sizeStr.str));

	if (ret)cbuff = get_next_script_var(script, &offset);

	if (ret)
	{
		hash_t		 hash;

		fileData.size = size;
		fileData.str  = malloc_c(fileData.size);

		ret = (uncompress(fileData.str, &size, cbuff.str, cbuff.len) == Z_OK) ? 1 : 0;

		if (ret)
		{
			fileData.len = size;

			mbedtls_sha256(fileData.str, fileData.len, hash, 0);
			tree_manager_set_child_value_vstr(file, "filename", &fileName);
			tree_manager_set_child_value_vstr(file, "mime", &mime);
			tree_manager_set_child_value_vstr(file, "filedata", &fileData);
			tree_manager_set_child_value_hash(file, "dataHash", hash);
		}

		free_string(&fileData);
	}

	free_string(&fileName);
	free_string(&mime);
	free_string(&cbuff);
	free_string(&sizeStr);

	return ret;
}


OS_API_C_FUNC(int) parse_sig_seq(const struct string *sign_seq, struct string *sign, unsigned char *hashtype, int rev)
{
	unsigned char 	seq_len;
	size_t			slen, rlen;
	if (sign_seq->len < 69)return 0;

	if ((sign_seq->str[0] == 0x30) && (sign_seq->str[2] == 0x02))
	{
		unsigned char *s, *r,*sig;
		size_t last_r, last_s;
		unsigned int n;
		seq_len  = sign_seq->str[1];
		rlen	 = sign_seq->str[3];

		if ((4 + rlen+2)>= sign_seq->len)return 0;

		if (sign_seq->str[4 + rlen] == 2)
			slen = sign_seq->str[4 + rlen + 1];

		if (seq_len != (slen + rlen + 4))return 0;

		*hashtype = sign_seq->str[sign_seq->len - 1];
		
		if (rlen == 33)
		{
			last_r = 31;
			r = sign_seq->str + 4 + 1;
		}
		else
		{
			last_r = rlen-1;
			r		= sign_seq->str + 4;
		}
			

		if (slen == 33)
		{
			last_s	= 31;
			s		= sign_seq->str + 4 + rlen + 2 + 1;
		}
		else
		{
			last_s	= slen-1;
			s		= sign_seq->str + 4 + rlen + 2;
		}

		sign->len = 64;
		sign->size = sign->len + 1;
		sign->str = malloc_c(sign->size);
		sig = sign->str;

		if (rev == 1)
		{
			n = 0;
			while (n <= last_r)
			{
				sig[(last_r - n)] = r[n];
				n++;
			}
		}
		else
		{
			memcpy_c(sig, r, rlen);
		}

		if (rlen < 32)
			memset_c(&sig[rlen], 0, 32 - rlen);

		if (rev == 1)
		{
			n = 0;
			while (n <= last_s)
			{
				sig[(last_s - n) + 32] = s[n];
				n++;
			}
		}
		else
		{
			memcpy_c(sig+32, s, slen);
		}

		if (slen < 32)
			memset_c(&sig[slen + 32], 0, 32 - slen);

		
		return 1;
	}
	return 0;

}

OS_API_C_FUNC(int) get_insig_info(const struct string *script, struct string *sign, struct string *pubk, unsigned char *hash_type)
{
	struct string	sigseq = { PTR_NULL };
	size_t			offset = 0;
	int				ret = 0;

	if (script->len == 0)return 0;
	if (script->str == PTR_NULL)return 0;

	sigseq = get_next_script_var(script, &offset);
	if (sigseq.str == PTR_NULL)return 0;
	if (sigseq.len < 69)
	{
		free_string(&sigseq);
		return 0;
	}
	ret		= parse_sig_seq			(&sigseq, sign, hash_type,1);
	(*pubk) = get_next_script_var	(script, &offset);
	
	free_string(&sigseq);
	return ret;
}

int get_insig(const struct string *script, struct string *sign_seq, struct string *pubk)
{
	size_t offset = 0;
	(*sign_seq) = get_next_script_var(script, &offset);
	if (sign_seq->str == PTR_NULL)return 0;
	if (sign_seq->len < 69)
	{
		free_string(sign_seq);
		return 0;
	}
	(*pubk) = get_next_script_var(script, &offset);
	return 1;
}


int check_sign(const struct string *sign, const struct string *pubK, const hash_t txh)
{
	int ret=0;

	if (sign->str == PTR_NULL)return 0;

	if (pubK->len == 33)
	{
		hash_t h;
		struct string ppk = { PTR_NULL };
		unsigned char *p,*dp;
		int n = 32;
		
		ppk.str = malloc_c(33);
		
		ppk.len = 33;
		p		= pubK->str;
		dp      = ppk.str;
		
		dp[0] = p[0];
		while (n--)
		{
			h[n]		= txh[31-n];
			dp[n + 1]	= p[(31 - n) + 1];
		}
		ret = crypto_sign_open(sign, h, &ppk);
		free_string(&ppk);
	}
	return ret;
}

int get_script_data(const struct string *script, size_t *offset,struct string *data)
{
	unsigned char opcode;
	if ((*offset) >= script->len) return 0;
	opcode = script->str[(*offset)];
	switch (opcode)
	{
		case 0x4c:

			if (((*offset)+1) >= script->len) return 0;

			data->len = *((unsigned char *)(&script->str[(*offset) + 1]));

			if (((*offset) + 2 + data->len) > script->len)return 0;

			data->size = data->len + 1;
			data->str = malloc_c(data->size);
			memcpy_c(data->str, &script->str[(*offset) + 2], data->len);
			data->str[data->len] = 0;
			(*offset) += 2 + data->len;
		return 1;
		break;
		case 0x4D:

			if (((*offset) + 2) >= script->len) return 0;

			data->len = *((unsigned short *)(&script->str[(*offset) + 1]));
			
			if (((*offset) + 3 + data->len) > script->len)return 0;

			data->size = data->len + 1;
			data->str = malloc_c(data->size);
			memcpy_c(data->str, &script->str[(*offset) + 3], data->len);
			data->str[data->len] = 0;
			(*offset) += 3 + data->len;
		return 1;
		break;
	}

	return 0;
}

OS_API_C_FUNC(int) get_out_script_return_val(const struct string *script, struct string *data)
{
	if (script->str == PTR_NULL)return 0;
	if (script->len <26)return 0;

	if (script->str[25] == 0x6a)
	{
		size_t offset = 26;
		return get_script_data(script, &offset, data);
	}
	if (script->len <36)return 0;
	if (script->str[35] == 0x6a)
	{
		size_t offset = 36;
		return get_script_data(script, &offset, data);
	}
	return 0;
}



OS_API_C_FUNC(int) get_out_script_address(const struct string *script, struct string *pubk, btc_addr_t addr)
{
	unsigned char  *p;

	if (script == PTR_NULL)return 0;
	if (script->len == 0)return 0;
	if (script->str == PTR_NULL)return 0;

	p = (unsigned char  *)script->str;

	
	if ((p[0] == 33) && (p[34] == 0xAC))
	{
		if (pubk != PTR_NULL)
		{
			pubk->len = 33;
			pubk->size = pubk->len + 1;
			pubk->str = malloc_c(pubk->size);
			memcpy_c(pubk->str, script->str+1, 33);
		}
		key_to_addr(script->str + 1, addr);
		return 1;
	}
	else if ((p[0] == 0x76) && (p[1] == 0xA9) && (p[24] == 0xAC))
	{
		keyrh_to_addr(script->str + 3, addr);
		return 2;
	}
	return 0;
}

OS_API_C_FUNC(int) create_p2sh_script_byte(btc_addr_t addr,mem_zone_ref_ptr script_node,unsigned char val)
{
	unsigned char	addrBin[26];
	struct string  strKey = { PTR_NULL };
	size_t sz;

	sz = 25;
	b58tobin			(addrBin, &sz, addr, sizeof(btc_addr_t));
	make_string_l		(&strKey, addrBin + 1, 20);


	add_script_opcode	(script_node, 0x76);
	add_script_opcode	(script_node, 0xA9);
	add_script_var		(script_node, &strKey);
	add_script_opcode	(script_node, 0x88);
	add_script_opcode	(script_node, 0xAC);

	add_script_opcode	(script_node, 0x6a);
	add_script_push_data(script_node, &val,1);

	

	return 0;
}

OS_API_C_FUNC(int) create_p2sh_script_data(btc_addr_t addr, mem_zone_ref_ptr script_node, const unsigned char *data,size_t len)
{
	unsigned char	addrBin[26];
	struct string  strKey = { PTR_NULL };
	size_t sz;

	sz = 25;
	b58tobin(addrBin, &sz, addr, sizeof(btc_addr_t));
	make_string_l(&strKey, addrBin + 1, 20);


	add_script_opcode(script_node, 0x76);
	add_script_opcode(script_node, 0xA9);
	add_script_var(script_node, &strKey);
	add_script_opcode(script_node, 0x88);
	add_script_opcode(script_node, 0xAC);

	add_script_opcode(script_node, 0x6a);
	add_script_push_data(script_node, data, len);



	return 0;
}


OS_API_C_FUNC(int) create_p2sh_script(btc_addr_t addr,mem_zone_ref_ptr script_node)
{
	unsigned char	addrBin[26];
	struct string  strKey = { PTR_NULL };
	size_t sz;

	sz = 25;
	b58tobin			(addrBin, &sz, addr, sizeof(btc_addr_t));
	make_string_l		(&strKey, addrBin + 1, 20);


	add_script_opcode	(script_node, 0x76);
	add_script_opcode	(script_node, 0xA9);
	add_script_var		(script_node, &strKey);
	add_script_opcode	(script_node, 0x88);
	add_script_opcode	(script_node, 0xAC);

	return 0;
}

OS_API_C_FUNC(int) create_payment_script(const struct string *pubk, unsigned int type, mem_zone_ref_ptr script_node)
{
	if (type == 1)
	{
		char			keyHash[20];
		struct string  strKey;

		key_to_hash		 (pubk->str, keyHash);

		strKey.str  = keyHash;
		strKey.len  = 20;
		strKey.size = 20;

		add_script_opcode(script_node, 0x76);
		add_script_opcode(script_node, 0xA9);
		add_script_var	 (script_node, &strKey);
		add_script_opcode(script_node, 0x88);
		add_script_opcode(script_node, 0xAC);
	}
	else
	{
		add_script_var	 (script_node, pubk);
		add_script_opcode(script_node, 0xAC);
	}

	
	return 0;
}

OS_API_C_FUNC(int) create_payment_script_data(const struct string *pubk, unsigned int type, mem_zone_ref_ptr script_node, const  unsigned char *data, size_t len)
{
	if (type == 1)
	{
		char			keyHash[20];
		struct string  strKey;

		key_to_hash(pubk->str, keyHash);

		strKey.str = keyHash;
		strKey.len = 20;
		strKey.size = 20;

		add_script_opcode(script_node, 0x76);
		add_script_opcode(script_node, 0xA9);
		add_script_var(script_node, &strKey);
		add_script_opcode(script_node, 0x88);
		add_script_opcode(script_node, 0xAC);
	}
	else
	{
		add_script_var(script_node, pubk);
		add_script_opcode(script_node, 0xAC);
	}
	add_script_opcode	(script_node, 0x6a);
	add_script_push_data(script_node, data, len);

	return 0;
}


int check_txout_key(mem_zone_ref_ptr output, unsigned char *pkey, btc_addr_t inaddr)
{
	struct string oscript = { PTR_NULL };
	int ret=0;


	if (tree_manager_get_child_value_istr(output, NODE_HASH("script"), &oscript, 0))
	{
		btc_addr_t outaddr;
		if (pkey[0] != 0)
		{ 
			int n = 32;
			

			key_to_addr				(pkey, inaddr);
			get_out_script_address	(&oscript, PTR_NULL, outaddr);
			ret = (memcmp_c(outaddr, inaddr, sizeof(btc_addr_t)) == 0) ? 1 : 0;
		}
		free_string(&oscript);
		
	}
	return ret;
}