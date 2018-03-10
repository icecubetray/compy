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

	sc_ll_node_t* sc_ll_node_alloc_ex(const sc_qs_t frequency, const uint8_t value, const uint16_t flags, sc_ll_node_t *const left, sc_ll_node_t *const right);

	inline static sc_ll_node_t* sc_ll_node_alloc(const sc_qs_t frequency, const uint8_t value)  {
		return sc_ll_node_alloc_ex(
			frequency,
			value,
			0,
			NULL,
			NULL
		);
	}

#ifdef __cplusplus
}
#endif




#endif
