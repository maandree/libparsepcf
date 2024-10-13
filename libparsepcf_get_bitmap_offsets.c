/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_bitmap_offsets(const void *file, size_t size,
                               const struct libparsepcf_table *table,
                               const struct libparsepcf_bitmaps *meta,
                               size_t *offs, size_t first, size_t count)
{
	const char *text = file;
	int msb = table->format & LIBPARSEPCF_BYTE;
	size_t pos = table->offset + 8 + first * 4;
	size_t i;

	(void) size;

	for (i = 0; i < count; i++, pos += 4) {
		offs[i] = (size_t)PARSE_UINT32(&text[pos + 0], msb);
		if (offs[i] > meta->bitmap_size)
			goto ebfont;
	}

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
