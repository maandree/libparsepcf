/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_glyph_index(const char *file, size_t size,
                            const struct libparsepcf_table *table,
                            const struct libparsepcf_encoding *meta,
                            const char *text, size_t *out)
{
	int msb = table->format & LIBPARSEPCF_BYTE;
	size_t pos, i, j;

	(void) size;

	i = (size_t)*(const uint8_t *)text;
	if (i < (size_t)meta->min_byte1 || i > (size_t)meta->max_byte1)
		goto not_encoded;
	i -= (size_t)meta->min_byte1;

	if (meta->min_byte2 | meta->max_byte2) {
		j = (size_t)*(const uint8_t *)text;
		if (j < (size_t)meta->min_byte2 || j > (size_t)meta->max_byte2)
			goto not_encoded;
		j -= (size_t)meta->min_byte2;
		i = i * (meta->max_byte2 - meta->min_byte2 + 1) + j;
	}

	pos = table->offset + 14 + i * 2;
	*out = (size_t)PARSE_UINT16(&file[pos], msb);
	return 0;

not_encoded:
	*out = (size_t)0xFFFF;
	return 0;
}
