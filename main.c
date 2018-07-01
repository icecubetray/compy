#include "./huffman.h"
#include "./file.h"

#include "./tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>




#define SC_OPTION_HELP						'h'
#define SC_OPTION_COMPRESS					'c'
#define SC_OPTION_DECOMPRESS				'd'
#define SC_OPTION_INPUT_FILE				'i'
#define SC_OPTION_OUTPUT_FILE				'o'




void
parse_options(int argc, char *argv[]) {
	const static struct option options_long[] = {
		{ "help",		no_argument,		NULL, SC_OPTION_HELP		},
		{ "compress",	no_argument,		NULL, SC_OPTION_COMPRESS	},
		{ "decompress",	no_argument,		NULL, SC_OPTION_DECOMPRESS	},
		{ "if",			required_argument,	NULL, SC_OPTION_INPUT_FILE	},
		{ "of",			required_argument,	NULL, SC_OPTION_OUTPUT_FILE	},
		{ NULL,			0,					NULL, 0						}
	};

	const static char options_short[] = "hcdi:o:";


	/* Reset getopt's parsing index, ya never know. */
	optind = 1;


	int opt_result, opt_index;
	do {
		opt_result = getopt_long(argc, argv, options_short, options_long, &opt_index);
		if (opt_result == -1) {
			break;
		} else {
			switch (opt_result) {
				case SC_OPTION_HELP:
					printf("Usage: %s <mode> [options]\n", argv[0]);
					puts(
						"\n"
						"Mode:\n"
						"    -c, --compress      Compression mode\n"
						"    -d, --decompress    Decompression mode\n"
						"\n"
						"Options:\n"
						"    -h, --help          Show this help message\n"
						"    -i, --if=<file>     Specify input file path, or - for stdin\n"
						"    -o, --of=<file>     Specify output file path, or - for stdout"
					);
					exit(0);
				case SC_OPTION_COMPRESS:
					break;
				case SC_OPTION_DECOMPRESS:
					break;
				case SC_OPTION_INPUT_FILE:
					break;
				case SC_OPTION_OUTPUT_FILE:
					break;
			}
		}
	} while (1);


	/* Reset getopt's parsing index once more, to be nice. */
	optind = 1;
}




int
main(int argc, char *argv[], char *env[]) {
	if (sc_run_tests() != 0) {
		return 10;
	}


	parse_options(argc, argv);


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


	sc_file_t file;

	if (sc_file_open(&file, "./test.sca", 1) != SC_E_SUCCESS) {
		perror("Failed to open file.");
		return 5;
	}

	if (sc_file_write_header(&file, &huff) != SC_E_SUCCESS) {
		perror("Failed to write header.");
		return 6;
	}

	if (sc_file_write_data(&file, data, datalen) != SC_E_SUCCESS) {
		perror("Failed to write data.");
		return 7;
	}

	if (sc_file_close(&file) != SC_E_SUCCESS) {
		perror("failed to close file.");
		return 8;
	}



	if (sc_huffman_clear(&huff) != SC_E_SUCCESS) {
		perror("Huffman context clearing failed.");
		return 16;
	}

	return 0;
}
