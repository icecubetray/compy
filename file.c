#include "./file.h"

#include <string.h>
#include <stdlib.h>




const static uint8_t __file_magic[4] = { 0x20, 0x16, 0x11, 0x27 };




sc_result_t
sc_file_open(sc_file_t *const restrict file, const char *const restrict path, const unsigned int truncate) {
	if (file == NULL || path == NULL) {
		return SC_E_NULL;
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
		return SC_E_IO;
	}


	/* Clear out the file structure, store the file pointer, reset the state. */
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


	/* Check if we have bits left to flush. */
	if (file->last_bits > 0) {
		if (file->fp == NULL) {
			return SC_E_IO;
		}

		/* Fetch the bits that we still need to write, and write them. */
		uint8_t byte = (uint8_t)(file->last_byte & ((1 << file->last_bits) - 1));
		if (fwrite(&byte, sizeof(byte), 1, file->fp) != sizeof(byte)) {
			puts("fwrite() failed to write last byte");
			return SC_E_IO;
		}

		/* Since we had a partial write, we have to give a value to the trim byte so we
		** can determine what bits to read from the last byte.
		** The trim byte is the fifth byte in the header, which we initially wrote as a
		** zero to the file. Set it to the number of bits we wrote just now. */
		fseek(file->fp, 4, SEEK_SET);
		byte = (uint8_t)file->last_bits;
		if (fwrite(&byte, sizeof(byte), 1, file->fp) != sizeof(byte)) {
			puts("fwrite() failed to write trim byte");
			return SC_E_IO;
		}

		/* Clear the partial write state. */
		file->last_bits = file->last_byte = 0;
	}


	/* Close the underlying file, if one is opened. */
	if (file->fp != NULL) {
		if (fclose(file->fp) == 0) {
			file->fp = NULL;
		} else {
			return SC_E_IO;
		}
	}


	/* Clear out the file instance, everything went OK. */
	memset(file, 0, sizeof(*file));


	return SC_E_SUCCESS;
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
static __wrtree(FILE *fp, uint8_t *byte, unsigned int *index, sc_ll_node_t *node) {
	if (node == NULL) {
		return;
	}

	if ((node->flags & SC_LL_LEAF) == SC_LL_LEAF) {
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

	if (file->state != SC_FILE_STATE_IDLE && file->state != SC_FILE_STATE_WR_HEADER) {
		return SC_E_STATE;
	}


	unsigned int populated;
	register unsigned int i;
	sc_file_header_node_t *hnode = NULL;

	/* Check if the mapping is empty, if so populate it. */
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
		return SC_E_IO;
	}


	/* Write the tree to the file. */
	uint8_t byte = 0;
	unsigned int index = 0;
	__wrtree(file->fp, &byte, &index, context->tree_root);
	file->last_bits = index;
	file->last_byte = byte;


	/* Flush the header. */
	if (fflush(file->fp) != 0) {
		return SC_E_IO;
	}


	/* Keep track of what we're doing with this instance. */
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

	if (file->fp == NULL || ferror(file->fp) != 0) {
		return SC_E_IO;
	}

	if (file->state != SC_FILE_STATE_WR_HEADER && file->state != SC_FILE_STATE_WR_DATA) {
		return SC_E_STATE;
	}

	if (file->header.populated == 0) {
		return SC_E_NOT_READY;
	}


	const sc_file_header_node_t *node = NULL;
	const uint8_t *const data8 = (const uint8_t *const)data;

	uint8_t buffer[512];

	register unsigned int
		i, jo,
		nbits, rbits, wbits, cbits,
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


		index = ((nbits - 1) / 8);
		cbits = ~0;
		rbits = (bits + nbits);

		while (rbits >= 8) {
			wbits = (8 - bits);
			cbits = ((nbits - wbits) % 8);

			if (bits > 0) {
				if ((cbits + wbits) > 8) {
					byte <<= wbits;
					byte |= ((node->data[index] & ((1 << (nbits % 8)) - 1)) << (8 - cbits));
					byte |= ((node->data[--index] & (((1 << (wbits - (nbits % 8))) - 1) << cbits)) >> cbits);
				} else {
					byte = ((byte << wbits) | ((node->data[index] & (((1 << wbits) - 1) << cbits)) >> cbits));
				}
				bits = 0;
				nbits -= wbits;

				if ((nbits & 7) == 0) {
					--index;
				}
			} else if (nbits >= 8) {
				if (cbits > 0) {
					wbits = (8 - cbits);
					byte = (((node->data[index] & ((1 << cbits) - 1)) << wbits) | ((node->data[--index] & (((1 << wbits) - 1) << cbits)) >> cbits));
				} else if (wbits == 8) {
					byte = node->data[index--];
				} else {
					byte = 0;
				}
				nbits -= 8;
			} else {
				rbits -= 8;
				break;
			}


			buffer[buffer_index] = (uint8_t)byte;
			byte = 0;

			if (++buffer_index >= sizeof(buffer)) {
				buffer_index = 0;
				if (fwrite(buffer, sizeof(*buffer), (sizeof(buffer) / sizeof(*buffer)), file->fp) != sizeof(buffer)) {
					perror("fwrite()");
					abort();
				}
			}

			rbits -= 8;
		}

		if (rbits > 0) {
			bits = rbits;
			byte = ((byte << nbits) | ((node->data[index] & (((1 << nbits) - 1) << 0)) >> 0));
		} else {
			bits = byte = 0;
		}
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


	/* Keep track of what we're doing with this instance. */
	file->state = SC_FILE_STATE_WR_DATA;


	return SC_E_SUCCESS;
}




#define SC_DECODE_NODE 0
#define SC_DECODE_VALUE 1
#define SC_DECODE_DATA 2

struct sc_file_decode_state {
	FILE *fp;
	sc_ll_node_t *root;
	sc_ll_node_t *last_parent;
	sc_ll_node_t *last_lookup;
	size_t buffer_index;
	uint8_t buffer[2048];
	unsigned int state;
	unsigned int bits_left;
	uint8_t bits;
};

void static __process(struct sc_file_decode_state *const state, uint8_t bit);

sc_result_t
sc_file_restore(sc_file_t *file, FILE *fp_restore) {
	// TODO: sanity


	rewind(file->fp);


	size_t magic_read = 0;
	uint8_t magic[4];

	size_t trim_read = 0;
	uint8_t trim = 0;


	register unsigned int i, j, end;
	size_t read;
	uint8_t buffer[2048], byte;

	struct sc_file_decode_state state;
	memset(&state, 0, sizeof(state));
	state.fp = fp_restore;

	while ((read = fread(buffer, sizeof(*buffer), (sizeof(buffer) / sizeof(*buffer)), file->fp)) > 0) {
		/* Reset cursor and end marker. */
		i = 0;
		end = (unsigned int)read;

		/* Ensure we have magic. */
		for (; magic_read < 4 && i < end; ++i) {
			magic[magic_read] = buffer[i];

			if (++magic_read == 4) {
				if (memcmp(magic, __file_magic, sizeof(__file_magic)) != 0) {
					return SC_E_DATA;
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
				__process(&state, ((byte >> j) & 0x01));
			}
		}
	}

	if (state.buffer_index > 0) {
		if (fwrite(state.buffer, sizeof(*state.buffer), state.buffer_index, state.fp) != state.buffer_index) {
			perror("fwrite()");
			abort();
		}
	}

	if (state.root != NULL) {
		sc_ll_node_free(state.root, 1);
	}

	return SC_E_SUCCESS;
}


void
static
__process(struct sc_file_decode_state *const state, uint8_t bit) {
	if (state->state == SC_DECODE_NODE) {
		if (bit == 1) {
			state->state = SC_DECODE_VALUE;
			state->bits_left = 8;
			state->bits = 0;
		} else {
			if (state->root == NULL) {
				state->root = sc_ll_node_alloc(0, 0, 0);
				state->last_lookup = state->root;
			}
			if (state->last_parent == NULL) {
				state->last_parent = state->root;
			} else {
				if (state->last_parent->left == NULL) {
					sc_ll_node_t *node = sc_ll_node_alloc(0, 0, 0);
					node->parent = state->last_parent;
					state->last_parent->left = node;
					state->last_parent = node;
				} else if (state->last_parent->right == NULL) {
					sc_ll_node_t *node = sc_ll_node_alloc(0, 0, 0);
					node->parent = state->last_parent;
					state->last_parent->right = node;
					state->last_parent = node;
				} else {
					while ((state->last_parent = state->last_parent->parent) != NULL) {
						if (state->last_parent->left == NULL || state->last_parent->right == NULL) {
							break;
						}
					}
					if (state->last_parent == state->root) {
						// all full
						state->state = SC_DECODE_DATA;
					}
				}
			}
		}
	} else if (state->state == SC_DECODE_VALUE) {
		state->bits |= (bit << --state->bits_left);

		if (state->bits_left == 0) {
			sc_ll_node_t *node = sc_ll_node_alloc(0, state->bits, SC_LL_LEAF);
			if (state->last_parent->left == NULL) {
				state->last_parent->left = node;
			} else if (state->last_parent->right == NULL) {
				state->last_parent->right = node;

				while ((state->last_parent = state->last_parent->parent) != NULL) {
					if (state->last_parent->left == NULL || state->last_parent->right == NULL) {
						break;
					}
				}
			} else {
				fputs("not supposed to get here\n", stderr);
				abort();
			}

			state->state = ((state->last_parent == NULL) ? SC_DECODE_DATA : SC_DECODE_NODE);
		}
	} else if (state->state == SC_DECODE_DATA) {
		if (bit == 1) {
			state->last_lookup = state->last_lookup->left;
		} else if (bit == 0) {
			state->last_lookup = state->last_lookup->right;
		}

		if ((state->last_lookup->flags & SC_LL_LEAF) == SC_LL_LEAF) {
			state->buffer[state->buffer_index] = state->last_lookup->value;
			state->last_lookup = state->root;

			if (++state->buffer_index >= sizeof(state->buffer)) {
				if (fwrite(state->buffer, sizeof(*state->buffer), (sizeof(state->buffer) / sizeof(*state->buffer)), state->fp) != sizeof(state->buffer)) {
					fputs("fwrite() failed\n", stderr);
					abort();
				}

				state->buffer_index = 0;
			}
		}
	}
}
