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


sc_ll_node_t *
sc_ll_find_end(sc_ll_node_t *root) {
	if (root == NULL) {
		return NULL;
	}

	sc_ll_node_t *end = root;
	for (;;) {
		if (end->right == NULL) {
			break;
		}

		end = end->right;
	}

	return end;
}


int
sc_ll_node_push(sc_ll_node_t *root, sc_ll_node_t *node) {
	sc_ll_node_t *end = sc_ll_find_end(root);
	if (end == NULL) {
		return 1;
	}

	end->right = node;
	node->left = end;

	return 0;
}


int
sc_ll_node_swap(sc_ll_node_t *node1, sc_ll_node_t *node2) {
	if (node1 == NULL || node2 == NULL) {
		return 1;
	}

	sc_ll_node_t *swap;

	swap = node1->left;
	node1->left = node2->left;
	node2->left = swap;

	swap = node1->right;
	node1->right = node2->right;
	node2->right = swap;

	return 0;
}




int
sc_ll_traverse(sc_ll_node_t *start, void(*nodefun)(const sc_ll_node_t *node), const sc_direction_t direction) {
	if (start == NULL || nodefun == NULL) {
		return 1;
	}

	sc_ll_node_t *cursor = start;
	do {
		nodefun(cursor);

		switch (direction) {
			case SC_DIRECTION_FORWARD:
				cursor = cursor->left;
				break;
			case SC_DIRECTION_BACKWARD:
				cursor = cursor->right;
				break;
			default:
				return 2;
		}
	} while (cursor != NULL);
}




int
sc_ll_traverse_tree(sc_ll_node_t *start, void(*nodefun)(const sc_ll_node_t *node)) {
	if (start == NULL || nodefun == NULL) {
		return 1;
	}

	nodefun(start);

	if (start->left != NULL) {
		sc_ll_traverse_tree(start->left, nodefun);
	}

	if (start->right != NULL) {
		sc_ll_traverse_tree(start->right, nodefun);
	}

	return 0;
}
