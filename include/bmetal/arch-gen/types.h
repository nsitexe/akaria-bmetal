/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_GEN_TYPES_H_
#define BAREMETAL_CRT_ARCH_GEN_TYPES_H_

#if !defined(__ASSEMBLER__)
#include <stdint.h>
#endif /* !__ASSEMBLER__ */

#include <bmetal/bmetal.h>

#if !defined(__ASSEMBLER__)

#define __SCHAR_TYPE        signed char
#define __SSHORT_TYPE       signed short
#define __SINT_TYPE         signed int
#define __SLONG_TYPE        signed long
#define __SLONGLONG_TYPE    signed long long
#define __UCHAR_TYPE        unsigned char
#define __USHORT_TYPE       unsigned short
#define __UINT_TYPE         unsigned int
#define __ULONG_TYPE        unsigned long
#define __ULONGLONG_TYPE    unsigned long long

#define __SINT8_TYPE        int8_t
#define __SINT16_TYPE       int16_t
#define __SINT32_TYPE       int32_t
#define __SINT64_TYPE       int64_t
#define __UINT8_TYPE        uint8_t
#define __UINT16_TYPE       uint16_t
#define __UINT32_TYPE       uint32_t
#define __UINT64_TYPE       uint64_t

#ifndef __arch_clockid_t
#define __arch_clockid_t    __SINT_TYPE
#endif

#ifndef __arch_loff_t
#define __arch_loff_t    __SLONGLONG_TYPE
#endif

#ifndef __arch_off_t
#define __arch_off_t     __SLONG_TYPE
#endif

#ifndef __arch_rlim_t
#define __arch_rlim_t     __ULONG_TYPE
#endif

#ifndef __arch_ssize_t
#if UINTPTR_MAX == UINT64_MAX
#define __arch_ssize_t      __SLONG_TYPE
#else /* UINTPTR_MAX != UINT64_MAX */
#define __arch_ssize_t      __SINT_TYPE
#endif /* UINTPTR_MAX != UINT64_MAX */
#endif

#ifndef __arch_pid_t
#define __arch_pid_t    __SINT_TYPE
#endif

#ifndef __arch_suseconds_t
#define __arch_suseconds_t    __SLONG_TYPE
#endif

#ifndef __arch_time_t
#define __arch_time_t    __SLONGLONG_TYPE
#endif

#ifndef __arch_uid_t
#define __arch_uid_t    __UINT_TYPE
#endif

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_ARCH_GEN_TYPES_H_ */
