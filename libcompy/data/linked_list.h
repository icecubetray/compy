#ifndef __LIBCOMPY_DATA_LINKED_LIST_H
#define __LIBCOMPY_DATA_LINKED_LIST_H




#include "../core/platform.h"
#include "../core/result.h"

#include "./quicksort.h"




#define COMPY_NODE_LEAF						0x0001
#define COMPY_NODE_LEFT						0x0002
#define COMPY_NODE_RIGHT					0x0004




typedef struct compy_node compy_node_t;

struct compy_node {
	struct compy_node *parent;
	struct compy_node *left;
	struct compy_node *right;

	compy_qsval_t frequency;

	uint16_t flags;
	uint8_t value;
};


typedef struct compy_linked_node compy_linked_node_t;

struct compy_linked_node {
	struct compy_node *node;
	struct compy_linked_node *previous;
	struct compy_linked_node *next;
};




#ifdef __cplusplus
extern "C" {
#endif

	/*!
	 * \brief Allocates a compy node.
	 * \param frequency The frequency of the leaf, or combined frequency of the non-leaf.
	 * \param value The value for the node.
	 * \param flags The flags for the node.
	 * \param left The left-hand node of the new node.
	 * \param right The right-hand node of the new node.
	 * \return A pointer to the new linked-list node, or `NULL`
	 *         when an error occurs.
	 */
	compy_node_t* compy_node_alloc_ex(const compy_qsval_t frequency, const uint8_t value, const uint16_t flags, compy_node_t *const left, compy_node_t *const right);

	/*!
	 * \brief Allocates a compy node.
	 * \param frequency The frequency of the leaf, or combined frequency of the non-leaf.
	 * \param value The value for the node.
	 * \param flags The flags for the node.
	 * \return A pointer to the new linked-list node, or `NULL`
	 *         when an error occurs.
	 */
	compy_node_t* compy_node_alloc(const compy_qsval_t frequency, const uint8_t value, const uint16_t flags);

	/*!
	 * \brief Frees a compy node and optionally its children.
	 * \param node The node to free.
	 * \param children_too Whether to recursively free its children.
	 * \return
	 *		`#COMPY_E_NULL` when a `NULL` pointer is encountered
	 *		`#COMPY_E_SUCCESS` otherwise
	 */
	compy_result_t compy_node_free(compy_node_t *const node, const unsigned int children_too);

#ifdef __cplusplus
}
#endif




#endif
