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


	return SC_E_SUCCESS;
}


sc_result_t
sc_file_close(sc_file_t *const file) {
	if (file == NULL) {
		return SC_E_NULL;
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


	const sc_ll_node_t
		*const *const tree_lookup = (const sc_ll_node_t *const *const)context->tree_lookup,
		*tnode = NULL;

	sc_file_header_node_t *hnode = NULL;

	size_t
		nbits,
		data_idx;

	register unsigned int i, current;
	for (i = 256; i--;) {
		if (tree_lookup[i] != NULL) {
			tnode = tree_lookup[i];
			hnode = &file->header.map[i];

			puts("");
			printf("currently processing %u @ %p / %p\n", i, tnode, hnode);

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

				current <<= 1;
				if ((tnode->flags & SC_LL_LEFT) == SC_LL_LEFT) {
					current |= 1;
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

					printf("%X ", hnode->data[data_idx]);

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

				printf("%X", hnode->data[data_idx-1]);
			}

			hnode->nbits = nbits;

			printf("\nused %u bit%s\n", nbits, ((nbits == 1) ? "" : "s"));
		}
	}


	if (fwrite("header\n", 1, 7, file->fp) != 7) {
		return SC_E_IO;
	}


	return SC_E_SUCCESS;
}
