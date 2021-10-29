/* See LICENSE file for copyright and license details. */
#include "common.h"


uint32_t
libparsepcf_parse_msb_uint32__(const void *buf)
{
	const uint8_t *bs = buf;
	uint32_t a = (uint32_t)bs[0] << 24;
	uint32_t b = (uint32_t)bs[1] << 16;
	uint32_t c = (uint32_t)bs[2] << 8;
	uint32_t d = (uint32_t)bs[3] << 0;
	return a | b | c | d;
}
