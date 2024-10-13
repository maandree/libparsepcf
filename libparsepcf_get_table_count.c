/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_table_count(const void *file, size_t size, size_t *countp)
{
	const char *text = file;
	uint32_t count;

	if (size < 8 || text[0] != 1 || text[1] != 'f' || text[2] != 'c' || text[3] != 'p')
		goto ebfont;

	count = libparsepcf_parse_lsb_uint32__(&text[4]);
	if (count > (size - 8) / 16)
		goto ebfont;

	*countp = (size_t)count;
	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
