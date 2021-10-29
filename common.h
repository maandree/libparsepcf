/* See LICENSE file for copyright and license details. */
#include "libparsepcf.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>


#if defined(__GNUC__)
# define PURE  __attribute__((__pure__))
# define CONST __attribute__((__const__))
#else
# define PURE
# define CONST
#endif


/* Table formats: */
#define LIBPARSEPCF_DEFAULT_FORMAT              UINT32_C(0x00000000)
#define LIBPARSEPCF_INKBOUNDS                   UINT32_C(0x00000200)
#define LIBPARSEPCF_ACCELERATOR_WITH_INK_BOUNDS UINT32_C(0x00000100)
#define LIBPARSEPCF_COMPRESSED_METRICS          UINT32_C(0x00000100)

/* Table format modifiers: */
#define LIBPARSEPCF_GLYPH_PAD_MASK (UINT32_C(3) << 0)
#define LIBPARSEPCF_BYTE           (UINT32_C(1) << 2)
#define LIBPARSEPCF_BIT            (UINT32_C(1) << 3)
#define LIBPARSEPCF_SCAN_UNIT_MASK (UINT32_C(3) << 4)


PURE uint32_t libparsepcf_parse_lsb_uint32__(const void *);
PURE uint32_t libparsepcf_parse_msb_uint32__(const void *);

PURE uint16_t libparsepcf_parse_lsb_uint16__(const void *);
PURE uint16_t libparsepcf_parse_msb_uint16__(const void *);

CONST int16_t libparsepcf_parse_int16_from_unsigned__(uint16_t);
CONST int32_t libparsepcf_parse_int32_from_unsigned__(uint32_t);

#define PARSE_UINT32(TEXT, MSB)\
	((MSB) ? libparsepcf_parse_msb_uint32__(TEXT) : libparsepcf_parse_lsb_uint32__(TEXT))

#define PARSE_UINT16(TEXT, MSB)\
	((MSB) ? libparsepcf_parse_msb_uint16__(TEXT) : libparsepcf_parse_lsb_uint16__(TEXT))

#define PARSE_INT32(TEXT, MSB)\
	(libparsepcf_parse_int32_from_unsigned__(PARSE_UINT32(TEXT, MSB)))

#define PARSE_INT16(TEXT, MSB)\
	(libparsepcf_parse_int16_from_unsigned__(PARSE_UINT16(TEXT, MSB)))


#undef PURE
#undef CONST
