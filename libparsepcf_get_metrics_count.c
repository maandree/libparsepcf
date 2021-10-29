/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_metrics_count(const char *file, size_t size, const struct libparsepcf_table *table, size_t *countp)
{
	size_t pos;
	int msb = table->format & LIBPARSEPCF_BYTE;
	int compressed = table->format & LIBPARSEPCF_COMPRESSED_METRICS;

	if (table->size < (compressed ? 6 : 8))
		goto ebfont;

	pos = table->offset;

	if (table->format != libparsepcf_parse_lsb_uint32__(&file[pos]))
		goto ebfont;
	pos += 4;

	if (compressed) {
		*countp = (size_t)PARSE_UINT16(&file[pos], msb);
		pos += 2;
		if (*countp > (size - pos) / 5)
			goto ebfont;
	} else {
		*countp = (size_t)PARSE_UINT32(&file[pos], msb);
		pos += 4;
		if (*countp > (size - pos) / 12)
			goto ebfont;
	}

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
