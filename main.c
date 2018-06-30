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
			return 10;
		}
	}


	const char __data[] = "abcabcabd";
	const char *data = __data;
	if (argc > 1) {
		data = argv[1];
	}


	sc_huffman_t huff;

	if (sc_huffman_init(&huff) != SC_E_SUCCESS) {
		perror("Huffman context initialization failed.");
		return 1;
	}

	const size_t datalen = strlen(data);
	if (datalen == 1 && *data == '-') {
		uint8_t buffer[512];
		int running = 1;
		size_t r;
		do {
			if ((r = fread(buffer, sizeof(*buffer), (sizeof(buffer) / sizeof(*buffer)), stdin)) == 0) {
				running = 0;
				break;
			}

			if (sc_huffman_process(&huff, buffer, r) != SC_E_SUCCESS) {
				perror("Huffman processing failed.");
				return 2;
			}
		} while (running == 1);
	} else {
		if (sc_huffman_process(&huff, data, datalen) != SC_E_SUCCESS) {
			perror("Huffman processing failed.");
			return 2;
		}
	}

	if (sc_huffman_tree_build(&huff) != SC_E_SUCCESS) {
		perror("Huffman tree build failed.");
		return 3;
	}

	if (sc_huffman_tree_print(&huff) != SC_E_SUCCESS) {
		perror("Huffman tree print failed.");
		return 4;
	}

	if (sc_huffman_clear(&huff) != SC_E_SUCCESS) {
		perror("Huffman context clearing failed.");
		return 5;
	}


	return 0;
}
