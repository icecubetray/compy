#include "./quicksort.h"




size_t
static sc_quicksort_find_pivot(sc_qs_pair_t *const array, size_t left, size_t right, const sc_qs_t cmp) {
	const size_t right_cpy = right;
	--left;

	sc_qs_pair_t swap;

	for (;;) {
		for (; array[++left].qsvalue < cmp;) {
			;
		}
		for (; right && array[--right].qsvalue > cmp;) {
			;
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
static sc_quicksort_recurse(sc_qs_pair_t *const array, size_t left, size_t right) {
	if (left < right) {
		size_t pivot = sc_quicksort_find_pivot(
			array,
			left,
			right,
			array[right].qsvalue
		);

		sc_quicksort_recurse(
			array,
			left,
			(pivot ? (pivot - 1) : 0)
		);

		sc_quicksort_recurse(
			array,
			(pivot + 1),
			right
		);
	}
}


void
sc_quicksort(sc_qs_pair_t *const array, size_t index, const size_t size) {
	sc_quicksort_recurse(
		array,
		index,
		((index + size) - 1)
	);
}
