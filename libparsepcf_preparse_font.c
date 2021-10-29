/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_preparse_font(const char *file, size_t size, struct libparsepcf_font *font)
{
	size_t min = SIZE_MAX, max = 0, table_i, table_n;
	struct libparsepcf_table *tables = NULL;

	if (libparsepcf_get_table_count(file, size, &table_n))
		goto fail;
	tables = calloc(table_n, sizeof(*tables));
	if (!tables)
		goto fail;
	if (libparsepcf_get_tables(file, size, tables, 0, table_n))
		goto fail;

	for (table_i = 0; table_i < table_n; table_i++) {
		if (tables[table_i].type == LIBPARSEPCF_PROPERTIES) {
			font->prob_table = &tables[table_i];
			if (libparsepcf_get_properties(file, size, &tables[table_i], &font->props))
				goto fail;
		} else if (tables[table_i].type == LIBPARSEPCF_ACCELERATORS && !font->accel_table) {
			font->accel_table = &tables[table_i];
			if (libparsepcf_get_accelerators(file, size, &tables[table_i], &font->accels))
				goto fail;
		} else if (tables[table_i].type == LIBPARSEPCF_BDF_ACCELERATORS) {
			font->accel_table = &tables[table_i];
			if (libparsepcf_get_accelerators(file, size, &tables[table_i], &font->accels))
				goto fail;
		} else if (tables[table_i].type == LIBPARSEPCF_METRICS) {
			font->mtx_table = &tables[table_i];
			if (libparsepcf_get_metrics_count(file, size, &tables[table_i], &font->metrics))
				goto fail;
		} else if (tables[table_i].type == LIBPARSEPCF_INK_METRICS) {
			font->inkmtx_table = &tables[table_i];
			if (libparsepcf_get_metrics_count(file, size, &tables[table_i], &font->ink_metrics))
				goto fail;
		} else if (tables[table_i].type == LIBPARSEPCF_BDF_ENCODINGS) {
			font->enc_table = &tables[table_i];
			if (libparsepcf_get_encoding(file, size, &tables[table_i], &font->encoding))
				goto fail;
		} else if (tables[table_i].type == LIBPARSEPCF_BITMAPS) {
			font->bitmap_table = &tables[table_i];
			if (libparsepcf_get_bitmaps(file, size, &tables[table_i], &font->bitmaps))
				goto fail;
		} else if (tables[table_i].type == LIBPARSEPCF_GLYPH_NAMES) {
			font->name_table = &tables[table_i];
			if (libparsepcf_get_glyph_names(file, size, &tables[table_i], &font->names))
				goto fail;
		} else if (tables[table_i].type == LIBPARSEPCF_SWIDTHS) {
			font->swidth_table = &tables[table_i];
			if (libparsepcf_get_swidth_count(file, size, &tables[table_i], &font->swidths))
				goto fail;
		}
	}

	if (font->mtx_table) {
		if (font->metrics < min)
			min = font->metrics;
		if (font->metrics > max)
			max = font->metrics;
	}
	if (font->inkmtx_table) {
		if (font->ink_metrics < min)
			min = font->ink_metrics;
		if (font->ink_metrics > max)
			max = font->ink_metrics;
	}
	if (font->bitmap_table) {
		if (font->bitmaps.glyph_count < min)
			min = font->bitmaps.glyph_count;
		if (font->bitmaps.glyph_count > max)
			max = font->bitmaps.glyph_count;
	}
	if (font->name_table) {
		if (font->names.glyph_count < min)
			min = font->names.glyph_count;
		if (font->names.glyph_count > max)
			max = font->names.glyph_count;
	}
	if (font->swidth_table) {
		if (font->swidths < min)
			min = font->swidths;
		if (font->swidths > max)
			max = font->swidths;
	}

	if (min != max || !font->mtx_table || !font->enc_table)
		goto ebfont;
	if (!font->inkmtx_table && font->accel_table && font->accels.ink_metrics)
		goto ebfont;
	font->glyph_count = min;
	font->_tables = tables;

	return 0;

ebfont:
	errno = EBFONT;
fail:
	free(tables);
	return -1;
}
