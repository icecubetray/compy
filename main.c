#include "./huffman.h"
#include "./file.h"

#include "./tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>




#define DEBUG								1




#define SC_OPTION_HELP						'h'
#define SC_OPTION_COMPRESS					'c'
#define SC_OPTION_DECOMPRESS				'd'
#define SC_OPTION_INPUT_FILE				'i'
#define SC_OPTION_OUTPUT_FILE				'o'




typedef enum sc_mode {
	SC_MODE_INIT = 0,
	SC_MODE_COMPRESS = 1,
	SC_MODE_DECOMPRESS = 2
} sc_mode_t;




void
parse_options(int argc, char *argv[], sc_mode_t *const out_mode, char **const out_input_file, char **const out_output_file) {
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


	sc_mode_t mode = SC_MODE_INIT;
	char *input_file = NULL, *output_file = NULL;

	int opt_result, opt_index;
	do {
		opt_result = getopt_long(argc, argv, options_short, options_long, &opt_index);
		if (opt_result == -1) {
			break;
		} else {
			switch (opt_result) {
				case SC_OPTION_HELP:
					printf("Usage: %s <mode> <input_file> [options]\n", argv[0]);
					puts(
						"\n"
						"Mode:\n"
						"    -c, --compress      Compression mode\n"
						"    -d, --decompress    Decompression mode\n"
						"\n"
						"Options:\n"
						"    -h, --help          Show this help message\n"
						"    -i, --if=<file>     Specify input file path\n"
						"    -o, --of=<file>     Specify output file path"
					);
					exit(EXIT_SUCCESS);
				case SC_OPTION_COMPRESS:
					mode = SC_MODE_COMPRESS;
					break;
				case SC_OPTION_DECOMPRESS:
					mode = SC_MODE_DECOMPRESS;
					break;
				case SC_OPTION_INPUT_FILE:
					input_file = optarg;
					break;
				case SC_OPTION_OUTPUT_FILE:
					output_file = optarg;
					break;
			}
		}
	} while (1);


	/* Reset getopt's parsing index once more, to be nice. */
	optind = 1;


	*out_mode = mode;
	*out_input_file = input_file;
	*out_output_file = output_file;
}




int
main(int argc, char *argv[], char *env[]) {
	if (sc_run_tests() != 0) {
		return 10;
	}




	sc_mode_t mode;
	char *input_file, *output_file, *alloc = NULL;

	parse_options(argc, argv, &mode, &input_file, &output_file);


	if (mode == SC_MODE_INIT) {
		fputs("No mode specified.\n", stderr);
		exit(EXIT_FAILURE);
	}

	if (input_file == NULL) {
		fputs("No input file specified.\n", stderr);
		exit(EXIT_FAILURE);
	}

	if (output_file == NULL) {
		const size_t iflen = strlen(input_file);

		output_file = alloc = malloc(iflen + 4 + 1);
		memcpy(output_file, input_file, iflen);
		strncpy(output_file + iflen, ".sca\0", 4 + 1);
	}


	FILE *fp = fopen(input_file, "r");
	if (fp == NULL) {
		perror("fopen()");
		exit(EXIT_FAILURE);
	}




	uint8_t buffer[512];
	int running;
	size_t r;




	sc_huffman_t huff;

	if (sc_huffman_init(&huff) != SC_E_SUCCESS) {
		fputs("Huffman context initialization failed.\n", stderr);
		return 1;
	}

	rewind(fp);
	running = 1;
	do {
		if ((r = fread(buffer, sizeof(*buffer), (sizeof(buffer) / sizeof(*buffer)), fp)) == 0) {
			running = 0;
			break;
		}

		if (sc_huffman_process(&huff, buffer, r) != SC_E_SUCCESS) {
			fputs("Failed to process data.\n", stderr);
			abort(); // TODO
		}
	} while (running == 1);

	if (sc_huffman_tree_build(&huff) != SC_E_SUCCESS) {
		fputs("Huffman tree build failed.\n", stderr);
		return 3;
	}

#if (DEBUG)
	if (sc_huffman_tree_print(&huff) != SC_E_SUCCESS) {
		fputs("Huffman tree print failed.\n", stderr);
		return 4;
	}
#endif




	sc_file_t file;

	if (sc_file_open(&file, output_file, 1) != SC_E_SUCCESS) {
		fputs("Failed to open file.\n", stderr);
		return 5;
	}

	if (sc_file_write_header(&file, &huff) != SC_E_SUCCESS) {
		fputs("Failed to write header.\n", stderr);
		return 6;
	}

	rewind(fp);
	running = 1;
	do {
		if ((r = fread(buffer, sizeof(*buffer), (sizeof(buffer) / sizeof(*buffer)), fp)) == 0) {
			running = 0;
			break;
		}

		if (sc_file_write_data(&file, buffer, r) != SC_E_SUCCESS) {
			fputs("Failed to write data.\n", stderr);
			abort(); // TODO
		}
	} while (running == 1);

	if (sc_file_close(&file) != SC_E_SUCCESS) {
		fputs("failed to close file.\n", stderr);
		return 8;
	}




	if (alloc != NULL) {
		free(alloc);
		output_file = alloc = NULL;
	}

	if (sc_huffman_clear(&huff) != SC_E_SUCCESS) {
		fputs("Huffman context clearing failed.\n", stderr);
		return 16;
	}




	return 0;
}
