#include "./tests.h"

#include "../data/quicksort.h"

#include <stdio.h>
#include <string.h>




int
compy_test_quicksort() {
	compy_quicksortable_t test_data[] = {
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
	const size_t size = (sizeof(test_data) / sizeof(*test_data));
	compy_quicksortable_t array[size];

	int cond_asc;
	{
		memcpy(array, test_data, sizeof(test_data));
		compy_quicksortable_t expect[] = {
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

		const int cond_asc_1 = (memcmp(array, expect, sizeof(array)) != 0);
		//print_array(array, size);
		compy_quicksort(array, 0, size, COMPY_QS_MODE_ASCENDING);
		//print_array(array, size);
		const int cond_asc_2 = (memcmp(array, expect, sizeof(array)) == 0);

		cond_asc = (cond_asc_1 && cond_asc_2);
	}

	int cond_desc;
	{
		memcpy(array, test_data, sizeof(test_data));
		compy_quicksortable_t expect[] = {
			{ .qsvalue = 45534, .tag = 0 },
			{ .qsvalue = 5767, .tag = 0 },
			{ .qsvalue = 3541, .tag = 0 },
			{ .qsvalue = 3253, .tag = 0 },
			{ .qsvalue = 1235, .tag = 0 },
			{ .qsvalue = 323, .tag = 0 },
			{ .qsvalue = 134, .tag = 0 },
			{ .qsvalue = 123, .tag = 0 },
			{ .qsvalue = 76, .tag = 0 },
			{ .qsvalue = 56, .tag = 0 }
		};

		const int cond_desc_1 = (memcmp(array, expect, sizeof(array)) != 0);
		//print_array(array, size);
		compy_quicksort(array, 0, size, COMPY_QS_MODE_DESCENDING);
		//print_array(array, size);
		const int cond_desc_2 = (memcmp(array, expect, sizeof(array)) == 0);

		cond_desc = (cond_desc_1 && cond_desc_2);
	}

	return !(cond_asc && cond_desc);
}




const char ord_indicators[][3] = {
	"st",
	"nd",
	"rd",
	"th"
};

int
compy_run_tests() {
	const struct {
		int(*func)();
		char name[16];
	} test_cases[] = {
		{
			.func = compy_test_quicksort,
			.name = "quicksort"
		}
	};

	int failed = 0;

	size_t tcase = (sizeof(test_cases) / sizeof(*test_cases));
	for (; tcase--;) {
		if (test_cases[tcase].func() != 0) {
			++failed;
			printf(
				"%u%s "
				"failed test: %s\n",
				failed, ((failed < 4) ? ord_indicators[failed - 1] : ord_indicators[3]),
				test_cases[tcase].name
			);
		}
	}

	if (failed > 0) {
		puts("tests failed");
		return failed;
	}

	return 0;
}
