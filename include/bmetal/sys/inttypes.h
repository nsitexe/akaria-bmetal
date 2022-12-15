/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_INTTYPES_H_
#define BAREMETAL_CRT_SYS_INTTYPES_H_

#if !defined(__ASSEMBLER__)
#include <stdint.h>
#endif /* !__ASSEMBLER__ */

#include <bmetal/bmetal.h>

#if !defined(__ASSEMBLER__)

#if CONFIG_USE_NEWLIB && (UINTPTR_MAX == UINT32_MAX)
#define __PREFIX32 "l"
#else
#define __PREFIX32 ""
#endif

#if UINTPTR_MAX == UINT64_MAX
#define __PREFIX64  "l"
#define __PREFIXPTR "l"
#else
#define __PREFIX64  "ll"
#define __PREFIXPTR ""
#endif

#define PRId8   "d"
#define PRId16  "d"
#define PRId32  __PREFIX32  "d"
#define PRId64  __PREFIX64  "d"
#define PRIdMAX __PREFIX64  "d"
#define PRIdPTR __PREFIXPTR "d"

#define PRIdLEAST8  "d"
#define PRIdLEAST16 "d"
#define PRIdLEAST32 __PREFIX32 "d"
#define PRIdLEAST64 __PREFIX64 "d"

#define PRIdFAST8  "d"
#define PRIdFAST16 "d"
#define PRIdFAST32 __PREFIX32 "d"
#define PRIdFAST64 __PREFIX64 "d"

#define PRIi8   "i"
#define PRIi16  "i"
#define PRIi32  __PREFIX32  "i"
#define PRIi64  __PREFIX64  "i"
#define PRIiMAX __PREFIX64  "i"
#define PRIiPTR __PREFIXPTR "i"

#define PRIiLEAST8  "i"
#define PRIiLEAST16 "i"
#define PRIiLEAST32 __PREFIX32 "i"
#define PRIiLEAST64 __PREFIX64 "i"

#define PRIiFAST8  "i"
#define PRIiFAST16 "i"
#define PRIiFAST32 __PREFIX32 "i"
#define PRIiFAST64 __PREFIX64 "i"

#define PRIo8   "o"
#define PRIo16  "o"
#define PRIo32  __PREFIX32  "o"
#define PRIo64  __PREFIX64  "o"
#define PRIoMAX __PREFIX64  "o"
#define PRIoPTR __PREFIXPTR "o"

#define PRIoLEAST8  "o"
#define PRIoLEAST16 "o"
#define PRIoLEAST32 __PREFIX32 "o"
#define PRIoLEAST64 __PREFIX64 "o"

#define PRIoFAST8  "o"
#define PRIoFAST16 "o"
#define PRIoFAST32 __PREFIX32 "o"
#define PRIoFAST64 __PREFIX64 "o"

#define PRIu8   "u"
#define PRIu16  "u"
#define PRIu32  __PREFIX32  "u"
#define PRIu64  __PREFIX64  "u"
#define PRIuMAX __PREFIX64  "u"
#define PRIuPTR __PREFIXPTR "u"

#define PRIuLEAST8  "u"
#define PRIuLEAST16 "u"
#define PRIuLEAST32 __PREFIX32 "u"
#define PRIuLEAST64 __PREFIX64 "u"

#define PRIuFAST8  "u"
#define PRIuFAST16 "u"
#define PRIuFAST32 __PREFIX32 "u"
#define PRIuFAST64 __PREFIX64 "u"

#define PRIx8   "x"
#define PRIx16  "x"
#define PRIx32  __PREFIX32  "x"
#define PRIx64  __PREFIX64  "x"
#define PRIxMAX __PREFIX64  "x"
#define PRIxPTR __PREFIXPTR "x"

#define PRIxLEAST8  "x"
#define PRIxLEAST16 "x"
#define PRIxLEAST32 __PREFIX32 "x"
#define PRIxLEAST64 __PREFIX64 "x"

#define PRIxFAST8  "x"
#define PRIxFAST16 "x"
#define PRIxFAST32 __PREFIX32 "x"
#define PRIxFAST64 __PREFIX64 "x"

#define PRIX8   "X"
#define PRIX16  "X"
#define PRIX32  __PREFIX32  "X"
#define PRIX64  __PREFIX64  "X"
#define PRIXMAX __PREFIX64  "X"
#define PRIXPTR __PREFIXPTR "X"

#define PRIXLEAST8  "X"
#define PRIXLEAST16 "X"
#define PRIXLEAST32 __PREFIX32 "X"
#define PRIXLEAST64 __PREFIX64 "X"

#define PRIXFAST8  "X"
#define PRIXFAST16 "X"
#define PRIXFAST32 __PREFIX32 "X"
#define PRIXFAST64 __PREFIX64 "X"

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_SYS_INTTYPES_H_ */
