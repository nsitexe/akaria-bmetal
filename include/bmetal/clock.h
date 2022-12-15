/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_CLOCK_H_
#define BAREMETAL_CRT_CLOCK_H_

#include <bmetal/bmetal.h>
#include <bmetal/sys/time.h>

#if !defined(__ASSEMBLER__)

int __clock_get_realtime(struct timespec64 *tsp);
int __clock_set_realtime(const struct timespec64 *tsp);
int __clock_get_monotonic(struct timespec64 *tsp);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_CLOCK_H_ */
