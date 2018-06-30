#include "./huffman.h"

#include "./linked-list.h"
#include "./quicksort.h"

#include <stdio.h>
#include <string.h>




sc_result_t
sc_huffman_init(sc_huffman_t *const context) {
	if (context == NULL) {
		return SC_E_NULL;
	}

	memset(context, 0, sizeof(*context));

	return SC_E_SUCCESS;
}


sc_result_t
sc_huffman_clear(sc_huffman_t *const context) {
	if (context == NULL) {
		return SC_E_NULL;
	}

	sc_ll_node_free(context->tree_root, 1);

	memset(context, 0, sizeof(*context));

	return SC_E_SUCCESS;
}




sc_result_t
sc_huffman_process(sc_huffman_t *const restrict context, const void *const restrict data, const size_t size) {
	if (context == NULL || data == NULL) {
		return SC_E_NULL;
	}

	if (size == 0) {
		return SC_E_PARAM;
	}

	if (context->tree_root != NULL) {
		return SC_E_LOCKED;
	}


	const uint8_t *const data8 = ((const uint8_t *const)data);
	sc_qs_t *const freqs = context->frequencies;


	/* Use the value as an index to a counter registry, and for each value in the provided data increment its
	** frequency by as many times as it occurs. */

	register size_t i;
	for (i = size; i--;) {
		++freqs[data8[i]];
	}


	return 0;
}




sc_result_t
sc_huffman_build_tree(sc_huffman_t *const context) {
	if (context == NULL) {
		return SC_E_NULL;
	}

	if (context->tree_root != NULL) {
		return SC_E_LOCKED;
	}


	size_t n = 0;
	sc_qs_pair_t freqs_sorted[256]; {
		const sc_qs_t *const freqs = context->frequencies;
		register unsigned int i;

		/* Set the tag and sortable value of all pairs according to
 		** the measured frequencies. */
		for (i = 256; i--;) {
			freqs_sorted[i].tag = i;
			freqs_sorted[i].qsvalue = freqs[i];
		}

		sc_quicksort(freqs_sorted, 0, 256, SC_QS_MODE_ASCENDING);

		/* Determine the number of workable values. */
		for (i = 256; i-- && freqs_sorted[i].qsvalue > 0;) {
			++n;
		}
	}


	sc_qs_pair_t
		*const freqs = &freqs_sorted[((sizeof(freqs_sorted) / sizeof(*freqs_sorted)) - n)],
		*current = NULL;

	sc_ll_node_t
		*nonleaf = NULL,
		*left = NULL,
		*right = NULL,
		**tree_lookup = context->tree_lookup;

	size_t sn = 0;
	// FIXME: tree is currently as clever as a rock (excluding CPUs)
	while (sn < n) {
		/* First we determine what is left, and what is right. */

		current = &freqs[sn++];

		if (nonleaf != NULL) {
			left = nonleaf;
		} else {
			left = sc_ll_node_alloc(
				current->qsvalue,
				current->tag,
				SC_LL_LEAF
			);
			tree_lookup[current->tag] = left;

			/* Shift current to the next pair. */
			current = &freqs[sn++];
		}

		right = sc_ll_node_alloc(
			current->qsvalue,
			current->tag,
			SC_LL_LEAF
		);
		tree_lookup[current->tag] = right;


		/* Then we stick to our decision, and hang them under a non-leaf node. */

		left->flags |= SC_LL_LEFT;
		right->flags |= SC_LL_RIGHT;

		nonleaf = sc_ll_node_alloc_ex(
			(left->frequency + right->frequency),
			0,
			0,
			left, right
		);
		left->parent = right->parent = nonleaf;
	}


	context->tree_root = nonleaf;


	return SC_E_SUCCESS;
}


sc_result_t
sc_huffman_print_tree(sc_huffman_t *context) {
	if (context == NULL) {
		return SC_E_NULL;
	}


	const sc_ll_node_t
		*const *const tree_lookup = (const sc_ll_node_t *const *const)context->tree_lookup,
		*node = NULL;


	// TODO: sort by freq

	register unsigned int i, j;
	for (i = 0; i < 256; ++i) {
		if (tree_lookup[i] != NULL) {
			printf("% 3u (%u)\t", i, context->frequencies[i]);

			size_t n = 0;
			node = tree_lookup[i];
			while ((node = node->parent) != NULL) {
				++n;
			}

			char binbuf[n + 1];

			node = tree_lookup[i];
			for (j = n; j--;) {
				if (node == NULL) {
					break;
				}

				/* Check if we reached the root of the tree. */
				if (node->parent == NULL) {
					break;
				}

				if ((node->flags & SC_LL_LEFT) == SC_LL_LEFT) {
					binbuf[j] = '1';
				} else if ((node->flags & SC_LL_RIGHT) == SC_LL_RIGHT) {
					binbuf[j] = '0';
				} else {
					fputs("?", stdout);
				}

				node = node->parent;
			}

			binbuf[n] = '\0';
			puts(binbuf);
		}
	}


	return SC_E_SUCCESS;
}
