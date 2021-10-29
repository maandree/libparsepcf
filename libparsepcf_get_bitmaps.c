/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_bitmaps(const char *file, size_t size,
                        const struct libparsepcf_table *table,
                        struct libparsepcf_bitmaps *out)
{
	size_t pos, glyph_pad;
	int msb = table->format & LIBPARSEPCF_BYTE;

	(void) size;

	if (table->size < 8)
		goto ebfont;

	pos = table->offset;

	if (table->format != libparsepcf_parse_lsb_uint32__(&file[pos]))
		goto ebfont;
	pos += 4;

	out->glyph_count = (size_t)PARSE_UINT32(&file[pos], msb);
	pos += 4;

	if (16 > table->size - (pos - table->offset) ||
	    out->glyph_count > (table->size - (pos - table->offset) - 16) / 4)
		goto ebfont;
	pos += out->glyph_count * 4;
	glyph_pad = (size_t)(table->format & LIBPARSEPCF_GLYPH_PAD_MASK);
	out->bitmap_size = (size_t)PARSE_UINT32(&file[pos + 4 * glyph_pad], msb);
	pos += 16;

	out->bitmap_data = (const void *)&file[pos];
	if (out->bitmap_size > table->size - (pos - table->offset))
		goto ebfont;

	out->bit_packing = (size_t)1 << ((table->format & LIBPARSEPCF_SCAN_UNIT_MASK) >> 4);
	out->row_padding = (size_t)1 << ((table->format & LIBPARSEPCF_GLYPH_PAD_MASK) >> 0);
	out->lsbyte = !!(table->format & LIBPARSEPCF_BYTE);
	out->lsbit = !!(table->format & LIBPARSEPCF_BIT);

	if (out->row_padding < out->bit_packing)
		goto ebfont;

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
