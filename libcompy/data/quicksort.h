#ifndef __SC_QUICKSORT_H
#define __SC_QUICKSORT_H




#include "../core/platform.h"

#include <stddef.h>




typedef enum sc_qs_mode {
	SC_QS_MODE_ASCENDING = 1,
	SC_QS_MODE_DESCENDING = 2
} sc_qs_mode_t;

typedef size_t sc_qs_t;

typedef struct sc_qs_pair {
	sc_qs_t qsvalue;
	sc_qs_t tag;
} sc_qs_pair_t;




#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief Quicksorts the specified \p array using the specified \p mode, starting at \p index and processing up to \p size elements.
	 * @param array The array to sort.
	 * @param index The index to start at.
	 * @param size The number of elements to sort.
	 * @param mode Whether to sort in an ascending manner or descending manner; `#SC_QS_MODE_ASCENDING` or `#SC_QS_MODE_DESCENDING`.
	 */
	void sc_quicksort(sc_qs_pair_t *const array, size_t index, const size_t size, const int mode);

#ifdef __cplusplus
}
#endif




#endif
