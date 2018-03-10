#include "./linked-list.h"

#include <stdlib.h>




sc_ll_node_t*
sc_ll_node_alloc_ex(const sc_qs_t frequency, const uint8_t value, const uint16_t flags, sc_ll_node_t *const left, sc_ll_node_t *const right) {
	sc_ll_node_t *node = (sc_ll_node_t*)malloc(sizeof(*node));
	if (node == NULL) {
		return NULL;
	}

	node->frequency = frequency;
	node->value = value;
	node->flags = flags;
	node->left = left;
	node->right = right;

	return node;
}
