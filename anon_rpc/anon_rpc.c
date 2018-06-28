//copyright antoine bentue-ferrer 2016
#include <base/std_def.h>
#include <base/std_mem.h>
#include <base/mem_base.h>
#include <base/std_str.h>

#include <strs.h>
#include <tree.h>
#include <fsio.h>
#include <sha256.h>
#include <crypto.h>
#include <mem_stream.h>
#include <tpo_mod.h>

#include "../node_adx/node_api.h"
#include "../block_adx/block_api.h"
#include "../wallet/wallet_api.h"

mem_zone_ref			my_node = { PTR_INVALID };

OS_API_C_FUNC(int) set_node_anon_wallet(mem_zone_ref_ptr node, tpo_mod_file *pos_mod)
{
	my_node.zone = PTR_NULL;
	copy_zone_ref(&my_node, node);

	return 1;
}
OS_API_C_FUNC(int) dumpprivkey(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	dh_key_t	 key;
	struct string ekey = { 0 };
	btc_addr_t   pubaddr;
	mem_zone_ref param = { PTR_NULL };

	tree_manager_get_child_at		(params, 0, &param);
	tree_manager_get_node_btcaddr	(&param, 0, pubaddr);
	release_zone_ref				(&param);


	if (!get_anon_key(pubaddr, key))
		return 0;

	privkey_to_addr(key, &ekey);
	tree_manager_set_child_value_vstr(result, "privkey", &ekey);


	free_string(&ekey);

	return 1;

}
OS_API_C_FUNC(int) generatekeys(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	
	btc_addr_t   pubaddr;
	char         clabel[32];
	mem_zone_ref label_n = { PTR_NULL };
	int			 ret;


	memset_c					 (clabel, 0, 32);

	tree_manager_get_child_at	 (params, 0, &label_n);
	tree_manager_get_node_str	 (&label_n, 0, clabel, 32, 0);
	release_zone_ref			 (&label_n);

	ret = generate_new_keypair(clabel, pubaddr);
	if (ret)
		tree_manager_set_child_value_btcaddr(result, "address", pubaddr);
	

	return ret;
}

OS_API_C_FUNC(int) accesstest(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	tree_manager_create_node		("result", NODE_GFX_BOOL, result);
	tree_manager_write_node_dword	(result, 0, 1);


	return 1;
}


OS_API_C_FUNC(int) walletpassphrase(mem_zone_ref_const_ptr params, unsigned int rpc_mode, mem_zone_ref_ptr result)
{
	struct string	password = { 0 };
	mem_zone_ref	pn = { PTR_NULL };
	unsigned int	timeout;
	int				ret;


	if (tree_manager_get_node_num_children(params) < 2)return 0;

	tree_manager_get_child_at	(params, 0, &pn);
	tree_manager_get_node_istr	(&pn, 0, &password, 0);
	release_zone_ref			(&pn);

	tree_manager_get_child_at	(params, 1, &pn);
	tree_mamanger_get_node_dword(&pn, 0, &timeout);
	release_zone_ref			(&pn);

	ret=set_anon_pw				(password.str, timeout);

	free_string					(&password);

	
	return ret;
}