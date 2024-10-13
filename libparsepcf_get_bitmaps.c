/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_bitmaps(const void *file, size_t size,
                        const struct libparsepcf_table *table,
                        struct libparsepcf_bitmaps *meta)
{
	const char *text = file;
	size_t pos, glyph_pad;
	int msb = table->format & LIBPARSEPCF_BYTE;

	(void) size;

	if (table->size < 8)
		goto ebfont;

	pos = table->offset;

	if (table->format != libparsepcf_parse_lsb_uint32__(&text[pos]))
		goto ebfont;
	pos += 4;

	meta->glyph_count = (size_t)PARSE_UINT32(&text[pos], msb);
	pos += 4;

	if (16 > table->size - (pos - table->offset) ||
	    meta->glyph_count > (table->size - (pos - table->offset) - 16) / 4)
		goto ebfont;
	pos += meta->glyph_count * 4;
	glyph_pad = (size_t)(table->format & LIBPARSEPCF_GLYPH_PAD_MASK);
	meta->bitmap_size = (size_t)PARSE_UINT32(&text[pos + 4 * glyph_pad], msb);
	pos += 16;

	meta->bitmap_data = (const void *)&text[pos];
	if (meta->bitmap_size > table->size - (pos - table->offset))
		goto ebfont;

	meta->bit_packing = (size_t)1 << ((table->format & LIBPARSEPCF_SCAN_UNIT_MASK) >> 4);
	meta->row_padding = (size_t)1 << ((table->format & LIBPARSEPCF_GLYPH_PAD_MASK) >> 0);
	meta->msbyte_first = !!(table->format & LIBPARSEPCF_BYTE);
	meta->msbit_first = !!(table->format & LIBPARSEPCF_BIT);

	if (meta->row_padding < meta->bit_packing)
		goto ebfont;

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
