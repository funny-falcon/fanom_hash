#ifndef LUCKY777_HASH_H
#define LUCKY777_HASH_H
/*
 * Copyright (C) 2017 Sokolov Yura aka funny_falcon
 *
 * It is in public domain, free to use and modify by everyone for any usage.
 * I will be glad, if you put copyright notice above in your code or distribution, but you don't ought to.
 * I'm not responsible for any damage cause by this piece of code.
 *
 * But if you like licenses, than you may count it is under MIT Licence.
 */

/* convenient function to hash buffer with 32bit seed */
static inline uint32_t  lucky777_string_hash(const void *buf, size_t len, uint32_t seed);
/* convenient function to hash buffer with 64bit seed */
static inline uint32_t  lucky777_string_hash2(const void *buf, size_t len, uint32_t seed1, uint32_t seed2);
/* convenient function to hash buffer with 96bit seed */
static inline uint32_t  lucky777_string_hash3(const void *buf, size_t len, uint32_t seed1, uint32_t seed2, uint32_t seed3);
/* permutate state with integer value.
 * Note: initial state should not be all-zero. */
static inline void lucky777_permute(uint32_t v, uint32_t *a, uint32_t *b);
/* finalize state to hash value */
static inline uint32_t lucky777_finalize(uint32_t a, uint32_t b);

/* IMPLEMENTATION */
#ifndef LUCKY777_READ_UNALIGNED
#define LUCKY777_READ_UNALIGNED 1
#endif
#define l7_rotl(x, n) (((x) << (n)) | ((x) >> (sizeof(x)*8 - (n))))
#define l7_rotr(x, n) (((x) << (sizeof(x)*8 - (n))) | ((x) >> (n)))

#if LUCKY777_READ_UNALIGNED
static inline uint32_t lucky777_load_u32(const uint8_t *buf)
{
	return *(uint32_t*)buf;
}
#else
static inline uint32_t lucky777_load_u32(const uint8_t *buf)
{
	const uint8_t *p = (const uint8_t*)buf;
	return p[0] | (p[1]<<8) | (p[2]<<16) | (p[3]<<24);
}
#endif

static inline uint32_t
lucky777_load_u24(const uint8_t *v, unsigned len)
{
	uint32_t x = v[0];
	x |= (uint32_t)v[len/2] << 8;
	x |= (uint32_t)v[len-1] << 16;
	return x;
}

static inline void
lucky777_permute(uint32_t v, uint32_t *a, uint32_t *b)
{
	*a += v;
	*b ^= *a;
	*a = l7_rotl(*a, 7) + *b;
	*b = l7_rotr(*b, 7) ^ *a;
	*a = l7_rotl(*a, 7);
}

static inline uint32_t
lucky777_finalize(uint32_t a, uint32_t b)
{
	a ^= b; a += l7_rotr(b, 3);
	b ^= a; b += l7_rotl(a, 5);
	a ^= b; a += l7_rotr(b, 8);
	b ^= a; b += l7_rotl(a, 16);
	return b;
}

static uint32_t
lucky777_permute_string(const uint8_t *v, size_t len, uint32_t seed1, uint32_t seed2, uint32_t seed3)
{
	uint32_t a=seed1;
	uint32_t b=seed2;
	uint32_t l = len << 24;
	uint32_t t = 0;
	for (;len > 3; len-=4, v+=4) {
		t = lucky777_load_u32(v);
		lucky777_permute(t, &a, &b);
	}
	if (len) l |= lucky777_load_u24(v, len);
	lucky777_permute(l, &a, &b);
	b ^= seed3;
	return lucky777_finalize(a, b);
}

static inline uint32_t
lucky777_string_hash(const void* d, size_t len, uint32_t seed)
{
	const uint32_t c = 0xeb0d2f41;
	return lucky777_permute_string((const uint8_t*)d, len, seed, seed^c, l7_rotl(seed,15)^c);
}

static inline uint32_t
lucky777_string_hash2(const void* d, size_t len, uint32_t seed1, uint32_t seed2)
{
	return lucky777_permute_string((const uint8_t*)d, len, seed1, seed2, seed1^seed2);
}

static inline uint32_t
lucky777_string_hash3(const void* d, size_t len, uint32_t seed1, uint32_t seed2, uint32_t seed3)
{
	return lucky777_permute_string((const uint8_t*)d, len, seed1, seed2, seed3);
}

#undef l7_rotl
#undef l7_rotr

#endif
