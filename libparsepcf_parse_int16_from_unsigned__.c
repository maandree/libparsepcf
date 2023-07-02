/* See LICENSE file for copyright and license details. */
#include "common.h"


int16_t
libparsepcf_parse_int16_from_unsigned__(uint16_t u)
{
	if (u & UINT16_C(0x8000))
		return (int16_t)(uint16_t)~u - 1;
	else
		return (int16_t)u;
}
