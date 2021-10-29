/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_tables(const char *file, size_t size, struct libparsepcf_table *tables, size_t first, size_t count)
{
	size_t pos = 8 + first * 16;
	size_t i;

	for (i = 0; i < count; i++, pos += 16) {
		tables[i].type   = libparsepcf_parse_lsb_uint32__(&file[pos +  0]);
		tables[i].format = libparsepcf_parse_lsb_uint32__(&file[pos +  4]);
		tables[i].size   = libparsepcf_parse_lsb_uint32__(&file[pos +  8]);
		tables[i].offset = libparsepcf_parse_lsb_uint32__(&file[pos + 12]);

		if ((size_t)tables[i].offset > size)
			goto ebfont;

		/* For some reasons files specify table sizes such that they
		 * actually except the boundary of the file, despite not using
		 * that much data. Don't including this fix, but instead check
		 * that the table size is not too large, will break your
		 * favourite fonts. */
		if ((size_t)tables[i].size > size - (size_t)tables[i].offset) {
#if SIZE_MAX > UINT32_MAX
			if (size - (size_t)tables[i].offset > (size_t)UINT32_MAX)
				goto ebfont;
#endif
			tables[i].size = (uint32_t)(size - (size_t)tables[i].offset);
		}
	}

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
