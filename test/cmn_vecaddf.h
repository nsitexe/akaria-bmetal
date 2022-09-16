#ifndef CMN_VECADDF_H__
#define CMN_VECADDF_H__

#include <stdio.h>

/* Please define this macro to show debug messages */
#define DEBUG_PRINT

#if __riscv_vector == 1
#include <riscv_vector.h>

#define vecadd    vecadd_rvv
#else /* __riscv_vector == 1 */
#define vecadd    vecadd_scalar
#endif /* __riscv_vector == 1 */

#ifdef USE_FLOAT
#define vecadd_rvv       vecadd32_rvv
#define vecadd_scalar    vecadd32_scalar
#define fp_eq            fp32_eq
#endif /* USE_FLOAT */

#ifdef USE_DOUBLE
#define vecadd_rvv       vecadd64_rvv
#define vecadd_scalar    vecadd64_scalar
#define fp_eq            fp64_eq
#endif /* USE_DOUBLE */

#ifdef DEBUG_PRINT
#define dbgprintf    printf
#else /* DEBUG_PRINT */
#define dbgprintf    noprintf
#endif /* DEBUG_PRINT */

static inline int __attribute__((format(printf, 1, 2))) noprintf(const char *format, ...)
{
	return 0;
}

#if __riscv_vector == 1
void vecadd32_rvv(const float *a, const float *b, float *c, int n);
void vecadd64_rvv(const double *a, const double *b, double *c, int n);
#endif /* __riscv_vector == 1 */

void vecadd32_scalar(const float *a, const float *b, float *c, int n);
void vecadd64_scalar(const double *a, const double *b, double *c, int n);
int fp32_eq(float reference, float actual, float relErr);
int fp64_eq(double reference, double actual, double relErr);
int check32(const float *a, const float *b, const float *c, float *expect, int n);
int check64(const double *a, const double *b, const double *c, double *expect, int n);
void dump32(const float *a, const float *b, const float *c, int n);
void dump64(const double *a, const double *b, const double *c, int n);

#endif /* CMN_VECADDF_H__ */
