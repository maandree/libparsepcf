/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libparsepcf_destroy_preparsed_font(struct libparsepcf_font *font)
{
	free(font->_tables);
	memset(font, 0, sizeof(*font));
}
