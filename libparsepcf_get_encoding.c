/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_encoding(const void *file, size_t size,
                         const struct libparsepcf_table *table,
                         struct libparsepcf_encoding *meta)
{
	const char *text = file;
	int msb = table->format & LIBPARSEPCF_BYTE;
	size_t pos;

	(void) size;

	if (table->size < 14)
		goto ebfont;

	pos = table->offset;

	if (table->format != libparsepcf_parse_lsb_uint32__(&text[pos]))
		goto ebfont;
	pos += 4;

	meta->min_byte2 = PARSE_UINT16(&text[pos + 0], msb);
	meta->max_byte2 = PARSE_UINT16(&text[pos + 2], msb);
	meta->min_byte1 = PARSE_UINT16(&text[pos + 4], msb);
	meta->max_byte1 = PARSE_UINT16(&text[pos + 6], msb);
	meta->default_glyph = PARSE_UINT16(&text[pos + 8], msb);
	pos += 10;

	if (meta->min_byte2 > meta->max_byte2 || meta->max_byte2 > 255 ||
	    meta->min_byte1 > meta->max_byte1 || meta->max_byte1 > 255)
		goto ebfont;

	meta->char_count  = (size_t)(meta->max_byte2 - meta->min_byte2 + 1);
	meta->char_count *= (size_t)(meta->max_byte1 - meta->min_byte1 + 1);
	if (meta->char_count > table->size - (pos - table->offset) / 2)
		goto ebfont;

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
