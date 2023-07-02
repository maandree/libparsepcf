/* See LICENSE file for copyright and license details. */
#include "common.h"


int32_t
libparsepcf_parse_int32_from_unsigned__(uint32_t u)
{
	if (u & UINT32_C(0x80000000))
		return (int32_t)(uint32_t)~u - 1;
	else
		return (int32_t)u;
}
