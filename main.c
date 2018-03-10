#include <stdio.h>
#include <string.h>

#include "./tests.h"

#include "./linked-list.h"
#include "./quicksort.h"




const char ord_indicators[][3] = {
	"st",
	"nd",
	"rd",
	"th"
};
int main(int argc, char *argv[], char *env[]) {
	{
		const struct {
			int(*func)();
			char name[16];
		} test_cases[] = {
			{
				.func = sc_test_quicksort,
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
			return 1;
		}
	}


	const char data[] = "abcabcabd";

	size_t
		i,
		len = strlen(data);


	sc_ll_node_t *root = NULL, *last = NULL;
	sc_qs_pair_t freqs[256];
	memset(freqs, 0, sizeof(freqs));


	for (i = len; i--;) {
		++freqs[data[i]].qsvalue;
	}
	for (i = 256; i--;) {
		freqs[i].tag = i;
	}


	sc_quicksort(freqs, 0, (sizeof(freqs) / sizeof(*freqs)));


	for (i = (sizeof(freqs) / sizeof(*freqs)); i--;) {
		if (root == NULL) {
			root = sc_ll_node_alloc(freqs[i].qsvalue, (uint8_t)freqs[i].tag);
			last = root;
		} else {
			last->left = sc_ll_node_alloc_ex(freqs[i].qsvalue, (uint8_t)freqs[i].tag, 0, NULL, last);
			last = last->left;
		}
	}


	last = root;
	do {
		if (last->frequency == 0) {
			break;
		}

		printf("%u: %u\n", last->frequency, last->value);
	} while (last = last->left);
}
