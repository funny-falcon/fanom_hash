#ifndef FANOM_HASH_H
#define FANOM_HASH_H
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

/* fanom - FAst NOn-Multiplicative hash */

/* convenient function to hash buffer with 64bit seed */
static inline uint64_t  fanom64_string_hash(const void *buf, size_t len, uint64_t seed);
/* convenient function to hash buffer with 128bit strong random seed */
static inline uint64_t  fanom64_string_hash2(const void *buf, size_t len, uint64_t seed1, uint64_t seed2);

/* IMPLEMENTATION */
#ifndef FANOM_READ_UNALIGNED
#define FANOM_READ_UNALIGNED 1
#endif
#define fn_rotl(x, n) (((x) << (n)) | ((x) >> (sizeof(x)*8 - (n))))

#if FANOM_READ_UNALIGNED
static inline uint32_t fanom64_load_u32(const uint8_t *buf) {
	return *(uint32_t*)buf;
}
static inline uint64_t fanom64_load_u64(const uint8_t *buf) {
	return *(uint64_t*)buf;
}
#else
static inline uint32_t fanom64_load_u32(const uint8_t *buf) {
	const uint8_t *p = (const uint8_t*)buf;
	return p[0] | (p[1]<<8) | (p[2]<<16) | (p[3]<<24);
}
static inline uint32_t fanom64_load_u64(const uint8_t *buf) {
	const uint8_t *p = (const uint8_t*)buf;
	return p[0] | (p[1]<<8) | (p[2]<<16) | (p[3]<<24) |
		((uint64_t)p[4] << 32) | ((uint64_t)p[5] << 40) |
		((uint64_t)p[6] << 48) | ((uint64_t)p[7] << 56);
}
#endif

static inline uint32_t
fanom64_load_u24(const uint8_t *v, unsigned len)
{
	uint32_t x = v[0];
	x |= (uint32_t)v[len/2] << 8;
	x |= (uint32_t)v[len-1] << 16;
	return x;
}

static inline uint64_t
fanom64_permute_string(const uint8_t *v, size_t len, uint64_t seed1, uint64_t seed2)
{
	uint64_t a=0;
	uint64_t b=0;
	uint64_t c=seed1;
	uint64_t d=seed2;
	uint64_t l = len;
	uint64_t t = 0;
	switch (len) {
	case 0: break;
	case 1: case 2: case 3:
		a = fanom64_load_u24(v, len);
		break;
	case 4:
		a = fanom64_load_u32(v);
		break;
	case 5: case 6: case 7:
		a = fanom64_load_u32(v);
		b = fanom64_load_u24(v+4, len&3);
		break;
	case 8:
		a = fanom64_load_u32(v);
		b = fanom64_load_u32(v+4);
		break;
	case 9: case 10: case 11: case 12: case 13: case 14: case 15:
		a = fanom64_load_u64(v);
		b = fanom64_load_u64(v+len-8);
		break;
	default:
		for(; len>16; len-=16, v+=16) {
			a ^= fanom64_load_u64(v);
			b ^= fanom64_load_u64(v+8);
			c += a;
			d += b;
			a = fn_rotl(a, 5) - d;
			b = fn_rotl(b, 7) - c;
			c = fn_rotl(c, 24) ^ a;
			d = fn_rotl(d, 1) ^ b;
		}
		a ^= fanom64_load_u64(v+len-16);
		b ^= fanom64_load_u64(v+len-8);
	}
	c += b; c -= fn_rotl(a, 9);
	d += a; d -= fn_rotl(b, 58);
	t = (seed1 ^ l) - fn_rotl(a^b, 7);
	t ^= d; t -= fn_rotl(d,46);
	c ^= t; c -= fn_rotl(t,22);
	d ^= c; d -= fn_rotl(c,50);
	t ^= d; t -= fn_rotl(d,31);
	c ^= t; c -= fn_rotl(t,7);
	d ^= c; d -= fn_rotl(c,27);
	t ^= d; t -= fn_rotl(d,48);
	return t;
}

static inline uint64_t
fanom64_string_hash(const void* d, size_t len, uint64_t seed)
{
	const uint64_t c = ((uint64_t)0x6385a7c9<<32)|0xeb0d2f41;
	return fanom64_permute_string((const uint8_t*)d, len, seed, seed^c);
}

static inline uint64_t
fanom64_string_hash2(const void* d, size_t len, uint64_t seed1, uint64_t seed2)
{
	return fanom64_permute_string((const uint8_t*)d, len, seed1, seed2);
}
#undef fn_rotl
#endif
