/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_metrics_count(const void *file, size_t size,
                              const struct libparsepcf_table *table,
                              size_t *count)
{
	const char *text = file;
	size_t pos;
	int msb = table->format & LIBPARSEPCF_BYTE;
	int compressed = table->format & LIBPARSEPCF_COMPRESSED_METRICS;

	if (table->size < (compressed ? 6 : 8))
		goto ebfont;

	pos = table->offset;

	if (table->format != libparsepcf_parse_lsb_uint32__(&text[pos]))
		goto ebfont;
	pos += 4;

	if (compressed) {
		*count = (size_t)PARSE_UINT16(&text[pos], msb);
		pos += 2;
		if (*count > (size - pos) / 5)
			goto ebfont;
	} else {
		*count = (size_t)PARSE_UINT32(&text[pos], msb);
		pos += 4;
		if (*count > (size - pos) / 12)
			goto ebfont;
	}

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
