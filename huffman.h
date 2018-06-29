#ifndef __SC_HUFFMAN_H
#define __SC_HUFFMAN_H




#include "./types.h"

#include "./linked-list.h"
#include "./linked-list-quicksort.h"




typedef struct sc_huffman {
	sc_ll_node_t *tree_root;
	sc_ll_node_t *tree_lookup[256];
	sc_qs_t frequencies[256];
} sc_huffman_t;




#ifdef __cplusplus
extern "C" {
#endif

	sc_result_t sc_huffman_init(sc_huffman_t *const context);
	sc_result_t sc_huffman_clear(sc_huffman_t *const context);
	sc_result_t sc_huffman_process(sc_huffman_t *const restrict context, const void *const restrict data, const size_t size);
	sc_result_t sc_huffman_build_tree(sc_huffman_t *const context);

#ifdef __cplusplus
}
#endif




#endif
