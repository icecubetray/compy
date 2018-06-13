#include "./huffman.h"

#include "./linked-list.h"
#include "./linked-list-quicksort.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>




int
sc_huffman_build_tree(const uint8_t *const data, const size_t length) {
#define FREQ_VARIATIONS		(UINT8_MAX + 1)
	if (data == NULL) {
		return 1;
	}

	if (length == 0) {
		return 2;
	}


	register size_t i;


	sc_qs_pair_t frequencies[FREQ_VARIATIONS];
	memset(frequencies, 0, sizeof(frequencies));

	for (i = FREQ_VARIATIONS; i--;) {
		frequencies[i].tag = i;
	}
	for (i = length; i--;) {
		++frequencies[data[i]].qsvalue;
	}

	sc_quicksort(frequencies, 0, FREQ_VARIATIONS, SC_QS_MODE_ASCENDING);


	// TODO: tree building logic


	return 100;
}
