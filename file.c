#include "./file.h"

#include <string.h>
#include <stdlib.h>




sc_result_t
sc_file_open(sc_file_t *const restrict file, const char *const restrict path, const unsigned int truncate) {
	if (file == NULL || path == NULL) {
		return SC_E_NULL;
	}


	char modes[2 + 1] = "?+\0";
	if (truncate) {
		modes[0] = 'w';
	} else {
		modes[0] = 'r';
	}


	FILE *fp = fopen(path, modes);
	if (fp == NULL) {
		return SC_E_IO;
	}


	memset(file, 0, sizeof(*file));
	file->fp = fp;
	file->state = SC_FILE_STATE_IDLE;


	return SC_E_SUCCESS;
}


sc_result_t
sc_file_close(sc_file_t *const file) {
	if (file == NULL) {
		return SC_E_NULL;
	}


	if (file->last_bits > 0) {
		if (file->fp == NULL) {
			return SC_E_IO;
		}

		uint8_t byte = (uint8_t)(file->last_byte & ((1 << file->last_bits) - 1));
		if (fwrite(&byte, sizeof(byte), 1, file->fp) != sizeof(byte)) {
			puts("fwrite() failed to write last byte");
			return SC_E_IO;
		}

		file->last_bits = file->last_byte = 0;
	}


	if (file->fp != NULL) {
		if (fclose(file->fp) == 0) {
			file->fp = NULL;
		} else {
			return SC_E_IO;
		}
	}

	memset(file, 0, sizeof(*file));


	return SC_E_SUCCESS;
}




sc_result_t
sc_file_write_header(sc_file_t *const restrict file, const sc_huffman_t *const restrict context) {
	if (file == NULL || context == NULL) {
		return SC_E_NULL;
	}

	if (file->fp == NULL || ferror(file->fp) != 0) {
		return SC_E_IO;
	}

	if (context->tree_root == NULL) {
		return SC_E_NOT_READY;
	}

	if (file->state != SC_FILE_STATE_IDLE) {
		return SC_E_STATE;
	}


	unsigned int populated;
	register unsigned int i;
	sc_file_header_node_t *hnode = NULL;

	if ((populated = file->header.populated) == 0) {
		const sc_ll_node_t
			*const *const tree_lookup = (const sc_ll_node_t *const *const)context->tree_lookup,
			*tnode = NULL;

		size_t
			nbits,
			data_idx;

		register unsigned int current;
		for (i = 256, populated = 0; i--;) {
			if (tree_lookup[i] != NULL) {
				++populated;
				tnode = tree_lookup[i];
				hnode = &file->header.map[i];

				data_idx = 0;
				current = 0;

				for (nbits = 0;;) {
					if (tnode == NULL) {
						break;
					}

					/* Check if we reached the root of the tree. */
					if (tnode->parent == NULL) {
						break;
					}

					if ((tnode->flags & SC_LL_LEFT) == SC_LL_LEFT) {
						current |= (1 << (nbits % 8));
					} else if ((tnode->flags & SC_LL_RIGHT) == SC_LL_RIGHT) {
						/* NOOP */
						// current |= 0;
					} else {
						puts("not left or right");
						abort(); // TODO
					}

					/* Check if nbits is a multiple of 8. */
					if ((++nbits & 7) == 0) {
						hnode->data[data_idx] = (uint8_t)(current & 0xFF);
						current = 0;

						if (++data_idx > sizeof(hnode->data)) {
							puts("data_idx bounds");
							abort(); // TODO
						}
					}

					tnode = tnode->parent;
				}

				/* Check if there's any bits left, and if so fetch the remaining ones. */
				if ((nbits & 7) != 0) {
					hnode->data[data_idx++] = (uint8_t)(current & 0xFF);
					current = 0;
				}

				hnode->nbits = nbits;
			}
		}

		file->header.populated = populated;
	}


	/* Check if everything went okay. */
	if (populated == 0) {
		return SC_E_DATA;
	}


	/* Rewind the stream. */
	if (fseek(file->fp, 0, SEEK_SET) != 0) {
		return SC_E_IO;
	}


	const static uint8_t magic[4] = { 0x20, 0x16, 0x11, 0x27 };

	/* Write our magic to indicate the file type. */
	if (fwrite(magic, sizeof(*magic), (sizeof(magic) / sizeof(*magic)), file->fp) != sizeof(magic)) {
		return SC_E_IO;
	}

	/* Write the map count. */
	if (fwrite(&file->header.populated, sizeof(uint8_t), 1, file->fp) != sizeof(uint8_t)) {
		return SC_E_IO;
	}

	/* Write all map nodes. */
	uint8_t buffer[34];
	register size_t nbits, nbytes;
	for (i = 256; i--;) {
		hnode = &file->header.map[i];

		if ((nbits = hnode->nbits) > 0) {
			/* The node's value. */
			buffer[0] = (uint8_t)(i & 0xFF);

			/* The number of bits used by the node's data. */
			buffer[1] = (uint8_t)(nbits & 0xFF);

			/* The node's data. */
			nbytes = ((nbits / 8) + 1);
			memcpy(&buffer[2], hnode->data, nbytes);

			nbytes += 2;
			if (fwrite(buffer, 1, nbytes, file->fp) != nbytes) {
				return SC_E_IO;
			}
		}
	}


	/* Flush the header. */
	if (fflush(file->fp) != 0) {
		return SC_E_IO;
	}


	file->state = SC_FILE_STATE_WR_HEADER;


	return SC_E_SUCCESS;
}




sc_result_t
sc_file_write_data(sc_file_t *const restrict file, const void *const restrict data, const size_t size) {
	if (file == NULL || data == NULL) {
		return SC_E_NULL;
	}

	if (size == 0) {
		return SC_E_PARAM;
	}

	if (file->state != SC_FILE_STATE_WR_HEADER) {
		return SC_E_STATE;
	}

	if (file->header.populated == 0) {
		return SC_E_NOT_READY;
	}


	const sc_file_header_node_t *node = NULL;
	const uint8_t *const data8 = (const uint8_t *const)data;

	uint8_t buffer[512];

	register unsigned int
		i,
		nbits, rbits, wbits,
		bits = file->last_bits,
		byte = file->last_byte,
		buffer_index = 0, index;

	uint8_t value;
	for (i = 0; i < size; ++i) {
		/* Fetch the current value. */
		value = data8[i];

		/* Fetch the header node associated with the current value. */
		node = &file->header.map[value];

		/* Check if we can even process the current value. */
		if ((nbits = node->nbits) == 0) {
			puts("unmapped byte");
			abort(); // TODO
		}


		index = 0;
		wbits = 0;


		/* Determine how many bits we have to process. */
		rbits = (bits + nbits);

		/* While we have more than or equal to 8 bits remaining. */
		while (rbits >= 8) {
			/* Check if we need to flush the buffer, and do so if required. */
			if (buffer_index >= sizeof(buffer)) {
				if (fwrite(buffer, 1, buffer_index, file->fp) != buffer_index) {
					puts("buffer flush fail");
					abort(); // TODO
				}

				buffer_index = 0;
			}

			/* If we have some bits left from a previous iteration, first process those bits.
 			** Otherwise write the full bytes. */
			if (bits > 0) {
				/* Determine number of bits to fill up with the current value. */
				wbits = (8 - bits);

				/* Push the remaining value all the way to the left and add the filler bits
 				** from the current value. */
				byte = ((byte << wbits) | (node->data[index] & ((1 << wbits) - 1)));

				/* Reset bits, we won't be needing these anymore during this iteration. */
				bits = 0;
			} else {
				byte = (node->data[index] & ((1 << (8 - wbits)) - 1));
				byte |= (node->data[++index] & ((1 << wbits) - 1));
			}

			/* Write the byte to the buffer. */
			buffer[buffer_index++] = byte;
			rbits -= 8;
		}

		if (rbits == 0) {
			byte = bits = 0;
			continue;
		}

		nbits = (rbits - bits);

		/* We have some bits left to process, pass them on to the next iteration. */
		byte = ((byte << nbits) | (node->data[index] & ((1 << nbits) - 1)));
		bits = rbits;
	}

	if (buffer_index > 0) {
		if (fwrite(buffer, 1, buffer_index, file->fp) != buffer_index) {
			puts("fwrite() failed to write full stuff");
			return SC_E_IO;
		}
	}

	if (fflush(file->fp) != 0) {
		puts("fflush() failed");
		return SC_E_IO;
	}

	/* Save our state for the next call, or final processing. */
	file->last_bits = bits;
	file->last_byte = byte;


	return SC_E_SUCCESS;
}
