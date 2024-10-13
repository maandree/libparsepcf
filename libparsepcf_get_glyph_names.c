/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_glyph_names(const void *file, size_t size,
                            const struct libparsepcf_table *table,
                            struct libparsepcf_glyph_names *meta)
{
	const char *text = file;
	size_t pos;
	int msb = table->format & LIBPARSEPCF_BYTE;

	(void) size;

	if (table->size < 8)
		goto ebfont;

	pos = table->offset;

	if (table->format != libparsepcf_parse_lsb_uint32__(&text[pos]))
		goto ebfont;
	pos += 4;

	meta->glyph_count = (size_t)PARSE_UINT32(&text[pos], msb);
	pos += 4;

	if (4 > table->size - (pos - table->offset) ||
	    meta->glyph_count > (table->size - (pos - table->offset) - 4) / 4)
		goto ebfont;
	pos += meta->glyph_count * 4;
	meta->strings_size = (size_t)PARSE_UINT32(&text[pos], msb);
	pos += 4;

	meta->strings = &text[pos];
	if (meta->strings_size > table->size - (pos - table->offset))
		goto ebfont;

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
