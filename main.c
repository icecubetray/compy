#include <stdio.h>
#include <string.h>

#include "./tests.h"

#include "./linked-list.h"
#include "./linked-list-quicksort.h"




void
static print_ll(sc_ll_node_t *node) {
	do {
		if (node->frequency == 0) {
			break;
		}

		printf("%u: %u\n", node->frequency, node->value);
	} while (node = node->left);
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


	sc_ll_node_t *root = NULL, *last = NULL;
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


	sc_quicksort(freqs, 0, (sizeof(freqs) / sizeof(*freqs)));

	// Create a linked list with the least frequent nodes on the
	// left and the most frequent nodes on the right.
	for (i = (sizeof(freqs) / sizeof(*freqs)); i--;) {
		if (root == NULL) {
			root = sc_ll_node_alloc(freqs[i].qsvalue, (uint8_t)freqs[i].tag);
			last = root;
		} else {
			last->left = sc_ll_node_alloc_ex(freqs[i].qsvalue, (uint8_t)freqs[i].tag, SC_LL_LEAF, NULL, last);
			last = last->left;
		}
	}

	print_ll(root);

	sc_ll_node_t
		*leaf1 = NULL,
		*leaf2 = NULL,
		*nonleaf = NULL,
		*next = NULL;
	size_t freq;

	// The next step is to pick the last two leaf nodes and hang
	// them under a non-leaf node, until all nodes are under such
	// a root node. We do this to create a tree.

	// Iterate through the linked list to the right, beginning at
	// the left.
	for (;;) {
		leaf1 = last;
		if (leaf1 == NULL) {
			break;
		}

		leaf2 = last->right;
		if (leaf2 == NULL) {
			break;
		}

		next = leaf2->right;
		next->left = NULL;
		leaf1->left = leaf1->right = NULL;
		leaf2->left = leaf2->right = NULL;

		// Create a non-leaf node with the combined frequency of
		// both leaf nodes and the two leaf nodes hanging under
		// it.
		freq = (leaf1->frequency + leaf2->frequency);
		nonleaf = sc_ll_node_alloc_ex(
			freq,
			0,
			0,
			leaf1,
			leaf2
		);

		// Find a spot to put the non-leaf node back in the tree.
		do {
			if (freq <= next->frequency) {
				if (next->left != NULL) {

				}
			}
		} while (next = next->right);
	}

	print_ll(last);
}
