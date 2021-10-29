/* See LICENSE file for copyright and license details. */
#include "common.h"



int
libparsepcf_get_accelerators(const char *file, size_t size,
                             const struct libparsepcf_table *table,
                             struct libparsepcf_accelerators *out)
{
	int msb = table->format & LIBPARSEPCF_BYTE;
	int with_ink_bounds = table->format & LIBPARSEPCF_ACCELERATOR_WITH_INK_BOUNDS;
	size_t pos;

	(void) size;

	if (table->size < 24 + (with_ink_bounds ? 4 : 2) * 12)
		goto ebfont;

	pos = table->offset;

	if (table->format != libparsepcf_parse_lsb_uint32__(&file[pos]))
		goto ebfont;
	pos += 4;

	out->no_overlap       = !!file[pos + 0];
	out->constant_metrics = !!file[pos + 1];
	out->terminal_font    = !!file[pos + 2];
	out->constant_width   = !!file[pos + 3];
	out->ink_inside       = !!file[pos + 4];
	out->ink_metrics      = !!file[pos + 5];
	out->draw_rtl         = !!file[pos + 6];
	out->have_ink_bounds  = !!with_ink_bounds;
	pos += 8;

	out->font_ascent  = PARSE_INT32(&file[pos + 0], msb);
	out->font_descent = PARSE_INT32(&file[pos + 4], msb);
	out->max_overlap  = PARSE_INT32(&file[pos + 8], msb);
	pos += 12;

	out->min_bounds.left_side_bearing    = PARSE_INT16(&file[pos + 0], msb);
	out->min_bounds.right_side_bearing   = PARSE_INT16(&file[pos + 2], msb);
	out->min_bounds.character_width      = PARSE_INT16(&file[pos + 4], msb);
	out->min_bounds.character_ascent     = PARSE_INT16(&file[pos + 6], msb);
	out->min_bounds.character_descent    = PARSE_INT16(&file[pos + 8], msb);
	out->min_bounds.character_attributes = PARSE_UINT16(&file[pos + 10], msb);
	pos += 12;

	out->max_bounds.left_side_bearing    = PARSE_INT16(&file[pos + 0], msb);
	out->max_bounds.right_side_bearing   = PARSE_INT16(&file[pos + 2], msb);
	out->max_bounds.character_width      = PARSE_INT16(&file[pos + 4], msb);
	out->max_bounds.character_ascent     = PARSE_INT16(&file[pos + 6], msb);
	out->max_bounds.character_descent    = PARSE_INT16(&file[pos + 8], msb);
	out->max_bounds.character_attributes = PARSE_UINT16(&file[pos + 10], msb);
	pos += 12;

	if (with_ink_bounds) {
		out->min_ink_bounds.left_side_bearing    = PARSE_INT16(&file[pos + 0], msb);
		out->min_ink_bounds.right_side_bearing   = PARSE_INT16(&file[pos + 2], msb);
		out->min_ink_bounds.character_width      = PARSE_INT16(&file[pos + 4], msb);
		out->min_ink_bounds.character_ascent     = PARSE_INT16(&file[pos + 6], msb);
		out->min_ink_bounds.character_descent    = PARSE_INT16(&file[pos + 8], msb);
		out->min_ink_bounds.character_attributes = PARSE_UINT16(&file[pos + 10], msb);
		pos += 12;

		out->max_ink_bounds.left_side_bearing    = PARSE_INT16(&file[pos + 0], msb);
		out->max_ink_bounds.right_side_bearing   = PARSE_INT16(&file[pos + 2], msb);
		out->max_ink_bounds.character_width      = PARSE_INT16(&file[pos + 4], msb);
		out->max_ink_bounds.character_ascent     = PARSE_INT16(&file[pos + 6], msb);
		out->max_ink_bounds.character_descent    = PARSE_INT16(&file[pos + 8], msb);
		out->max_ink_bounds.character_attributes = PARSE_UINT16(&file[pos + 10], msb);
		pos += 12;
	} else {
		memcpy(&out->min_ink_bounds, &out->min_bounds, sizeof(out->min_bounds));
		memcpy(&out->max_ink_bounds, &out->max_bounds, sizeof(out->max_bounds));
	}

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
