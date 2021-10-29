/* See LICENSE file for copyright and license details. */
#include "common.h"


uint16_t
libparsepcf_parse_lsb_uint16__(const void *buf)
{
	const uint8_t *bs = buf;
	uint16_t a = (uint16_t)bs[0] << 0;
	uint16_t b = (uint16_t)bs[1] << 8;
	return a | b;
}
