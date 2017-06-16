#ifndef FANOM32_HASH_H
#define FANOM32_HASH_H
/*
 * Copyright (C) 2016 Sokolov Yura aka funny_falcon
 *
 * It is in public domain, free to use and modify by everyone for any usage.
 * I will be glad, if you put copyright notice above in your code or distribution, but you don't ought to.
 * I'm not responsible for any damage cause by this piece of code.
 *
 * But if you like licenses, than you may count it is under MIT Licence.
 */

/* API */

/* fanom32 - FAst NOn-Multiplicative 32bit hash */

/* convenient function to hash buffer with 32bit seed */
static inline uint32_t  fanom32_string_hash(const void *buf, size_t len, uint32_t seed);
/* convenient function to hash buffer with 64bit seed */
static inline uint32_t  fanom32_string_hash2(const void *buf, size_t len, uint32_t seed1, uint32_t seed2);

/* IMPLEMENTATION */
#ifndef FANOM_READ_UNALIGNED
#define FANOM_READ_UNALIGNED 1
#endif
#define fn_rotl(x, n) (((x) << (n)) | ((x) >> (sizeof(x)*8 - (n))))

#if FANOM_READ_UNALIGNED
static inline uint32_t fanom32_load_u32(const uint8_t *buf)
{
	return *(uint32_t*)buf;
}
#else
static inline uint32_t fanom32_load_u32(const uint8_t *buf)
{
	const uint8_t *p = (const uint8_t*)buf;
	return p[0] | (p[1]<<8) | (p[2]<<16) | (p[3]<<24);
}
#endif

static inline uint32_t
fanom32_load_u24(const uint8_t *v, unsigned len)
{
	uint32_t x = v[0];
	x |= (uint32_t)v[len/2] << 8;
	x |= (uint32_t)v[len-1] << 16;
	return x;
}

#define unlikely(a) (__builtin_expect(!!(a), 0))
#define likely(a) (__builtin_expect(!!(a), 1))
static uint32_t
fanom32_permute_string(const uint8_t *v, size_t len, uint32_t seed1, uint32_t seed2)
{
	uint32_t a=0;
	uint32_t b=0;
	uint32_t c=seed1;
	uint32_t d=seed2;
	uint32_t l = len;
	uint32_t t = 0;
	switch (len) {
	case 0: break;
	case 1: case 2: case 3:
		a = t = fanom32_load_u24(v, len);
		break;
	case 4:
		a = t = fanom32_load_u32(v);
		break;
	case 5: case 6: case 7:
		a = fanom32_load_u32(v);
		b = fanom32_load_u24(v+4, len&3);
		break;
	case 8:
		a = fanom32_load_u32(v);
		b = fanom32_load_u32(v+4);
		break;
	default:
		for(; len>8; len-=8, v+=8) {
			a ^= fanom32_load_u32(v);
			b ^= fanom32_load_u32(v+4);
			c += a;
			d += b;
			a = fn_rotl(a, 5) - d;
			b = fn_rotl(b, 7) - c;
			c = fn_rotl(c, 24) ^ a;
			d = fn_rotl(d, 1) ^ b;
		}
		a ^= fanom32_load_u32(v+len-8);
		b ^= fanom32_load_u32(v+len-4);
	}
	c += b; c -= fn_rotl(a, 9);
	d += a; d -= fn_rotl(b, 18);
	t = (seed2 ^ l) - fn_rotl(a^b,7);
	t += c; t += fn_rotl(d,13);
	d ^= c; d -= fn_rotl(c,25);
	t ^= d; t -= fn_rotl(d,16);
	c ^= t; c -= fn_rotl(t,4);
	d ^= c; d -= fn_rotl(c,14);
	t ^= d; t -= fn_rotl(d,24);
	return t;
}

static inline uint32_t
fanom32_string_hash(const void* d, size_t len, uint32_t seed)
{
	const uint32_t c = 0xeb0d2f41;
	return fanom32_permute_string((const uint8_t*)d, len, seed, seed^c);
}

static inline uint32_t
fanom32_string_hash2(const void* d, size_t len, uint32_t seed1, uint32_t seed2)
{
	return fanom32_permute_string((const uint8_t*)d, len, seed1, seed2);
}

#undef fn_rotl
#endif
