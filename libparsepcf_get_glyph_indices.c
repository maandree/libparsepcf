/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_glyph_indices(const void *file, size_t size,
                              const struct libparsepcf_table *table,
                              const struct libparsepcf_encoding *meta,
                              size_t *indices, size_t first, size_t count)
{
	const char *text = file;
	int msb = table->format & LIBPARSEPCF_BYTE;
	size_t pos = table->offset + 14 + first * 2;
	size_t i;

	(void) size;
	(void) meta;

	for (i = 0; i < count; i++, pos += 2)
		indices[i] = (size_t)PARSE_UINT16(&text[pos], msb);

	return 0;
}
