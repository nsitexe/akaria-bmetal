/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_REBOOT_H_
#define BAREMETAL_CRT_SYS_REBOOT_H_

#include <bmetal/bmetal.h>

#define REBOOT_MAGIC1     0xfee1dead
#define REBOOT_MAGIC2     672274793

#define RB_AUTOBOOT       0x01234567
#define RB_DISABLE_CAD    0
#define RB_ENABLE_CAD     0x89abcdef
#define RB_HALT_SYSTEM    0xcdef0123
#define RB_KEXEC          0x45584543
#define RB_POWER_OFF      0x4321fedc
#define RB_SW_SUSPEND     0xd000fce2

#endif /* BAREMETAL_CRT_SYS_REBOOT_H_ */
