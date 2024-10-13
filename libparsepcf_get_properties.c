/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_properties(const void *file, size_t size,
                           const struct libparsepcf_table *table,
                           struct libparsepcf_properties *meta)
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

	meta->property_count = (size_t)PARSE_UINT32(&text[pos], msb);
	pos += 4;

	if (4 > table->size - (pos - table->offset) || 
	    meta->property_count > (table->size - (pos - table->offset) - 4) / 9)
		goto ebfont;
	pos += meta->property_count * 9;
	pos += (4 - (meta->property_count & 3)) & 3;
	if (pos - table->offset > table->size - 4)
		goto ebfont;
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
