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


	sc_ll_node_t
		*nonleaf = NULL,
		*left = NULL,
		*right = NULL;

	size_t sn = 0;
	while (sn < n) {
		if (nonleaf != NULL) {
			left = nonleaf;
		} else {
			left = sc_ll_node_alloc_ex(frequencies[offset + sn].qsvalue, frequencies[offset + sn].tag, SC_LL_LEAF, NULL, NULL, NULL, NULL);
			++sn;
		}

		right = sc_ll_node_alloc_ex(frequencies[offset + sn].qsvalue, frequencies[offset + sn].tag, SC_LL_LEAF, NULL, NULL, NULL, NULL);
		++sn;

		nonleaf = sc_ll_node_alloc_ex(left->frequency + right->frequency, 0, 0, NULL, NULL, left, right);
	}


	return 0;
}
