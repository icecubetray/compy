#ifndef __SC_LINKED_LIST_H
#define __SC_LINKED_LIST_H




#include "../core/platform.h"
#include "../core/result.h"

#include "./quicksort.h"




#define SC_LL_LEAF							0x0001
#define SC_LL_LEFT							0x0002
#define SC_LL_RIGHT							0x0004




typedef struct sc_ll_node sc_ll_node_t;
struct sc_ll_node {
	struct sc_ll_node *parent;
	struct sc_ll_node *left;
	struct sc_ll_node *right;

	sc_qs_t frequency;

	uint16_t flags;
	uint8_t value;
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
	 * \param flags The flags for the node.
	 * \return A pointer to the new linked-list node, or `NULL`
	 *         when an error occurs.
	 */
	sc_ll_node_t* sc_ll_node_alloc(const sc_qs_t frequency, const uint8_t value, const uint16_t flags);

	/*!
	 * \brief Frees a linked-list node and optionally its children.
	 * \param node The node to free.
	 * \param children_too Whether to recursively free its children.
	 * \return `#SC_E_NULL` if \p node is `NULL`, `#SC_E_SUCCESS` otherwise.
	 */
	sc_result_t sc_ll_node_free(sc_ll_node_t *const node, const unsigned int children_too);

#ifdef __cplusplus
}
#endif




#endif
