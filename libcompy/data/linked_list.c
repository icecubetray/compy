#include "./linked_list.h"

#include <stdlib.h>
#include <stdio.h>




static size_t __ctr_alloc = 0;




compy_node_t*
compy_node_alloc_ex(const compy_qsval_t frequency, const uint8_t value, const uint16_t flags, compy_node_t *const left, compy_node_t *const right) {
	compy_node_t *node = (compy_node_t*)malloc(sizeof(*node));

	if (node == NULL) {
		return NULL;
	}


	/* Failsafe in case we're running amok. */
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


compy_node_t *
compy_node_alloc(const compy_qsval_t frequency, const uint8_t value, const uint16_t flags)  {
	return compy_node_alloc_ex(
		frequency,
		value,
		flags,
		NULL,
		NULL
	);
}




compy_result_t
compy_node_free(compy_node_t *const node, const unsigned int children_too) {
	if (node == NULL) {
		return COMPY_E_NULL;
	}


	if (children_too) {
		/* Recurse left child. */
		if (compy_node_free(node->left, children_too) == COMPY_E_SUCCESS) {
			node->left = NULL;
		}

		/* Recurse right child. */
		if (compy_node_free(node->right, children_too) == COMPY_E_SUCCESS) {
			node->right = NULL;
		}
	}

	free(node);
	--__ctr_alloc;


	return COMPY_E_SUCCESS;
}
