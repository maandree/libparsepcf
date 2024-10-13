/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_glyph_name_subtable(const void *file, size_t size,
                                    const struct libparsepcf_table *table,
                                    const struct libparsepcf_glyph_names *meta,
                                    const char **names, size_t first, size_t count)
{
	const char *text = file;
	size_t pos, i, off;
	int msb = table->format & LIBPARSEPCF_BYTE;

	(void) size;

	if (table->size < 8)
		goto ebfont;

	pos = table->offset + 8 + first * 4;
	for (i = 0; i < count; i++, pos += 4) {
		off = (size_t)PARSE_UINT32(&text[pos], msb);
		if (off > meta->strings_size)
			goto ebfont;
		names[i] = &meta->strings[off];
		if (!memchr(names[i], 0, meta->strings_size - off))
			goto ebfont;
	}

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
