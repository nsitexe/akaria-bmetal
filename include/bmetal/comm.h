/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DRV_COMM
#define BAREMETAL_CRT_DRV_COMM

#if !defined(__ASSEMBLER__)
#include <stdint.h>
#endif /* !__ASSEMBLER__ */

#define BAREMETAL_CRT_COMM_SECTION         ".comm"
#define BAREMETAL_CRT_COMM_AREA_SECTION    ".comm_area"
#define BAREMETAL_CRT_COMM_MAGIC           0xbadcafe5

#define __COMM_ARG_NOTUSED    0
/* Direct value */
#define __COMM_ARG_VAL        1
/* cl_mem */
#define __COMM_ARG_MEM        2

#if !defined(__ASSEMBLER__)

struct __comm_section {
	uint32_t magic;
};

/**
 * @brief Header of comm_area section that is for host and device
 *        communication in accelerator mode.
 */
struct __comm_area_header {
	/**
	 * Store magic number (see BAREMETAL_CRT_COMM_MAGIC macro).
	 * Runtime ignore the section if this is not equal magic number.
	 */
	uint32_t magic;
	/** Number of arguments of an application main function. */
	uint32_t num_args;
	/** Set 0 if an application is running, 1 if finished. */
	uint32_t done;

	/** Store return value of an application main function. */
	uint32_t ret_main;
};

/**
 * @brief Header of an argument for main function.
 *
 * @code
 * argv[0]
 *   header
 *   value
 * argv[1]
 *   header
 *   value
 * ...
 * argv[num_args - 1]
 *   header
 *   value
 * @endcode
 */
struct __comm_arg_header {
	/** Type of argument (see COMM_ARG_ macros). */
	uint32_t argtype;
	/**
	 * Index of argument. Currently runtime do not refer this parameter.
	 * Arguments are sorted by host side and parsed from top to bottom.
	 * */
	uint32_t index;
	/** Size of argument. */
	uint64_t size;
};

/* Communication area for host */
extern char __comm_area[];

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_DRV_COMM */
