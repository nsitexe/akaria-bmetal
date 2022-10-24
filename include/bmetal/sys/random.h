/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_RANDOM_H_
#define BAREMETAL_CRT_SYS_RANDOM_H_

#include <bmetal/bmetal.h>

/* Flags for sys_getrandom */
#define GRND_NONBLOCK    0x0001
#define GRND_RANDOM      0x0002
#define GRND_INSECURE    0x0004

#endif /* BAREMETAL_CRT_SYS_RANDOM_H_ */
