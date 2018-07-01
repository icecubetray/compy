#ifndef __SC_FILE_H
#define __SC_FILE_H




#include "./types.h"

#include "./huffman.h"

#include <stdio.h>




#define SC_FILE_STATE_UNTOUCHED				1
#define SC_FILE_STATE_WR_HEADER				2




typedef struct sc_file_header_node {
	size_t nbits;
	uint8_t data[32];
} sc_file_header_node_t;

typedef struct sc_file_header {
	sc_file_header_node_t map[256];
} sc_file_header_t;

typedef struct sc_file {
	FILE *fp;
	sc_file_header_t header;
	unsigned int state;
} sc_file_t;




#ifdef __cplusplus
extern "C" {
#endif

	sc_result_t sc_file_open(sc_file_t *const restrict file, const char *const restrict path, const unsigned int truncate);
	sc_result_t sc_file_close(sc_file_t *const file);

	sc_result_t sc_file_write_header(sc_file_t *const restrict file, const sc_huffman_t *const restrict context);

#ifdef __cplusplus
}
#endif




#endif
