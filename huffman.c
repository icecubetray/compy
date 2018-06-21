#include "./huffman.h"

#include "./linked-list.h"
#include "./linked-list-quicksort.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>




int sc_tree_layer(sc_ll_node_t **nodes, const size_t n_nodes);




int
sc_huffman_build_tree(const uint8_t *const data, const size_t length) {
#define FREQ_VARIATIONS		(UINT8_MAX + 1)
	if (data == NULL) {
		return 1;
	}

	if (length == 0) {
		return 2;
	}


	register size_t i;


	size_t n = 0;

	sc_qs_pair_t frequencies[FREQ_VARIATIONS];
	memset(frequencies, 0, sizeof(frequencies));

	for (i = FREQ_VARIATIONS; i--;) {
		frequencies[i].tag = i;
	}

	for (i = length; i--;) {
		++frequencies[data[i]].qsvalue;
	}

	sc_quicksort(frequencies, 0, FREQ_VARIATIONS, SC_QS_MODE_ASCENDING);

	for (i = FREQ_VARIATIONS; i-- && frequencies[i].qsvalue > 0;) {
		++n;
	}

	const size_t offset = (i + 1);


	sc_ll_node_t **nodes = malloc((sizeof(*nodes) * n)), next;

	for (i = 0; i < n; ++i) {
		sc_ll_node_t *prev = ((i > 0) ? nodes[i - 1] : NULL);
		nodes[i] = sc_ll_node_alloc_ex(
			frequencies[offset + i].qsvalue,
			frequencies[offset + i].tag,
			SC_LL_LEAF,
			prev,
			NULL,
			NULL,
			NULL
		);
	}

	sc_ll_node_t *last = nodes[i - 1];
	while (last->prev != NULL) {
		last->prev->next = last;
		last = last->prev;
	}


	return sc_tree_layer(nodes, n);
}




int
sc_tree_layer(sc_ll_node_t **nodes, const size_t n_nodes) {
	if (n_nodes < 2) {
		return 0;
	}

	size_t n = 0;

	/* Logic scope */ {
		sc_ll_node_t *last = nodes[0], *prev = NULL;
		while (last->left != NULL) {
			prev = last;
			last = last->left;
			++n;
		}
		printf("depth: %u\n", n);

		sc_ll_node_t *left = last;
		sc_ll_node_t *right = left->next;

		sc_ll_node_t *nonleaf = sc_ll_node_alloc_ex(
			(left->frequency + right->frequency),
			0,
			0,
			NULL,
			right->next,
			left,
			right
		);

		left->prev = NULL;
		if (right->next) {
			right->next->prev = nonleaf;
		}
		right->next = NULL;

		if (prev != NULL) {
			prev->left = nonleaf;
		}

		n = 0;
		last = nonleaf;
		while (last->next != NULL) {
			++n;
			last = last->next;
		}
	}

	return sc_tree_layer(nodes, n);
}
