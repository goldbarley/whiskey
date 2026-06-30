#ifndef WHIS_MATH_H
#define WHIS_MATH_H

#include "attr.h"

#include <stdint.h>

#if defined(__GNUC__) || defined(__clang__)
#define WHIS_CTZ(n) __builtin_ctz(n)
#define WHIS_CTZ64(n) __builtin_ctzll(n)
#else

WHIS_INLINE WHIS_EXPORT
uint8_t wh_ctz_fallback(uint32_t n)
{
	if (!n)
		return 32;

	static const uint8_t hash[32] = {
		0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
		31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
	};
	return hash[(uint32_t)((n & -n) * UINT32_C(0x077CB531)) >> 27];
}

WHIS_INLINE WHIS_EXPORT
uint8_t wh_ctz64_fallback(uint64_t n)
{
	if (!n)
		return 64;

	static const uint8_t hash[64] = {
		0, 1, 48, 2, 57, 49, 28, 3, 61, 58, 50, 42, 38, 29, 17, 4, 62,
		55, 59, 36, 53, 51, 43, 22, 45, 39, 33, 30, 24, 18, 12, 5, 63,
		47, 56, 27, 60, 41, 37, 16, 54, 35, 52, 21, 44, 32, 23, 11, 46,
		26, 40, 15, 34, 20, 31, 10, 25, 14, 19, 9, 13, 8, 7, 6
	};
	return hash[(uint64_t)((n & -n) * UINT64_C(0x03F79D71B4CB0A89)) >> 58];
}

#define WHIS_CTZ(n) wh_ctz_fallback(n)
#define WHIS_CTZ64(n) wh_ctz64_fallback(n)
#endif /* Compilers */

#define WHIS_SET_BIT64(m, i) \
	((m) |= (UINT64_C(1) << (i)))
#define WHIS_CLR_BIT64(m, i) \
	((m) &= ~(UINT64_C(1) << (i)))

#endif /* WHIS_MATH_H */
