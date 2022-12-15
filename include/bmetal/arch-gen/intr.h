/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_ARCH_GEN_INTR_H_
#define BAREMETAL_CRT_ARCH_GEN_INTR_H_

#include <bmetal/bmetal.h>

#if !defined(__ASSEMBLER__)

#ifndef __arch_get_intr_handler
#  error Please implement arch_get_intr_handler() function.
#endif

#ifndef __arch_set_intr_handler
#  error Please implement arch_set_intr_handler() function.
#endif

#ifndef __arch_get_exc_handler
#  error Please implement arch_get_exc_handler() function.
#endif

#ifndef __arch_set_exc_handler
#  error Please implement arch_set_exc_handler() function.
#endif

#ifndef __arch_intr_enable_local
#  error Please implement arch_intr_enable_local() function.
#endif

#ifndef __arch_intr_disable_local
#  error Please implement arch_intr_disable_local() function.
#endif

#ifndef __arch_intr_restore_local
#  error Please implement arch_intr_restore_local() function.
#endif

#ifndef __arch_intr_save_local
#  error Please implement arch_intr_save_local() function.
#endif

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_ARCH_GEN_INTR_H_ */
