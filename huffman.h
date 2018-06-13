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

	int sc_huffman_build_tree(const uint8_t *const data, const size_t length);

#ifdef __cplusplus
}
#endif




#endif
