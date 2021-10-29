/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_table_count(const char *file, size_t size, size_t *countp)
{
	uint32_t count;

	if (size < 8 || file[0] != 1 || file[1] != 'f' || file[2] != 'c' || file[3] != 'p')
		goto ebfont;

	count = libparsepcf_parse_lsb_uint32__(&file[4]);
	if (count > (size - 8) / 16)
		goto ebfont;

	*countp = (size_t)count;
	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
