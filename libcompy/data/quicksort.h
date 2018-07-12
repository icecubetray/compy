#ifndef __COMPY_DATA_QUICKSORT_H
#define __COMPY_DATA_QUICKSORT_H




#include "../core/platform.h"

#include <stdint.h>
#include <stddef.h>




typedef enum compy_qs_mode {
	COMPY_QS_MODE_ASCENDING = 1,
	COMPY_QS_MODE_DESCENDING = 2
} compy_qs_mode_t;

typedef uint64_t compy_qsval_t;

typedef struct compy_quicksortable {
	compy_qsval_t qsvalue;
	compy_qsval_t tag;
} compy_quicksortable_t;




#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief Quicksorts the specified \p array using the specified \p mode, starting at \p index and processing up to \p size elements.
	 * @param array The array to sort.
	 * @param index The index to start at.
	 * @param size The number of elements to sort.
	 * @param mode Whether to sort in an ascending manner or descending manner; `#COMPY_QS_MODE_ASCENDING` or `#COMPY_QS_MODE_DESCENDING`.
	 */
	void compy_quicksort(compy_quicksortable_t *const array, size_t index, const size_t size, const compy_qs_mode_t mode);

#ifdef __cplusplus
}
#endif




#endif
