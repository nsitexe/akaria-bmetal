/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_TIME_H_
#define BAREMETAL_CRT_SYS_TIME_H_

#include <stdint.h>

#include <bmetal/bmetal.h>
#include <bmetal/sys/types.h>

struct timespec {
	time_t tv_sec;
	long tv_nsec;
};

#endif /* BAREMETAL_CRT_SYS_TIME_H_ */
