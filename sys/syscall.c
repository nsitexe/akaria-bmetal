/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>

#include <bmetal/printk.h>
#include <bmetal/syscall.h>

intptr_t __sys_unknown(intptr_t number, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f)
{
	printk("unknown syscall %"PRIdPTR"\n", number);

	return -ENOTSUP;
}
