/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_INTR_H_
#define BAREMETAL_CRT_INTR_H_

#include <bmetal/bmetal.h>

#if !defined(__ASSEMBLER__)

void __intr_enable_local(void);
void __intr_disable_local(void);
void __intr_restore_local(const long status);
void __intr_save_local(long *status);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_INTR_H_ */
