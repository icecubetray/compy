#ifndef __LIBCOMPY_DATA_FILE_H
#define __LIBCOMPY_DATA_FILE_H




#include "../core/platform.h"
#include "../core/result.h"

#include "../data/huffman.h"

#include <stdio.h>




#define COMPY_FILE_STATE_IDLE				1
#define COMPY_FILE_STATE_WR_HEADER			2
#define COMPY_FILE_STATE_WR_DATA			3




typedef struct compy_file_header_node {
	size_t nbits;
	uint8_t data[32];
} compy_file_header_node_t;

typedef struct compy_file_header {
	compy_file_header_node_t map[256];
	unsigned int populated;
} compy_file_header_t;

typedef struct compy_file {
	FILE *fp;
	compy_file_header_t header;
	unsigned int state;
	uint8_t last_byte;
	uint8_t last_bits;
} compy_file_t;




#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief Opens an archive file for reading and writing, optionally truncating it.
	 * @param file The file instance to work on.
	 * @param path The path to the file on the filesystem.
	 * @param truncate Whether to truncate or not.
	 * @return
	 *		`#COMPY_E_NULL` when a `NULL` pointer is encountered
	 *		`#COMPY_E_IO` if opening the file fails
	 *		`#COMPY_E_SUCCESS` otherwise
	 */
	compy_result_t compy_file_open(compy_file_t *const restrict file, const char *const restrict path, const unsigned int truncate);

	/**
	 * @brief Closes the given archive file, flushing the last bits when necessary.
	 * @param file The file instance to work on.
	 * @return
	 *		`#COMPY_E_NULL` when a `NULL` pointer is encountered
	 *		`#COMPY_E_IO` when writing or closing fails
	 *		`#COMPY_E_SUCCESS` otherwise
	 */
	compy_result_t compy_file_close(compy_file_t *const file);

	/**
	 * @brief Writes the header of the archive file to the filesystem.
	 * @param file The file instance to work on.
	 * @param context The huffman structure this file will be using the bit patterns from.
	 * @return
	 *		`#COMPY_E_NULL` when a `NULL` pointer is encountered
	 *		`#COMPY_E_IO` if the underlying file is not opened (properly), or if seeking/writing/flushing fails
	 *		`#COMPY_E_NOT_READY` if the given huffman structure is not yet ready
	 *		`#COMPY_E_STATE` if the file instance if in an invalid state, such as when data is already written
	 *		`#COMPY_E_DATA` if determining the bit patterns failed due to apparent lack of data
	 *		`#COMPY_E_SUCCESS` otherwise
	 */
	compy_result_t compy_file_write_header(compy_file_t *const restrict file, const compy_huffman_t *const restrict context);

	/**
	 * @brief Writes the input data as bit patterns to the archive file on the filesystem.
	 * @param file The file instance to work on.
	 * @param data The input data source.
	 * @param size The number of bytes to read from the input data source.
	 * @return
	 *		`#COMPY_E_NULL` when a `NULL` pointer is encountered
	 *		`#COMPY_E_PARAM` if \p size is zero
	 *		`#COMPY_E_IO` if the underlying file is not opened (properly), or if writing/flushing fails
	 *		`#COMPY_E_STATE` if the file instance if in an invalid state, such as when the header is not yet written
	 *		`#COMPY_E_NOT_READY` if there are no mapped values in the header, such as when the header is not processed properly
	 *		`#COMPY_E_SUCCESS` otherwise
	 */
	compy_result_t compy_file_write_data(compy_file_t *const restrict file, const void *const restrict data, const size_t size);

	compy_result_t compy_file_restore(compy_file_t *const restrict file, FILE *const restrict fp_restore);

#ifdef __cplusplus
}
#endif




#endif
