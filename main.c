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

	size_t
		i,
		len = strlen(data);


	sc_qs_pair_t freqs[256];
	memset(freqs, 0, sizeof(freqs));

	// Count frequencies.
	for (i = len; i--;) {
		++freqs[data[i]].qsvalue;
	}

	// Tag what byte they're for.
	for (i = 256; i--;) {
		freqs[i].tag = i;
	}

	sc_quicksort(freqs, 0, 256);


	sc_ll_node_t **ws = malloc(0);
	size_t n_ws = 0, index;
	for (i = 256; i--;) {
		if (freqs[i].qsvalue < 1) {
			continue;
		}

		index = n_ws++;
		void *ptr = realloc(ws, (sizeof(*ws) * n_ws));
		if (ptr == NULL) {
			fputs("Out of memory.\n", stderr);
			abort();
		} else {
			ws = ptr;
		}

		ws[index] = sc_ll_node_alloc_ex(freqs[i].qsvalue, (uint8_t)freqs[i].tag, SC_LL_LEAF, NULL, NULL);

		printf("%u: %u %u\n", index, ws[index]->frequency, ws[index]->value);
	}


	sc_huffman_t context;
	memset(&context, 0, sizeof(context));

	sc_ll_node_t *root = NULL;
	if (__sc_huffman_build_tree_layer(&context, &root, ws, n_ws) == 0) {
		puts("okay");

		sc_ll_traverse_tree(root, print_sc_node);
	} else {
		puts("not okay");
	}


	/* Currently the first row is listed, so we need to reconstruct a tree from the branch nodes and recurse into it
	** until there is only an orphan left. */
}
