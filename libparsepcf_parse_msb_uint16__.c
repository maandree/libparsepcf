/* See LICENSE file for copyright and license details. */
#include "common.h"


uint16_t
libparsepcf_parse_msb_uint16__(const void *buf)
{
	const uint8_t *bs = buf;
	uint16_t a = (uint16_t)bs[0] << 8;
	uint16_t b = (uint16_t)bs[1] << 0;
	return a | b;
}
