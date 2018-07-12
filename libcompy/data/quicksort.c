#include "./quicksort.h"




size_t
static
__quicksort_find_pivot(compy_quicksortable_t *const array, size_t left, size_t right, const compy_qsval_t cmp, const int ascending) {
	const size_t right_cpy = right;

	--left;

	compy_quicksortable_t swap;

	for (;;) {
		if (ascending) {
			for (; array[++left].qsvalue < cmp;) {
				;
			}
			for (; right && array[--right].qsvalue > cmp;) {
				;
			}
		} else {
			for (; array[++left].qsvalue > cmp;) {
				;
			}
			for (; right && array[--right].qsvalue < cmp;) {
				;
			}
		}

		if (left >= right) {
			break;
		}


		swap = array[left];
		array[left] = array[right];
		array[right] = swap;
	}

	swap = array[left];
	array[left] = array[right_cpy];
	array[right_cpy] = swap;

	return left;
}


void
static
__quicksort_recurse(compy_quicksortable_t *const array, size_t left, size_t right, const int ascending) {
	if (left < right) {
		size_t pivot = __quicksort_find_pivot(
			array,
			left,
			right,
			array[right].qsvalue,
			ascending
		);

		__quicksort_recurse(
			array,
			left,
			(pivot ? (pivot - 1) : 0),
			ascending
		);

		__quicksort_recurse(
			array,
			(pivot + 1),
			right,
			ascending
		);
	}
}


void
compy_quicksort(compy_quicksortable_t *const array, size_t index, const size_t size, const compy_qs_mode_t mode) {
	__quicksort_recurse(
		array,
		index,
		((index + size) - 1),
		(mode == COMPY_QS_MODE_ASCENDING)
	);
}
