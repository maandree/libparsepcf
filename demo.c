/* See LICENSE file for copyright and license details. */
#include "libparsepcf.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


static char *file = NULL;
static size_t len = 0;

static size_t table_n;
static size_t table_i;
static struct libparsepcf_table *tables;

static struct libparsepcf_font font;


static void
load_file(void)
{
	size_t size = 0;
	ssize_t r;

	if (isatty(STDIN_FILENO)) {
		fprintf(stderr, "An uncompressed PCF file is required as standard input\n");
		exit(1);
	}

	for (;;) {
		if (size == len) {
			file = realloc(file, size += 8096);
			if (!file) {
				perror("realloc");
				exit(1);
			}
		}
		r = read(STDIN_FILENO, &file[len], size - len);
		if (r <= 0) {
			if (!r)
				break;
			perror("read");
			exit(1);
		}
		len += (size_t)r;
	}
}


static void
print_properties(void)
{
	size_t prop_i;
	struct libparsepcf_properties props;
	struct libparsepcf_property_subtable *proptab;

	if (libparsepcf_get_properties(file, len, &tables[table_i], &props)) {
		perror("libparsepcf_get_properties");
		exit(1);
	}
	printf("    PROPERTIES:\n");
	printf("\tnprops: %zu\n", props.property_count);
	printf("\tstrlen: %zu\n", props.strings_size);
	if (!props.property_count)
		return;
	proptab = calloc(props.property_count, sizeof(*proptab));
	if (!proptab) {
		perror("calloc");
		exit(1);
	}
	if (libparsepcf_get_property_subtable(file, len, &tables[table_i], &props,
	                                      proptab, 0, props.property_count)) {
		perror("libparsepcf_get_property_subtable");
		exit(1);
	}
	for (prop_i = 0; prop_i < props.property_count; prop_i++) {
		printf("\t    #%zu\n", prop_i);
		printf("\t\tname: %s\n", proptab[prop_i].name);
		if (proptab[prop_i].is_string_property)
			printf("\t\tvalue: \"%s\"\n", proptab[prop_i].value.string_value);
		else
			printf("\t\tvalue: %i\n", proptab[prop_i].value.signed_value);
	}
	free(proptab);
}


static void
print_metrics(const char *header)
{
	struct libparsepcf_metrics *metrics;
	size_t mtx_i, mtx_n;

	if (libparsepcf_get_metrics_count(file, len, &tables[table_i], &mtx_n)) {
		perror("libparsepcf_get_metrics_count");
		exit(1);
	}
	metrics = calloc(mtx_n, sizeof(*metrics));
	if (!metrics) {
		perror("calloc");
		exit(1);
	}
	if (libparsepcf_get_metrics(file, len, &tables[table_i], metrics, 0, mtx_n)) {
		perror("libparsepcf_get_metrics");
		exit(1);
	}
	printf("    %s:\n", header);
	printf("\tcount: %zu\n", mtx_n);
	for (mtx_i = 0; mtx_i < mtx_n; mtx_i++) {
		printf("\t#%zu: lsb=%i, rsb=%i, width=%i, ascent=%i, descent=%i, attributes=%x\n",
		       mtx_i, metrics[mtx_i].left_side_bearing, metrics[mtx_i].right_side_bearing,
		       metrics[mtx_i].character_width, metrics[mtx_i].character_ascent,
		       metrics[mtx_i].character_descent, metrics[mtx_i].character_attributes);
	}
	free(metrics);
}


static void
print_glyph_names(void)
{
	size_t name_i;
	struct libparsepcf_glyph_names names;
	const char **nametab;

	if (libparsepcf_get_glyph_names(file, len, &tables[table_i], &names)) {
		perror("libparsepcf_get_glyph_names");
		exit(1);
	}
	printf("    GLYPH NAMES:\n");
	printf("\tnglyph: %zu\n", names.glyph_count);
	printf("\tstrlen: %zu\n", names.strings_size);
	if (!names.glyph_count)
		return;
	nametab = calloc(names.glyph_count, sizeof(*nametab));
	if (!nametab) {
		perror("calloc");
		exit(1);
	}
	if (libparsepcf_get_glyph_name_subtable(file, len, &tables[table_i], &names,
	                                        nametab, 0, names.glyph_count)) {
		perror("libparsepcf_get_glyph_name_subtable");
		exit(1);
	}
	for (name_i = 0; name_i < names.glyph_count; name_i++)
		printf("\t    #%zu: %s\n", name_i, nametab[name_i]);
	free(nametab);
}


static void
print_bitmaps(void)
{
	size_t bitmap_i;
	struct libparsepcf_bitmaps bitmaps;
	size_t *bitmaptab;

	if (libparsepcf_get_bitmaps(file, len, &tables[table_i], &bitmaps)) {
		perror("libparsepcf_get_bitmaps");
		exit(1);
	}
	printf("    BITMAPS:\n");
	printf("\tnglyph: %zu\n", bitmaps.glyph_count);
	printf("\tsize: %zu\n", bitmaps.bitmap_size);
	printf("\tpacking: %zu\n", bitmaps.bit_packing);
	printf("\tpadding: %zu\n", bitmaps.row_padding);
	printf("\tlsbyte: %i\n", bitmaps.lsbyte);
	printf("\tlsbit: %i\n", bitmaps.lsbit);
	if (!bitmaps.glyph_count)
		return;
	bitmaptab = calloc(bitmaps.glyph_count, sizeof(*bitmaptab));
	if (!bitmaptab) {
		perror("calloc");
		exit(1);
	}
	if (libparsepcf_get_bitmap_offsets(file, len, &tables[table_i], &bitmaps,
	                                   bitmaptab, 0, bitmaps.glyph_count)) {
		perror("libparsepcf_get_bitmap_offsets");
		exit(1);
	}
	for (bitmap_i = 0; bitmap_i < bitmaps.glyph_count; bitmap_i++) {
		printf("\t    #%zu: %zu -> %p (maxsize=%zu)\n",
		       bitmap_i, bitmaptab[bitmap_i],
		       (const void *)&bitmaps.bitmap_data[bitmaptab[bitmap_i]],
		       bitmaps.bitmap_size - bitmaptab[bitmap_i]);
	}
	free(bitmaptab);
}


static void
print_bdf_encodings(void)
{
	struct libparsepcf_encoding encoding;
	size_t *indices;
	size_t index_i;

	if (libparsepcf_get_encoding(file, len, &tables[table_i], &encoding)) {
		perror("libparsepcf_get_encoding:");
		exit(1);
	}
	printf("    BDF ENCODINGS:\n");
	printf("\trange2: [%u, %u]\n", encoding.min_byte2, encoding.max_byte2);
	printf("\trange1: [%u, %u]\n", encoding.min_byte1, encoding.max_byte1);
	printf("\tdefault: %u\n", encoding.default_char);
	printf("\tnglyph: %zu\n", encoding.glyph_count);
	if (!encoding.glyph_count)
		return;
	indices = calloc(encoding.glyph_count, sizeof(*indices));
	if (libparsepcf_get_glyph_indices(file, len, &tables[table_i], &encoding,
	                                  indices, 0, encoding.glyph_count)) {
		perror("libparsepcf_get_glyph_indices");
		exit(1);
	}
	for (index_i = 0; index_i < encoding.glyph_count; index_i++) {
		if (indices[index_i] == LIBPARSEPCF_NOT_ENCODED)
			printf("\t#%#zx: not encoded\n", index_i);
		else
			printf("\t#%#zx: %zu\n", index_i, indices[index_i]);
	}
	free(indices);
}


static void
print_accelerators(const char *header)
{
	struct libparsepcf_accelerators accel;

	if (libparsepcf_get_accelerators(file, len, &tables[table_i], &accel)) {
		perror("libparsepcf_get_accelerators:");
		exit(1);
	}
	printf("    %s:\n", header);
	printf("\tno overlap: %i\n", accel.no_overlap);
	printf("\tconstant metrics: %i\n", accel.constant_metrics);
	printf("\tterminal font: %i\n", accel.terminal_font);
	printf("\tconstant width: %i\n", accel.constant_width);
	printf("\tink inside: %i\n", accel.ink_inside);
	printf("\tink metrics: %i\n", accel.ink_metrics);
	printf("\tdraw rtl: %i\n", accel.draw_rtl);
	printf("\thave ink bounds: %i\n", accel.have_ink_bounds);
	printf("\tfont ascent: %i\n", accel.font_ascent);
	printf("\tfont descent: %i\n", accel.font_descent);
	printf("\tmax overlap: %i\n", accel.max_overlap);
	printf("\tmin bounds: lsb=%i, rsb=%i, width=%i, ascent=%i, descent=%i, attributes=%x\n",
	       accel.min_bounds.left_side_bearing, accel.min_bounds.right_side_bearing,
	       accel.min_bounds.character_width, accel.min_bounds.character_ascent,
	       accel.min_bounds.character_descent, accel.min_bounds.character_attributes);
	printf("\tmax bounds: lsb=%i, rsb=%i, width=%i, ascent=%i, descent=%i, attributes=%x\n",
	       accel.max_bounds.left_side_bearing, accel.max_bounds.right_side_bearing,
	       accel.max_bounds.character_width, accel.max_bounds.character_ascent,
	       accel.max_bounds.character_descent, accel.max_bounds.character_attributes);
	if (accel.have_ink_bounds) {
		printf("\tmin ink bounds: lsb=%i, rsb=%i, width=%i, ascent=%i, descent=%i, attributes=%x\n",
		       accel.min_ink_bounds.left_side_bearing, accel.min_ink_bounds.right_side_bearing,
		       accel.min_ink_bounds.character_width, accel.min_ink_bounds.character_ascent,
		       accel.min_ink_bounds.character_descent, accel.min_ink_bounds.character_attributes);
		printf("\tmax ink bounds: lsb=%i, rsb=%i, width=%i, ascent=%i, descent=%i, attributes=%x\n",
		       accel.max_ink_bounds.left_side_bearing, accel.max_ink_bounds.right_side_bearing,
		       accel.max_ink_bounds.character_width, accel.max_ink_bounds.character_ascent,
		       accel.max_ink_bounds.character_descent, accel.max_ink_bounds.character_attributes);
	}
}


static void
print_swidths(void)
{
	size_t sw_i, sw_n;
	int32_t *swidths;

	if (libparsepcf_get_swidth_count(file, len, &tables[table_i], &sw_n)) {
		perror("libparsepcf_get_swidth_count");
		exit(1);
	}
	printf("    SWIDTHS:\n");
	printf("\tnglyph: %zu\n", sw_n);
	if (!sw_n)
		return;
	swidths = calloc(sw_n, sizeof(*swidths));
	if (libparsepcf_get_swidths(file, len, &tables[table_i], swidths, 0, sw_n)) {
		perror("libparsepcf_get_swidths");
		exit(1);
	}
	for (sw_i = 0; sw_i < sw_n; sw_i++)
		printf("\t#%zu: %i milli-ems\n", sw_i, swidths[sw_i]);
}


static void
print_font(void)
{
	if (libparsepcf_get_table_count(file, len, &table_n)) {
		perror("libparsepcf_get_table_count");
		exit(1);
	}
	fprintf(stderr, "size: %zu\n", len);
	fprintf(stderr, "ntables: %zu\n", table_n);

	tables = calloc(table_n, sizeof(*tables));
	if (!tables) {
		perror("calloc");
		exit(1);
	}
	if (libparsepcf_get_tables(file, len, tables, 0, table_n)) {
		perror("libparsepcf_get_tables");
		exit(1);
	}

	for (table_i = 0; table_i < table_n; table_i++) {
		printf("table #%zu: type=%#x, format=%#x, offset=%u, size=%u, (end=%u)\n",
		       table_i, tables[table_i].type, tables[table_i].format,
		       tables[table_i].offset, tables[table_i].size,
		       tables[table_i].offset + tables[table_i].size);

		switch (tables[table_i].type) {
#if 1
		case LIBPARSEPCF_PROPERTIES: /* X font atoms */
			print_properties();
			break;
#endif
#if 0
		case LIBPARSEPCF_METRICS: /* bitmap size */
			print_metrics("METRICS");
			break;
#endif
#if 0
		case LIBPARSEPCF_INK_METRICS: /* minimium bounding box metrics */
			print_metrics("INK METRICS");
			break;
#endif
#if 0
		case LIBPARSEPCF_GLYPH_NAMES: /* PostScript names */
			print_glyph_names();
			break;
#endif
#if 0
		case LIBPARSEPCF_BITMAPS:
			print_bitmaps();
			break;
#endif
#if 0
		case LIBPARSEPCF_BDF_ENCODINGS:
			print_bdf_encodings();
			break;
#endif
#if 1
		case LIBPARSEPCF_ACCELERATORS:
			print_accelerators("ACCELERATORS");
			break;
#endif
#if 1
		case LIBPARSEPCF_BDF_ACCELERATORS: /* prefered over LIBPARSEPCF_ACCELERATORS */
			print_accelerators("BDF ACCELERATORS");
			break;
#endif
#if 0
		case LIBPARSEPCF_SWIDTHS:
			print_swidths();
			break;
#endif
		default:
			(void) print_properties;
			(void) print_metrics;
			(void) print_glyph_names;
			(void) print_bitmaps;
			(void) print_bdf_encodings;
			(void) print_accelerators;
			(void) print_swidths;
			break;
		}
	}

	free(tables);
}


static void
print_glyph_info(size_t glyph, const struct libparsepcf_metrics *mtx)
{
	struct libparsepcf_metrics inkmtx;
	const char *name;
	int32_t s32;

	if (font.name_table) {
		if (libparsepcf_get_glyph_name_subtable(file, len, font.name_table, &font.names, &name, glyph, 1)) {
			perror("libparsepcf_get_glyph_name_subtable");
			exit(1);
		}
		printf("glyph name: %s\n", name);
	}
	if (font.swidth_table) {
		if (libparsepcf_get_swidths(file, len, font.swidth_table, &s32, glyph, 1)) {
			perror("libparsepcf_get_swidths");
			exit(1);
		}
		printf("scalable width: %i milli-ems\n", s32);
	}
	printf("metrics:\n");
	printf("    left side bearing:    %i\n", mtx->left_side_bearing);
	printf("    right side bearing:   %i\n", mtx->right_side_bearing);
	printf("    character width:      %i\n", mtx->character_width);
	printf("    character ascent:     %i\n", mtx->character_ascent);
	printf("    character descent:    %i\n", mtx->character_descent);
	printf("    character attributes: %u\n", mtx->character_attributes);	
	if (font.inkmtx_table) {
		if (libparsepcf_get_metrics(file, len, font.inkmtx_table, &inkmtx, glyph, 1)) {
			perror("libparsepcf_get_metrics");
			exit(1);
		}
		printf("ink metrics:\n");
		printf("    left side bearing:    %i\n", inkmtx.left_side_bearing);
		printf("    right side bearing:   %i\n", inkmtx.right_side_bearing);
		printf("    character width:      %i\n", inkmtx.character_width);
		printf("    character ascent:     %i\n", inkmtx.character_ascent);
		printf("    character descent:    %i\n", inkmtx.character_descent);
		printf("    character attributes: %u\n", inkmtx.character_attributes);
	}
	if (font.accel_table) {
		printf("font ascent: %i\n", font.accels.font_ascent);
		printf("font descent: %i\n", font.accels.font_descent);
	}
}


static void
print_glyph(size_t glyph)
{
	struct libparsepcf_metrics mtx;
	size_t width, height, bitmap_size, bitmap_offset;
	size_t row_size, padding, y, x, bit, byte, packing;
	int32_t font_ascent, font_descent;
	size_t extra_left, extra_right, print_width;
	const uint8_t *bitmap;
	const char *pixel;

	if (glyph >= font.glyph_count) {
		fprintf(stderr, "specified glyph does not exist\n");
		exit(1);
	}
	if (libparsepcf_get_metrics(file, len, font.mtx_table, &mtx, glyph, 1)) {
		perror("libparsepcf_get_metrics");
		exit(1);
	}
	padding = font.bitmaps.row_padding - 1;
	packing = font.bitmaps.bit_packing - 1;
	width = (size_t)((int32_t)mtx.right_side_bearing - (int32_t)mtx.left_side_bearing);
	height = (size_t)((int32_t)mtx.character_ascent + (int32_t)mtx.character_descent);
	row_size = width / 8 + !!(width & 7);
	row_size = row_size + ((font.bitmaps.row_padding - (row_size & padding)) & padding);
	if (font.accel_table) {
		font_ascent = font.accels.font_ascent;
		font_descent = font.accels.font_descent;
	} else {
		font_ascent = mtx.character_ascent;
		font_descent = mtx.character_descent;
	}

	print_glyph_info(glyph, &mtx);

	if (libparsepcf_get_bitmap_offsets(file, len, font.bitmap_table, &font.bitmaps, &bitmap_offset, glyph, 1)) {
		perror("libparsepcf_get_bitmap_offsets");
		exit(1);
	}
	bitmap_size = font.bitmaps.bitmap_size - bitmap_offset;
	bitmap = &font.bitmaps.bitmap_data[bitmap_offset];
	if (height && row_size > bitmap_size / height) {
		perror("bitmap is smaller than expected");
		exit(1);
	}
	printf("\n");

	extra_left = mtx.left_side_bearing > 0 ? (size_t)mtx.left_side_bearing : 0;
	extra_right = mtx.character_width > mtx.right_side_bearing ? (size_t)(mtx.character_width - mtx.right_side_bearing) : 0;
	print_width = extra_left + width + extra_right;

	/* If the font's ascent is greater than the character's, print the excees in black */
	if (font_ascent > mtx.character_ascent) {
		for (y = 0; y < (size_t)(font_ascent - mtx.character_ascent); y++) {
			for (x = 0; x < print_width; x++)
				printf("\033[1;30m[]");
			printf("\033[0m\n");
		}
	}

	for (y = 0; y < height; y++, bitmap += row_size) {
		/* Draw baseline */
		if ((int64_t)y + 1 == (int64_t)mtx.character_ascent)
			printf("\033[4m");

		/* If glyph is offset for caret, print offset in black */
		for (x = 0; x < extra_left; x++)
			printf("\033[1;30m[]");

		/* Draw glyph */
		for (x = 0; x < width; x++) {
			bit = font.bitmaps.lsbit ? 7 - x % 8 : x % 8;
			byte = x / 8;
			if (!font.bitmaps.lsbyte)
				byte = ((byte & ~packing) | (packing - (byte & packing)));

			/* Use <> to mark dots outside of the glyph box */
			if (mtx.left_side_bearing < 0 &&
			    x < (size_t)-mtx.left_side_bearing) {
				pixel = "<>";
			} else if (mtx.right_side_bearing > mtx.character_width &&
			           x >= (size_t)(mtx.character_width + mtx.left_side_bearing)) {
				pixel = "<>";
			} else if (mtx.character_ascent > font_ascent &&
			           y < (size_t)(mtx.character_ascent - font_ascent)) {
				pixel = "<>";
			} else if (mtx.character_descent > font_descent &&
				   y >= (size_t)(mtx.character_ascent + font_descent)) {
				pixel = "<>";
			} else {
				pixel = "[]";
			}

			printf("\033[%sm%s", ((bitmap[byte] >> bit) & 1) ? "1;37" : "2;37", pixel);
		}

		/* If horizontal advance is larger than character width, print extent in black */
		for (x = 0; x < extra_right; x++)
			printf("\033[1;30m[]");

		printf("\033[0m\n");
	}

	/* If the font's descent is greater than the character's, print the excees in black */
	if (font_descent > mtx.character_descent) {
		for (y = 0; y < (size_t)(font_descent - mtx.character_descent); y++) {
			for (x = 0; x < print_width; x++)
				printf("\033[1;30m[]");
			printf("\033[0m\n");
		}
	}
}


static void
draw_glyph(size_t glyph, int32_t *xp, char ***linesp, int32_t *leftp, int32_t *rightp, int32_t *ascentp, int32_t *descentp)
{
	struct libparsepcf_metrics mtx;
	const uint8_t *bitmap;
	int32_t left, right;
	size_t height, width, old_height, old_width, y, x, ypos, xpos;
	size_t padding, packing, row_size, bit, byte, bitmap_offset, bitmap_size;

	if (libparsepcf_get_metrics(file, len, font.mtx_table, &mtx, glyph, 1)) {
		perror("libparsepcf_get_metrics");
		exit(1);
	}

	xpos = (size_t)*xp;
	left = *xp + mtx.left_side_bearing;
	right = *xp + mtx.right_side_bearing;
	*xp += mtx.character_width;
	height = (size_t)(*ascentp + *descentp);
	width = (size_t)(*leftp + *rightp);

	if (-left > *leftp) {
		*leftp = -left;
		old_width = width;
		width = (size_t)(*leftp + *rightp);
		for (y = 0; y < height; y++) {
			(*linesp)[y] = realloc((*linesp)[y], width);
			if (!(*linesp)[y]) {
				perror("realloc");
				exit(1);
			}
			memmove(&(*linesp)[y][width - old_width], (*linesp)[y], old_width);
			memset((*linesp)[y], 0, width - old_width);
		}
	}
	xpos += (size_t)*leftp;

	if (right > *rightp) {
		*rightp = right;
		old_width = width;
		width = (size_t)(*leftp + *rightp);
		for (y = 0; y < height; y++) {
			(*linesp)[y] = realloc((*linesp)[y], width);
			if (!(*linesp)[y]) {
				perror("realloc");
				exit(1);
			}
			memset(&(*linesp)[y][old_width], 0, width - old_width);
		}
	}

	if (mtx.character_ascent > *ascentp) {
		*ascentp = mtx.character_ascent;
		old_height = height;
		height = (size_t)(*ascentp + *descentp);
		*linesp = realloc(*linesp, height * sizeof(**linesp));
		memmove(&(*linesp)[height - old_height], *linesp, old_height * sizeof(**linesp));
		for (y = 0; y < height - old_height; y++) {
			(*linesp)[y] = NULL;
			if (width) {
				(*linesp)[y] = calloc(width, 1);
				if (!(*linesp)[y]) {
					perror("calloc");
					exit(1);
				}
			}
		}
	}

	if (mtx.character_descent > *descentp) {
		*descentp = mtx.character_descent;
		old_height = height;
		height = (size_t)(*ascentp + *descentp);
		*linesp = realloc(*linesp, height * sizeof(**linesp));
		for (y = old_height; y < height; y++) {
			(*linesp)[y] = NULL;
			if (width) {
				(*linesp)[y] = calloc(width, 1);
				if (!(*linesp)[y]) {
					perror("calloc");
					exit(1);
				}
			}
		}
	}

	padding = font.bitmaps.row_padding - 1;
	packing = font.bitmaps.bit_packing - 1;
	width = (size_t)((int32_t)mtx.right_side_bearing - (int32_t)mtx.left_side_bearing);
	height = (size_t)((int32_t)mtx.character_ascent + (int32_t)mtx.character_descent);
	row_size = width / 8 + !!(width & 7);
	row_size = row_size + ((font.bitmaps.row_padding - (row_size & padding)) & padding);

	if (libparsepcf_get_bitmap_offsets(file, len, font.bitmap_table, &font.bitmaps, &bitmap_offset, glyph, 1)) {
		perror("libparsepcf_get_bitmap_offsets");
		exit(1);
	}
	bitmap_size = font.bitmaps.bitmap_size - bitmap_offset;
	bitmap = &font.bitmaps.bitmap_data[bitmap_offset];
	if (height && row_size > bitmap_size / height) {
		perror("bitmap is smaller than expected");
		exit(1);
	}

	ypos = (size_t)(*ascentp - mtx.character_ascent);
	for (y = 0; y < height; y++, bitmap += row_size) {
		for (x = 0; x < width; x++) {
			bit = font.bitmaps.lsbit ? 7 - x % 8 : x % 8;
			byte = x / 8;
			if (!font.bitmaps.lsbyte)
				byte = ((byte & ~packing) | (packing - (byte & packing)));
			if ((bitmap[byte] >> bit) & 1)
				(*linesp)[y + ypos][x + xpos] |= 1;
		}
	}
}


static void
print_line(const char *str)
{
	const uint8_t *s = (const void *)str;
	int32_t xpos = 0, left = 0, right = 0, ascent = 0, descent = 0;
	char **lines = NULL;
	uint32_t codepoint = 0, hi, lo;
	size_t glyph, n = 0, y, x, width, height;

	while (*s) {
		/* Very sloppy UTF-8 decoding */
		if (n) {
			if ((*s & 0xC0) != 0x80) {
				n = 0;
				glyph = (size_t)font.encoding.default_char;
				goto have_glyph;
			} else {
				n -= 1;
				codepoint <<= 6;
				codepoint |= *s++ & 0x3F;
			}
		} else if ((*s & 0xC0) == 0x80) {
			glyph = (size_t)font.encoding.default_char;
			s++;
			goto have_glyph;
		} else if (*s & 0x80) {
			codepoint = (uint32_t)*s++;
			n = 0;
			while (codepoint & 0x80) {
				codepoint <<= 1;
				n += 1;
			}
			codepoint &= 0xFF;
			codepoint >>= n--;
			continue;
		} else {
			codepoint = (uint32_t)*s++;
		}

		/* Map codepoint to glyph index */
		if (font.encoding.min_byte1 || font.encoding.max_byte1) {
			if (codepoint > UINT32_C(0xFFFF) ||
			    codepoint < (uint32_t)font.encoding.min_byte2 ||
			    codepoint > (uint32_t)font.encoding.max_byte2) {
				glyph = (size_t)font.encoding.default_char;
				goto have_glyph;
			}
			glyph = codepoint - (uint32_t)font.encoding.min_byte2;
		} else {
			hi = (codepoint >> 8) & UINT32_C(0xFF);
			lo = (codepoint >> 0) & UINT32_C(0xFF);
			if (codepoint > UINT32_C(0xFF) ||
			    hi < (uint32_t)font.encoding.min_byte1 ||
			    hi > (uint32_t)font.encoding.max_byte1 ||
			    lo < (uint32_t)font.encoding.min_byte2 ||
			    lo > (uint32_t)font.encoding.max_byte2) {
				glyph = (size_t)font.encoding.default_char;
				goto have_glyph;
			}
			hi -= (size_t)font.encoding.min_byte1;
			lo -= (size_t)font.encoding.min_byte2;
			glyph = hi * (size_t)(font.encoding.max_byte2 - font.encoding.min_byte2 + 1) + lo;
		}
		/* For the purpose of the demo we are assuming ASCII/UCS-2 */
		if (libparsepcf_get_glyph_indices(file, len, font.enc_table, &font.encoding, &glyph, glyph, 1)) {
			perror("libparsepcf_get_glyph_indices");
			exit(1);
		}

	have_glyph:
		if (glyph == LIBPARSEPCF_NOT_ENCODED)
			glyph = (size_t)font.encoding.default_char;

		draw_glyph(glyph, &xpos, &lines, &left, &right, &ascent, &descent);
	}

	width = (size_t)(left + right);
	height = (size_t)(ascent + descent);
	for (y = 0; y < height; y++) {
		if ((int64_t)y + 1 == (int64_t)ascent)
			printf("\033[4m");
		for (x = 0; x < width; x++)
			printf("\033[%sm[]", lines[y][x] ? "1;37" : "1;30");
		printf("\033[0m\n");
		free(lines[y]);
	}
	free(lines);
}


int
main(int argc, char *argv[])
{
	size_t glyph;
	if (argc) {
		argc--;
		argv++;
	}
	load_file();
	if (!argc) {
		print_font();
	} else {
		if (libparsepcf_preparse_font(file, len, &font)) {
			perror("libparsepcf_preparse_font");
			exit(1);
		}
		if (argc == 2 && !strcmp(argv[0], "-g")) {
			glyph = (size_t)strtoul(argv[1], NULL, 0);
			print_glyph(glyph);
		} else if (argc == 2 && !strcmp(argv[0], "-x")) {
			glyph = (size_t)strtoul(argv[1], NULL, 16);
			print_glyph(glyph);
		} else {
			printf("This demo assumes the font uses ASCII or UCS-2 encoding\n");
			for (; argc--; argv++) {
				print_line(*argv);
				printf("\n");
			}
		}
		libparsepcf_destroy_preparsed_font(&font);
	}
	free(file);
	return 0;
}
