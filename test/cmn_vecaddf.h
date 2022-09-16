#ifndef CMN_VECADDF_H__
#define CMN_VECADDF_H__

#include <stdio.h>

#if __riscv_vector == 1
#include <riscv_vector.h>

#define vecadd    vecadd_rvv
#else /* __riscv_vector == 1 */
#define vecadd    vecadd_scalar
#endif /* __riscv_vector == 1 */

#ifdef DEBUG
#define dbgprintf    printf
#else
#define dbgprintf    noprintf
#endif

static inline int __attribute__((format(printf, 1, 2))) noprintf(const char *format, ...)
{
	return 0;
}

#if __riscv_vector == 1
void vecadd_rvv(const float *a, const float *b, float *c, int n);
#endif /* __riscv_vector == 1 */

void vecadd_scalar(const float *a, const float *b, float *c, int n);
int fp_eq(float reference, float actual, float relErr);

#endif /* CMN_VECADDF_H__ */
