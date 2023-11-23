/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_INTR_H_
#define BAREMETAL_CRT_INTR_H_

#include <bmetal/bmetal.h>

#if !defined(__ASSEMBLER__)

void k_intr_enable_local(void);
void k_intr_disable_local(void);
void k_intr_restore_local(const long status);
void k_intr_save_local(long *status);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_INTR_H_ */
