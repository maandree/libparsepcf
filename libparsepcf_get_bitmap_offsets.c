/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_bitmap_offsets(const char *file, size_t size,
                               const struct libparsepcf_table *table,
                               const struct libparsepcf_bitmaps *meta,
                               size_t *out, size_t first, size_t count)
{
	int msb = table->format & LIBPARSEPCF_BYTE;
	size_t pos = table->offset + 8 + first * 4;
	size_t i;

	(void) size;

	for (i = 0; i < count; i++, pos += 4) {
		out[i] = (size_t)PARSE_UINT32(&file[pos + 0], msb);
		if (out[i] > meta->bitmap_size)
			goto ebfont;
	}

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
