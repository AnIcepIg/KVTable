#pragma once

static inline int bit_is_power2(unsigned v)
{
	return !(v & (v - 1));
}

static inline unsigned bit_round_up_power2(unsigned v)
{
	if (bit_is_power2(v)) return v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

static inline int bit_max(int x, int y)
{
	return x ^ ((x ^ y) & -(x < y));
}

static inline unsigned bit_max(unsigned x, unsigned y)
{
	return x ^ ((x ^ y) & -(x < y));
}

static inline int bit_min(int x, int y)
{
	return y ^ ((x ^ y) & -(x < y));
}

static inline unsigned bit_min(unsigned x, unsigned y)
{
	return y ^ ((x ^ y) & -(x < y));
}
