#ifndef __COMPY_DATA_HUFFMAN_H
#define __COMPY_DATA_HUFFMAN_H




#include "../core/platform.h"
#include "../core/result.h"

#include "./linked_list.h"

#include <stdio.h>




typedef struct compy_huffman {
	compy_node_t *tree_root;
	compy_node_t *tree_lookup[256];
	compy_qsval_t frequencies[256];
} compy_huffman_t;




#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief Initializes the specified huffman structure.
	 * @param context The huffman structure to initialize.
	 * @return
	 *		`#COMPY_E_NULL` when a `NULL` pointer is encountered
	 *		`#COMPY_E_SUCCESS` otherwise
	 */
	compy_result_t compy_huffman_init(compy_huffman_t *const context);

	/**
	 * @brief Clears the specified huffman instance.
	 * @param context The huffman instance to clear.
	 * @return
	 *		`#COMPY_E_NULL` when a `NULL` pointer is encountered
	 *		`#COMPY_E_SUCCESS` otherwise
	 */
	compy_result_t compy_huffman_clear(compy_huffman_t *const context);

	/**
	 * @brief Processes the specified data on the specified huffman instance in order to determine
	 *        character frequencies.
	 * @param context The huffman instance to work on.
	 * @param data The input data source.
	 * @param size The number of bytes to read from the input source.
	 * @return
	 *		`#COMPY_E_NULL` when a `NULL` pointer is encountered
	 *		`#COMPY_E_PARAM` when \p size is zero
	 *		`#COMPY_E_LOCKED` if a tree is already built
	 *		`#COMPY_E_SUCCESS` otherwise
	 */
	compy_result_t compy_huffman_process(compy_huffman_t *const restrict context, const void *const restrict data, const size_t size);

	/**
	 * @brief Builds a huffman tree for the specified huffman instance.
	 * @param context The huffman instance to work on.
	 * @return
	 *		`#COMPY_E_NULL` when a `NULL` pointer is encountered
	 *		`#COMPY_E_LOCKED` if a tree is already built
	 *		`#COMPY_E_SUCCESS` otherwise
	 */
	compy_result_t compy_huffman_tree_build(compy_huffman_t *const context);

	/**
	 * @brief Prints the tree of the specified huffman instance to the given file.
	 * @param context The huffman instance to work on.
	 * @param file The output file to write to.
	 * @return
	 *		`#COMPY_E_NULL` when a `NULL` pointer is encountered
	 *		`#COMPY_E_SUCCESS` otherwise
	 */
	compy_result_t compy_huffman_tree_print(compy_huffman_t *const restrict context, FILE *const restrict file);

#ifdef __cplusplus
}
#endif




#endif
