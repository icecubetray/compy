#include "./huffman.h"

#include "./tests.h"

#include <stdio.h>
#include <string.h>




const char ord_indicators[][3] = {
	"st",
	"nd",
	"rd",
	"th"
};

int
main(int argc, char *argv[], char *env[]) {
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


	const char __data[] = "abcabcabd";
	const char *data = __data;
	if (argc > 1) {
		data = argv[1];
	}


	sc_huffman_t huff;

	sc_huffman_init(&huff);
	sc_huffman_process(&huff, data, strlen(data));
	sc_huffman_build_tree(&huff);
	sc_huffman_clear(&huff);


	return 0;
}
