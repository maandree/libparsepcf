/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libparsepcf_destroy_preparsed_font(struct libparsepcf_font *font)
{
	if (font) {
		free(font->_tables);
		*font = (struct libparsepcf_font){0};
	}
}
