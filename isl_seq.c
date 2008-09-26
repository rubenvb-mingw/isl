#include "isl_seq.h"

void isl_seq_clr(isl_int *p, unsigned len)
{
	int i;
	for (i = 0; i < len; ++i)
		isl_int_set_si(p[i], 0);
}

void isl_seq_neg(isl_int *dst, isl_int *src, unsigned len)
{
	int i;
	for (i = 0; i < len; ++i)
		isl_int_neg(dst[i], src[i]);
}

void isl_seq_cpy(isl_int *dst, isl_int *src, unsigned len)
{
	int i;
	for (i = 0; i < len; ++i)
		isl_int_set(dst[i], src[i]);
}

void isl_seq_swp_or_cpy(isl_int *dst, isl_int *src, unsigned len)
{
	int i;
	for (i = 0; i < len; ++i)
		isl_int_swap_or_set(dst[i], src[i]);
}

void isl_seq_scale(isl_int *dst, isl_int *src, isl_int m, unsigned len)
{
	int i;
	for (i = 0; i < len; ++i)
		isl_int_mul(dst[i], src[i], m);
}

void isl_seq_scale_down(isl_int *dst, isl_int *src, isl_int m, unsigned len)
{
	int i;
	for (i = 0; i < len; ++i)
		isl_int_divexact(dst[i], src[i], m);
}

void isl_seq_combine(isl_int *dst, isl_int m1, isl_int *src1,
			isl_int m2, isl_int *src2, unsigned len)
{
	int i;
	isl_int tmp;

	isl_int_init(tmp);
	for (i = 0; i < len; ++i) {
		isl_int_mul(tmp, m1, src1[i]);
		isl_int_addmul(tmp, m2, src2[i]);
		isl_int_set(dst[i], tmp);
	}
	isl_int_clear(tmp);
}

/*
 * Let d = dst[pos] and s = src[pos]
 * dst is replaced by |s| dst - sgn(s)d src
 */
void isl_seq_elim(isl_int *dst, isl_int *src, unsigned pos, unsigned len,
		  isl_int *m)
{
	isl_int a;
	isl_int b;

	if (isl_int_is_zero(dst[pos]))
		return;

	isl_int_init(a);
	isl_int_init(b);

	isl_int_gcd(a, src[pos], dst[pos]);
	isl_int_divexact(b, dst[pos], a);
	if (isl_int_is_pos(src[pos]))
		isl_int_neg(b, b);
	isl_int_divexact(a, src[pos], a);
	isl_int_abs(a, a);
	isl_seq_combine(dst, a, dst, b, src, len);

	if (m)
		isl_int_mul(*m, *m, a);

	isl_int_clear(a);
	isl_int_clear(b);
}

int isl_seq_eq(isl_int *p1, isl_int *p2, unsigned len)
{
	int i;
	for (i = 0; i < len; ++i)
		if (isl_int_ne(p1[i], p2[i]))
			return 0;
	return 1;
}

int isl_seq_cmp(isl_int *p1, isl_int *p2, unsigned len)
{
	int i;
	int cmp;
	for (i = 0; i < len; ++i)
		if ((cmp = isl_int_cmp(p1[i], p2[i])) != 0)
			return cmp;
	return 0;
}

int isl_seq_is_neg(isl_int *p1, isl_int *p2, unsigned len)
{
	int i;

	for (i = 0; i < len; ++i) {
		if (isl_int_abs_ne(p1[i], p2[i]))
			return 0;
		if (isl_int_is_zero(p1[i]))
			continue;
		if (isl_int_eq(p1[i], p2[i]))
			return 0;
	}
	return 1;
}

int isl_seq_first_non_zero(isl_int *p, unsigned len)
{
	int i;

	for (i = 0; i < len; ++i)
		if (!isl_int_is_zero(p[i]))
			return i;
	return -1;
}

int isl_seq_abs_min_non_zero(isl_int *p, unsigned len)
{
	int i, min = isl_seq_first_non_zero(p, len);
	if (min < 0)
		return -1;
	for (i = min + 1; i < len; ++i) {
		if (isl_int_is_zero(p[i]))
			continue;
		if (isl_int_abs_lt(p[i], p[min]))
			min = i;
	}
	return min;
}

void isl_seq_gcd(isl_int *p, unsigned len, isl_int *gcd)
{
	int i, min = isl_seq_abs_min_non_zero(p, len);

	if (min < 0) {
		isl_int_set_si(*gcd, 0);
		return;
	}
	isl_int_abs(*gcd, p[min]);
	for (i = 0; isl_int_cmp_si(*gcd, 1) > 0 && i < len; ++i) {
		if (i == min)
			continue;
		if (isl_int_is_zero(p[i]))
			continue;
		isl_int_gcd(*gcd, *gcd, p[i]);
	}
}

void isl_seq_inner_product(isl_int *p1, isl_int *p2, unsigned len,
			   isl_int *prod)
{
	int i;
	if (len == 0) {
		isl_int_set_si(*prod, 0);
		return;
	}
	isl_int_mul(*prod, p1[0], p2[0]);
	for (i = 1; i < len; ++i)
		isl_int_addmul(*prod, p1[i], p2[i]);
}

u_int32_t isl_seq_hash(isl_int *p, unsigned len, unsigned bits)
{
	int i;
	u_int32_t hash = 2166136261u;

	for (i = 0; i < len; ++i) {
		if (isl_int_is_zero(p[i]))
			continue;
		hash *= 16777619;
		hash ^= (i & 0xFF);
		hash = isl_int_hash(p[i], hash);
	}
	if (bits == 32)
		return hash;
	if (bits >= 16)
		return (hash >> bits) ^ (hash & (((u_int32_t)1 << bits) - 1));
	return ((hash >> bits) ^ hash) & (((u_int32_t)1 << bits) - 1);
}
