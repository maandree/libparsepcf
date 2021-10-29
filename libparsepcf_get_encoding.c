/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_encoding(const char *file, size_t size,
                         const struct libparsepcf_table *table,
                         struct libparsepcf_encoding *out)
{
	int msb = table->format & LIBPARSEPCF_BYTE;
	size_t pos;

	(void) size;

	if (table->size < 14)
		goto ebfont;

	pos = table->offset;

	if (table->format != libparsepcf_parse_lsb_uint32__(&file[pos]))
		goto ebfont;
	pos += 4;

	out->min_byte2 = PARSE_UINT16(&file[pos + 0], msb);
	out->max_byte2 = PARSE_UINT16(&file[pos + 2], msb);
	out->min_byte1 = PARSE_UINT16(&file[pos + 4], msb);
	out->max_byte1 = PARSE_UINT16(&file[pos + 6], msb);
	out->default_char = PARSE_UINT16(&file[pos + 8], msb);
	pos += 10;

	if (out->min_byte2 > out->max_byte2 || out->max_byte2 > 255 ||
	    out->min_byte1 > out->max_byte1 || out->max_byte1 > 255)
		goto ebfont;

	out->glyph_count = (size_t)(out->max_byte2 - out->min_byte2 + 1) * (size_t)(out->max_byte1 - out->min_byte1 + 1);
	if (out->glyph_count > table->size - (pos - table->offset) / 2)
		goto ebfont;

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
