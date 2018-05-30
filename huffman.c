#include "./huffman.h"

#include "./linked-list-quicksort.h"

#include <stdlib.h>
#include <stdio.h> // remove




static
sc_ll_node_t *
__select_next_node(size_t *const index, sc_ll_node_t *const *const nodes, const size_t n_nodes) {
	size_t idx = *index;
	sc_ll_node_t *found = NULL;
	for (; idx < n_nodes; ++idx) {
		if (nodes[idx] == NULL) {
			continue;
		}

		if (nodes[idx]->frequency == 0) {
			continue;
		}

		found = nodes[idx];
		break;
	}

	*index = (idx + 1);
	return found;
}




int
__sc_huffman_build_tree_layer(sc_huffman_t *const context, sc_ll_node_t **out_root, sc_ll_node_t *const *const nodes, const size_t n_nodes) {
	if (nodes == NULL || out_root == NULL || n_nodes == 0) {
		return 1;
	}

	if (n_nodes == 1) {
		*out_root = nodes[0];
		return 0;
	}


	sc_ll_node_t **ws = malloc(0);
	size_t n_ws = 0;

	sc_ll_node_t
		*left = NULL,
		*right = NULL;

	size_t index;
	size_t i = 0;
	for (; i  < n_nodes;) {
		puts("");

		left = __select_next_node(&i, nodes, n_nodes);
		if (left == NULL) {
			// We're done.
			break;
		}

		right = __select_next_node(&i, nodes, n_nodes);
		if (right == NULL) {
			// left is orphaned
		}

		index = n_ws++;

		void *const ptr = realloc(
			ws,
			(sizeof(*ws) * n_ws)
		);
		if (ptr == NULL) {
			return 2;
		} else {
			ws = ptr;
		}

		if (right == NULL) {
			ws[index] = left;
			break;
		} else {
			ws[index] = sc_ll_node_alloc_ex(
				(left->frequency + right->frequency),
				0,
				0,
				left, right
			);
		}
	}

	sc_qs_pair_t qs[n_ws];
	for (i = n_ws; i--;) {
		qs[i].tag = i;
		qs[i].qsvalue = ws[i]->frequency;
	}
	sc_quicksort(qs, 0, n_ws);

	sc_ll_node_t *swap = NULL;
	for (i = n_ws; i--;) {
		swap = ws[qs[i].tag];
		ws[qs[i].tag] = ws[i];
		ws[i] = swap;
	}


	// TODO: free nodes if needed?

	return __sc_huffman_build_tree_layer(context, out_root, ws, n_ws);
}
