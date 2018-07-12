#ifndef __SC_HUFFMAN_H
#define __SC_HUFFMAN_H




#include "../core/platform.h"
#include "../core/result.h"

#include "./linked_list.h"

#include <stdio.h>




typedef struct sc_huffman {
	sc_ll_node_t *tree_root;
	sc_ll_node_t *tree_lookup[256];
	sc_qs_t frequencies[256];
} sc_huffman_t;




#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief Initializes the specified huffman structure.
	 * @param context The huffman structure to initialize.
	 * @return
	 *		`#SC_E_NULL` when a `NULL` pointer is encountered
	 *		`#SC_E_SUCCESS` otherwise
	 */
	sc_result_t sc_huffman_init(sc_huffman_t *const context);

	/**
	 * @brief Clears the specified huffman instance.
	 * @param context The huffman instance to clear.
	 * @return
	 *		`#SC_E_NULL` when a `NULL` pointer is encountered
	 *		`#SC_E_SUCCESS` otherwise
	 */
	sc_result_t sc_huffman_clear(sc_huffman_t *const context);

	/**
	 * @brief Processes the specified data on the specified huffman instance in order to determine
	 *        character frequencies.
	 * @param context The huffman instance to work on.
	 * @param data The input data source.
	 * @param size The number of bytes to read from the input source.
	 * @return
	 *		`#SC_E_NULL` when a `NULL` pointer is encountered
	 *		`#SC_E_PARAM` when \p size is zero
	 *		`#SC_E_LOCKED` if a tree is already built
	 *		`#SC_E_SUCCESS` otherwise
	 */
	sc_result_t sc_huffman_process(sc_huffman_t *const restrict context, const void *const restrict data, const size_t size);

	/**
	 * @brief Builds a huffman tree for the specified huffman instance.
	 * @param context The huffman instance to work on.
	 * @return
	 *		`#SC_E_NULL` when a `NULL` pointer is encountered
	 *		`#SC_E_LOCKED` if a tree is already built
	 *		`#SC_E_SUCCESS` otherwise
	 */
	sc_result_t sc_huffman_tree_build(sc_huffman_t *const context);

	/**
	 * @brief Prints the tree of the specified huffman instance to the given file.
	 * @param context The huffman instance to work on.
	 * @param file The output file to write to.
	 * @return
	 *		`#SC_E_NULL` when a `NULL` pointer is encountered
	 *		`#SC_E_SUCCESS` otherwise
	 */
	sc_result_t sc_huffman_tree_print(sc_huffman_t *const restrict context, FILE *const restrict file);

#ifdef __cplusplus
}
#endif




#endif
