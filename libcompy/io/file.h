#ifndef __SC_FILE_H
#define __SC_FILE_H




#include "../core/platform.h"
#include "../core/result.h"

#include "../data/huffman.h"

#include <stdio.h>




#define SC_FILE_STATE_IDLE					1
#define SC_FILE_STATE_WR_HEADER				2
#define SC_FILE_STATE_WR_DATA				3




typedef struct sc_file_header_node {
	size_t nbits;
	uint8_t data[32];
} sc_file_header_node_t;

typedef struct sc_file_header {
	sc_file_header_node_t map[256];
	unsigned int populated;
} sc_file_header_t;

typedef struct sc_file {
	FILE *fp;
	sc_file_header_t header;
	unsigned int state;
	uint8_t last_byte;
	uint8_t last_bits;
} sc_file_t;




#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief Opens an archive file for reading and writing, optionally truncating it.
	 * @param file The file instance to work on.
	 * @param path The path to the file on the filesystem.
	 * @param truncate Whether to truncate or not.
	 * @return
	 *		`#SC_E_NULL` when a `NULL` pointer is encountered
	 *		`#SC_E_IO` if opening the file fails
	 *		`#SC_E_SUCCESS` otherwise
	 */
	sc_result_t sc_file_open(sc_file_t *const restrict file, const char *const restrict path, const unsigned int truncate);

	/**
	 * @brief Closes the given archive file, flushing the last bits when necessary.
	 * @param file The file instance to work on.
	 * @return
	 *		`#SC_E_NULL` when a `NULL` pointer is encountered
	 *		`#SC_E_IO` when writing or closing fails
	 *		`#SC_E_SUCCESS` otherwise
	 */
	sc_result_t sc_file_close(sc_file_t *const file);

	/**
	 * @brief Writes the header of the archive file to the filesystem.
	 * @param file The file instance to work on.
	 * @param context The huffman structure this file will be using the bit patterns from.
	 * @return
	 *		`#SC_E_NULL` when a `NULL` pointer is encountered
	 *		`#SC_E_IO` if the underlying file is not opened (properly), or if seeking/writing/flushing fails
	 *		`#SC_E_NOT_READY` if the given huffman structure is not yet ready
	 *		`#SC_E_STATE` if the file instance if in an invalid state, such as when data is already written
	 *		`#SC_E_DATA` if determining the bit patterns failed due to apparent lack of data
	 *		`#SC_E_SUCCESS` otherwise
	 */
	sc_result_t sc_file_write_header(sc_file_t *const restrict file, const sc_huffman_t *const restrict context);

	/**
	 * @brief Writes the input data as bit patterns to the archive file on the filesystem.
	 * @param file The file instance to work on.
	 * @param data The input data source.
	 * @param size The number of bytes to read from the input data source.
	 * @return
	 *		`#SC_E_NULL` when a `NULL` pointer is encountered
	 *		`#SC_E_PARAM` if \p size is zero
	 *		`#SC_E_IO` if the underlying file is not opened (properly), or if writing/flushing fails
	 *		`#SC_E_STATE` if the file instance if in an invalid state, such as when the header is not yet written
	 *		`#SC_E_NOT_READY` if there are no mapped values in the header, such as when the header is not processed properly
	 *		`#SC_E_SUCCESS` otherwise
	 */
	sc_result_t sc_file_write_data(sc_file_t *const restrict file, const void *const restrict data, const size_t size);

	sc_result_t sc_file_restore(sc_file_t *file, FILE *fp_restore);

#ifdef __cplusplus
}
#endif




#endif
