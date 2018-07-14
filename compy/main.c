#include <libcompy/libcompy.h>
#include <libcompy/core/tests.h>

#include <libserum/io/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>




#define DEBUG								1




#define COMPY_OPTION_HELP					'h'
#define COMPY_OPTION_VERBOSE				'v'
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




ls_log_level_t
static
get_log_level(const unsigned int verbosity) {
	ls_log_level_t level = (LS_LOG_LEVEL_SEVERE + verbosity);

	if (level == 0 || level > LS_LOG_LEVEL_COUNT) {
		level = LS_LOG_LEVEL_DEBUG;
	}

	return level;
}




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

	printf("Usage: %s <mode> [options]\n", executable);
	puts(
		"\n"
		"Mode:\n"
		"    -c, --compress      Compression mode\n"
		"    -d, --decompress    Decompression mode\n"
		"\n"
		"Options:\n"
		"    -h, --help          Show this help message\n"
		"    -v, --verbose       Increase verbosity, repeatable, starts at severe\n"
		"    -i, --in=<file>     Specify input file path\n"
		"    -o, --out=<file>    Specify output file path\n"
		"    -l, --log=<file>    Specify log file path"
	);

	exit(code);
}




void
static
parse_options(int argc, char *argv[], ls_log_level_t *const out_log_level, compy_mode_t *const out_mode, char **const out_input_file, char **const out_output_file, char **const out_log_file) {
	const static struct option options_long[] = {
		{ "help",		no_argument,		NULL, COMPY_OPTION_HELP			},
		{ "verbose",	no_argument,		NULL, COMPY_OPTION_VERBOSE		},
		{ "compress",	no_argument,		NULL, COMPY_OPTION_COMPRESS		},
		{ "decompress",	no_argument,		NULL, COMPY_OPTION_DECOMPRESS	},
		{ "in",         required_argument,  NULL, COMPY_OPTION_INPUT_FILE	},
		{ "out",		required_argument,	NULL, COMPY_OPTION_OUTPUT_FILE	},
		{ "log",		required_argument,	NULL, COMPY_OPTION_LOG_FILE		},
		{ NULL,			0,					NULL, 0							}
	};

	const static char options_short[] = "hvcdi:o:l:";


	/* Reset getopt's parsing index, ya never know. */
	optind = 1;


	unsigned int verbosity = 0;
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
				case COMPY_OPTION_VERBOSE:
					++verbosity;
					break;
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


	*out_log_level = get_log_level(verbosity);
	*out_mode = mode;
	*out_input_file = input_file;
	*out_output_file = output_file;
	*out_log_file = log_file;
}



int static handle_compress(const ls_log_t *const restrict log, const char *const input_file, const char *const output_file);
int static handle_decompress(const ls_log_t *const restrict log, const char *const input_file, const char *const output_file);

int
main(int argc, char *argv[], char *env[]) {
	if (compy_run_tests() != 0) {
		return 10;
	}


	ls_log_level_t level;
	compy_mode_t mode;
	char *input_file, *output_file, *log_file, *alloc = NULL;

	parse_options(argc, argv, &level, &mode, &input_file, &output_file, &log_file);


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
		if (mode == COMPY_MODE_COMPRESS) {
			const size_t iflen = strlen(input_file);

			output_file = alloc = malloc(iflen + 4 + 1);
			memcpy(output_file, input_file, iflen);
			strncpy(output_file + iflen, ".sca\0", 4 + 1);
		} else {
			exit_usage(
				argv[0],
				3,
				"No output file specified."
			);
		}
	}


	ls_log_t logobj, *const log = &logobj;
	FILE *log_fp = NULL;
	if (log_file != NULL) {
		log_fp = fopen(log_file, "w");
		if (log_fp == NULL) {
			fputs("Failed to open log file.", stderr);
			perror("fopen()");
			return 3;
		}
	} else {
		log_fp = stdout;
	}


	if (ls_log_init_ex(log, 0, level, log_fp) != LS_E_SUCCESS) {
		fputs("Failed to initialize logger.\n", stderr);
		return 4;
	}


	int result = -1;
	if (mode == COMPY_MODE_COMPRESS) {
		result = handle_compress(log, input_file, output_file);
	} else if (mode == COMPY_MODE_DECOMPRESS) {
		result = handle_decompress(log, input_file, output_file);
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


	return result;
}




int
static
handle_compress(const ls_log_t *const restrict log, const char *const input_file, const char *const output_file) {
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


	return 0;
}




int
static
handle_decompress(const ls_log_t *const restrict log, const char *const input_file, const char *const output_file) {
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


	return 0;
}
