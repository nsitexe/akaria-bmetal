/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_GEN_TYPES_H_
#define BAREMETAL_CRT_ARCH_GEN_TYPES_H_

#if !defined(__ASSEMBLER__)
#include <stdint.h>
#endif /* !__ASSEMBLER__ */

#include <bmetal/bmetal.h>

#if !defined(__ASSEMBLER__)

#define K_SCHAR_TYPE        signed char
#define K_SSHORT_TYPE       signed short
#define K_SINT_TYPE         signed int
#define K_SLONG_TYPE        signed long
#define K_SLONGLONG_TYPE    signed long long
#define K_UCHAR_TYPE        unsigned char
#define K_USHORT_TYPE       unsigned short
#define K_UINT_TYPE         unsigned int
#define K_ULONG_TYPE        unsigned long
#define K_ULONGLONG_TYPE    unsigned long long

#define K_SINT8_TYPE        int8_t
#define K_SINT16_TYPE       int16_t
#define K_SINT32_TYPE       int32_t
#define K_SINT64_TYPE       int64_t
#define K_UINT8_TYPE        uint8_t
#define K_UINT16_TYPE       uint16_t
#define K_UINT32_TYPE       uint32_t
#define K_UINT64_TYPE       uint64_t

#ifndef k_arch_clockid_t
#define k_arch_clockid_t    K_SINT_TYPE
#endif

#ifndef k_arch_mode_t
#define k_arch_mode_t    K_USHORT_TYPE
#endif

#ifndef k_arch_loff_t
#define k_arch_loff_t    K_SLONGLONG_TYPE
#endif

#ifndef k_arch_off_t
#define k_arch_off_t     K_SLONG_TYPE
#endif

#ifndef k_arch_rlim_t
#define k_arch_rlim_t     K_ULONG_TYPE
#endif

#ifndef k_arch_ssize_t
#if UINTPTR_MAX == UINT64_MAX
#define k_arch_ssize_t      K_SLONG_TYPE
#else /* UINTPTR_MAX != UINT64_MAX */
#define k_arch_ssize_t      K_SINT_TYPE
#endif /* UINTPTR_MAX != UINT64_MAX */
#endif

#ifndef k_arch_pid_t
#define k_arch_pid_t    K_SINT_TYPE
#endif

#ifndef k_arch_suseconds_t
#define k_arch_suseconds_t    K_SLONG_TYPE
#endif

#ifndef k_arch_time_t
#define k_arch_time_t    K_SLONGLONG_TYPE
#endif

#ifndef k_arch_uid_t
#define k_arch_uid_t    K_UINT_TYPE
#endif

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_ARCH_GEN_TYPES_H_ */
