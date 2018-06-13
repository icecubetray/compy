#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./tests.h"

#include "./huffman.h"
#include "./linked-list.h"
#include "./linked-list-quicksort.h"




void
__attribute__((nonnull))
static print_sc_node(const sc_ll_node_t *const node) {
	printf("%p: F=%03u V=%03u L=%p R=%p\n", node, node->frequency, node->value, node->left, node->right);
}


void
static print_ll(sc_ll_node_t *node) {
	/*do {
		if (node->frequency == 0) {
			break;
		}

		printf("%u: %u\n", node->frequency, node->value);
	} while (node = node->left);*/
	sc_ll_traverse(node, print_sc_node, SC_DIRECTION_BACKWARD);
}




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


	const char __data[] = "abcabcabd";
	const char *data = __data;
	if (argc > 1) {
		data = argv[1];
	}

	return sc_huffman_build_tree(data, strlen(data));
}
