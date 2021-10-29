/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libparsepcf_get_property_subtable(const char *file, size_t size,
                                  const struct libparsepcf_table *table,
                                  const struct libparsepcf_properties *meta,
                                  struct libparsepcf_property_subtable *props,
                                    size_t first, size_t count)
{
	int msb = table->format & LIBPARSEPCF_BYTE;
	size_t pos = table->offset + 8 + first * 9;
	size_t i, off, maxlen;

	(void) size;

	for (i = 0; i < count; i++, pos += 9) {
		off = (size_t)PARSE_UINT32(&file[pos + 0], msb);
		if (off > meta->strings_size)
			goto ebfont;
		maxlen = meta->strings_size - off;
		props[i].name = &meta->strings[off];
		if (!memchr(props[i].name, 0, maxlen))
			goto ebfont;

		props[i].is_string_property = !!file[pos + 4];

		if (props[i].is_string_property) {
			off = (size_t)PARSE_UINT32(&file[pos + 5], msb);
			if (off > meta->strings_size)
				goto ebfont;
			maxlen = meta->strings_size - off;
			props[i].value.string_value = &meta->strings[off];
			if (!memchr(props[i].value.string_value, 0, maxlen))
				goto ebfont;
		} else {
			props[i].value.signed_value = PARSE_INT32(&file[pos + 5], msb);
		}
	}

	return 0;

ebfont:
	errno = EBFONT;
	return -1;
}
