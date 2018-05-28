#ifndef __SC_LINKED_LIST_H
#define __SC_LINKED_LIST_H




#include "./types.h"




#define SC_LL_LEAF							0x0001




typedef struct sc_ll_node sc_ll_node_t;
struct sc_ll_node {
	struct sc_ll_node *left;
	struct sc_ll_node *right;

	sc_qs_t frequency;

	uint16_t flags;
	uint8_t value;

	uint8_t __pad;
};




#ifdef __cplusplus
extern "C" {
#endif

	/*!
	 * \brief Allocates a linked-list node.
	 * \param frequency
	 * \param value The value for the node.
	 * \param flags The flags for the node.
	 * \param left The left-hand node of the new node.
	 * \param right The right-hand node of the new node.
	 * \return A pointer to the new linked-list node, or `NULL`
	 *         when an error occurs.
	 */
	sc_ll_node_t* sc_ll_node_alloc_ex(const sc_qs_t frequency, const uint8_t value, const uint16_t flags, sc_ll_node_t *const left, sc_ll_node_t *const right);

	/*!
	 * \brief Allocates a linked-list node.
	 * \param frequency
	 * \param value The value for the node.
	 * \return A pointer to the new linked-list node, or `NULL`
	 *         when an error occurs.
	 */
	inline static sc_ll_node_t* sc_ll_node_alloc(const sc_qs_t frequency, const uint8_t value)  {
		return sc_ll_node_alloc_ex(
			frequency,
			value,
			0,
			NULL,
			NULL
		);
	}

	/*!
	 * \brief Searches for the end of the linked-list using the
	 *        given node.
	 * \param root A node in the linked-list from which to start
	 *             searching for the end of the list.
	 * \return A pointer to the last node in the list, or `NULL`
	 *         when an error occurs.
	 */
	sc_ll_node_t *sc_ll_find_end(sc_ll_node_t *root);

	/*!
	 * \brief Pushes `node` to the end of `root`.
	 *
	 * \param root A node in the linked-list from which to start
	 *	           searching for the end of the list.
	 * \param node The node to append to the end of the list.
	 */
	int sc_ll_node_push(sc_ll_node_t *root, sc_ll_node_t *node);

	/*!
	 * \brief Swaps the given nodes around.
	 *
	 * \param node1 The first candidate node for the swap.
	 * \param node2 The second candidate node for the swap.
	 */
	int sc_ll_node_swap(sc_ll_node_t *node1, sc_ll_node_t *node2);

	/*!
	 * \brief Traverses a linked list starting at the node specified by \p start.
	 *
	 * \param start The node to start traversing from.
	 * \param nodefun The function to call for each node.
	 * \param direction The direction to traverse.
	 *
	 * \return `0` on success, `1` on a `NULL` pointer, `2` on an invalid direction.
	 */
	int sc_ll_traverse(sc_ll_node_t *start, void(*nodefun)(const sc_ll_node_t *node), const sc_direction_t direction);

#ifdef __cplusplus
}
#endif




#endif
