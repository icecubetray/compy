#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./tests.h"

#include "./linked-list.h"
#include "./linked-list-quicksort.h"




void
__attribute__((nonnull))
static print_sc_node(const sc_ll_node_t *const node) {
	if (node->frequency == 0) {
		return;
	}

	printf("%u: %u (%c)\n", node->frequency, node->value, node->value);
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

	print_ll(last);

	sc_ll_node_t **stack = malloc(0);
	size_t n_stack = 0;

	sc_ll_node_t
		*cursor = last,
		*left = NULL,
		*right = NULL,
		*orphan = NULL;

	do {
		if (cursor->frequency == 0) {
			continue;
		}

		left = cursor;
		right = cursor->right;

		if (right == NULL) {
			orphan = left;
		} else {
			cursor = right;
		}

		size_t index = n_stack++;

		/* realloc() and friends return NULL when out of memory */
		void *ptr = realloc(stack, (sizeof(*stack) * n_stack));
		if (ptr == NULL) {
			fputs("Out of memory.\n", stderr);
			abort();
		}
		stack = ptr;

		/* If we end up with an orphan, it gets pushed to the stack as itself. It also
		** means we reached the end of the list. */
		if (orphan) {
			stack[index] = orphan;
			break;
		} else {
			/* No orphan, so both left and right get hung under a new node with their
			** combined frequency. */
			stack[index] = sc_ll_node_alloc_ex(left->frequency + right->frequency, 0, 0, left, right);
		}
	} while (cursor = cursor->right);

	// create qs pairs
	sc_qs_pair_t *stack_qs = malloc((sizeof(*stack_qs) * n_stack));
	if (stack_qs == NULL) {
		fputs("Out of memory (QS).\n", stderr);
		abort();
	}

	for (i = n_stack; i--;) {
		stack_qs[i].tag = i;
		stack_qs[i].qsvalue = stack[i]->frequency;
	}

	sc_quicksort(stack_qs, 0, n_stack);

	puts("");
	puts("QS");

	for (i = n_stack; i--;) {
		sc_ll_node_t *qsn = stack[stack_qs[i].tag];
		if (qsn->left != NULL && qsn->right != NULL) {
			printf("%u: %u=%c, %u=%c\n", qsn->frequency, qsn->left->frequency, qsn->left->value, qsn->right->frequency, qsn->right->value);
		} else {
			printf("%u: %c (orphan)\n", qsn->frequency, qsn->value);
		}
	}


	/* Currently the first row is listed, so we need to reconstruct a tree from the branch nodes and recurse into it
	** until there is only an orphan left. */
}
