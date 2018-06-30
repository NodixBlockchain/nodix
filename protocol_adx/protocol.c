//copyright antoine bentue-ferrer 2016
#include <base/std_def.h>
#include <base/std_mem.h>
#include <base/mem_base.h>
#include <base/std_str.h>

#include <sha256.h>
#include <fsio.h>
#include <strs.h>
#include <tree.h>

C_IMPORT ctime_t C_API_FUNC  get_time_c();

const unsigned int	GETDATA_TX		= 1;
const unsigned int	GETDATA_BLOCK	= 2;
const unsigned int	GETDATA_FILE	= 0x10;
unsigned int		magic			= 0xCDCDCDCD;

hash_t				null_hash	= { 0x66 };
char				def_str[33] = { 0xFF };
unsigned char		def_vint[5]	= { 0xEF };
unsigned char		null_vint	= 0xAB;
unsigned int		ping_nonce	= 1;

unsigned int NODE_HASH_txsout = 0xCDCDCDCD, NODE_HASH_script = 0xCDCDCDCD, NODE_HASH_txsin = 0xCDCDCDCD, NODE_HASH_version = 0xCDCDCDCD, NODE_HASH_prev = 0xCDCDCDCD, NODE_HASH_merkle_root = 0xCDCDCDCD, NODE_HASH_time = 0xCDCDCDCD, NODE_HASH_bits, NODE_HASH_nonce = 0xCDCDCDCD, NODE_HASH_services = 0xCDCDCDCD, NODE_HASH_addr = 0xCDCDCDCD, NODE_HASH_port = 0xCDCDCDCD, NODE_HASH_p2p_addr = 0xCDCDCDCD, NODE_HASH_locktime = 0xCDCDCDCD, NODE_HASH_tx_hash = 0xCDCDCDCD, NODE_HASH_value = 0xCDCDCDCD, NODE_HASH_sequence = 0xCDCDCDCD, NODE_HASH_idx = 0xCDCDCDCD, NODE_HASH_size = 0xCDCDCDCD, NODE_HASH_cmd = 0xCDCDCDCD, NODE_HASH_payload = 0xCDCDCDCD;

OS_API_C_FUNC(int) init_protocol(mem_zone_ref_ptr params)
{
	mem_zone_ref log = { PTR_NULL };

	if (!tree_manager_get_child_value_i32(params, NODE_HASH("magic"), &magic))return 0;

	memset_c	(null_hash, 0, sizeof(hash_t));
	strcpy_cs	(def_str, sizeof(def_str), "empty");

	null_vint	= 0;

	def_vint[0] = 0xFE;
	def_vint[1] = 0;
	def_vint[2] = 0;
	def_vint[3] = 0;
	def_vint[4] = 0;

	NODE_HASH_script = NODE_HASH("script");
	NODE_HASH_txsin = NODE_HASH("txsin");
	NODE_HASH_txsout = NODE_HASH("txsout");
	NODE_HASH_version = NODE_HASH("version");
	NODE_HASH_prev = NODE_HASH("prev");
	NODE_HASH_merkle_root = NODE_HASH("merkle_root");
	NODE_HASH_time = NODE_HASH("time");
	NODE_HASH_bits = NODE_HASH("bits");
	NODE_HASH_nonce = NODE_HASH("nonce");
	NODE_HASH_services = NODE_HASH("services");
	NODE_HASH_addr = NODE_HASH("addr");
	NODE_HASH_port = NODE_HASH("port");
	NODE_HASH_p2p_addr = NODE_HASH("p2p_addr");
	NODE_HASH_locktime = NODE_HASH("locktime");
	NODE_HASH_tx_hash = NODE_HASH("txid");
	NODE_HASH_value = NODE_HASH("value");
	NODE_HASH_sequence = NODE_HASH("sequence");
	NODE_HASH_idx = NODE_HASH("idx");
	NODE_HASH_size = NODE_HASH("size");
	NODE_HASH_cmd = NODE_HASH("cmd");
	NODE_HASH_payload = NODE_HASH("payload");

	tree_manager_create_node			("log", NODE_LOG_PARAMS, &log);
	tree_manager_set_child_value_i32	(&log, "magic", magic);
	log_message							("p2p protocol ok magic : %magic% ", &log);
	release_zone_ref					(&log);

	return 1;
}


OS_API_C_FUNC(void) get_magic(unsigned int *inmagic)
{
	*inmagic = magic;
}


OS_API_C_FUNC(int) add_bitcore_addr(mem_zone_ref_ptr node, ipv4_t ip, unsigned short port,unsigned int services)
{
	mem_zone_ref	addr_node = { PTR_NULL };
	
	if (!tree_manager_find_child_node(node, NODE_HASH_p2p_addr, NODE_BITCORE_ADDR, &addr_node))
		tree_manager_add_child_node(node, "p2p_addr", NODE_BITCORE_ADDR, &addr_node);

	tree_manager_set_child_value_i64	(&addr_node, "services", services);
	tree_manager_set_child_value_ipv4	(&addr_node, "addr", ip);
	tree_manager_set_child_value_i16	(&addr_node, "port", port);
	release_zone_ref					(&addr_node);

	return 1;
}



OS_API_C_FUNC(size_t) init_node(mem_zone_ref_ptr key)
{
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref_ptr	sub = PTR_NULL;
	ipv4_t				ip = { 0xFF };
	mem_zone_ref		vin = { PTR_NULL },	txin_list = { PTR_NULL };
	unsigned short		port = 0xFFFF;
 	struct string 		def_vstr;
	
	def_vstr.str  = def_str;
	def_vstr.len  = 32;
    def_vstr.size = 33;

	switch (tree_mamanger_get_node_type(key))
	{
		case NODE_GFX_INT:
			tree_manager_write_node_dword(key, 0, 0);
		break;
		case NODE_GFX_BINT:
			tree_manager_write_node_qword(key, 0, 0);
		break;
		case NODE_BITCORE_BLOCK_HASH:
		case NODE_FILE_HASH:
		case NODE_BITCORE_TX_HASH:
		case NODE_BITCORE_HASH:
		case NODE_BIN_DATA:
			tree_manager_write_node_hash(key, 0, null_hash);
		break;
		case NODE_BITCORE_VSTR:
			tree_manager_write_node_vstr(key, 0, &def_vstr);
		break;
		case NODE_BITCORE_VINT:
			tree_manager_write_node_vint(key, 0, &def_vint);
		break;
		case NODE_BITCORE_BLK_HDR:
			tree_manager_set_child_value_i32(key, "version", 0);
			tree_manager_set_child_value_hash(key, "prev", null_hash);
			tree_manager_set_child_value_hash(key, "merkle_root", null_hash);
			tree_manager_set_child_value_i32(key, "time", 0);
			tree_manager_set_child_value_i32(key, "bits", 0);
			tree_manager_set_child_value_i32(key, "nonce", 0);
			//tree_manager_set_child_value_vint(key, "ntx", &def_vint);
		break;
		case NODE_BITCORE_PUBKEY:
		{
			unsigned char pk[33] = { 0 };
			tree_manager_write_node_data(key, pk, 0,33);
		}
		break;
		case NODE_BITCORE_ECDSA_SIG:
			tree_manager_write_node_byte(key, 0, 70);
			tree_manager_write_node_byte(key, 1, 0x30);
			tree_manager_write_node_byte(key, 2, 68);
			tree_manager_write_node_byte(key, 3, 0x02);
			tree_manager_write_node_byte(key, 4, 32);
			tree_manager_write_node_data(key, null_hash, 5, 32);
			tree_manager_write_node_byte(key, 37, 0x02);
			tree_manager_write_node_byte(key, 38, 32);
			tree_manager_write_node_data(key, null_hash, 39, 32);
		break;
		case NODE_BITCORE_TXIN:
			tree_manager_set_child_value_hash(key, "txid", null_hash);
			tree_manager_set_child_value_i32(key, "idx", 0xFFFFFFFF);
			tree_manager_set_child_value_vstr(key, "script", &def_vstr);
		break;
		case NODE_BITCORE_TXOUT:
			tree_manager_set_child_value_i64(key, "value", 0);
			tree_manager_set_child_value_vstr(key, "script", &def_vstr);
			break;
		case NODE_BITCORE_TX:
			tree_manager_set_child_value_i32(key, "version", 0);
			tree_manager_set_child_value_i32(key, "time", 0);
		
			if (!tree_manager_find_child_node(key, NODE_HASH_txsin, NODE_BITCORE_VINLIST, &txin_list))
				tree_manager_add_child_node(key, "txsin", NODE_BITCORE_VINLIST, &txin_list);
		
			tree_manager_add_child_node	(&txin_list, "txin", NODE_BITCORE_TXIN, &vin);
			init_node					(&vin);
			release_zone_ref			(&vin);
			release_zone_ref			(&txin_list);

			if (!tree_manager_find_child_node(key, NODE_HASH_txsout, NODE_BITCORE_VOUTLIST, &txin_list))
				tree_manager_add_child_node(key, "txsout", NODE_BITCORE_VOUTLIST, &txin_list);

			tree_manager_add_child_node (&txin_list, "txout", NODE_BITCORE_TXOUT, &vin);
			init_node					(&vin);
			release_zone_ref			(&vin);
			release_zone_ref			(&txin_list);


			tree_manager_set_child_value_i32(key, "locktime", 0);

		break;
		case NODE_BITCORE_TX_LIST:
			for (tree_manager_get_first_child(key, &my_list, &sub); ((sub != NULL) && (sub->zone != NULL)); tree_manager_get_next_child(&my_list, &sub))
			{
				init_node(sub);
			}
		break;
		case NODE_BITCORE_ADDR:
			tree_manager_set_child_value_i64(key, "services", 0);
			tree_manager_set_child_value_ipv4(key, "addr", ip);
			tree_manager_set_child_value_i16(key, "port", port);
		break;
		case NODE_BITCORE_ADDRT:
			tree_manager_set_child_value_i32(key, "time", 0);
			tree_manager_set_child_value_i64(key, "services", 0);
			tree_manager_set_child_value_ipv4(key, "addr", ip);
			tree_manager_set_child_value_i16(key, "port", port);
		break;
	}
	return 1;
}

OS_API_C_FUNC(size_t)	get_node_size(mem_zone_ref_ptr key)
{
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref_ptr	sub = PTR_NULL;
	mem_zone_ref		txin_list = { PTR_NULL };
	size_t				szData, nc;
	unsigned int		type;
	unsigned char		*data;

	type = tree_mamanger_get_node_type(key);
	if ((type >> 24) == 0x1E)
	{
		szData = sizeof(hash_t);
		return szData;
	}

	szData = 0;
	switch (tree_mamanger_get_node_type(key))
	{
	case NODE_GFX_INT:
		szData += 4;
		break;
	case NODE_GFX_BINT:
		szData += 8;
		break;
	case NODE_RT_VEC3:
		szData += sizeof(float) * 3;
	break;
	case NODE_GFX_4UC:
		szData += sizeof(vec_4uc_t);
	break;
	case NODE_BIN_DATA:
		szData += sizeof(hash_t);
	break;
	case NODE_GFX_FLOAT:
		szData += sizeof(float);
	break;
	case NODE_GFX_DOUBLE:
		szData += sizeof(double);
	break;
	case NODE_BITCORE_PUBKEY:
		szData += 33;
	break;
	case NODE_BITCORE_BLOCK_HASH:
	case NODE_FILE_HASH :
	case NODE_BITCORE_TX_HASH:
	case NODE_BITCORE_HASH:
		szData += 32;
	break;
	case NODE_BITCORE_BLK_HDR_LIST:
		nc = tree_manager_get_node_num_children(key);

		if (nc < 0xFD)
			szData += 1;
		else if (nc < 0xFFFF)
			szData += 3;
		else
			szData += 5;

		for (tree_manager_get_first_child(key, &my_list, &sub); ((sub != NULL) && (sub->zone != NULL)); tree_manager_get_next_child(&my_list, &sub))
		{
			szData += get_node_size(sub);

			data = tree_manager_get_child_data(sub, NODE_HASH("ntx"), 0);
			if (data != PTR_NULL)
			{
				if ((*((unsigned char *)(data + 80))) < 0xFD)
					szData++;
				else
					szData += 3;
			}
			else
				szData++;

		}
	break;
	case NODE_BITCORE_HASH_LIST:
		nc = tree_manager_get_node_num_children(key);
		if (nc < 0xFD)
			szData += 1;
		else if (nc < 0xFFFF)
			szData += 3;
		else
			szData += 5;

		szData += nc * 36;
	break;
	case NODE_BITCORE_ECDSA_SIG:
	{
		unsigned char sig_check,sig_len;
		tree_mamanger_get_node_byte (key, 1 , &sig_check);
		if (sig_check == 0x30)
		{
			tree_mamanger_get_node_byte(key, 0, &sig_len);
			szData += sig_len;
		}
	}
	break;
	case NODE_BITCORE_LOCATOR:

		nc = tree_manager_get_node_num_children(key);
		if (nc < 0xFD)
			szData += 1;
		else if (nc < 0xFFFF)
			szData += 3;
		else
			szData += 5;

		szData += nc * 32;
		break;
	case NODE_BITCORE_VSTR:
		data = tree_mamanger_get_node_data_ptr(key, 0);
		if (*data < 0xFD)
			szData += 1 + *data;
		else if ((*data) == 0xFD)
			szData += 3 + (*((unsigned short *)(data + 1)));
		else if ((*data) == 0xFE)
			szData += 5 + (*((unsigned int *)(data + 1)));
		else if ((*data) == 0xFF)
			szData += 9 + (*((uint64_t *)(data + 1)));
	break;
	case NODE_BITCORE_VINT:
		data = tree_mamanger_get_node_data_ptr(key, 0);
		if (*data < 0xFD)
			szData += 1;
		else if ((*data) == 0xFD)
			szData += 3;
		else if ((*data) == 0xFE)
			szData += 5;
		else if ((*data) == 0xFF)
			szData += 9;
		break;
	case NODE_BITCORE_BLK_HDR:
		szData += 80; 		


	break;
	case NODE_BITCORE_ADDR:
		szData += 26;
		break;
	case NODE_BITCORE_ADDRT:
		szData += 30;
	break;
	case NODE_BITCORE_ADDR_LIST:

		nc = tree_manager_get_node_num_children(key);
		if (nc < 0xFD)
			szData += 1;
		else if (nc < 0xFFFF)
			szData += 3;
		else
			szData += 5;

		for (tree_manager_get_first_child(key, &my_list, &sub); ((sub != NULL) && (sub->zone != NULL)); tree_manager_get_next_child(&my_list, &sub))
		{
			szData += get_node_size(sub);
		}
	break;
	case NODE_BITCORE_TXIN:
		szData += 32;
		szData += 4;
		data = tree_manager_get_child_data(key, NODE_HASH_script, 0);

		if (data != PTR_NULL)
		{
			if (*data < 0xFD)
				szData += 1 + *data;
			else if ((*data) == 0xFD)
				szData += 3 + (*((unsigned short *)(data + 1)));
			else if ((*data) == 0xFE)
				szData += 5 + (*((unsigned int *)(data + 1)));
			else if ((*data) == 0xFF)
				szData += 9 + (*((uint64_t *)(data + 1)));
		}
		szData += 4;
		break;
	case NODE_BITCORE_TXOUT:
		szData += 8;
		data = tree_manager_get_child_data(key, NODE_HASH_script, 0);
		if (data==PTR_NULL)
			szData += 0;
		else if (*data < 0xFD)
			szData += 1 + *data;
		else if ((*data) == 0xFD)
			szData += 3 + (*((unsigned short *)(data + 1)));
		else if ((*data) == 0xFE)
			szData += 5 + (*((unsigned int *)(data + 1)));
		else if ((*data) == 0xFF)
			szData += 9 + (*((uint64_t *)(data + 1)));
		break;
	case NODE_BITCORE_VINLIST:
		nc = tree_manager_get_node_num_children(key);
		if (nc < 0xFD)
			szData += 1;
		else if (nc < 0xFFFF)
			szData += 3;
		else
			szData += 5;
		for (tree_manager_get_first_child(key, &my_list, &sub); ((sub != NULL) && (sub->zone != NULL)); tree_manager_get_next_child(&my_list, &sub))
		{
			szData += get_node_size(sub);
		}
		break;
	case NODE_BITCORE_VOUTLIST:
		nc = tree_manager_get_node_num_children(key);
		if (nc < 0xFD)
			szData += 1;
		else if (nc < 0xFFFF)
			szData += 3;
		else
			szData += 5;
		for (tree_manager_get_first_child(key, &my_list, &sub); ((sub != NULL) && (sub->zone != NULL)); tree_manager_get_next_child(&my_list, &sub))
		{
			szData += get_node_size(sub);
		}
		break;
	case NODE_BITCORE_TX:
		szData += 4;//version
		szData += 4;//time

		tree_manager_find_child_node(key, NODE_HASH_txsin, NODE_BITCORE_VINLIST, &txin_list);
		szData += get_node_size(&txin_list);
		release_zone_ref(&txin_list);

		tree_manager_find_child_node(key, NODE_HASH_txsout, NODE_BITCORE_VOUTLIST, &txin_list);
		szData += get_node_size(&txin_list);
		release_zone_ref(&txin_list);

		szData += 4;//lock time

	break;

	case NODE_BITCORE_TX_LIST:
		nc = tree_manager_get_node_num_children(key);
		if (nc < 0xFD)
			szData += 1;
		else if (nc < 0xFFFF)
			szData += 3;
		else
			szData += 5;
		for (tree_manager_get_first_child(key, &my_list, &sub); ((sub != NULL) && (sub->zone != NULL)); tree_manager_get_next_child(&my_list, &sub))
		{
			szData += get_node_size(sub);
		}
		
		break;
	}

	return szData;
}

OS_API_C_FUNC(size_t) compute_payload_size(mem_zone_ref_ptr payload_node)
{
	size_t				szData;
	mem_zone_ref_ptr	key = PTR_NULL;
	mem_zone_ref		my_list = { PTR_NULL };
	szData = 0;

	for (tree_manager_get_first_child(payload_node, &my_list, &key); ((key != NULL) && (key->zone != NULL)); tree_manager_get_next_child(&my_list, &key))
	{
		szData += get_node_size(key);
	}
	return szData;
}

OS_API_C_FUNC(char *) write_node(mem_zone_ref_const_ptr key, unsigned char *payload)
{
	mem_zone_ref txin_list = { PTR_NULL }, my_list = { PTR_NULL }, tmp = { PTR_NULL };
	ipv4_t			ip;
	mem_zone_ref_ptr sub = PTR_NULL;
	unsigned char	*data;
	unsigned int	 nc, type;
	unsigned short   port;

	type = tree_mamanger_get_node_type(key);

	if ((type >> 24) == 0x1E)
	{
		data = tree_mamanger_get_node_data_ptr(key, 0);
		if (data != PTR_NULL)
		{
			memcpy_c(payload, data, sizeof(hash_t));
			payload += sizeof(hash_t);
		}
		return payload;
	}

	switch (type)
	{
	case NODE_GFX_SIGNED_INT:
	case NODE_GFX_INT:
		tree_mamanger_get_node_dword(key, 0, (unsigned int *)payload);
		payload += 4;
		break;
	case NODE_GFX_SIGNED_BINT:
	case NODE_GFX_BINT:
		tree_mamanger_get_node_qword(key, 0, (uint64_t *)payload);
		payload += 8;
	break;
	case NODE_RT_VEC3:
		data = tree_mamanger_get_node_data_ptr(key, 0);
		memcpy_c(payload, data, sizeof(float)*3);
		payload += sizeof(float) * 3;
	break;
	case NODE_GFX_4UC:
		tree_manager_get_node_4uc	(key, 0, payload);
		payload += sizeof(vec_4uc_t);
	break;
	case NODE_GFX_FLOAT:
		tree_mamanger_get_node_float(key, 0, (float *)payload);
		payload += sizeof(float);
	break;
	case NODE_BIN_DATA:
		tree_manager_get_node_hash(key, 0, (unsigned char *)payload);
		payload += sizeof(hash_t);
	break;
	case NODE_BITCORE_PUBKEY:
		data	=	tree_mamanger_get_node_data_ptr(key, 0);
		memcpy_c	(payload, data, 33);
		payload += 33;
	break;
	case NODE_BITCORE_BLOCK_HASH:
	case NODE_BITCORE_TX_HASH:
	case NODE_FILE_HASH:
	case NODE_BITCORE_HASH:
		tree_manager_get_node_hash(key, 0, (unsigned char *)payload);
		payload += sizeof(hash_t);
	break;

	case NODE_BITCORE_BLK_HDR_LIST:

		nc = tree_manager_get_node_num_children(key);

		if (nc < 0xFD)
			*((unsigned char*)(payload++)) = nc;
		else if (nc < 0xFFFF)
		{
			*((unsigned char*)(payload++)) = 0xFD;
			*((unsigned short*)(payload)) = nc;
			payload += 2;
		}
		else 
		{
			*((unsigned char*)(payload++)) = 0xFE;
			*((unsigned int*)(payload)) = nc;
			payload += 5;
		}

		for (tree_manager_get_first_child(key, &my_list, &sub); ((sub != NULL) && (sub->zone != NULL)); tree_manager_get_next_child(&my_list, &sub))
		{
			payload = write_node(sub, payload);
			if (tree_manager_find_child_node(key, NODE_HASH("ntx"), NODE_BITCORE_VINT, &tmp))
			{
				data = tree_mamanger_get_node_data_ptr(&tmp, 0);
				if (*data < 0xFD)
				{
					*(payload++) = *data;
				}
				else 
				{
					*(payload++) = *data;
					(*((unsigned short *)(payload))) = (*((unsigned short *)(data + 1)));
					payload += 2;
				}
				release_zone_ref(&tmp);
			}
			else
			{
				*(payload++) = 0;
			}
		}

	break;
	case NODE_BITCORE_HASH_LIST:
		nc = tree_manager_get_node_num_children(key);

		if (nc < 0xFD)
			*((unsigned char*)(payload++)) = nc;
		else if (nc < 0xFFFF)
		{
			*((unsigned char*)(payload++))  = 0xFD;
			*((unsigned short*)(payload))	= nc;
			payload += 2;
		}

		for (tree_manager_get_first_child(key, &my_list, &sub); ((sub != NULL) && (sub->zone != NULL)); tree_manager_get_next_child(&my_list, &sub))
		{
			if (tree_mamanger_get_node_type(sub) == NODE_BITCORE_BLOCK_HASH)
			{
				*((unsigned int *)(payload)) = GETDATA_BLOCK;
				payload += 4;
			}
			else if (tree_mamanger_get_node_type(sub) == NODE_BITCORE_TX_HASH)
			{
				*((unsigned int *)(payload)) = GETDATA_TX;
				payload += 4;
			}
			else if (tree_mamanger_get_node_type(sub) == NODE_FILE_HASH)
			{
				*((unsigned int *)(payload)) = GETDATA_FILE;
				payload += 4;
			}
			tree_manager_get_node_hash(sub, 0, (unsigned char *)(payload));
			payload += 32;
		}
		break;
	case NODE_BITCORE_LOCATOR:
		nc = tree_manager_get_node_num_children(key);

		if (nc < 0xFD)
			*((unsigned char*)(payload++)) = nc;
		else if (nc < 0xFFFF)
		{
			*((unsigned char*)(payload++)) = 0xFD;
			*((unsigned short*)(payload)) = nc;
			payload += 2;
		}
		else
		{
			*((unsigned char*)(payload++)) = 0xFE;
			*((unsigned int*)(payload)) = nc;
			payload += 4;
		}

		for (tree_manager_get_first_child(key, &my_list, &sub); ((sub != NULL) && (sub->zone != NULL)); tree_manager_get_next_child(&my_list, &sub))
		{
			tree_manager_get_node_hash(sub, 0, (unsigned char *)(payload));
			payload += 32;
		}
		break;
	case NODE_BITCORE_VSTR:
		payload += tree_manager_get_node_vstr(key, 0, payload);
		break;
	case NODE_BITCORE_VINT:
		data = tree_mamanger_get_node_data_ptr(key, 0);
		if (*data < 0xFD)
		{
			*(payload++) = *data;
		}
		else if (*data == 0xFD)
		{
			*(payload++) = *data;
			(*((unsigned short *)(payload))) = (*((unsigned short *)(data + 1)));
			payload += 2;
		}
		else if (*data == 0xFE)
		{
			*(payload++) = *data;
			(*((unsigned int *)(payload))) = (*((unsigned int *)(data + 1)));
			payload += 4;
		}
		else if (*data == 0xFF)
		{
			*(payload++) = *data;
			(*((uint64_t *)(payload))) = (*((uint64_t *)(data + 1)));
			payload += 8;
		}
	break;
	case NODE_BITCORE_BLK_HDR:

		tree_manager_get_child_value_i32(key, NODE_HASH_version		, (unsigned int *)(payload));
		payload = mem_add(payload, 4);
		tree_manager_get_child_value_hash(key, NODE_HASH_prev		, (unsigned char *)(payload));
		payload = mem_add(payload, 32);
		tree_manager_get_child_value_hash(key, NODE_HASH_merkle_root, ((unsigned char *)(payload)));
		payload = mem_add(payload, 32);
		tree_manager_get_child_value_i32(key, NODE_HASH_time		, ((unsigned int *)(payload)));
		payload = mem_add(payload, 4);
		tree_manager_get_child_value_i32(key, NODE_HASH_bits		, ((unsigned int *)(payload)));
		payload = mem_add(payload, 4);
		tree_manager_get_child_value_i32(key, NODE_HASH_nonce		, ((unsigned int *)(payload)));
		payload = mem_add(payload, 4);



	break;
	case NODE_BITCORE_ADDR:
		tree_manager_get_child_value_i64(key, NODE_HASH_services, (uint64_t *)payload);
		tree_manager_get_child_value_ipv4(key, NODE_HASH_addr, ip);
		tree_manager_get_child_value_i16(key, NODE_HASH_port, &port);
		payload += 8;

		*((uint64_t	*)(payload)) = 0x0;		//(12 bytes 00 00 00 00 00 00 00 00 00 00 FF FF, followed by the 4 bytes of the IPv4 address).
		payload += 8;
		*((unsigned int	*)(payload)) = 0xFFFF0000;
		payload += 4;
		*((unsigned char*)(payload + 0)) = ip[0];
		*((unsigned char*)(payload + 1)) = ip[1];
		*((unsigned char*)(payload + 2)) = ip[2];
		*((unsigned char*)(payload + 3)) = ip[3];
		payload += 4;
		*((unsigned char*)(payload + 0)) = (unsigned char)(port >> 8);
		*((unsigned char*)(payload + 1)) = (unsigned char)(port & 0xFF);
		payload += 2;
	break;
	case NODE_BITCORE_ADDRT:
		tree_manager_get_child_value_i32(key, NODE_HASH_time, (unsigned int*)payload);
		payload += 4;
		tree_manager_get_child_value_i64(key, NODE_HASH_services, (uint64_t *)payload);
		payload += 8;

		tree_manager_get_child_value_ipv4(key, NODE_HASH_addr, ip);
		tree_manager_get_child_value_i16(key, NODE_HASH_port, &port);

		*((uint64_t	*)(payload)) = 0x0;		//(12 bytes 00 00 00 00 00 00 00 00 00 00 FF FF, followed by the 4 bytes of the IPv4 address).
		payload += 8;
		*((unsigned int	*)(payload)) = 0xFFFF0000;
		payload += 4;
		*((unsigned char*)(payload + 0)) = ip[0];
		*((unsigned char*)(payload + 1)) = ip[1];
		*((unsigned char*)(payload + 2)) = ip[2];
		*((unsigned char*)(payload + 3)) = ip[3];
		payload += 4;
		*((unsigned char*)(payload + 0)) = (unsigned char)(port >> 8);
		*((unsigned char*)(payload + 1)) = (unsigned char)(port & 0xFF);
		payload += 2;
	break;
	case NODE_BITCORE_ADDR_LIST:
			nc = tree_manager_get_node_num_children(key);

			if (nc < 0xFD)
				*((unsigned char*)(payload++)) = nc;
			else if (nc < 0xFFFF)
			{
				*((unsigned char*)(payload++)) = 0xFD;
				*((unsigned short*)(payload)) = nc;
				payload += 2;
			}
			else
			{
				*((unsigned char*)(payload++)) = 0xFE;
				*((unsigned int*)(payload)) = nc;
				payload += 4;
			}
			
			for (tree_manager_get_first_child(key, &my_list, &sub); ((sub != NULL) && (sub->zone != NULL)); tree_manager_get_next_child(&my_list, &sub))
			{
				payload = write_node(sub, payload);
			}
	break;
	case NODE_BITCORE_TXIN:
		tree_manager_get_child_value_hash			(key, NODE_HASH_tx_hash		, payload);
		payload += 32;
		tree_manager_get_child_value_i32			(key, NODE_HASH_idx			, (unsigned int *)payload);
		payload += 4;
		payload += tree_manager_get_child_value_vstr(key, NODE_HASH_script		, payload);
		tree_manager_get_child_value_i32			(key, NODE_HASH_sequence		, (unsigned int *)payload);
		payload += 4;
	break;
	case NODE_BITCORE_TXOUT:
		tree_manager_get_child_value_i64			(key, NODE_HASH_value, (uint64_t *)payload);
		payload += 8;
		payload += tree_manager_get_child_value_vstr(key, NODE_HASH_script, payload);
	break;
	case NODE_BITCORE_VINLIST:
		nc = tree_manager_get_node_num_children(key);

		if (nc < 0xFD)
			*((unsigned char*)(payload++)) = nc;
		else if (nc < 0xFFFF)
		{
			*((unsigned char*)(payload++)) = 0xFD;
			*((unsigned short*)(payload)) = nc;
			payload += 2;
		}
		else
		{
			*((unsigned char*)(payload++)) = 0xFE;
			*((unsigned int*)(payload)) = nc;
			payload += 4;
		}

		for (tree_manager_get_first_child(key, &my_list, &sub); ((sub != NULL) && (sub->zone != NULL)); tree_manager_get_next_child(&my_list, &sub))
		{
			payload = write_node(sub, payload);
		}
	break;
	case NODE_BITCORE_VOUTLIST:
		nc = tree_manager_get_node_num_children(key);

		if (nc < 0xFD)
			*((unsigned char*)(payload++)) = nc;
		else if (nc < 0xFFFF)
		{
			*((unsigned char*)(payload++)) = 0xFD;
			*((unsigned short*)(payload)) = nc;
			payload += 2;
		}
		else
		{
			*((unsigned char*)(payload++)) = 0xFE;
			*((unsigned int*)(payload)) = nc;
			payload += 4;
		}


		for (tree_manager_get_first_child(key, &my_list, &sub); ((sub != NULL) && (sub->zone != NULL)); tree_manager_get_next_child(&my_list, &sub))
		{
			payload = write_node(sub, payload);
		}
	break;
		case NODE_BITCORE_TX:
		tree_manager_get_child_value_i32(key, NODE_HASH_version, (unsigned int*)payload);
		payload += 4;
		tree_manager_get_child_value_i32(key, NODE_HASH_time, (unsigned int *)payload);
		payload += 4;

		tree_manager_find_child_node	(key, NODE_HASH_txsin, NODE_BITCORE_VINLIST, &txin_list);
		payload = write_node(&txin_list,payload);
		release_zone_ref(&txin_list);

		tree_manager_find_child_node(key, NODE_HASH_txsout, NODE_BITCORE_VOUTLIST, &txin_list);
		payload = write_node(&txin_list, payload);
		release_zone_ref(&txin_list);

		tree_manager_get_child_value_i32(key, NODE_HASH_locktime, (unsigned int *)payload);
		payload += 4;

	break;
	case NODE_BITCORE_TX_LIST:
		nc = tree_manager_get_node_num_children(key);

		if (nc < 0xFD)
			*((unsigned char*)(payload++)) = nc;
		else if (nc < 0xFFFF)
		{
			*((unsigned char*)(payload++)) = 0xFD;
			*((unsigned short*)(payload)) = nc;
			payload += 2;
		}
		else
		{
			*((unsigned char*)(payload++)) = 0xFE;
			*((unsigned int*)(payload)) = nc;
			payload += 4;
		}
		for (tree_manager_get_first_child(key, &my_list, &sub); ((sub != NULL) && (sub->zone != NULL)); tree_manager_get_next_child(&my_list, &sub))
		{
			payload = write_node(sub, payload);
		}
		
		break;
		case NODE_GFX_OBJECT:
		
		for (tree_manager_get_first_child(key, &my_list, &sub); ((sub != NULL) && (sub->zone != NULL)); tree_manager_get_next_child(&my_list, &sub))
		{
			payload = write_node(sub, payload);
		}
		break;
	}
	
	return payload;
}


OS_API_C_FUNC(size_t)read_node(mem_zone_ref_ptr key, const unsigned char *payload,size_t len)
{
	unsigned int		n, nc,type;
	mem_zone_ref		txin_list = { PTR_NULL }, my_list = { PTR_NULL };
	mem_zone_ref_ptr	sub = PTR_NULL;
	struct string		str;
	float				*data_f;
	vec_4uc_t			data_uc;
	unsigned short		port;
	size_t				sz,read,rd;
	ipv4_t				ip;

	type = tree_mamanger_get_node_type(key);

	read = 0;

	if ((type >> 24) == 0x1E)
	{
		if ((read + sizeof(hash_t)) > len)return INVALID_SIZE;
		tree_manager_write_node_hash(key, 0, ((unsigned char *)(payload + read)));
		read += sizeof(hash_t);
		return (read);
	}
	

	switch (type)
	{
	case NODE_GFX_INT:

		if ((read + sizeof(unsigned int)) > len)return INVALID_SIZE;
		tree_manager_write_node_dword(key, 0, *((unsigned int *)(payload + read)));
		read += 4;
	break;
	case NODE_GFX_BINT:

		if ((read + sizeof(uint64_t)) > len)return INVALID_SIZE;
		tree_manager_write_node_qword(key, 0, *((uint64_t *)(payload + read)));
		read += sizeof(uint64_t);
		break;
	case NODE_RT_VEC3:
		if ((read + (sizeof(float) * 3)) > len)return INVALID_SIZE;
		data_f = (float *)(payload+read);
		tree_manager_write_node_vec3f(key, 0, data_f[0], data_f[1], data_f[2]);
		read += sizeof(float) * 3;
	break;
	case NODE_GFX_4UC:
		if ((read + (sizeof(vec_4uc_t))) > len)return INVALID_SIZE;
		data_uc[0] = payload[read + 0];
		data_uc[1] = payload[read + 1];
		data_uc[2] = payload[read + 2];
		data_uc[3] = payload[read + 3];
		tree_manager_write_node_4uc(key, 0, data_uc);
		read += sizeof(vec_4uc_t);
	break;
	case NODE_GFX_FLOAT:
		if ((read + sizeof(float)) > len)return INVALID_SIZE;
		tree_manager_write_node_float(key, 0, *((float *)(payload+read)));
		read += sizeof(float);
	break;
	case NODE_BIN_DATA:
		if ((read + sizeof(hash_t)) > len)return INVALID_SIZE;
		tree_manager_write_node_hash(key, 0, ((unsigned char *)(payload+read)));
		read += sizeof(hash_t);
	break;
	case NODE_BITCORE_PUBKEY:
		if ((read + 33) > len)return INVALID_SIZE;
		tree_manager_write_node_data(key, ((unsigned char *)(payload + read)), 0, 33);
		read += 33;
	break;
	case NODE_BITCORE_BLOCK_HASH:
	case NODE_BITCORE_TX_HASH:
	case NODE_FILE_HASH:
	case NODE_BITCORE_HASH:
		if ((read + sizeof(hash_t)) > len)return INVALID_SIZE;
		tree_manager_write_node_hash(key, 0, ((unsigned char *)(payload+read)));
		read += sizeof(hash_t);
	break;
	case NODE_BITCORE_LOCATOR:

		if ((read + 2) > len)return INVALID_SIZE;

		if (*(payload + read) < 0xFD)
		{
			nc = *(payload+read);
			read++;
		}
		else if (*(payload + read) == 0xFD)
		{
			read++;
			if ((read + 2) > len)return INVALID_SIZE;
			nc = *((unsigned short *)(payload+read));
			read += 2;
		}
		else if (*(payload + read) < 0xFE)
		{
			read++;
			if ((read + 4) > len)return INVALID_SIZE;
			nc = *((unsigned int *)(payload+read));
			read += 4;
		}
		else if (*(payload + read) < 0xFF)
		{
			read++;
			if ((read + 8) > len)return INVALID_SIZE;
			nc = *((uint64_t *)(payload + read));
			read += 8;
		}
		n = 0;
		while (n<nc)
		{
			mem_zone_ref ssub = { PTR_NULL };

			if ((read + sizeof(hash_t)) > len)return INVALID_SIZE;

			if (!tree_manager_get_child_at(key, n, &ssub))
				tree_manager_add_child_node(key, "hash", NODE_BITCORE_HASH, &ssub);

			tree_manager_write_node_hash(&ssub, 0, (unsigned char *)(payload+read));
			release_zone_ref(&ssub);
			read += 32;
		}
	break;
	case NODE_BITCORE_BLK_HDR_LIST:

		if ((read + 1) > len)return INVALID_SIZE;

		if (*(payload + read) < 0xFD)
		{
			nc = *(payload + read);
			read++;
		}
		else if (*(payload + read) == 0xFD)
		{
			read++;
			if ((read + 2) > len)return INVALID_SIZE;
			nc = *((unsigned short *)(payload + read));
			read += 2;
		}
		else if (*(payload + read) < 0xFE)
		{
			read++;
			if ((read + 4) > len)return INVALID_SIZE;
			nc = *((unsigned int *)(payload + read));
			read += 4;
		}
		else if (*(payload + read) < 0xFF)
		{
			read++;
			if ((read + 8) > len)return INVALID_SIZE;
			nc = *((uint64_t *)(payload + read));
			read += 8;
		}

		n = 0;
		while (n<nc)
		{
			mem_zone_ref ssub = { PTR_NULL };
			if (tree_manager_get_child_at(key, n, &ssub))
			{
				

				if ((rd = read_node(&ssub, payload + read, len - read)) == INVALID_SIZE){
					release_zone_ref(&ssub);
					return rd;
				}

				read += rd;

				if ((read + 1) > len)return INVALID_SIZE;
				if ((*(payload + read) == 0xFD) && ((read + 3) > len))return INVALID_SIZE;
				
				tree_manager_set_child_value_vint(&ssub, "ntx", payload+read);

				if (*(payload + read) < 0xFD)
					read++;
				else if (*(payload + read) == 0xFD)
					read += 3;

				release_zone_ref(&ssub);
			}
			n++;
		}
	break;
	case NODE_BITCORE_HASH_LIST:

		if ((read + 1) > len)return INVALID_SIZE;

		if (*(payload + read) < 0xFD)
		{
			nc = *(payload + read);
			read++;
		}
		else if (*(payload + read) == 0xFD)
		{
			read++;
			if ((read + 2) > len)return INVALID_SIZE;
			nc = *((unsigned short *)(payload + read));
			read += 2;
		}
		else if (*(payload + read) < 0xFE)
		{
			read++;
			if ((read + 4) > len)return INVALID_SIZE;
			nc = *((unsigned int *)(payload + read));
			read += 4;
		}
		else if (*(payload + read) < 0xFF)
		{
			read++;
			if ((read + 8) > len)return INVALID_SIZE;
			nc = *((uint64_t *)(payload + read));
			read += 8;
		}
		n = 0;
		while (n<nc)
		{
			mem_zone_ref ssub = { PTR_NULL };
			unsigned int type = NODE_BITCORE_HASH;


			if ((read + 36) > len)return INVALID_SIZE;

			if (*((unsigned int *)(payload+read)) == GETDATA_BLOCK)
			{
				type	= NODE_BITCORE_BLOCK_HASH;
				read += 4;
			}
			if (*((unsigned int *)(payload + read)) == GETDATA_TX)
			{
				type = NODE_BITCORE_TX_HASH;
				read += 4;
			}
			if (*((unsigned int *)(payload + read)) == GETDATA_FILE)
			{
				type = NODE_FILE_HASH;
				read += 4;
			}
			if (!tree_manager_get_child_at(key, n, &ssub))
				tree_manager_add_child_node(key, "hash", type, &ssub);

			tree_manager_write_node_hash(&ssub, 0, (unsigned char *)(payload+read));
			release_zone_ref(&ssub);
			read += 32;
			n++;
		}
		break;
		case NODE_GFX_STR:
		case NODE_BITCORE_VSTR:

		if ((read + 1) > len)return INVALID_SIZE;

		if (*((unsigned char *)(payload + read)) < 0xFD)
		{
			sz = (*((unsigned char *)(payload + read))) + 1;
		}
		else if (*((unsigned char *)(payload + read)) == 0xFD)
		{
			if ((read + 3) > len)return INVALID_SIZE;
			sz = (*((unsigned short *)(payload+read+1))) + 3;
		}
		else if (*((unsigned char *)(payload + read)) == 0xFE)
		{
			if ((read + 5) > len)return INVALID_SIZE;
			sz = (*((unsigned int *)(payload + read + 1))) + 5;
		}
		else if (*((unsigned char *)(payload + read)) == 0xFF)
		{
			if ((read + 9) > len)return INVALID_SIZE;
			sz = (*((uint64_t *)(payload + read + 1))) + 9;
		}
		if ((read + sz) > len)return INVALID_SIZE;
		tree_manager_write_node_data(key, payload+read, 0, sz);
		read += sz;
		break;
	case NODE_BITCORE_VINT:

		if ((read + 1) > len)return INVALID_SIZE;

		if (*((unsigned char *)(payload+read)) < 0xFD)
			sz = 1;
		else if (*((unsigned char *)(payload + read)) == 0xFD)
			sz = 3;
		else if (*((unsigned char *)(payload + read)) == 0xFE)
			sz = 5;
		else if (*((unsigned char *)(payload + read)) == 0xFF)
			sz =  9;

		if ((read + sz) > len)return INVALID_SIZE;
		tree_manager_write_node_data(key, payload+read, 0, sz);
		read += sz;
		break;
	case NODE_BITCORE_ECDSA_SIG:

		if (len == 1)
		{
			unsigned char sz = *((unsigned char *)(payload + read));

			tree_manager_write_node_data(key, payload + read, 0, sz); 
			read++;
			return read;
		}

		if ((read + 5) > len)return INVALID_SIZE;

		if (*((unsigned char *)(payload + read + 1)) == 0x30)
		{
			unsigned char rlen, slen;
			read++;
			tree_manager_write_node_byte(key, 0, *((unsigned char *)(payload + read))); read++;
			tree_manager_write_node_byte(key, 1, *((unsigned char *)(payload + read))); read++;
			tree_manager_write_node_byte(key, 2, *((unsigned char *)(payload + read))); read++;
			rlen = *((unsigned char *)(payload + read));
			tree_manager_write_node_byte(key, 3, rlen); read++;

			if ((read + rlen) > len)return INVALID_SIZE;
			tree_manager_write_node_data(key, payload + read, 4, rlen); read += rlen;
			tree_manager_write_node_byte(key, rlen + 4, *((unsigned char *)(payload + read))); read++;
			slen = *((unsigned char *)(payload + read));
			if ((read + slen) > len)return INVALID_SIZE;
			tree_manager_write_node_byte(key, rlen + 5, slen); read++;
			tree_manager_write_node_data(key, payload + read, rlen + 6, slen); read += slen;
		}
		/*
		if (*((unsigned char *)(mem_add(payload,1))) == 0x30)
		{
			unsigned char rlen, slen;
			payload = mem_add(payload, 1);
			tree_manager_write_node_byte(key, 0, *((unsigned char *)(payload)));payload = mem_add(payload, 1);
			tree_manager_write_node_byte(key, 1, *((unsigned char *)(payload)));payload = mem_add(payload, 1);
			tree_manager_write_node_byte(key, 2, *((unsigned char *)(payload)));payload = mem_add(payload, 1);
			rlen	= *((unsigned char *)(payload));
			tree_manager_write_node_byte(key, 3, rlen); payload = mem_add(payload, 1);
			tree_manager_write_node_data(key, payload, 4, rlen);
			payload = mem_add(payload, rlen);
			tree_manager_write_node_byte(key, rlen + 4, *((unsigned char *)(payload)));payload = mem_add(payload, 1);
			slen   = *((unsigned char *)(payload));
			tree_manager_write_node_byte(key, rlen + 5, slen); payload = mem_add(payload, 1);
			tree_manager_write_node_data(key, payload, rlen + 6, slen); payload = mem_add(payload, slen);
		}*/
	break;
	case NODE_BITCORE_BLK_HDR:

		if ((read + 76) > len)return INVALID_SIZE;

		tree_manager_set_child_value_i32(key, "version"		, *((unsigned int *)(payload+read)));
		read += 4;
		tree_manager_set_child_value_hash(key, "prev", ((unsigned char *)(payload + read)));
		read += 32;
		tree_manager_set_child_value_hash(key, "merkle_root", ((unsigned char *)(payload + read)));
		read += 32;
		tree_manager_set_child_value_i32(key, "time", *((unsigned int *)(payload + read)));
		read += 4;
		tree_manager_set_child_value_i32(key, "bits", *((unsigned int *)(payload + read)));
		read += 4;

		if ((read + 4) <= len)
		{
			tree_manager_set_child_value_i32(key, "nonce", *((unsigned int *)(payload + read)));
			read += 4;
		}
		else
			tree_manager_set_child_value_i32(key, "nonce", 0);
		
	break;
	case NODE_BITCORE_ADDR:


		if ((read + 26) > len)return INVALID_SIZE;

		tree_manager_set_child_value_i64(key, "services", *((uint64_t *)(payload+read)));
		read += 8;
		//(12 bytes 00 00 00 00 00 00 00 00 00 00 FF FF, followed by the 4 bytes of the IPv4 address).
		ip[0] = *((unsigned char*)(payload + read + 12));
		ip[1] = *((unsigned char*)(payload + read + 13));
		ip[2] = *((unsigned char*)(payload + read + 14));
		ip[3] = *((unsigned char*)(payload + read + 15));
		read += 16;
		port = ((*((unsigned char*)(payload + read))) << 8) | (*((unsigned char*)(payload + read+ 1)));
		read += 2;

		tree_manager_set_child_value_ipv4(key, "addr", ip);
		tree_manager_set_child_value_i16 (key, "port", port);

	break;
	case NODE_BITCORE_ADDRT:

		if ((read + 30) > len)return INVALID_SIZE;

		tree_manager_set_child_value_i32(key, "time"	, *((unsigned int *)(payload+read)));
		read += 4;
		tree_manager_set_child_value_i64(key, "services", *((uint64_t *)(payload+read)));
		read += 8;
		//(12 bytes 00 00 00 00 00 00 00 00 00 00 FF FF, followed by the 4 bytes of the IPv4 address).
		ip[0] = *((unsigned char*)(payload + read + 12));
		ip[1] = *((unsigned char*)(payload + read + 13));
		ip[2] = *((unsigned char*)(payload + read + 14));
		ip[3] = *((unsigned char*)(payload + read + 15));
		read += 16;
		port = ((*((unsigned char*)(payload + read))) << 8) | (*((unsigned char*)(payload + read + 1)));
		read += 2;

		tree_manager_set_child_value_ipv4(key, "addr", ip);
		tree_manager_set_child_value_i16(key, "port", port);
	break;
	case NODE_BITCORE_ADDR_LIST:

		if ((read + 1) > len)return INVALID_SIZE;

		if (*(payload + read) < 0xFD)
		{
			nc = *(payload + read);
			read++;
		}
		else if (*(payload + read) == 0xFD)
		{
			read++;
			if ((read + 2) > len)return INVALID_SIZE;
			nc = *((unsigned short *)(payload + read));
			read += 2;
		}
		else if (*(payload + read) < 0xFE)
		{
			read++;
			if ((read + 4) > len)return INVALID_SIZE;
			nc = *((unsigned int *)(payload + read));
			read += 4;
		}
		else if (*(payload + read) < 0xFF)
		{
			read++;
			if ((read + 8) > len)return INVALID_SIZE;
			nc		= *((uint64_t *)(payload + read));
			read   += 8;
		}

		for (tree_manager_get_first_child(key, &my_list, &sub); ((sub != NULL) && (sub->zone != NULL)); tree_manager_get_next_child(&my_list, &sub))
		{
			if ((rd = read_node(sub, payload + read, len - read)) == INVALID_SIZE)
				return INVALID_SIZE;
			read += rd;
		}
	break;

	case NODE_BITCORE_TXIN:
		
		if ((read + 36) > len)return INVALID_SIZE;

		tree_manager_set_child_value_hash(key, "txid", payload+read);
		read += 32;
		tree_manager_set_child_value_i32(key, "idx", *((unsigned int *)(payload + read)));
		read += 4;

		if ((read + 1) > len)return INVALID_SIZE;

		if (*((unsigned char *)(payload+read)) < 0xFD)
		{
			str.len = (*((unsigned char *)(payload + read)));
			sz = 1;
		}
		else if (*((unsigned char *)(payload + read)) == 0xFD)
		{
			if ((read + 3) > len)return INVALID_SIZE;

			str.len = (*((unsigned short *)(payload +read + 1)));
			sz = 3;
		}
		else if (*((unsigned char *)(payload + read)) == 0xFE)
		{
			if ((read + 5) > len)return INVALID_SIZE;

			str.len = (*((unsigned int *)(payload + read + 1)));
			sz = 5;
		}
		else if (*((unsigned char *)(payload + read)) == 0xFF)
		{
			if ((read + 9) > len)return INVALID_SIZE;

			str.len = (*((uint64_t *)(payload + read + 1)));
			sz = 9;
		}

		str.size = str.len + 1;

		if (str.len > 0)
			str.str = payload+read+sz;
		else
			str.str = PTR_NULL;

		if ((read + sz + str.len) > len)return INVALID_SIZE;

		tree_manager_set_child_value_vstr(key, "script", &str);
		read+= (str.len + sz);
		
		tree_manager_set_child_value_i32(key, "sequence", *((unsigned int *)(payload+read)));
		read += 4;
	break;
	case NODE_BITCORE_TXOUT:

		if ((read + 9) > len)return INVALID_SIZE;

		tree_manager_set_child_value_i64(key, "value", *((uint64_t *)(payload+read)));
		read += 8;
		if (*((unsigned char *)(payload+read)) < 0xFD)
		{
			str.len = (*((unsigned char *)(payload + read)));
			sz = 1;
		}
		else if (*((unsigned char *)(payload + read)) == 0xFD)
		{
			str.len = (*((unsigned short *)(payload + read+1)));
			sz = 3;
		}
		else if (*((unsigned char *)(payload + read)) == 0xFE)
		{
			str.len = (*((unsigned int *)(payload + read + 1)));
			sz = 5;
		}
		else if (*((unsigned char *)(payload + read)) == 0xFF)
		{
			str.len = (*((uint64_t *)(payload + read + 1)));
			sz = 9;
		}

		if ((read + sz + str.len) > len)return INVALID_SIZE;

		if (str.len > 0)
		{
			str.size = str.len + 1;
			str.str  = payload + read + sz;
			tree_manager_set_child_value_vstr(key, "script", &str);
		}
		else
		{
			struct string null_str;
			null_str.str = PTR_NULL;
			null_str.len = 0;
			null_str.size = 0;
			
			tree_manager_set_child_value_vstr(key, "script", &null_str);
		}
		read += str.len + sz;

	break;
	case NODE_BITCORE_VINLIST:

		if ((read + 1) > len)return INVALID_SIZE;

		if (*(payload + read) < 0xFD)
		{
			nc = *(payload+read);
			read++;
		}
		else if (*(payload + read) == 0xFD)
		{
			read++;
			if ((read + 2) > len)return INVALID_SIZE;
			nc = *((unsigned short *)(payload+read));
			read += 2;
		}
		else if (*(payload + read) < 0xFE)
		{
			read++;
			if ((read + 4) > len)return INVALID_SIZE;
			nc = *((unsigned int *)(payload+read));
			read += 4;
		}
		else if (*(payload + read) < 0xFF)
		{
			read++;
			if ((read + 8) > len)return INVALID_SIZE;
			nc = *((uint64_t *)(payload+read));
			read  += 8;
		}
		n = 0;
		while (n<nc)
		{
			mem_zone_ref ssub = { PTR_NULL };
			

			if (!tree_manager_get_child_at(key, n, &ssub))
				tree_manager_add_child_node(key, "txin", NODE_BITCORE_TXIN, &ssub);

			if ((rd = read_node(&ssub, payload + read, len - read)) == INVALID_SIZE)
			{
				release_zone_ref(&ssub);
				return rd;
			}
			
			read += rd;
			release_zone_ref(&ssub);
			n++;
		}
	break;
	case NODE_BITCORE_VOUTLIST:
		if ((read + 1) > len)return INVALID_SIZE;

		if (*(payload + read) < 0xFD)
		{
			nc = *(payload + read);
			read++;
		}
		else if (*(payload + read) == 0xFD)
		{
			read++;
			if ((read + 2) > len)return INVALID_SIZE;
			nc = *((unsigned short *)(payload + read));
			read += 2;
		}
		else if (*(payload + read) < 0xFE)
		{
			read++;
			if ((read + 4) > len)return INVALID_SIZE;
			nc = *((unsigned int *)(payload + read));
			read += 4;
		}
		else if (*(payload + read) < 0xFF)
		{
			read++;
			if ((read + 8) > len)return INVALID_SIZE;
			nc = *((uint64_t *)(payload + read));
			read += 8;
		}
		n = 0;
		while (n<nc)
		{
			mem_zone_ref ssub = { PTR_NULL };
			

			if (!tree_manager_get_child_at(key, n, &ssub))
				tree_manager_add_child_node(key, "txout", NODE_BITCORE_TXOUT, &ssub);

			if ((rd = read_node(&ssub, payload + read, len - read)) == INVALID_SIZE){
				release_zone_ref(&ssub);
				return rd;
			}

			read += rd;

			release_zone_ref(&ssub);
			n++;
		}
		break;
	case NODE_BITCORE_TX:

		if ((read + 8) > len)return INVALID_SIZE;

		tree_manager_set_child_value_i32(key, "version", *((unsigned int *)(payload+read)));
		read += 4;
		tree_manager_set_child_value_i32(key, "time", *((unsigned int *)(payload + read)));
		read += 4;

		tree_manager_find_child_node(key, NODE_HASH_txsin, NODE_BITCORE_VINLIST, &txin_list);
		if ((rd = read_node(&txin_list, payload + read, len - read)) == INVALID_SIZE){ release_zone_ref(&txin_list); return rd; }
		read += rd;
		release_zone_ref(&txin_list);

		tree_manager_find_child_node(key, NODE_HASH_txsout, NODE_BITCORE_VOUTLIST, &txin_list);
		if ((rd = read_node(&txin_list, payload + read, len - read)) == INVALID_SIZE){ release_zone_ref(&txin_list); return rd; }
		read += rd;
		release_zone_ref(&txin_list);
		if ((read + 4) > len)return INVALID_SIZE;
		tree_manager_set_child_value_i32(key, "locktime", *((unsigned int *)(payload+read)));
		read += 4;

	break;

	case NODE_BITCORE_TX_LIST:

		if ((read + 1) > len)return INVALID_SIZE;

		if (*((unsigned char *)(payload+read)) < 0xFD)
			sz = 1;
		else if (*((unsigned char *)(payload + read)) == 0xFD)
			sz = 3;
		else if (*((unsigned char *)(payload + read)) == 0xFE)
			sz = 5;
		else if (*((unsigned char *)(payload + read)) == 0xFF)
			sz = 9;

		if ((read + sz) > len)return INVALID_SIZE;

		read += sz;

		for (tree_manager_get_first_child(key, &my_list, &sub); ((sub != NULL) && (sub->zone != NULL)); tree_manager_get_next_child(&my_list, &sub))
		{
			if ((rd = read_node(sub, payload + read, len - read)) == INVALID_SIZE)
			{
				dec_zone_ref(sub);
				release_zone_ref(&my_list);
				return rd;
			}
			read += rd;
		}
		break;
	}

	if (read == INVALID_SIZE)
	{
		log_output("read_node error \n");
	}
	return read;
}
OS_API_C_FUNC(int) unserialize_children(mem_zone_ref_ptr obj, const_mem_ptr payload,size_t len)
{
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref_ptr	key;
	size_t				read, rd;

	read = 0;

	for (tree_manager_get_first_child(obj, &my_list, &key); ((key != NULL) && (key->zone != NULL)); tree_manager_get_next_child(&my_list, &key))
	{
		if ((rd = read_node(key, mem_add(payload,read),len-read)) == INVALID_SIZE){

			log_output("read_node error \n");
			dec_zone_ref(key);
			release_zone_ref(&my_list);
			return 0;
		}
		read += rd;
	}

	return 1;
}
OS_API_C_FUNC(int) unserialize_message(mem_zone_ref_ptr msg, const_mem_ptr payload, size_t len, const char *model)
{
	mem_zone_ref		payload_node = { PTR_NULL };
	mem_zone_ref		my_list = { PTR_NULL };
	mem_zone_ref_ptr	key;
	size_t				szData,read,rd;
	int					created=0;
	
	if (payload == PTR_NULL)return 0;
	if (len == 0)return 0;
	if (!tree_manager_get_child_value_i32(msg, NODE_HASH_size, &szData))return 0;
	if (szData == 0)return 0;

	tree_manager_find_child_node(msg, NODE_HASH_payload, NODE_BITCORE_PAYLOAD, &payload_node);

	if (payload_node.zone == PTR_NULL)
	{
		if (!tree_manager_json_loadb(model, strlen_c(model), &payload_node))return 0;
		created = 1;
	}

	read = 0;
	
	for (tree_manager_get_first_child(&payload_node, &my_list, &key); ((key != NULL) && (key->zone != NULL)); tree_manager_get_next_child(&my_list, &key))
	{
		if ((rd = read_node(key, mem_add(payload, read), len - read)) == INVALID_SIZE)
		{
			log_output("read_node error \n");

			dec_zone_ref(key);
			release_zone_ref(&my_list);

			release_zone_ref(&payload_node);
			return 0;
		}
		read += rd;
	}
	if (created==1)
		tree_manager_node_add_child		(msg, &payload_node);

	release_zone_ref				(&payload_node);
	return 1;
}

OS_API_C_FUNC(void) serialize_children(mem_zone_ref_ptr node, unsigned char *payload)
{
	mem_zone_ref_ptr	key;
	mem_zone_ref		my_list = { PTR_NULL };

	for (tree_manager_get_first_child(node, &my_list, &key); ((key != NULL) && (key->zone != NULL)); tree_manager_get_next_child(&my_list, &key))
	{
		payload = write_node(key, payload);
	}
}

OS_API_C_FUNC(char *) serialize_message(mem_zone_ref_ptr msg)
{
	mem_zone_ref		payload_node = { PTR_NULL };
	char				cmd_node[12];
	mem_zone_ref		my_list = { PTR_NULL };
	size_t				szData;
	unsigned char		*buffer, *payload;
	unsigned char		checksum1[32];
	unsigned char		checksum[32];

	if (!tree_manager_get_child_value_i32(msg, NODE_HASH_size, &szData))return PTR_NULL;
	if (!tree_manager_get_child_value_str(msg, NODE_HASH_cmd, cmd_node, 12, 16))return PTR_NULL;
	
	buffer = calloc_c(24 + szData, 1);
	if (buffer == PTR_NULL)
		return PTR_NULL;

	memcpy_c(buffer, &magic, 4);
	
	memset_c	(&buffer[4], 0, 12);
	strcpy_cs	(&buffer[4], 12, cmd_node);

	memcpy_c	(&buffer[16], &szData, 4);

	if ((szData > 0) && tree_manager_find_child_node(msg, NODE_HASH_payload, NODE_BITCORE_PAYLOAD, &payload_node))
	{
		payload = buffer + 4 + 12 + 4 + 4;

		serialize_children(&payload_node, payload);
		payload = buffer + 4 + 12 + 4 + 4;
		mbedtls_sha256(payload, szData, checksum1, 0);
		mbedtls_sha256(checksum1, 32, checksum, 0);
		memcpy_c(&buffer[20], checksum, 4);
		release_zone_ref(&payload_node);
	}
	else
	{
		/*mbedtls_sha256(PTR_NULL, 0, checksum1, 0);
		mbedtls_sha256(checksum1, 32, checksum, 0);*/
		buffer[20] = 0x5D;
		buffer[21] = 0xF6;
		buffer[22] = 0xE0;
		buffer[23] = 0xE2; 
	}

	
	
	return buffer;
}

OS_API_C_FUNC(int) create_verack_message(mem_zone_ref_ptr node, mem_zone_ref_ptr ver_pack)
{
	tree_manager_create_node		("message", NODE_BITCORE_MSG, ver_pack);
	tree_manager_set_child_value_str(ver_pack, "cmd", "verack");
	tree_manager_set_child_value_i32(ver_pack, "size", 0);
	tree_manager_set_child_value_i32(ver_pack, "sent", 0);
	return 1;
}

OS_API_C_FUNC(int) create_mempool_message(mem_zone_ref_ptr node, mem_zone_ref_ptr mempool_pack)
{
	tree_manager_create_node("message", NODE_BITCORE_MSG, mempool_pack);
	tree_manager_set_child_value_str(mempool_pack, "cmd", "mempool");
	tree_manager_set_child_value_i32(mempool_pack, "size", 0);
	tree_manager_set_child_value_i32(mempool_pack, "sent", 0);
	return 1;
}

OS_API_C_FUNC(int) create_addr_message(mem_zone_ref_ptr node, mem_zone_ref_ptr addrs, mem_zone_ref_ptr addr_pack)
{
	mem_zone_ref		inv_vec = { PTR_NULL }, payload = { PTR_NULL }, maddrs = { PTR_NULL };
	size_t				pl_size;

	tree_manager_create_node		("message", NODE_BITCORE_MSG, addr_pack);
	tree_manager_set_child_value_str(addr_pack, "cmd", "addr");

	tree_manager_add_child_node		(addr_pack, "payload" , NODE_BITCORE_PAYLOAD, &payload);
	tree_manager_add_child_node		(&payload , "addr_list", NODE_BITCORE_ADDR_LIST, &maddrs);
	tree_manager_copy_children_ref	(&maddrs, addrs);
	release_zone_ref				(&maddrs);

	pl_size = compute_payload_size	(&payload);
	release_zone_ref				(&payload);
	
	tree_manager_set_child_value_i32(addr_pack, "size", pl_size);
	tree_manager_set_child_value_i32(addr_pack, "sent", 0);

	return 1;
}

OS_API_C_FUNC(int) create_getdata_message(mem_zone_ref_ptr node, mem_zone_ref_ptr hash_list, mem_zone_ref_ptr blk_hdr_pack)
{
	mem_zone_ref		inv_vec = { PTR_NULL }, payload = { PTR_NULL }, nhashl = { PTR_NULL };
	size_t				pl_size;
	int					cnt;

	tree_manager_create_node("message", NODE_BITCORE_MSG, blk_hdr_pack);
	tree_manager_set_child_value_str(blk_hdr_pack, "cmd", "getdata");

	cnt = tree_manager_get_node_num_children(hash_list);

	tree_manager_add_child_node		(blk_hdr_pack, "payload", NODE_BITCORE_PAYLOAD, &payload);
	tree_manager_add_child_node		(&payload, "hash list", NODE_BITCORE_HASH_LIST, &nhashl);
	tree_manager_copy_children_ref	(&nhashl, hash_list);
	release_zone_ref				(&nhashl);
	pl_size = compute_payload_size  (&payload);
	release_zone_ref				(&payload);
	cnt = tree_manager_get_node_num_children(hash_list);

	tree_manager_set_child_value_i32(blk_hdr_pack, "size", pl_size);
	tree_manager_set_child_value_i32(blk_hdr_pack, "sent", 0);

	return 1;
}
OS_API_C_FUNC(int) create_getheaders_message(mem_zone_ref_ptr node, unsigned int version, mem_zone_ref_ptr blk_locator,hash_t hashstop, mem_zone_ref_ptr blk_hdr_pack)
{
	mem_zone_ref		payload = { PTR_NULL };
	size_t				pl_size;
	int					n,cnt;
	
	tree_manager_create_node("message", NODE_BITCORE_MSG, blk_hdr_pack);
	tree_manager_set_child_value_str (blk_hdr_pack, "cmd", "getheaders");

	cnt = tree_manager_get_node_num_children(blk_locator);

	tree_manager_add_child_node				(blk_hdr_pack, "payload", NODE_BITCORE_PAYLOAD, &payload);
	tree_manager_set_child_value_i32		(&payload, "version", version);
	tree_manager_set_child_value_vint32		(&payload, "count", cnt);
	for (n = 0; n < cnt;n++)
	{
		mem_zone_ref hash_node = { PTR_NULL }, loc = { PTR_NULL };
		if (tree_manager_get_child_at(blk_locator, n, &hash_node))
		{
			tree_manager_node_dup(&payload, &hash_node, &loc, 0xFFFFFFFF);
			release_zone_ref(&loc);
			release_zone_ref(&hash_node);
		}

	}
	tree_manager_set_child_value_hash(&payload, "hashstop", hashstop);

	pl_size = compute_payload_size(&payload);
	release_zone_ref(&payload);

	tree_manager_set_child_value_i32(blk_hdr_pack, "size", pl_size);
	tree_manager_set_child_value_i32(blk_hdr_pack, "sent", 0);

	return 1;

}
OS_API_C_FUNC(int) create_getblocks_message(mem_zone_ref_ptr node, unsigned int version, mem_zone_ref_ptr blk_locator, mem_zone_ref_ptr getblk_pack)
{
	mem_zone_ref		payload = { PTR_NULL };
	size_t				pl_size;
	

	if (!tree_manager_create_node("message", NODE_BITCORE_MSG, getblk_pack))return 0;
	
	
	tree_manager_set_child_value_str	(getblk_pack, "cmd", "getblocks");
	tree_manager_add_child_node			(getblk_pack, "payload", NODE_BITCORE_PAYLOAD, &payload);
	tree_manager_set_child_value_i32	(&payload, "version", version);
	tree_manager_node_add_child			(&payload, blk_locator);
	tree_manager_set_child_value_hash	(&payload, "hashstop", null_hash);
	pl_size = compute_payload_size		(&payload);
	release_zone_ref					(&payload);
	
	tree_manager_set_child_value_i32(getblk_pack, "size", pl_size);
	tree_manager_set_child_value_i32(getblk_pack, "sent", 0);

	return 1;

}
OS_API_C_FUNC(int) create_inv_message(mem_zone_ref_ptr node, mem_zone_ref_ptr hash_list, mem_zone_ref_ptr inv_pack)
{
	mem_zone_ref		payload = { PTR_NULL };
	size_t				pl_size;
	int					cnt;

	if (!tree_manager_create_node("message", NODE_BITCORE_MSG, inv_pack))return 0;


	cnt = tree_manager_get_node_num_children(hash_list);

	tree_manager_set_child_value_str	(inv_pack, "cmd", "inv");
	tree_manager_add_child_node			(inv_pack, "payload", NODE_BITCORE_PAYLOAD, &payload);
	tree_manager_node_add_child			(&payload, hash_list);
	pl_size = compute_payload_size(&payload);
	release_zone_ref(&payload);
	tree_manager_get_node_num_children(hash_list);
	tree_manager_set_child_value_i32(inv_pack, "size", pl_size);
	tree_manager_set_child_value_i32(inv_pack, "sent", 0);

	return 1;

}
OS_API_C_FUNC(int) create_headers_message(mem_zone_ref_ptr header_hash_list,mem_zone_ref_ptr block_pack)
{
	mem_zone_ref		headers = { PTR_NULL },my_hash_list = { PTR_NULL }, payload = { PTR_NULL };
	mem_zone_ref_ptr	hash_node = PTR_NULL;
	size_t				pl_size;

	if (!tree_manager_create_node("message", NODE_BITCORE_MSG, block_pack))return 0;


	tree_manager_set_child_value_str(block_pack, "cmd", "headers");
	tree_manager_add_child_node		(block_pack, "payload", NODE_BITCORE_PAYLOAD, &payload);

	tree_manager_add_child_node		(&payload, "headers", NODE_BITCORE_BLK_HDR_LIST, &headers);

	for (tree_manager_get_first_child(header_hash_list, &my_hash_list, &hash_node); ((hash_node != PTR_NULL) && (hash_node->zone != PTR_NULL)); tree_manager_get_next_child(&my_hash_list, &hash_node))
	{
		hash_t			h;
		mem_zone_ref	hdr = { PTR_NULL };

		tree_manager_get_node_hash(hash_node, 0, h);
		if (tree_manager_add_child_node(&headers, "blk", NODE_BITCORE_BLK_HDR, &hdr))
		{
			tree_manager_set_child_value_hash	(&hdr, "blkHash",h);
			release_zone_ref					(&hdr);
		}
	}
	pl_size = compute_payload_size(&payload);

	
	release_zone_ref(&headers);
	release_zone_ref(&payload);
	
	tree_manager_set_child_value_i32(block_pack, "size", pl_size);
	tree_manager_set_child_value_i32(block_pack, "sent", 0);

	return 1;

}
OS_API_C_FUNC(int) create_block_message(mem_zone_ref_ptr node, mem_zone_ref_ptr header, mem_zone_ref_ptr tx_list, struct string *signature, mem_zone_ref_ptr block_pack)
{
	mem_zone_ref		payload = { PTR_NULL };
	size_t				pl_size;
	int					cnt;

	if (!tree_manager_create_node("message", NODE_BITCORE_MSG, block_pack))return 0;

	cnt = tree_manager_get_node_num_children(tx_list);

	tree_manager_set_child_value_str(block_pack, "cmd", "block");
	tree_manager_add_child_node(block_pack, "payload", NODE_BITCORE_PAYLOAD, &payload);
	tree_manager_node_add_child		 (&payload, header);
	tree_manager_node_add_child		 (&payload, tx_list);
	tree_manager_set_child_value_vstr(&payload, "signature", signature);

	pl_size = compute_payload_size(&payload);
	release_zone_ref(&payload);
	tree_manager_get_node_num_children(tx_list);
	tree_manager_set_child_value_i32(block_pack, "size", pl_size);
	tree_manager_set_child_value_i32(block_pack, "sent", 0);

	return 1;

}

OS_API_C_FUNC(int) create_file_message(mem_zone_ref_ptr node, mem_zone_ref_ptr file, mem_zone_ref_ptr file_pack)
{
	mem_zone_ref		payload = { PTR_NULL }, appNode = { PTR_NULL }, fileData = { PTR_NULL };
	size_t				pl_size;

	if (!tree_manager_find_child_node(file, NODE_HASH("appName"), NODE_BITCORE_VSTR, &appNode))return 0;
	if (!tree_manager_find_child_node(file, NODE_HASH("fileData"), NODE_BITCORE_VSTR, &fileData)){release_zone_ref(&appNode); return 0;}
	if (!tree_manager_create_node("message", NODE_BITCORE_MSG, file_pack)){ release_zone_ref(&fileData); release_zone_ref(&appNode); return 0; }

		
	tree_manager_set_child_value_str(file_pack, "cmd", "file");
	tree_manager_add_child_node		(file_pack, "payload", NODE_BITCORE_PAYLOAD, &payload);
	tree_manager_node_add_child		(&payload, &appNode);
	tree_manager_node_add_child		(&payload, &fileData);

	pl_size = compute_payload_size(&payload);
	release_zone_ref(&payload);
	release_zone_ref(&appNode);
	release_zone_ref(&fileData);

	tree_manager_set_child_value_i32(file_pack, "size", pl_size);
	tree_manager_set_child_value_i32(file_pack, "sent", 0);


	return 1;

}

OS_API_C_FUNC(int) create_tx_message(mem_zone_ref_ptr node, mem_zone_ref_ptr tx, mem_zone_ref_ptr tx_pack)
{
	mem_zone_ref		payload = { PTR_NULL };
	size_t				pl_size;

	if (!tree_manager_create_node("message", NODE_BITCORE_MSG, tx_pack))return 0;
	tree_manager_set_child_value_str(tx_pack, "cmd", "tx");
	if (tree_manager_add_child_node(tx_pack, "payload", NODE_BITCORE_PAYLOAD, &payload))
	{
		tree_manager_node_add_child(&payload, tx);
		pl_size = compute_payload_size(&payload);
		release_zone_ref(&payload);
	}
	tree_manager_set_child_value_i32(tx_pack, "size", pl_size);
	tree_manager_set_child_value_i32(tx_pack, "sent", 0);

	return 1;

}
OS_API_C_FUNC(int) create_getaddr_message(mem_zone_ref_ptr node, mem_zone_ref_ptr addr_pack)
{
	tree_manager_create_node("message", NODE_BITCORE_MSG, addr_pack);
	tree_manager_set_child_value_str(addr_pack, "cmd", "getaddr");
	tree_manager_set_child_value_i32(addr_pack, "size", 0);
	tree_manager_set_child_value_i32(addr_pack, "sent", 0);
	return 1;
}

OS_API_C_FUNC(int) create_ping_message(mem_zone_ref_ptr node, uint64_t nonce, mem_zone_ref_ptr ver_pack)
{
	mem_zone_ref		payload = { PTR_NULL };
	size_t				pl_size;

	tree_manager_create_node("message", NODE_BITCORE_MSG, ver_pack);
	tree_manager_set_child_value_str(ver_pack, "cmd", "ping");
	tree_manager_add_child_node(ver_pack, "payload", NODE_BITCORE_PAYLOAD, &payload);
	tree_manager_set_child_value_i64(&payload, "nonce", nonce);
	pl_size = compute_payload_size(&payload);
	release_zone_ref(&payload);
	tree_manager_set_child_value_i32(ver_pack, "size", pl_size);
	return 1;
}
OS_API_C_FUNC(int) create_pong_message(mem_zone_ref_ptr node, uint64_t nonce, mem_zone_ref_ptr ver_pack)
{
	mem_zone_ref		payload = { PTR_NULL };
	size_t				pl_size;
	
	tree_manager_create_node("message", NODE_BITCORE_MSG, ver_pack);
	tree_manager_set_child_value_str(ver_pack, "cmd", "pong");
	tree_manager_add_child_node		(ver_pack, "payload", NODE_BITCORE_PAYLOAD, &payload);
	tree_manager_set_child_value_i64(&payload, "nonce", nonce);
	pl_size = compute_payload_size(&payload);
	release_zone_ref(&payload);
	tree_manager_set_child_value_i32(ver_pack, "size", pl_size);
	return 1;
}
OS_API_C_FUNC(int) create_version_message(mem_zone_ref_ptr node, mem_zone_ref_ptr target_node_addr, mem_zone_ref_ptr ver_pack)
{
	struct string		user_agent = { PTR_NULL };
	mem_zone_ref		payload = { PTR_NULL };
	mem_zone_ref		self_addr = { PTR_NULL };
	uint64_t			nblks;
	uint64_t			services;
	size_t				pl_size;
	unsigned int		ver;
	int					ret;


	ret = tree_manager_get_child_value_istr			(node, NODE_HASH("user_agent"), &user_agent, 0);
	if (ret)ret = tree_manager_get_child_value_i32	(node, NODE_HASH_version, &ver);
	if (ret)ret = tree_manager_find_child_node		(node, NODE_HASH_p2p_addr, NODE_BITCORE_ADDR, &self_addr);
	if (ret)ret = tree_manager_get_child_value_i64  (&self_addr, NODE_HASH_services, &services);
	if (ret)ret = tree_manager_get_child_value_i64	(node, NODE_HASH("block_height"), &nblks);
	if (!ret)
	{
		free_string(&user_agent);
		release_zone_ref(&self_addr);
		return 0;
	}
	
	/*
	4	magic	uint32_t	Magic value indicating message origin network, and used to seek to next message when stream state is unknown
	12	command	char[12]	ASCII string identifying the packet content, NULL padded(non - NULL padding results in packet rejected)
	4	length	uint32_t	Length of payload in number of bytes
	4	checksum	uint32_t	First 4 bytes of sha256(sha256(payload))
	?   payload	uchar[]	The actual data
	*/
	tree_manager_create_node			("message", NODE_BITCORE_MSG, ver_pack);
	tree_manager_set_child_value_str	(ver_pack, "cmd", "version");

	/*
	4	version	int32_t	Identifies protocol version being used by the node
	8	services	uint64_t	bitfield of features to be enabled for this connection
	8	timestamp	int64_t	standard UNIX timestamp in seconds
	26	addr_recv	net_addr	The network address of the node receiving this message
	Fields below require version >= 106
	26	addr_from	net_addr	The network address of the node emitting this message
	8	nonce	uint64_t	Node random nonce, randomly generated every time a version packet is sent.This nonce is used to detect connections to self.
	?    user_agent	var_str	User Agent(0x00 if string is 0 bytes long)
	4	start_height	int32_t	The last block received by the emitting node
	*/
	if (tree_manager_add_child_node(ver_pack, "payload", NODE_BITCORE_PAYLOAD, &payload))
	{
		tree_manager_set_child_value_i32(&payload, "proto_ver", ver);
		tree_manager_set_child_value_i64(&payload, "services", services);
		tree_manager_set_child_value_i64(&payload, "timestamp", get_time_c());
		tree_manager_node_add_child(&payload, target_node_addr);
		tree_manager_node_add_child(&payload, &self_addr);
		tree_manager_set_child_value_i64(&payload, "nonce", ping_nonce++);
		tree_manager_set_child_value_vstr(&payload, "user_agent", &user_agent);
		tree_manager_set_child_value_i64(&payload, "last_blk", nblks );
		pl_size = compute_payload_size(&payload);
		release_zone_ref(&payload);
	}
	release_zone_ref(&self_addr);

	tree_manager_set_child_value_i32(ver_pack, "size", pl_size);

	return 1;
}

OS_API_C_FUNC(int) new_message(const struct string *data, mem_zone_ref_ptr msg)
{
	mem_zone_ref_ptr	key;
	mem_zone_ref		list = { PTR_NULL },my_list = { PTR_NULL };
	mem_zone_ref		payload_node = { PTR_NULL };
	struct string		pack_str = { PTR_NULL };
	size_t				cnt = 0;
	size_t				elSz = 0;
	unsigned int		elType = 0;
	int					ret,nc;

	if (!strncmp_c(&data->str[4], "version", 7))
		make_string(&pack_str, "{(\"payload\",0x0B000010) (0x02)\"proto_ver\" : 0,\"services\" : 0, \"timestamp\" : 0, (0x0B000040)\"their_addr\":\"\", (0x0B000040)\"my_addr\":\"\",\"nonce\":0,(0x0B000100)\"user_agent\":\"\", (0x02)\"last_blk\":0}");
	else if (!strncmp_c(&data->str[4], "ping", 4))
		make_string(&pack_str, "{(\"payload\",0x0B000010) \"nonce\":0}");
	else if (!strncmp_c(&data->str[4], "pong", 4))
		make_string(&pack_str, "{(\"payload\",0x0B000010) \"nonce\":0}");
	else if (!strncmp_c(&data->str[4], "addr", 4))
	{
		unsigned int    first, n;
		unsigned char	c;

		switch ((c = *((unsigned char *)(data->str + 24))))
		{
			default	 : cnt = c; break;
			case 0xFD: cnt = *((unsigned short *)(data->str + 24 + 1)); break;
			case 0xFE: cnt = *((unsigned int *)(data->str + 24 + 1)); break;
			case 0xFF: cnt = *((uint64_t *)(data->str + 24 + 1)); break;
		}

		elSz = 0;
		elType = 0;

		make_string(&pack_str, "{(\"payload\",0x0B000010) (0x0B000020)\"addrs\":[");
		first = 1;
		n = cnt;
		while (n--)
		{
			if (!first){ cat_cstring(&pack_str, ","); }
			first = 0;
			cat_cstring(&pack_str, "{(\"addr\",0x0B000080)}");
		}
		cat_cstring(&pack_str, "]}");

		cnt = 0;
	}
	else if (!strncmp_c(&data->str[4], "headers", 7))
	{
		unsigned char	c;

		switch ((c = *((unsigned char *)(data->str + 24))))
		{
			default: cnt = c; break;
			case 0xFD: cnt = *((unsigned short *)(data->str + 24 + 1)); break;
			case 0xFE: cnt = *((unsigned int *)(data->str + 24 + 1)); break;
			case 0xFF: cnt = *((uint64_t *)(data->str + 24 + 1)); break;
		}

		elSz   = 82; 
		elType = NODE_BITCORE_BLK_HDR;
		make_string(&pack_str, "{(\"payload\",0x0B000010) (0x0B000400)\"headers\":[]}");
	}
	else if (!strncmp_c(&data->str[4], "block", 5))
	{
		unsigned int    first, n;
		unsigned char	c;

		switch ((c = *((unsigned char *)(data->str + 24+80))))
		{
		default: cnt = c; break;
		case 0xFD: cnt = *((unsigned short *)(data->str + 24 + 80+1)); break;
		case 0xFE: cnt = *((unsigned int *)(data->str + 24 + 80 + 1)); break;
		case 0xFF: cnt = *((uint64_t *)(data->str + 24 + 80 + 1)); break;
		}

		make_string(&pack_str, "{(\"payload\",0x0B000010)  (0x0B000800)\"header\":\"\", (0x0B004000)\"txs\":[");
		first = 1;
		n = cnt;
		while (n--)
		{
			if (!first){ cat_cstring(&pack_str, ","); }
			cat_cstring(&pack_str, "{(\"tx\",0x0B008000)}");
			first = 0;
		}
		cat_cstring(&pack_str, "], (0x0B800000)\"signature\":\"\"}");
		cnt = 0;
	}
	else if (!strncmp_c(&data->str[4], "tx", 2))
	{
		make_string(&pack_str, "{(\"payload\",0x0B000010)  (0x0B008000)\"tx\":\"\"}");
		cnt = 0;
	}
	else if (!strncmp_c(&data->str[4], "file", 2))
	{
		make_string(&pack_str, "{(\"payload\",0x0B000010)  (0x0B000100)\"appName\":\"\", (0x0B000100)\"fileData\":\"\"}");
		cnt = 0;
	}
	else if (!strncmp_c(&data->str[4], "getdata", 7))
	{
		unsigned int    first, n;
		unsigned int	type;
		unsigned char	c;

		c = *((unsigned char *)(data->str + 24));

		if (c < 0xFD)
		{
			cnt = c;
			type = *(unsigned char *)(data->str +24+ 1);
		}
		else if (c == 0xFD)
		{
			cnt = *((unsigned short *)(data->str + 24 + 1));
			type = *(unsigned char *)(data->str + 24 + 3);
		}
		else if (c == 0xFE)
		{
			cnt = *((unsigned int *)(data->str + 24 + 1));
			type = *(unsigned char *)(data->str + 24 + 5);
		}
		else if (c == 0xFF)
		{
			cnt = *((uint64_t *)(data->str + 24 + 1));
			type = *(unsigned char *)(data->str + 24 + 9);
			
		}
		make_string(&pack_str, "{(\"payload\",0x0B000010) (0x0B003000)\"hashes\":[");
		first = 1;
		n = cnt;
		while (n--)
		{
			if (!first){ cat_cstring(&pack_str, ","); }

			if (type == GETDATA_BLOCK)
				cat_cstring(&pack_str, "{(\"hash\",0x0B005000)}");
			else if (type == GETDATA_TX)
				cat_cstring(&pack_str, "{(\"hash\",0x0B007000)}");
			else if (type == GETDATA_FILE)
				cat_cstring(&pack_str, "{(\"hash\",0x0A000080)}");
			first = 0;
		}
		cat_cstring(&pack_str, "]}");
		cnt = 0;
		elSz = 0;
	}
	else if (!strncmp_c(&data->str[4], "getblocks", 9))
	{
		unsigned int	version;
		unsigned int    first, n;
		unsigned char	c;

		version = *((unsigned int *)(data->str + 24));
		
		switch ((c = *((unsigned char *)(data->str + 24+4))))
		{
			default: cnt = c; break;
			case 0xFD: cnt = *((unsigned short *)(data->str + 24 + 5)); break;
			case 0xFE: cnt = *((unsigned int *)(data->str + 24 + 5)); break;
			case 0xFF: cnt = *((uint64_t *)(data->str + 24 + 5)); break;
		}

		make_string(&pack_str, "{(\"payload\",0x0B000010) (0x02)\"version\": 0, (0x0B003000)\"hashes\":[");
		first = 1;
		n = cnt;
		while (n--)
		{
			if (!first){ cat_cstring(&pack_str, ","); }

			cat_cstring(&pack_str, "{(\"hash\",0x0B001000)}");
			first = 0;
		}
		cat_cstring(&pack_str, "],(0x0B001000)\"stop\": 0}");
		cnt = 0;
		elSz = 0;
	}
	else if (!strncmp_c(&data->str[4], "inv", 3))
	{
		unsigned int    first, n;
		unsigned int	type;
		unsigned char	c;

		c = *((unsigned char *)(data->str + 24));

		if (c < 0xFD)
		{
			cnt = c;
			type = *(unsigned char *)(data->str +24+ 1);
		}
		else if (c == 0xFD)
		{
			cnt = *((unsigned short *)(data->str + 24 + 1));
			type = *(unsigned char *)(data->str + 24 + 3);
		}
		else if (c == 0xFE)
		{
			cnt = *((unsigned int *)(data->str + 24 + 1));
			type = *(unsigned char *)(data->str + 24 + 5);
		}
		else if (c == 0xFF)
		{
			cnt = *((uint64_t *)(data->str + 24 + 1));
			type = *(unsigned char *)(data->str + 24 + 9);

		}

		make_string(&pack_str, "{(\"payload\",0x0B000010) (0x0B003000)\"hashes\":[");
		first = 1;
		n = cnt;
		while (n--)
		{
			if (!first){ cat_cstring(&pack_str, ","); }
			if (type == GETDATA_TX)
				cat_cstring(&pack_str, "{(\"hash\",0x0B007000)}"); 
			else if (type == GETDATA_BLOCK)
				cat_cstring(&pack_str, "{(\"hash\",0x0B005000)}");
			else if (type == GETDATA_FILE)
				cat_cstring(&pack_str, "{(\"hash\",0x0A000080)}");
			
			first = 0;
		}
		cat_cstring(&pack_str, "]}");
		cnt	 = 0;
		elSz = 0;
	}
	else
	{
		tree_manager_set_child_value_str(msg, "cmd", &data->str[4]);
		tree_manager_set_child_value_i32(msg, "size", (*((unsigned int *)(&data->str[16]))));
		tree_manager_set_child_value_i32(msg, "sum", *((unsigned int *)(&data->str[20])));
		tree_manager_set_child_value_i32(msg, "cnt", cnt);
		tree_manager_set_child_value_i32(msg, "elSz", elSz);
		tree_manager_set_child_value_i32(msg, "elType", elType);
		tree_manager_node_add_child		(msg, &payload_node);
		return 1;
	}
		
	ret=tree_manager_json_loadb	(pack_str.str, pack_str.len, &payload_node);
	free_string					(&pack_str);

	if (!ret)return 0;

	if (elSz>0)
		tree_manager_get_child_at(&payload_node, 0, &list);
	else
		list.zone = payload_node.zone;

	nc = tree_manager_get_node_num_children(&list);
	if (nc > 0)
	{
		for (tree_manager_get_first_child(&list, &my_list, &key); ((key != NULL) && (key->zone != NULL)); tree_manager_get_next_child(&my_list, &key))
		{
			init_node(key);
		}
	}

	tree_manager_set_child_value_str (msg, "cmd"	, &data->str[4]);
	tree_manager_set_child_value_i32 (msg, "size", (*((unsigned int *)(&data->str[16]))));
	tree_manager_set_child_value_i32 (msg, "sum"	, *((unsigned int *)(&data->str[20])));
	tree_manager_set_child_value_i32 (msg, "cnt"	, cnt);
	tree_manager_set_child_value_si32(msg, "handled", -1);
	tree_manager_set_child_value_i32 (msg, "done"	, 0);
	tree_manager_set_child_value_i32 (msg, "elSz"	, elSz);
	tree_manager_set_child_value_i32 (msg, "elType"	, elType);
	tree_manager_node_add_child		 (msg, &payload_node);
	
	if (elSz)
		release_zone_ref(&list);

	release_zone_ref(&payload_node);
	

	return 1;
}
