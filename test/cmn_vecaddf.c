#include <math.h>
#include <stdio.h>

#include "cmn_vecaddf.h"

#if __riscv_vector == 1
void vecadd_rvv(const float *a, const float *b, float *c, int n)
{
	vfloat32m1_t va, vb, vc;
	size_t l;

	dbgprintf("----- use rvv f32\n");

	for (; n > 0; n -= l) {
		l = vsetvl_e32m1(n);
		va = vle32_v_f32m1(a, l);
		a += l;
		vb = vle32_v_f32m1(b, l);
		b += l;
		vc = vfadd_vv_f32m1(va, vb, l);
		vse32_v_f32m1(c, vc, l);
		c += l;
	}
}
#endif /* __riscv_vector == 1 */

void vecadd_scalar(const float *a, const float *b, float *c, int n)
{
	dbgprintf("----- use scalar f32\n");

	for (int i = 0; i < n; i++) {
		c[i] = a[i] + b[i];
	}
}

int fp_eq(float reference, float actual, float relErr)
{
	/* if near zero, do absolute error instead. */
	float absErr = relErr * ((fabsf(reference) > relErr) ? fabsf(reference) : relErr);
	return fabsf(actual - reference) < absErr;
}
