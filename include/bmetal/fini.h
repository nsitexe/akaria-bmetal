/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_FINI_H_
#define BAREMETAL_CRT_FINI_H_

#include <bmetal/bmetal.h>

#if !defined(__ASSEMBLER__)

int __fini_reboot(void);
int __fini_power_off(void);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_FINI_H_ */
