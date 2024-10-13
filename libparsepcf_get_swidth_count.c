/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_swidth_count(const void *file, size_t size,
                             const struct libparsepcf_table *table,
                             size_t *count)
{
	const char *text = file;
	size_t pos;
	int msb = table->format & LIBPARSEPCF_BYTE;

	if (table->size < 8)
		goto ebfont;

	pos = table->offset;

	if (table->format != libparsepcf_parse_lsb_uint32__(&text[pos]))
		goto ebfont;
	pos += 4;

	*count = (size_t)PARSE_UINT32(&text[pos], msb);
	pos += 4;

	if (*count > (size - pos) / 4)
		goto ebfont;

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
