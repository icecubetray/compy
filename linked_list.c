#include "./linked_list.h"

#include <stdlib.h>
#include <stdio.h>




static size_t __ctr_alloc = 0;




sc_ll_node_t*
sc_ll_node_alloc_ex(const sc_qs_t frequency, const uint8_t value, const uint16_t flags, sc_ll_node_t *const left, sc_ll_node_t *const right) {
	sc_ll_node_t *node = (sc_ll_node_t*)malloc(sizeof(*node));

	if (node == NULL) {
		return NULL;
	}


	if (++__ctr_alloc > 2000) {
		fputs("hold ya horses.\n", stderr);
		abort();
	}


	node->parent = NULL;

	node->left = left;
	node->right = right;

	node->frequency = frequency;
	node->flags = flags;
	node->value = value;


	return node;
}


sc_ll_node_t *
sc_ll_node_alloc(const sc_qs_t frequency, const uint8_t value, const uint16_t flags)  {
	return sc_ll_node_alloc_ex(
		frequency,
		value,
		flags,
		NULL,
		NULL
	);
}




sc_result_t
sc_ll_node_free(sc_ll_node_t *const node, const unsigned int children_too) {
	if (node == NULL) {
		return SC_E_NULL;
	}

	if (children_too) {
		if (sc_ll_node_free(node->left, children_too) == SC_E_SUCCESS) {
			node->left = NULL;
		}
		if (sc_ll_node_free(node->right, children_too) == SC_E_SUCCESS) {
			node->right = NULL;
		}
	}

	free(node);

	return SC_E_SUCCESS;
}
