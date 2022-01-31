/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DRV_COMM
#define BAREMETAL_CRT_DRV_COMM

#include <stdint.h>

#define BAREMETAL_CRT_COMM_SECTION         ".comm"
#define BAREMETAL_CRT_COMM_AREA_SECTION    ".comm_area"
#define BAREMETAL_CRT_COMM_MAGIC           0xbadcafe5

struct __comm_section {
	uint32_t magic;
};

struct __comm_area_header {
	uint32_t magic;
	uint32_t num_args;
	uint32_t done;
	uint32_t ret_main;
};

struct __comm_arg_header {
	uint32_t argtype;
	uint32_t index;
	uint64_t size;
};

#define __COMM_ARG_NOTUSED    0
/* Direct value */
#define __COMM_ARG_VAL        1
/* cl_mem */
#define __COMM_ARG_MEM        2

/* Communication area for host */
extern char __comm_area[];

#endif /* BAREMETAL_CRT_DRV_COMM */
