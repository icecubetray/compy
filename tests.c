#include "./tests.h"

#include <stdio.h>
#include <string.h>

#include "./linked-list-quicksort.h"




void
static print_array(const sc_qs_pair_t *const array, const size_t size) {
	size_t i;
	for (i = 0; i < size; ++i) {
		printf("%u\n", array[i].qsvalue);
	}
	puts("");
}




int
sc_test_quicksort() {
	sc_qs_pair_t array[] = {
		{ .qsvalue = 134, .tag = 0 },
		{ .qsvalue = 3541, .tag = 0 },
		{ .qsvalue = 123, .tag = 0 },
		{ .qsvalue = 45534, .tag = 0 },
		{ .qsvalue = 1235, .tag = 0 },
		{ .qsvalue = 56, .tag = 0 },
		{ .qsvalue = 3253, .tag = 0 },
		{ .qsvalue = 323, .tag = 0 },
		{ .qsvalue = 76, .tag = 0 },
		{ .qsvalue = 5767, .tag = 0 }
	};
	sc_qs_pair_t expect[] = {
		{ .qsvalue = 56, .tag = 0 },
		{ .qsvalue = 76, .tag = 0 },
		{ .qsvalue = 123, .tag = 0 },
		{ .qsvalue = 134, .tag = 0 },
		{ .qsvalue = 323, .tag = 0 },
		{ .qsvalue = 1235, .tag = 0 },
		{ .qsvalue = 3253, .tag = 0 },
		{ .qsvalue = 3541, .tag = 0 },
		{ .qsvalue = 5767, .tag = 0 },
		{ .qsvalue = 45534, .tag = 0 }
	};
	const size_t size = (sizeof(array) / sizeof(*array));

	const int cond_1 = (memcmp(array, expect, sizeof(array)) != 0);
	//print_array(array, size);
	sc_quicksort(array, 0, size);
	//print_array(array, size);
	const int cond_2 = (memcmp(array, expect, sizeof(array)) == 0);

	return !(cond_1 && cond_2);
}
