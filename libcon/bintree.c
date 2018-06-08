#define LIBC_API C_EXPORT

#include "base/std_def.h"
#include "base/std_mem.h"
#include "base/mem_base.h"
#include <bin_tree.h>



int cmp_val(tree_entry e1, tree_entry e2)
{
	int n=0;

	while (n < 9)
	{
		if (e1[n] < e2[n])
			return -1;

		if (e1[n] > e2[n])
			return 1;
		n++;
	}

	return 0;
}

OS_API_C_FUNC(int) bt_insert(node ** tree, tree_entry val)
{
	node *temp = PTR_NULL;
	int cmp;
	if (!(*tree))
	{
		temp = (node *)malloc_c(sizeof(node));
		temp->left = temp->right = PTR_NULL;

		memcpy_c(&temp->data, val, sizeof(tree_entry));
		*tree = temp;
		return 1;
	}
	cmp = cmp_val(val, (*tree)->data);
	if (cmp < 0)
	{
		return bt_insert(&(*tree)->left, val);
	}
	else if (cmp > 0)
	{
		return bt_insert(&(*tree)->right, val);
	}
	return 0;
}



OS_API_C_FUNC(void) bt_deltree(node * tree)
{
	if (tree)
	{
		bt_deltree(tree->left);
		tree->left = PTR_NULL;
		bt_deltree(tree->right);
		tree->right = PTR_NULL;
		free_c(tree);
	}
}

OS_API_C_FUNC(node*) bt_search(node * tree, tree_entry val)
{
	int cmp;
	if (tree==PTR_NULL)return PTR_NULL;
	
	cmp = cmp_val(val, tree->data);

	if (cmp<0)
	{
		return bt_search(tree->left, val);
	}
	else if (cmp>0)
	{
		return bt_search(tree->right, val);
	}

	return tree;

}
