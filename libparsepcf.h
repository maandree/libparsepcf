/* See LICENSE file for copyright and license details. */
#ifndef LIBPARSEPCF_H
#define LIBPARSEPCF_H

#include <stddef.h>
#include <stdint.h>


/* Based on documentation from FontForge: https://fontforge.org/docs/techref/pcf-format.html */



/**
 * Table metadata
 */
struct libparsepcf_table {
	/**
	 * The table type, support values are:
	 * - LIBPARSEPCF_PROPERTIES
	 * - LIBPARSEPCF_ACCELERATORS
	 * - LIBPARSEPCF_METRICS
	 * - LIBPARSEPCF_BITMAPS
	 * - LIBPARSEPCF_INK_METRICS
	 * - LIBPARSEPCF_BDF_ENCODINGS
	 * - LIBPARSEPCF_SWIDTHS
	 * - LIBPARSEPCF_GLYPH_NAMES
	 * - LIBPARSEPCF_BDF_ACCELERATORS
	 * 
	 * (The set in these values are non-overlapping,
	 * however they cannot be combined.)
	 */
	uint32_t type;

	/**
	 * How the data in the table is formatted
	 * 
	 * This is only used internally, and supported
	 * values are not exposed the the user
	 */
	uint32_t format;

	/**
	 * The size of the table
	 */
	uint32_t size;

	/**
	 * The table's offset in the file
	 * 
	 * (The file is padded with NUL bytes to align
	 * each table to a 4 byte boundary)
	 */
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

/**
 * Get the number of tables in the file
 * 
 * @param   file    The file content
 * @param   size    The file size
 * @param   countp  Output parameter for the number of tables in the file
 * @return          0 on success, -1 on failure
 * 
 * @throws  EBFONT  The file is not a properly formatted PCF file
 */
int libparsepcf_get_table_count(const void *, size_t, size_t *);

/**
 * Get table metadata from the file
 * 
 * @param   file    The file content
 * @param   size    The file size
 * @param   tables  Output buffer for the table metadata
 * @param   first   The index of the first table to store in `tables`,
 *                  the first table in the file has the index 0
 * @param   count   The number of tables to store in `tables`;
 *                  may not exceed the number of tables as retrieved
 *                  by `libparsepcf_get_table_count` minus `first`
 * @return          0 on success, -1 on failure
 * 
 * @throws  EBFONT  The file is not a properly formatted PCF file
 */
int libparsepcf_get_tables(const void *, size_t,
                           struct libparsepcf_table *,
                           size_t, size_t);




/**
 * Property list
 */
struct libparsepcf_properties {
	/**
	 * The number of properties available
	 */
	size_t property_count;

	/**
	 * The size of `.strings`
	 * 
	 * Used internally to detect corruption
	 */
	size_t strings_size;

	/**
	 * Property names
	 * 
	 * This is a buffer which requires an offset
	 * table, that is found in the file, to parse
	 * 
	 * This is a pointer to data within the file
	 * 
	 * Intended for internal use
	 */
	const char *strings;
};

/**
 * Property name and value
 */
struct libparsepcf_property_subtable {
	/**
	 * The name of the property
	 */
	const char *name;

	/**
	 * 1 if the value is a string,
	 * 0 if the value is an integer
	 */
	int is_string_property;

	/**
	 * The value of the property,
	 * use `.value.signed_value` if `.is_string_property` is 0,
	 * use `.value.string_value` if `.is_string_property` is 1
	 */
	union {
		/**
		 * The value
		 * 
		 * Used if `.is_string_property` is 0
		 * (if the value is signed integer)
		 */
		int32_t signed_value;

		/**
		 * The value
		 * 
		 * Used if `.is_string_property` is 1
		 * (if the value is string)
		 */
		const char *string_value;
	} value;
};

/**
 * Get the properties available in the file
 * 
 * @param   file   The file content
 * @param   size   The file size
 * @param   table  The table in the file with the `.type` `LIBPARSEPCF_PROPERTIES`,
 *                 extracted using the `libparsepcf_get_tables` function
 * @param   meta   Output parameter for the list of properties
 * @return         0 on success, -1 on failure
 * 
 * @throws  EBFONT  The file is not a properly formatted PCF file
 */
int libparsepcf_get_properties(const void *, size_t,
                               const struct libparsepcf_table *,
                               struct libparsepcf_properties *);

/**
 * Get the names and values of properties in the file
 * 
 * The first four arguments shall be the same as
 * passed into `libparsepcf_get_properties`
 * 
 * @param   file   The file content
 * @param   size   The file size
 * @param   table  The table in the file with the `.type` `LIBPARSEPCF_PROPERTIES`,
 *                 extracted using the `libparsepcf_get_tables` function
 * @param   meta   List of properties
 * @param   props  Output parameter for the properties
 * @param   first  The index of the first property to store in `props`,
 *                 the first property in the file has the index 0
 * @param   count  The number of properties to store in `props`;
 *                 may not exceed the number of properties (`.property_count`)
 *                 as retrieved by `libparsepcf_get_properties` minus `first`
 * @return         0 on success, -1 on failure
 * 
 * @throws  EBFONT  The file is not a properly formatted PCF file
 */
int libparsepcf_get_property_subtable(const void *, size_t,
				      const struct libparsepcf_table *,
                                      const struct libparsepcf_properties *,
                                      struct libparsepcf_property_subtable *,
                                      size_t, size_t);




/**
 * Glyph metrics
 */
struct libparsepcf_metrics {
	/**
	 * The number of pixels right of (negative if left of)
	 * the origin (caret position) of the leftmost ink-on
	 * pixel in the glyph
	 */
	int16_t left_side_bearing;

	/**
	 * The number of pixels right of (negative if left of)
	 * the origin (caret position) of the rightmost ink-on
	 * pixel in the glyph
	 */
	int16_t right_side_bearing;

	/**
	 * The number of pixels the next characters origin shall
	 * be right of the current character's origin (that is
	 * the number of pixels the caret shall advance)
	 */
	int16_t character_width;

	/**
	 * The number of pixels the character ascend above
	 * the baseline (if non-positive, it's fully below
	 * the baseline)
	 */
	int16_t character_ascent;

	/**
	 * The number of pixels the character descend below
	 * the baseline (if non-positive, it's fully above
	 * the baseline)
	 */
	int16_t character_descent;

	/**
	 * Application-specific character attribute bits
	 */
	uint16_t character_attributes;
};

/**
 * Get number of glyph metrics available in the file
 * 
 * @param   file   The file content
 * @param   size   The file size
 * @param   table  The table in the file with the `.type` `LIBPARSEPCF_METRICS`
 *                 or `LIBPARSEPCF_INK_METRICS`, extracted using the
 *                 `libparsepcf_get_tables` function
 * @param   count  Output parameter for glyph metric count
 * @return         0 on success, -1 on failure
 * 
 * @throws  EBFONT  The file is not a properly formatted PCF file
 * 
 * The `LIBPARSEPCF_METRICS` table is used for layout information,
 * and the `LIBPARSEPCF_INK_METRICS` table is used for rendering
 * information; that is `LIBPARSEPCF_METRICS` contains the size
 * of the glyphs where as `LIBPARSEPCF_INK_METRICS` contains the
 * size of the glyph bitmaps.
 */
int libparsepcf_get_metrics_count(const void *, size_t,
                                  const struct libparsepcf_table *,
                                  size_t *);

/**
 * Get the metrics for glyphs
 * 
 * @param   file     The file content
 * @param   size     The file size
 * @param   table    The table in the file with the `.type` `LIBPARSEPCF_METRICS`
 *                   or `LIBPARSEPCF_INK_METRICS`, extracted using the
 *                   `libparsepcf_get_tables` function
 * @param   metrics  Output parameter for the glyph metrics
 * @param   first    The index of the first glyph's metrics to store in
 *                   `metrics`, the first glyph in the file has the index 0
 * @param   count    The number of glyph's metrics to store in `metrics`;
 *                   may not exceed the number of glyph as retrieved by
 *                   `libparsepcf_get_metrics_count` (stored in it's last
 *                   argument) minus `first`
 * @return           0 on success, -1 on failure
 * 
 * @throws  EBFONT  The file is not a properly formatted PCF file
 * 
 * The `LIBPARSEPCF_METRICS` table is used for layout information,
 * and the `LIBPARSEPCF_INK_METRICS` table is used for rendering
 * information; that is `LIBPARSEPCF_METRICS` contains the size
 * of the glyphs where as `LIBPARSEPCF_INK_METRICS` contains the
 * size of the glyph bitmaps.
 */
int libparsepcf_get_metrics(const void *, size_t,
                            const struct libparsepcf_table *,
                            struct libparsepcf_metrics *,
                            size_t, size_t);




/**
 * Glyph name list
 */
struct libparsepcf_glyph_names {
	/**
	 * The number of glyphs available
	 */
	size_t glyph_count;

	/**
	 * The size of `.strings`
	 * 
	 * Used internally to detect corruption
	 */
	size_t strings_size;

	/**
	 * Glyph names
	 * 
	 * This is a buffer which requires an offset
	 * table, that is found in the file, to parse
	 * 
	 * This is a pointer to data within the file
	 * 
	 * Primarily for internal use
	 */
	const char *strings;
};

/**
 * Get the PostScript glyph names available in the file
 * 
 * @param   file   The file content
 * @param   size   The file size
 * @param   table  The table in the file with the `.type` `LIBPARSEPCF_GLYPH_NAMES`,
 *                 extracted using the `libparsepcf_get_tables` function
 * @param   meta   Output parameter for the list of glyph names
 * @return         0 on success, -1 on failure
 * 
 * @throws  EBFONT  The file is not a properly formatted PCF file
 */
int libparsepcf_get_glyph_names(const void *, size_t,
                                const struct libparsepcf_table *,
                                struct libparsepcf_glyph_names *);

/**
 * Get the PostScript glyph names available in the file
 * 
 * The first four arguments shall be the same as
 * passed into `libparsepcf_get_glyph_names`
 * 
 * @param   file   The file content
 * @param   size   The file size
 * @param   table  The table in the file with the `.type` `LIBPARSEPCF_GLYPH_NAMES`,
 *                 extracted using the `libparsepcf_get_tables` function
 * @param   meta   List of glyph names
 * @param   names  Output parameter for the glyph names; each stored name
 *                 will be the pointer `file` with an offset
 * @param   first  The index of the first glyph's whose name to store in
 *                 `names`, the first glyph in the file has the index 0
 * @param   count  The number of glyph names to store in `names`;
 *                 may not exceed the number of glyph (`.glyph_count`)
 *                 as retrieved by `libparsepcf_get_glyph_names` minus `first`
 * @return         0 on success, -1 on failure
 * 
 * @throws  EBFONT  The file is not a properly formatted PCF file
 */
int libparsepcf_get_glyph_name_subtable(const void *, size_t,
                                        const struct libparsepcf_table *,
                                        const struct libparsepcf_glyph_names *,
                                        const char **, size_t, size_t);




/**
 * Glyph bitmap metadata
 */
struct libparsepcf_bitmaps {
	/**
	 * The number of glyph available
	 */
	size_t glyph_count;

	/**
	 * The size of `.bitmap_data`
	 */
	size_t bitmap_size;

	size_t bit_packing;

	/**
	 * The number of bytes each row is
	 * padded to align to
	 * 
	 * This will always be a power of two
	 */
	size_t row_padding;

	int lsbyte;
	int lsbit;
	const uint8_t *bitmap_data;
};

int libparsepcf_get_bitmaps(const void *, size_t,
                            const struct libparsepcf_table *,
                            struct libparsepcf_bitmaps *);

int libparsepcf_get_bitmap_offsets(const void *, size_t,
                                   const struct libparsepcf_table *,
                                   const struct libparsepcf_bitmaps *,
                                   size_t *, size_t, size_t);




/**
 * Glyph index lookup data
 * 
 * This structure describes how to lookup a glyph's index
 * based on the character's encoding. The properties
 * "CHARSET_REGISTRY" and "CHARSET_ENCODING" are used to
 * determine which character encoding scheme is used in
 * the font file.
 * 
 * This structure contains information for how to calculate
 * the codepoint (provided to `libparsepcf_get_glyph_indices`
 * to look up the index of a character's glyph) for a character.
 * The PCF font format support single-byte encodings and
 * dual-byte encodings (variable byte encodings are not
 * supported), if the fields `.min_byte1` and `.max_byte1`
 * are both set to 0, the file uses single-byte encoding,
 * otherwise it uses dual-byte encoding.
 * 
 * For single-byte encodings, the codepoint is calculated
 * by subtracting `.min_byte2` from the byte.
 * 
 * For dual-byte encodings, if the first byte is `byte1` and the
 * second byte is `byte2` for a character, the character's codepoint is
 * `(byte1-.min_byte1) * (.max_byte2-.min_byte2+1) + (byte2-.min_byte2)`,
 * or, using less operations, `byte1 * k + byte2 - m` where `k` and `m`
 * are precalculated as `k = .max_byte2 - .min_byte2 + 1` and
 * `m = .min_byte1 * k + .min_byte2`.
 */
struct libparsepcf_encoding {
	/**
	 * If the second byte (or only byte for single-byte
	 * encodings) is lower this value, this character
	 * has no glyph in the font
	 */
	uint16_t min_byte2;

	/**
	 * If the second byte (or only byte for single-byte
	 * encodings) is higher this value, this character
	 * has no glyph in the font
	 */
	uint16_t max_byte2;

	/**
	 * For dual-byte encodings, if the first byte is
	 * lower than this value, the character has no
	 * glyph in the font
	 * 
	 * Unused for single-byte encoding
	 */
	uint16_t min_byte1;

	/**
	 * For dual-byte encodings, if the first byte is
	 * higher than this value, the character has no
	 * glyph in the font
	 * 
	 * Unused for single-byte encoding
	 */
	uint16_t max_byte1;

	/**
	 * The index of the glyph to use when no glyph is
	 * available for a character
	 * 
	 * It value should _not_ be looked up with
	 * `libparsepcf_get_glyph_indices` but used as if
	 * `libparsepcf_get_glyph_indices` has already been
	 * called. In fact, the glyph reference by this
	 * field does not necessarily correspond to a
	 * character and thus may not be available in the
	 * encoding table
	 * 
	 * This name of this field is choosen for consistence
	 * with the documentation the implementation is based
	 * on, and is confusing, it ought to be called
	 * "default_glyph"
	 */
	uint16_t default_char;

	/**
	 * The number of entries available in the index table,
	 * this is not number of glyphs in the file, but rather
	 * the product of `.max_byte2 - .min_byte2 + 1` and
	 * `.max_byte1 - .min_byte1 + 1`.
	 * 
	 * This name of this field is choosen for consistence
	 * with the documentation the implementation is based
	 * on, and is confusing, it ought to be called "char_count"
	 */
	size_t glyph_count;
};

/**
 * Glyph index returned when there is no glyph
 * available for the character
 */
#define LIBPARSEPCF_NOT_ENCODED ((size_t)0xFFFFUL)

/**
 * Get the glyph index lookup metadata for the file
 * 
 * @param   file   The file content
 * @param   size   The file size
 * @param   table  The table in the file with the `.type` `LIBPARSEPCF_BDF_ENCODINGS`,
 *                 extracted using the `libparsepcf_get_tables` function
 * @param   meta   Output parameter for the glyph index lookup metadata
 * @return         0 on success, -1 on failure
 * 
 * @throws  EBFONT  The file is not a properly formatted PCF file
 */
int libparsepcf_get_encoding(const void *, size_t,
                             const struct libparsepcf_table *,
                             struct libparsepcf_encoding *);

/**
 * Get the glyph index in the file for characters
 * 
 * The first four arguments shall be the same as
 * passed into `libparsepcf_get_properties`
 * 
 * @param   file     The file content
 * @param   size     The file size
 * @param   table    The table in the file with the `.type` `LIBPARSEPCF_PROPERTIES`,
 *                   extracted using the `libparsepcf_get_tables` function
 * @param   meta     Glyph index lookup metadata
 * @param   indices  Output parameter for the glyph indices; for any character
 *                   the index `LIBPARSEPCF_NOT_ENCODED` is stored, the character
 *                   is not included in the font
 * @param   first    The codepoint of the first character whose index to store in
 *                   `indices`; see the description of `struct libparsepcf_encoding`
 *                   for information on how to calculate a character's codepoint
 * @param   count    The number of glyph indices to store in `indices`;
 *                   may not exceed the number of characters (`.glyph_count`)
 *                   as retrieved by `libparsepcf_get_encoding` minus `first`
 * @return           0 on success, -1 on failure
 * 
 * @throws  EBFONT  The file is not a properly formatted PCF file
 */
int libparsepcf_get_glyph_indices(const void *, size_t,
                                  const struct libparsepcf_table *,
                                  const struct libparsepcf_encoding *,
                                  size_t *, size_t, size_t);




/**
 * Information, available for optimisation, about the font
 */
struct libparsepcf_accelerators {
	/**
	 * Whether `metrics[i].right_side_bearing - metrics[i].character_width`
	 * less than or equal to `.min_bounds.left_side_bearing` for all `i`
	 */
	uint8_t no_overlap : 1;

	/**
	 * Whether all glyph use the same layout metrics
	 */
	uint8_t constant_metrics : 1;

	/**
	 * `.constant_metrics` and, for all characters, left side bearing = 0,
	 * right side bearing = character width, ascent = `.font_ascent`, and
	 * descent = `.font_descent`
	 */
	uint8_t terminal_font : 1;

	/**
	 * Whether all glyph have the same layout width
	 */
	uint8_t constant_width : 1;

	/**
	 * Whether all inked bits are inside the glyph's box
	 */
	uint8_t ink_inside : 1;

	/**
	 * Whether the ink metrics differ from the metrics for some glyph
	 */
	uint8_t ink_metrics : 1;

	/**
	 * Whether the font is a right-to-left font
	 */
	uint8_t draw_rtl : 1;

	/**
	 * If 0, `.ink_min_bounds` and `.ink_max_bounds` are just copies of
	 * `.min_bounds` and `.max_bounds`
	 */
	uint8_t have_ink_bounds : 1;

	/**
	 * The number of pixels the font ascend above the baseline
	 */
	int32_t font_ascent;

	/**
	 * The number of pixels the font descend below the baseline
	 */
	int32_t font_descent;

	/**
	 * The maximum horizontal overlap, in pixels, of two glyphs
	 */
	int32_t max_overlap;

	/**
	 * The lower bounds, over each glyph, of the layout metrics
	 */
	struct libparsepcf_metrics min_bounds;

	/**
	 * The upper bounds, over each glyph, of the layout metrics
	 */
	struct libparsepcf_metrics max_bounds;

	/**
	 * The lower bounds, over each glyph, of the ink metrics
	 */
	struct libparsepcf_metrics min_ink_bounds;

	/**
	 * The upper bounds, over each glyph, of the ink metrics
	 */
	struct libparsepcf_metrics max_ink_bounds;
};

/**
 * Get information, available for optimisation, about the font
 * 
 * @param   file   The file content
 * @param   size   The file size
 * @param   table  The table in the file with the `.type` `LIBPARSEPCF_ACCELERATORS`
 *                 or `LIBPARSEPCF_BDF_ACCELERATORS`, extracted using the
 *                 `libparsepcf_get_tables` function
 * @param   meta   Output parameter for the list of glyph names
 * @return         0 on success, -1 on failure
 * 
 * @throws  EBFONT  The file is not a properly formatted PCF file
 * 
 * `LIBPARSEPCF_BDF_ACCELERATORS` shall be prefered over
 * `LIBPARSEPCF_ACCELERATORS` of both tables are present.
 * `LIBPARSEPCF_BDF_ACCELERATORS` is more accurate and refers
 * only to the encoded characters in the font, whereas
 * `LIBPARSEPCF_ACCELERATORS` includes all glyphs
 */
int libparsepcf_get_accelerators(const void *, size_t,
                                 const struct libparsepcf_table *,
                                 struct libparsepcf_accelerators *);




/**
 * Get number of scalable glyph widths available in the file
 * 
 * @param   file   The file content
 * @param   size   The file size
 * @param   table  The table in the file with the `.type` `LIBPARSEPCF_SWIDTHS`
 *                 or `LIBPARSEPCF_SWIDTHS`, extracted using the
 *                 `libparsepcf_get_tables` function
 * @param   count  Output parameter for glyph metric count
 * @return         0 on success, -1 on failure
 * 
 * @throws  EBFONT  The file is not a properly formatted PCF file
 */
int libparsepcf_get_swidth_count(const void *, size_t,
                                 const struct libparsepcf_table *,
                                 size_t *);

/**
 * Get number of scalable glyph widths available in the file
 * 
 * @param   file    The file content
 * @param   size    The file size
 * @param   table   The table in the file with the `.type` `LIBPARSEPCF_SWIDTHS`
 *                  or `LIBPARSEPCF_SWIDTHS`, extracted using the
 *                  `libparsepcf_get_tables` function
 * @param   widths  Output parameter for the glyph's widths; each width
 *                  is encoded with the unit milli-em
 * @param   first   The index of the first glyph's width to store in
 *                  `width`, the first glyph in the file has the index 0
 * @param   count   The number of glyph's widths to store in `widths`;
 *                  may not exceed the number of glyph as retrieved by
 *                  `libparsepcf_get_swidth_count` (stored in it's last
 *                  argument) minus `first`
 * @return          0 on success, -1 on failure
 * 
 * @throws  EBFONT  The file is not a properly formatted PCF file
 */
int libparsepcf_get_swidths(const void *, size_t,
                            const struct libparsepcf_table *,
                            int32_t *, size_t, size_t);




/**
 * The tables, in the file, and metadata available in the tables
 */
struct libparsepcf_font {
	/**
	 * The `LIBPARSEPCF_PROPERTIES` table,
	 * `NULL` if the table is missing
	 */
	const struct libparsepcf_table *prob_table;

	/**
	 * Data extracted for `LIBPARSEPCF_PROPERTIES`
	 * using `libparsepcf_get_properties`
	 * 
	 * Only set if `.prob_table` is non-`NULL`
	 */
	struct libparsepcf_properties props;


	/**
	 * The `LIBPARSEPCF_BDF_ACCELERATORS` table,
	 * or if missing, the `LIBPARSEPCF_ACCELERATORS`
	 * table; `NULL` if both are missing
	 */
	const struct libparsepcf_table *accel_table;

	/**
	 * Data extracted for `LIBPARSEPCF_BDF_ACCELERATORS`
	 * or, if not present, `LIBPARSEPCF_ACCELERATORS`
	 * using `libparsepcf_get_accelerators`
	 * 
	 * Only set if `.accel_table` is non-`NULL`
	 */
	struct libparsepcf_accelerators accels;


	/**
	 * The `LIBPARSEPCF_METRICS` table
	 * 
	 * This will be non-`NULL` as `libparsepcf_preparse_font`
	 * fails with `EBFONT` if the table is missing
	 */
	const struct libparsepcf_table *mtx_table;

	/**
	 * The number of glyph layout metrics available in the font
	 * 
	 * This will be the same as `.glyph_count`
	 */
	size_t metrics;


	/**
	 * The `LIBPARSEPCF_INK_METRICS` table, `NULL`
	 * if the table is missing
	 * 
	 * If `NULL`, the ink metrics are the same as the
	 * layout metrics and `.mtx_table` should be used
	 * instead
	 */
	const struct libparsepcf_table *inkmtx_table;

	/**
	 * The number of glyph ink metrics available in the font
	 * 
	 * This will be the same as `.glyph_count` provided
	 * that `.inkmtx_table` is non-`NULL`
	 * 
	 * Only set if `.inkmtx_table` is non-`NULL`
	 */
	size_t ink_metrics;


	/**
	 * The `LIBPARSEPCF_BDF_ENCODINGS` table
	 * 
	 * This will be non-`NULL` as `libparsepcf_preparse_font`
	 * fails with `EBFONT` if the table is missing
	 */
	const struct libparsepcf_table *enc_table;

	/**
	 * Data extracted for `LIBPARSEPCF_BDF_ENCODINGS`
	 * using `libparsepcf_get_encoding`
	 */
	struct libparsepcf_encoding encoding;


	/**
	 * The `LIBPARSEPCF_BITMAPS` table,
	 * `NULL` if the table is missing
	 */
	const struct libparsepcf_table *bitmap_table;

	/**
	 * Data extracted for `LIBPARSEPCF_BITMAPS`
	 * using `libparsepcf_get_bitmaps`
	 * 
	 * Only set if `.bitmaps_table` is non-`NULL`
	 */
	struct libparsepcf_bitmaps bitmaps;


	/**
	 * The `LIBPARSEPCF_SWIDTHS` table,
	 * `NULL` if the table is missing
	 */
	const struct libparsepcf_table *swidth_table;

	/**
	 * The number of scalable glyph widths available
	 * in the font
	 * 
	 * Only set if `.swidth_table` is non-`NULL`
	 */
	size_t swidths;


	/**
	 * The `LIBPARSEPCF_GLYPH_NAMES` table,
	 * `NULL` if the table is missing
	 */
	const struct libparsepcf_table *name_table;

	/**
	 * Data extracted for `LIBPARSEPCF_GLYPH_NAMES`
	 * using `libparsepcf_get_glyph_names`
	 * 
	 * Only set if `.names_table` is non-`NULL`
	 */
	struct libparsepcf_glyph_names names;


	/**
	 * The number of glyphs available in the file
	 */
	size_t glyph_count;


	/**
	 * The array of all tables in the file
	 * 
	 * For internal use
	 */
	struct libparsepcf_table *_tables;
};

/**
 * Identify all tables in the file and extract the
 * metadata in each table
 * 
 * @param   file  The file content
 * @param   size  The file size
 * @param   font  Output parameter for the table data
 * @return        0 on success, -1 on failure
 * 
 * @throws  EBFONT  The file is not a properly formatted PCF file
 */
int libparsepcf_preparse_font(const void *, size_t, struct libparsepcf_font *);

/**
 * Deallocate memory allocated by `libparsepcf_preparse_font`
 * and erase the object's content
 * 
 * @param  font  The object to erase, and allocations to deallocate;
 *               `font` itself will not be deallocated, only memory
 *               stored inside it
 */
void libparsepcf_destroy_preparsed_font(struct libparsepcf_font *);



#endif
