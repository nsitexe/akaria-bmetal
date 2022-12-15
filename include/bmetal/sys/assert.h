/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_ASSERT_H_
#define BAREMETAL_CRT_SYS_ASSERT_H_

#include <bmetal/bmetal.h>

#if !defined(__ASSEMBLER__)

#if __STDC_VERSION__ >= 201112L && !defined __cplusplus
#define static_assert    _Static_assert
#endif

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_SYS_ASSERT_H_ */
