#include <libcompy/libcompy.h>

#include <libcompy/core/tests.h>

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>




#define DEBUG								1




#define COMPY_OPTION_HELP					'h'
#define COMPY_OPTION_COMPRESS				'c'
#define COMPY_OPTION_DECOMPRESS				'd'
#define COMPY_OPTION_INPUT_FILE				'i'
#define COMPY_OPTION_OUTPUT_FILE			'o'
#define COMPY_OPTION_LOG_FILE				'l'




typedef enum compy_mode {
	COMPY_MODE_INIT = 0,
	COMPY_MODE_COMPRESS = 1,
	COMPY_MODE_DECOMPRESS = 2
} compy_mode_t;




#if (COMPY_COLOR)
#	define FORMAT							"\x1B[0;91m%s\x1B[0m"
#else
#	define FORMAT							"%s"
#endif

void
static
__attribute__((noreturn))
exit_usage(const char *const executable, int code, const char *const message) {
	if (message != NULL) {
		const static char format[] = FORMAT "\n\n";

		fprintf(
			stderr,
			format,
			message
		);
	}

	printf("Usage: %s <mode> --in=<file> [options]\n", executable);
	puts(
		"\n"
		"Mode:\n"
		"    -c, --compress      Compression mode\n"
		"    -d, --decompress    Decompression mode\n"
		"\n"
		"Options:\n"
		"    -h, --help          Show this help message\n"
		"    -i, --in=<file>     Specify input file path\n"
		"    -o, --out=<file>    Specify output file path\n"
		"    -l, --log=<file>    Specify log file path"
	);

	exit(code);
}




void
static
parse_options(int argc, char *argv[], compy_mode_t *const out_mode, char **const out_input_file, char **const out_output_file, char **const out_log_file) {
	const static struct option options_long[] = {
		{ "help",		no_argument,		NULL, COMPY_OPTION_HELP			},
		{ "compress",	no_argument,		NULL, COMPY_OPTION_COMPRESS		},
		{ "decompress",	no_argument,		NULL, COMPY_OPTION_DECOMPRESS	},
		{ "in",         required_argument,  NULL, COMPY_OPTION_INPUT_FILE	},
		{ "out",		required_argument,	NULL, COMPY_OPTION_OUTPUT_FILE	},
		{ "log",		required_argument,	NULL, COMPY_OPTION_LOG_FILE		},
		{ NULL,			0,					NULL, 0							}
	};

	const static char options_short[] = "hcdi:o:l:";


	/* Reset getopt's parsing index, ya never know. */
	optind = 1;


	compy_mode_t mode = COMPY_MODE_INIT;
	char *input_file = NULL, *output_file = NULL, *log_file = NULL;

	int opt_result, opt_index;
	do {
		opt_result = getopt_long(argc, argv, options_short, options_long, &opt_index);
		if (opt_result == -1) {
			break;
		} else {
			switch (opt_result) {
				case COMPY_OPTION_HELP:
					exit_usage(
						argv[0],
						0,
						NULL
					);
				case COMPY_OPTION_COMPRESS:
					mode = COMPY_MODE_COMPRESS;
					break;
				case COMPY_OPTION_DECOMPRESS:
					mode = COMPY_MODE_DECOMPRESS;
					break;
				case COMPY_OPTION_INPUT_FILE:
					input_file = optarg;
					break;
				case COMPY_OPTION_OUTPUT_FILE:
					output_file = optarg;
					break;
				case COMPY_OPTION_LOG_FILE:
					log_file = optarg;
					break;
			}
		}
	} while (1);


	/* Reset getopt's parsing index once more, to be nice. */
	optind = 1;


	*out_mode = mode;
	*out_input_file = input_file;
	*out_output_file = output_file;
	*out_log_file = log_file;
}




int
main(int argc, char *argv[], char *env[]) {
	if (compy_run_tests() != 0) {
		return 10;
	}




	compy_mode_t mode;
	char *input_file, *output_file, *log_file, *alloc = NULL;

	parse_options(argc, argv, &mode, &input_file, &output_file, &log_file);


	if (mode == COMPY_MODE_INIT) {
		exit_usage(
			argv[0],
			1,
			"No mode specified."
		);
	}

	if (input_file == NULL) {
		exit_usage(
			argv[0],
			2,
			"No input file specified."
		);
	}


	if (output_file == NULL) {
		const size_t iflen = strlen(input_file);

		output_file = alloc = malloc(iflen + 4 + 1);
		memcpy(output_file, input_file, iflen);
		strncpy(output_file + iflen, ".sca\0", 4 + 1);
	}

	FILE *log_fp = NULL;
	if (log_file != NULL) {
		log_fp = fopen(log_file, "w");
		if (log_fp == NULL) {
			fputs("Failed to open log file.", stderr);
			perror("fopen()");
			return 3;
		}
	}
#if (DEBUG)
	else {
		log_fp = stdout;
	}
#endif




	if (mode == COMPY_MODE_COMPRESS) {
		FILE *fp = fopen(input_file, "r");
		if (fp == NULL) {
			perror("fopen()");
			return 4;
		}




		uint8_t buffer[2048];
		int running;
		size_t r;




		compy_huffman_t huff;

		if (compy_huffman_init(&huff) != COMPY_E_SUCCESS) {
			fputs("Huffman context initialization failed.\n", stderr);
			return 5;
		}

		rewind(fp);
		running = 1;
		do {
			if ((r = fread(buffer, sizeof(*buffer), (sizeof(buffer) / sizeof(*buffer)), fp)) == 0) {
				running = 0;
				break;
			}

			if (compy_huffman_process(&huff, buffer, r) != COMPY_E_SUCCESS) {
				fputs("Failed to process data.\n", stderr);
				abort(); // TODO
			}
		} while (running == 1);

		if (compy_huffman_tree_build(&huff) != COMPY_E_SUCCESS) {
			fputs("Huffman tree build failed.\n", stderr);
			return 6;
		}

		if (log_fp != NULL) {
			if (compy_huffman_tree_print(&huff, log_fp) != COMPY_E_SUCCESS) {
				fputs("Huffman tree print failed.\n", stderr);
				return 7;
			}
		}




		compy_file_t file;

		if (compy_file_open(&file, output_file, 1) != COMPY_E_SUCCESS) {
			fputs("Failed to open file.\n", stderr);
			return 8;
		}

		if (compy_file_write_header(&file, &huff) != COMPY_E_SUCCESS) {
			fputs("Failed to write header.\n", stderr);
			return 9;
		}

		rewind(fp);
		running = 1;
		do {
			if ((r = fread(buffer, sizeof(*buffer), (sizeof(buffer) / sizeof(*buffer)), fp)) == 0) {
				running = 0;
				break;
			}

			if (compy_file_write_data(&file, buffer, r) != COMPY_E_SUCCESS) {
				fputs("Failed to write data.\n", stderr);
				abort(); // TODO
			}
		} while (running == 1);

		if (compy_file_close(&file) != COMPY_E_SUCCESS) {
			fputs("failed to close file.\n", stderr);
			return 10;
		}


		if (fclose(fp) != 0) {
			perror("fclose()");
		}

		if (compy_huffman_clear(&huff) != COMPY_E_SUCCESS) {
			fputs("Huffman context clearing failed.\n", stderr);
			return 11;
		}
	} else if (mode == COMPY_MODE_DECOMPRESS) {
		compy_file_t file;

		if (compy_file_open(&file, input_file, 0) != COMPY_E_SUCCESS) {
			fputs("Failed to open file.\n", stderr);
			return 12;
		}

		FILE *fp_restore = fopen(output_file, "w");
		if (fp_restore == NULL) {
			perror("fopen()");
			return 13;
		}

		if (compy_file_restore(&file, fp_restore) != COMPY_E_SUCCESS) {
			fputs("Failed to load file.\n", stderr);
			return 14;
		}

		if (compy_file_close(&file) != COMPY_E_SUCCESS) {
			fputs("failed to close file.\n", stderr);
			return 15;
		}


		if (fclose(fp_restore) != 0) {
			perror("fclose()");
		}
	}




	if (alloc != NULL) {
		free(alloc);
		output_file = alloc = NULL;
	}

	if (log_fp != NULL && log_file != NULL) {
		if (fclose(log_fp) != 0) {
			perror("fclose()");
		}
	}




	return 0;
}
