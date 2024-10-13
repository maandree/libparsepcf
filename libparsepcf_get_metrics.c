/* See LICENSE file for copyright and license details. */
#include "common.h"


#define PARSE_INT8(TEXT) ((int16_t)((int)*(const uint8_t *)(TEXT) - 128))


int
libparsepcf_get_metrics(const void *file, size_t size,
                        const struct libparsepcf_table *table,
                        struct libparsepcf_metrics *metrics,
                        size_t first, size_t count)
{
	const char *text = file;
	size_t pos, i;
	int msb = table->format & LIBPARSEPCF_BYTE;
	int compressed = table->format & LIBPARSEPCF_COMPRESSED_METRICS;

	(void) size;

	pos = table->offset + (compressed ? 6 : 8);
	pos += first * (compressed ? 5 : 12);

	if (compressed) {
		for (i = 0; i < count; i++, pos += 5) {
			metrics[i].left_side_bearing  = PARSE_INT8(&text[pos + 0]);
			metrics[i].right_side_bearing = PARSE_INT8(&text[pos + 1]);
			metrics[i].character_width    = PARSE_INT8(&text[pos + 2]);
			metrics[i].character_ascent   = PARSE_INT8(&text[pos + 3]);
			metrics[i].character_descent  = PARSE_INT8(&text[pos + 4]);
			metrics[i].character_attributes = 0;
		}
	} else {
		for (i = 0; i < count; i++, pos += 12) {
			metrics[i].left_side_bearing    = PARSE_INT16(&text[pos + 0], msb);
			metrics[i].right_side_bearing   = PARSE_INT16(&text[pos + 2], msb);
			metrics[i].character_width      = PARSE_INT16(&text[pos + 4], msb);
			metrics[i].character_ascent     = PARSE_INT16(&text[pos + 6], msb);
			metrics[i].character_descent    = PARSE_INT16(&text[pos + 8], msb);
			metrics[i].character_attributes = PARSE_UINT16(&text[pos + 10], msb);
		}
	}

	for (i = 0; i < count; i++) {
		if (metrics[i].left_side_bearing > metrics[i].right_side_bearing ||
		    (int32_t)metrics[i].character_ascent < -(int32_t)metrics[i].character_descent)
			goto ebfont;
	}

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
