/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_metrics(const char *file, size_t size,
                        const struct libparsepcf_table *table,
                        struct libparsepcf_metrics *out, size_t first, size_t count)
{
	size_t pos, i;
	int msb = table->format & LIBPARSEPCF_BYTE;
	int compressed = table->format & LIBPARSEPCF_COMPRESSED_METRICS;

	(void) size;

	pos = table->offset + (compressed ? 6 : 8);
	pos += first * (compressed ? 5 : 12);

	if (compressed) {
		for (i = 0; i < count; i++, pos += 5) {
			out[i].left_side_bearing  = (int16_t)((int)((const uint8_t *)file)[pos + 0] - 128);
			out[i].right_side_bearing = (int16_t)((int)((const uint8_t *)file)[pos + 1] - 128);
			out[i].character_width    = (int16_t)((int)((const uint8_t *)file)[pos + 2] - 128);
			out[i].character_ascent   = (int16_t)((int)((const uint8_t *)file)[pos + 3] - 128);
			out[i].character_descent  = (int16_t)((int)((const uint8_t *)file)[pos + 4] - 128);
			out[i].character_attributes = 0;
			if (out[i].left_side_bearing > out[i].right_side_bearing ||
			    (int32_t)out[i].character_ascent < -(int32_t)out[i].character_descent)
				goto ebfont;
		}
	} else {
		for (i = 0; i < count; i++, pos += 12) {
			out[i].left_side_bearing    = PARSE_INT16(&file[pos + 0], msb);
			out[i].right_side_bearing   = PARSE_INT16(&file[pos + 2], msb);
			out[i].character_width      = PARSE_INT16(&file[pos + 4], msb);
			out[i].character_ascent     = PARSE_INT16(&file[pos + 6], msb);
			out[i].character_descent    = PARSE_INT16(&file[pos + 8], msb);
			out[i].character_attributes = PARSE_UINT16(&file[pos + 10], msb);
			if (out[i].left_side_bearing > out[i].right_side_bearing ||
			    (int32_t)out[i].character_ascent < -(int32_t)out[i].character_descent)
				goto ebfont;
		}
	}

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
