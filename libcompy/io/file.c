#include "./file.h"

#include <string.h>
#include <stdlib.h>




const static uint8_t __file_magic[4] = { 0x20, 0x16, 0x11, 0x27 };




compy_result_t
compy_file_open(compy_file_t *const restrict file, const char *const restrict path, const unsigned int truncate) {
	if (file == NULL || path == NULL) {
		return COMPY_E_NULL;
	}


	/* Determine file mode: [rw]+ */
	char modes[2 + 1] = "?+\0";
	if (truncate) {
		modes[0] = 'w';
	} else {
		modes[0] = 'r';
	}


	/* Open the file using the specified path, if fopen() fails it'll return NULL */
	FILE *fp = fopen(path, modes);
	if (fp == NULL) {
		return COMPY_E_IO;
	}


	/* Clear out the file structure, store the file pointer, reset the state. */
	memset(file, 0, sizeof(*file));
	file->fp = fp;
	file->state = COMPY_FILE_STATE_IDLE;


	return COMPY_E_SUCCESS;
}


compy_result_t
compy_file_close(compy_file_t *const file) {
	if (file == NULL) {
		return COMPY_E_NULL;
	}


	/* Check if we have bits left to flush. */
	if (file->last_bits > 0) {
		if (file->fp == NULL) {
			return COMPY_E_IO;
		}

		/* Fetch the bits that we still need to write, and write them. */
		uint8_t byte = (uint8_t)(file->last_byte & ((1 << file->last_bits) - 1));
		if (fwrite(&byte, sizeof(byte), 1, file->fp) != sizeof(byte)) {
			puts("fwrite() failed to write last byte");
			return COMPY_E_IO;
		}

		/* Since we had a partial write, we have to give a value to the trim byte so we
		** can determine what bits to read from the last byte.
		** The trim byte is the fifth byte in the header, which we initially wrote as a
		** zero to the file. Set it to the number of bits we wrote just now. */
		fseek(file->fp, 4, SEEK_SET);
		byte = (uint8_t)file->last_bits;
		if (fwrite(&byte, sizeof(byte), 1, file->fp) != sizeof(byte)) {
			puts("fwrite() failed to write trim byte");
			return COMPY_E_IO;
		}

		/* Clear the partial write state. */
		file->last_bits = file->last_byte = 0;
	}


	/* Close the underlying file, if one is opened. */
	if (file->fp != NULL) {
		if (fclose(file->fp) == 0) {
			file->fp = NULL;
		} else {
			return COMPY_E_IO;
		}
	}


	/* Clear out the file instance, everything went OK. */
	memset(file, 0, sizeof(*file));


	return COMPY_E_SUCCESS;
}




/*
** Static, private function to write a binary tree to the filesystem.
**
** Starting at the root of the tree, for each non-leaf node it will first
** recurse on its left child and then recurse on its right child.
** Upon encountering a non-leaf node it will write a 0 bit, while every
** leaf node will write a 1 bit followed by the 8 bits of the value the
** leaf node represents.
** The writes are done before any recursion is done.
*/
void
static __wrtree(FILE *fp, uint8_t *byte, unsigned int *index, compy_node_t *node) {
	if (node == NULL) {
		return;
	}

	if ((node->flags & COMPY_NODE_LEAF) == COMPY_NODE_LEAF) {
		/* Shl by one, append 1. */
		*byte = ((*byte << 1) | 1);

		if (++(*index) >= 8) {
			fwrite(byte, sizeof(*byte), 1, fp);
			*byte = *index = 0;
		}

		/* Temporarily flip index to the number of bits we need to fetch from the node. */
		*index = (8 - *index);

		/* Byte = bits already in byte pushed to the left, filled up with bits from the node. */
		*byte = ((*byte << *index) | ((node->value & (((1 << *index) - 1) << (8 - *index))) >> (8 - *index)));
		fwrite(byte, sizeof(*byte), 1, fp);

		/* Set index back to the number of bits currently set to be written. */
		if ((*index = (8 - *index)) > 0) {
			*byte = (node->value & ((1 << *index) - 1));
		} else {
			*byte = 0;
		}
	} else {
		/* A mere shl by one will simulate appending a zero. */
		*byte <<= 1;

		if (++(*index) >= 8) {
			fwrite(byte, sizeof(*byte), 1, fp);
			*byte = *index = 0;
		}

		/* Recurse on the left child. */
		__wrtree(fp, byte, index, node->left);

		/* Recurse on the right child. */
		__wrtree(fp, byte, index, node->right);
	}
}

compy_result_t
compy_file_write_header(compy_file_t *const restrict file, const compy_huffman_t *const restrict context) {
	if (file == NULL || context == NULL) {
		return COMPY_E_NULL;
	}

	if (file->fp == NULL || ferror(file->fp) != 0) {
		return COMPY_E_IO;
	}

	if (context->tree_root == NULL) {
		return COMPY_E_NOT_READY;
	}

	if (file->state != COMPY_FILE_STATE_IDLE && file->state != COMPY_FILE_STATE_WR_HEADER) {
		return COMPY_E_STATE;
	}


	unsigned int populated;
	register unsigned int i;
	compy_file_header_node_t *hnode = NULL;

	/* Check if the mapping is empty, if so populate it. */
	if ((populated = file->header.populated) == 0) {
		const compy_node_t
			*const *const tree_lookup = (const compy_node_t *const *const)context->tree_lookup,
			*tnode = NULL;

		size_t
			nbits,
			data_idx;

		/* Populate the map based on each set node. */
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

					if ((tnode->flags & COMPY_NODE_LEFT) == COMPY_NODE_LEFT) {
						current |= (1 << (nbits % 8));
					} else if ((tnode->flags & COMPY_NODE_RIGHT) == COMPY_NODE_RIGHT) {
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

					/* Move up the tree. */
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
		return COMPY_E_DATA;
	}


	/* Rewind the stream. */
	if (fseek(file->fp, 0, SEEK_SET) != 0) {
		return COMPY_E_IO;
	}


	uint8_t
		buffer[34],
		*buffptr = buffer;


	/* Prepare our magic to indicate the file type. */
	memcpy(buffptr, __file_magic, sizeof(__file_magic));
	buffptr += sizeof(__file_magic);

	/* Prepare the placeholder for the number of bits that should be used from the last byte. */
	*buffptr++ = 0;

	/* Write what we prepared to the file. */
	if (fwrite(buffer, sizeof(uint8_t), (buffptr - buffer), file->fp) != (buffptr - buffer)) {
		return COMPY_E_IO;
	}


	/* Write the tree to the file. */
	uint8_t byte = 0;
	unsigned int index = 0;
	__wrtree(file->fp, &byte, &index, context->tree_root);
	file->last_bits = index;
	file->last_byte = byte;


	/* Flush the header. */
	if (fflush(file->fp) != 0) {
		return COMPY_E_IO;
	}


#if (DEBUG)
	printf("header end at %lX\n", ftell(file->fp));
#endif


	/* Keep track of what we're doing with this instance. */
	file->state = COMPY_FILE_STATE_WR_HEADER;


	return COMPY_E_SUCCESS;
}




compy_result_t
compy_file_write_data(compy_file_t *const restrict file, const void *const restrict data, const size_t size) {
	if (file == NULL || data == NULL) {
		return COMPY_E_NULL;
	}

	if (size == 0) {
		return COMPY_E_PARAM;
	}

	if (file->fp == NULL || ferror(file->fp) != 0) {
		return COMPY_E_IO;
	}

	if (file->state != COMPY_FILE_STATE_WR_HEADER && file->state != COMPY_FILE_STATE_WR_DATA) {
		return COMPY_E_STATE;
	}

	if (file->header.populated == 0) {
		return COMPY_E_NOT_READY;
	}


	const compy_file_header_node_t *node = NULL;
	const uint8_t *const data8 = (const uint8_t *const)data;

	uint8_t buffer[512];

#if (DEBUG)
	/* When debugging, ensure variables aren't optimized out. */
	volatile
#endif
	register unsigned int
		data_index,
		bits_left,
		bits_processing,
		bits_needed,
		bit_index,
		bits = file->last_bits,
		byte = file->last_byte,
		buffer_index,
		node_index;

	for (data_index = 0, buffer_index = 0; data_index < size; ++data_index) {
		/* Fetch the header node associated with the current value. */
		node = &file->header.map[data8[data_index]];

		/* Check if we can even process the current value. */
		if ((bits_left = node->nbits) == 0) {
			puts("unmapped byte");
			abort(); // TODO
		}


		/* The start of the root is at the MSb of the MSB, so the index of the current byte
		** is set at the last one available which we can calculate using the number of bits we
		** have for the node. */
		node_index = ((bits_left - 1) / 8);

#if (DEBUG)
		/* When debugging, initialize this to something distinguisable. */
		bit_index = ~0;
#endif

		/* Number of bits to be processed. */
		bits_processing = (bits + bits_left);

		/* Check if we have to process at least one byte (8 bits), and loop so long as we do. */
		while (bits_processing >= 8) {
			/* Determine how many bits we have to fetch from the node. */
			bits_needed = (8 - bits);

			/* Determine the 0-index of the bit where we have to start fetching from. */
			bit_index = ((bits_left - bits_needed) % 8);

			/* Check if we have to compensate for the bits left behind by a previous iteration. */
			if (bits > 0) {
				/* Check if we have to bridge between bytes, or if we can just use the current byte. */
				if ((bit_index + bits_needed) > 8) {
					byte <<= bits_needed;
					byte |= ((node->data[node_index] & ((1 << (bits_left % 8)) - 1)) << (8 - bit_index));
					byte |= ((node->data[--node_index] & (((1 << (bits_needed - (bits_left % 8))) - 1) << bit_index)) >> bit_index);
				} else {
					byte = ((byte << bits_needed) | ((node->data[node_index] & (((1 << bits_needed) - 1) << bit_index)) >> bit_index));
				}

				bits = 0;
				bits_left -= bits_needed;

				/* Check if we exhausted the current byte of the node. */
				if ((bits_left & 7) == 0) {
					--node_index;
				}
			} else {
				/* Check if we have to bridge between bytes, or if we can just write a full byte. */
				if (bit_index > 0) {
					bits_needed = (8 - bit_index);
					byte = ((node->data[node_index] & ((1 << bit_index) - 1)) << bits_needed);
					byte |= ((node->data[--node_index] & (((1 << bits_needed) - 1) << bit_index)) >> bit_index);
				} else {
					byte = node->data[node_index--];
				}

				bits_left -= 8;
			}


			/* Write out the pattern to the buffer. */
			buffer[buffer_index] = (uint8_t)byte;
			byte = 0;

			/* Check if the buffer should flush. */
			if (++buffer_index >= sizeof(buffer)) {
				buffer_index = 0;
				if (fwrite(buffer, sizeof(*buffer), (sizeof(buffer) / sizeof(*buffer)), file->fp) != sizeof(buffer)) {
					perror("fwrite()");
					abort();
				}
			}

			/* We have 8 bits less to process. */
			bits_processing -= 8;
		}

		if (bits_processing > 0) {
			bits = bits_processing;
			byte = ((byte << bits_left) | ((node->data[node_index] & (((1 << bits_left) - 1) << 0)) >> 0));
		} else {
			bits = byte = 0;
		}
	}

	if (buffer_index > 0) {
		if (fwrite(buffer, 1, buffer_index, file->fp) != buffer_index) {
			puts("fwrite() failed to write full stuff");
			return COMPY_E_IO;
		}
	}

	if (fflush(file->fp) != 0) {
		puts("fflush() failed");
		return COMPY_E_IO;
	}

	/* Save our state for the next call, or final processing. */
	file->last_bits = bits;
	file->last_byte = byte;


	/* Keep track of what we're doing with this instance. */
	file->state = COMPY_FILE_STATE_WR_DATA;


	return COMPY_E_SUCCESS;
}




#define COMPY_DECODE_NODE					0
#define COMPY_DECODE_VALUE					1
#define COMPY_DECODE_DATA					2

compy_result_t
compy_file_restore(compy_file_t *const restrict file, FILE *const restrict fp_restore) {
	if (file == NULL || fp_restore == NULL) {
		return COMPY_E_NULL;
	}

	if (file->fp == NULL || ferror(file->fp) != 0) {
		return COMPY_E_IO;
	}


	rewind(file->fp);


	size_t magic_read = 0;
	uint8_t magic[4];

	size_t trim_read = 0;
	uint8_t trim = 0;


	size_t read, state_buffer_index = 0;
	uint8_t buffer[2048], state_buffer[2048];

	compy_node_t
		*state_root = NULL,
		*state_last_parent = NULL,
		*state_last_lookup = NULL;

#if (DEBUG)
	/* When debugging, ensure variables aren't optimized out. */
	//volatile
#endif
	register unsigned int
		i, j, end,
		bit, byte,
		state_state = 0,
		state_bits = 0,
		state_bits_left = 0;

	while ((read = fread(buffer, sizeof(*buffer), (sizeof(buffer) / sizeof(*buffer)), file->fp)) > 0) {
		/* Reset cursor and end marker. */
		i = 0;
		end = (unsigned int)read;

		/* Ensure we have magic. */
		for (; magic_read < 4 && i < end; ++i) {
			magic[magic_read] = buffer[i];

			if (++magic_read == 4) {
				if (memcmp(magic, __file_magic, sizeof(__file_magic)) != 0) {
					return COMPY_E_DATA;
				}
			}
		}

		/* Ensure we have the trim byte. */
		for (; trim_read < 1 && i < end; ++i, ++trim_read) {
			trim = buffer[i];
		}

		/* Read data. */
		for (; i < end; ++i) {
			byte = buffer[i];

			j = 8;
			if (trim > 0 && read < sizeof(buffer) && i == (read - 1)) {
				// last byte
				j = trim;
			}

			for (; j--;) {
				bit = ((byte >> j) & 0x01);

				if (state_state == COMPY_DECODE_NODE) {
					if (bit == 1) {
						state_state = COMPY_DECODE_VALUE;
						state_bits_left = 8;
						state_bits = 0;
					} else {
						if (state_root == NULL) {
							state_root = compy_node_alloc(0, 0, 0);
							state_last_lookup = state_root;
						}
						if (state_last_parent == NULL) {
							state_last_parent = state_root;
						} else {
							if (state_last_parent->left == NULL) {
								compy_node_t *node = compy_node_alloc(0, 0, 0);
								node->parent = state_last_parent;
								state_last_parent->left = node;
								state_last_parent = node;
							} else if (state_last_parent->right == NULL) {
								compy_node_t *node = compy_node_alloc(0, 0, 0);
								node->parent = state_last_parent;
								state_last_parent->right = node;
								state_last_parent = node;
							} else {
								while ((state_last_parent = state_last_parent->parent) != NULL) {
									if (state_last_parent->left == NULL || state_last_parent->right == NULL) {
										break;
									}
								}
								if (state_last_parent == state_root) {
									// all full
									state_state = COMPY_DECODE_DATA;
								}
							}
						}
					}
				} else if (state_state == COMPY_DECODE_VALUE) {
					state_bits |= (bit << --state_bits_left);

					if (state_bits_left == 0) {
						compy_node_t *node = compy_node_alloc(0, state_bits, COMPY_NODE_LEAF);
						if (state_last_parent->left == NULL) {
							state_last_parent->left = node;
						} else if (state_last_parent->right == NULL) {
							state_last_parent->right = node;

							while ((state_last_parent = state_last_parent->parent) != NULL) {
								if (state_last_parent->left == NULL || state_last_parent->right == NULL) {
									break;
								}
							}
						} else {
							fputs("not supposed to get here\n", stderr);
							abort();
						}

						state_state = ((state_last_parent == NULL) ? COMPY_DECODE_DATA : COMPY_DECODE_NODE);
					}
				} else if (state_state == COMPY_DECODE_DATA) {
					if (bit == 1) {
						state_last_lookup = state_last_lookup->left;
					} else if (bit == 0) {
						state_last_lookup = state_last_lookup->right;
					}

					if ((state_last_lookup->flags & COMPY_NODE_LEAF) == COMPY_NODE_LEAF) {
						state_buffer[state_buffer_index] = state_last_lookup->value;
						state_last_lookup = state_root;

						if (++state_buffer_index >= sizeof(state_buffer)) {
							if (fwrite(state_buffer, sizeof(*state_buffer), (sizeof(state_buffer) / sizeof(*state_buffer)), fp_restore) != sizeof(state_buffer)) {
								fputs("fwrite() failed\n", stderr);
								abort();
							}

							state_buffer_index = 0;
						}
					}
				}
			}
		}
	}

	if (state_buffer_index > 0) {
		if (fwrite(state_buffer, sizeof(*state_buffer), state_buffer_index, fp_restore) != state_buffer_index) {
			perror("fwrite()");
			abort();
		}
	}

	if (state_root != NULL) {
		compy_node_free(state_root, 1);
	}

	return COMPY_E_SUCCESS;
}
