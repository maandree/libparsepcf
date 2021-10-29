/* See LICENSE file for copyright and license details. */
#ifndef LIBPARSEPCF_H
#define LIBPARSEPCF_H

#include <stddef.h>
#include <stdint.h>


/* Based on documentation from FontForge: https://fontforge.org/docs/techref/pcf-format.html */


struct libparsepcf_table {
	uint32_t type;
	uint32_t format;
	uint32_t size;
	uint32_t offset;
};

/* Table types: */
#define LIBPARSEPCF_PROPERTIES       (UINT32_C(1) << 0)
#define LIBPARSEPCF_ACCELERATORS     (UINT32_C(1) << 1)
#define LIBPARSEPCF_METRICS          (UINT32_C(1) << 2)
#define LIBPARSEPCF_BITMAPS          (UINT32_C(1) << 3)
#define LIBPARSEPCF_INK_METRICS      (UINT32_C(1) << 4)
#define LIBPARSEPCF_BDF_ENCODINGS    (UINT32_C(1) << 5)
#define LIBPARSEPCF_SWIDTHS          (UINT32_C(1) << 6)
#define LIBPARSEPCF_GLYPH_NAMES      (UINT32_C(1) << 7)
#define LIBPARSEPCF_BDF_ACCELERATORS (UINT32_C(1) << 8)

int libparsepcf_get_table_count(const char *, size_t, size_t *countp);
int libparsepcf_get_tables(const char *, size_t, struct libparsepcf_table *tables, size_t first, size_t count);



struct libparsepcf_properties {
	size_t property_count;
	size_t strings_size;
	const char *strings;
};

struct libparsepcf_property_subtable {
	const char *name;
	int is_string_property;
	union {
		int32_t signed_value;
		const char *string_value;
	} value;
};

int libparsepcf_get_properties(const char *, size_t,
                               const struct libparsepcf_table *,
                               struct libparsepcf_properties *);

int libparsepcf_get_property_subtable(const char *, size_t,
				      const struct libparsepcf_table *,
                                      const struct libparsepcf_properties *,
                                      struct libparsepcf_property_subtable *, size_t, size_t);



struct libparsepcf_metrics {
	int16_t left_side_bearing;
	int16_t right_side_bearing;
	int16_t character_width;
	int16_t character_ascent;
	int16_t character_descent;
	uint16_t character_attributes;
};

int libparsepcf_get_metrics_count(const char *, size_t, const struct libparsepcf_table *, size_t *);

int libparsepcf_get_metrics(const char *, size_t,
                            const struct libparsepcf_table *,
                            struct libparsepcf_metrics *, size_t, size_t);



struct libparsepcf_glyph_names {
	size_t glyph_count;
	size_t strings_size;
	const char *strings;
};

int libparsepcf_get_glyph_names(const char *, size_t,
                                const struct libparsepcf_table *,
                                struct libparsepcf_glyph_names *);

int libparsepcf_get_glyph_name_subtable(const char *, size_t,
                                        const struct libparsepcf_table *,
                                        const struct libparsepcf_glyph_names *,
                                        const char **, size_t, size_t);



struct libparsepcf_bitmaps {
	size_t glyph_count;
	size_t bitmap_size;
	size_t bit_packing;
	size_t row_padding;
	int lsbyte;
	int lsbit;
	const uint8_t *bitmap_data;
};

int libparsepcf_get_bitmaps(const char *, size_t,
                            const struct libparsepcf_table *,
                            struct libparsepcf_bitmaps *);

int libparsepcf_get_bitmap_offsets(const char *, size_t,
                                   const struct libparsepcf_table *,
                                   const struct libparsepcf_bitmaps *,
                                   size_t *, size_t, size_t);



struct libparsepcf_encoding {
	uint16_t min_byte2;
	uint16_t max_byte2;
	uint16_t min_byte1;
	uint16_t max_byte1;
	uint16_t default_char;
	size_t glyph_count;
	/* If min_byte1 == 0 and max_byte1 == 0 (single byte encoding):
	 *   glyph_index = glyph_indices[encoding - min_char_or_byte2]
	 *   not included if encoding > max_byte2 or glyph_index == 0xFFFF
	 * 
	 * Otherwise (dual byte encoding):
	 *   [min_byte1, max_byte1] = allowed range of the more signficant of the 2 bytes (the first byte)
	 *   [min_byte2, max_byte2] = allowed range of the less signficant of the 2 bytes (the second byte)
	 *   e1 = encoding[0] - min_byte1
	 *   e2 = encoding[1] - min_byte2
	 *   glyph_index = glyph_indices[e1 * (max_byte2 - min_byte2 + 1) + e2]
	 *   not included if out of range or or glyph_index == 0xFFFF
	 */
};

#define LIBPARSEPCF_NOT_ENCODED ((size_t)0xFFFFUL)

int libparsepcf_get_encoding(const char *, size_t,
                             const struct libparsepcf_table *,
                             struct libparsepcf_encoding *);

int libparsepcf_get_glyph_indices(const char *, size_t,
                                  const struct libparsepcf_table *,
                                  const struct libparsepcf_encoding *,
                                  size_t *, size_t, size_t);



struct libparsepcf_accelerators {
	/**
	 * Whether metrics[i].right_side_bearing - metrics[i].character_width
	 * less than or equal to .min_bounds.left_side_bearing for all i
	 */
	uint8_t no_overlap : 1;
	uint8_t constant_metrics : 1;
	/**
	 * .constant_metrics and, for all characters, left side bearing = 0,
	 * right side bearing = character width, ascent = .font_ascent, and
	 * descent = .font_descent
	 */
	uint8_t terminal_font : 1;
	uint8_t constant_width : 1;
	/**
	 * Whether all inked bits are inside the glyph's box
	 */
	uint8_t ink_inside : 1;
	/**
	 * Whether the ink metrics differ from the metrics for some glyph
	 */
	uint8_t ink_metrics : 1;
	uint8_t draw_rtl : 1;
	/**
	 * If 0, .ink_min_bounds and .ink_max_bounds are just copies of
	 * .min_bounds and max_bounds
	 */
	uint8_t have_ink_bounds : 1;
	int32_t font_ascent;
	int32_t font_descent;
	int32_t max_overlap;
	struct libparsepcf_metrics min_bounds;
	struct libparsepcf_metrics max_bounds;
	struct libparsepcf_metrics min_ink_bounds;
	struct libparsepcf_metrics max_ink_bounds;
};

int libparsepcf_get_accelerators(const char *, size_t,
                                 const struct libparsepcf_table *,
                                 struct libparsepcf_accelerators *);



int libparsepcf_get_swidth_count(const char *, size_t, const struct libparsepcf_table *, size_t *);

int libparsepcf_get_swidths(const char *, size_t, const struct libparsepcf_table *, int32_t *, size_t, size_t);



struct libparsepcf_font {
	const struct libparsepcf_table *prob_table;
	struct libparsepcf_properties props;

	const struct libparsepcf_table *accel_table;
	struct libparsepcf_accelerators accels;

	const struct libparsepcf_table *mtx_table;
	size_t metrics;

	const struct libparsepcf_table *inkmtx_table;
	size_t ink_metrics;

	const struct libparsepcf_table *enc_table;
	struct libparsepcf_encoding encoding;

	const struct libparsepcf_table *bitmap_table;
	struct libparsepcf_bitmaps bitmaps;

	const struct libparsepcf_table *swidth_table;
	size_t swidths;
	
	const struct libparsepcf_table *name_table;
	struct libparsepcf_glyph_names names;

	size_t glyph_count;
	struct libparsepcf_table *_tables;
};

int libparsepcf_preparse_font(const char *, size_t, struct libparsepcf_font *);

void libparsepcf_destroy_preparsed_font(struct libparsepcf_font *);



#endif
