#ifndef __SC_HUFFMAN_H
#define __SC_HUFFMAN_H




#include "./types.h"

#include "./linked-list.h"




typedef struct sc_huffman {
	sc_ll_node_t *btree_root;
	sc_ll_node_t **__prev_layer;
	size_t __prev_layer_idx;
} sc_huffman_t;




#ifdef __cplusplus
extern "C" {
#endif

	int __sc_huffman_build_tree_layer(sc_huffman_t *const context, sc_ll_node_t **out_root, sc_ll_node_t *const *const nodes, const size_t n_nodes);

#ifdef __cplusplus
}
#endif




#endif
