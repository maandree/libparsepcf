/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_swidths(const void *file, size_t size,
                        const struct libparsepcf_table *table,
                        int32_t *widths, size_t first, size_t count)
{
	const char *text = file;
	int msb = table->format & LIBPARSEPCF_BYTE;
	size_t pos = table->offset + 8 + 4 * first;
	size_t i;

	(void) size;

	for (i = 0; i < count; i++, pos += 4)
		widths[i] = PARSE_INT32(&text[pos], msb);

	return 0;
}
