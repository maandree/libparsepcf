/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_swidth_count(const char *file, size_t size, const struct libparsepcf_table *table, size_t *countp)
{
	size_t pos;
	int msb = table->format & LIBPARSEPCF_BYTE;

	if (table->size < 8)
		goto ebfont;

	pos = table->offset;

	if (table->format != libparsepcf_parse_lsb_uint32__(&file[pos]))
		goto ebfont;
	pos += 4;

	*countp = (size_t)PARSE_UINT32(&file[pos], msb);
	pos += 4;

	if (*countp > (size - pos) / 4)
		goto ebfont;

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
