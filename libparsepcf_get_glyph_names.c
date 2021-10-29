/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_glyph_names(const char *file, size_t size,
                            const struct libparsepcf_table *table,
                            struct libparsepcf_glyph_names *out)
{
	size_t pos;
	int msb = table->format & LIBPARSEPCF_BYTE;

	(void) size;

	if (table->size < 8)
		goto ebfont;

	pos = table->offset;

	if (table->format != libparsepcf_parse_lsb_uint32__(&file[pos]))
		goto ebfont;
	pos += 4;

	out->glyph_count = (size_t)PARSE_UINT32(&file[pos], msb);
	pos += 4;

	if (4 > table->size - (pos - table->offset) ||
	    out->glyph_count > (table->size - (pos - table->offset) - 4) / 4)
		goto ebfont;
	pos += out->glyph_count * 4;
	out->strings_size = (size_t)PARSE_UINT32(&file[pos], msb);
	pos += 4;

	out->strings = &file[pos];
	if (out->strings_size > table->size - (pos - table->offset))
		goto ebfont;

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
