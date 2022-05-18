/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stdint.h>
#include <unistd.h>

#include <bmetal/syscall.h>
#include <bmetal/comm.h>
#include <bmetal/file.h>
#include <bmetal/inttypes.h>
#include <bmetal/lock.h>
#include <bmetal/mman.h>
#include <bmetal/printk.h>
#include <bmetal/smp.h>
#include <bmetal/string.h>
#include <bmetal/thread.h>

#if (CONFIG_HEAP_SIZE % __PAGE_SIZE) != 0
#  error Invalid heap size. It should be aligned page size. \
         Please check configs about HEAP_SIZE.
#endif

static struct __spinlock lock_mmap;
static char brk_area[CONFIG_BRK_SIZE];
static char *brk_cur = brk_area;
static char heap_area[CONFIG_HEAP_SIZE];
static char heap_used[CONFIG_HEAP_SIZE / __PAGE_SIZE];
static int dbg_heap_num = 1;

static const struct new_utsname uname = {
	.sysname    = "Linux",
	.nodename   = "",
	.release    = "5.15.0",
	.version    = "5.15.0",
	.machine    = "rv",
	.domainname = "",
};

intptr_t __sys_unknown(intptr_t number, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f)
{
	printk("%d: unknown syscall %"PRIdPTR"\n", __arch_get_cpu_id(), number);

	return -ENOTSUP;
}

int __sys_uname(struct new_utsname *name)
{
	kmemcpy(name, &uname, sizeof(uname));

	return 0;
}

static struct __file_desc *get_file_desc(int fd)
{
	struct __proc_info *pi = __proc_get_current();

	if (fd < 0 || CONFIG_MAX_FD <= fd) {
		printk("get_file_desc: fd %d is invalid\n", fd);
		return NULL;
	}

	return pi->fdset[fd];
}

static struct __file_desc *set_file_desc(int fd, struct __file_desc *desc)
{
	struct __proc_info *pi = __proc_get_current();
	struct __file_desc *olddesc;

	if (fd < 0 || CONFIG_MAX_FD <= fd) {
		printk("set_file_desc: fd %d is invalid\n", fd);
		return NULL;
	}

	olddesc = pi->fdset[fd];
	pi->fdset[fd] = desc;

	return olddesc;
}

int __sys_close(int fd)
{
	struct __file_desc *desc = get_file_desc(fd);
	int ret = 0;

	if (!desc) {
		return -EBADF;
	}

	if (desc->ops && desc->ops->close) {
		ret = desc->ops->close(desc);
	}

	set_file_desc(fd, NULL);

	return ret;
}

ssize_t __sys_write(int fd, const void *buf, size_t count)
{
	struct __file_desc *desc = get_file_desc(fd);
	ssize_t ret = 0;

	if (!desc || !desc->ops || !desc->ops->write) {
		return -EBADF;
	}
	if (count == 0) {
		return 0;
	}

	//printk("SYS_write(): fd:%d cnt:%d rd:%d\n", fd, (int)count, (int)ret);

	if (desc->ops && desc->ops->write) {
		ret = desc->ops->write(desc, buf, count);
	}

	return ret;
}

long __sys_exit(int status)
{
	struct __cpu_device *cpu = __cpu_get_current();
	struct __thread_info *ti;
	uintptr_t v;
	int r;

	__smp_lock();

	ti = __cpu_get_thread_task(cpu);
	if (!ti) {
		printk("sys_exit: cannot get task thread.\n");

		__smp_unlock();
		return -EINVAL;
	}

	r = __thread_stop(ti);
	if (r) {
		__smp_unlock();
		return r;
	}

	r = __thread_destroy(ti);
	if (r) {
		__smp_unlock();
		return r;
	}

	/* Notify to host when leader exit */
	if (__thread_get_leader(ti)) {
		struct __comm_area_header *h_area = (struct __comm_area_header *)__comm_area;

		h_area->ret_main = status;
		h_area->done = 1;
	}

	dwmb();
	__smp_unlock();

	__sys_context_switch();

	__arch_get_arg(cpu->regs, __ARCH_ARG_TYPE_RETVAL, &v);

	return v;
}

void *__sys_brk(void *addr)
{
	void *brk_start = &brk_area[0];
	void *brk_end = &brk_area[CONFIG_BRK_SIZE];

	if (addr == NULL) {
		return brk_cur;
	}
	if (addr < brk_start || brk_end < addr) {
		printk("sys_brk: addr:%p is out of bounds.\n", addr);
		return INT_TO_PTR(-ENOMEM);
	}

	brk_cur = addr;

	return addr;
}

static int __mmap_lock(void)
{
	__spinlock_lock(&lock_mmap);

	return 0;
}

static int __mmap_unlock(void)
{
	__spinlock_unlock(&lock_mmap);

	return 0;
}

static void *heap_area_start(void)
{
	return &heap_area[0];
}

static void *heap_area_end(void)
{
	return &heap_area[0] + CONFIG_HEAP_SIZE;
}

static void dump_heap_pages(void)
{
	printk("dump_heap_pages:\n");
	for (size_t i = 0; i < ARRAY_OF(heap_used); i++) {
		if (i % 16 == 0) {
			printk("  %4d: ", (int)i);
		}
		printk("%2x ", heap_used[i]);
		if (i % 16 == 15) {
			printk("\n");
		}
	}
	printk("\n");
}

static size_t get_cont_pages_num(size_t off_page)
{
	size_t r = 1;

	for (size_t i = off_page; i < ARRAY_OF(heap_used); i++, r++) {
		if (heap_used[i]) {
			break;
		}
	}

	return r;
}

static int set_page_flag(size_t off_page, size_t n_page, int val)
{
	for (size_t i = off_page; i < off_page + n_page; i++) {
		heap_used[i] = val;
	}

	return 0;
}

static size_t alloc_pages(size_t len)
{
	size_t size_page = (len + __PAGE_SIZE - 1) / __PAGE_SIZE;
	size_t i = 0;

	while (i < ARRAY_OF(heap_used)) {
		if (heap_used[i]) {
			i++;
			continue;
		}

		size_t n_page = get_cont_pages_num(i);
		if (n_page >= size_page) {
			set_page_flag(i, size_page, dbg_heap_num);
			dbg_heap_num += 1;

			printk("alloc_pages: heap:%d, page:%d-%d (%d KB)\n", dbg_heap_num,
				(int)i, (int)(i + size_page - 1), (int)(size_page * __PAGE_SIZE / 1024));
			//dump_heap_pages();
			return i;
		}

		i += n_page;
	}

	return -1;
}

static int free_pages(void *start, size_t length)
{
	uintptr_t off_addr = start - heap_area_start();
	size_t off_page = off_addr / __PAGE_SIZE;
	size_t n_page = length / __PAGE_SIZE;
	int need_dump = 0;

	if (off_addr & (__PAGE_SIZE - 1)) {
		n_page += 1;
	}

	for (size_t i = off_page; i < off_page + n_page; i++) {
		if (!heap_used[i]) {
			printk("free_pages: double free at %d.\n", (int)i);
			need_dump = 1;
		}
	}
	if (need_dump) {
		dump_heap_pages();
	}

	set_page_flag(off_page, n_page, 0);

	printk("free_pages: page:%d-%d (%d KB)\n", (int)off_page,
		(int)(off_page + n_page - 1), (int)(n_page * __PAGE_SIZE / 1024));

	return 0;
}

void *__sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	void *anon_ptr;
	int prot_req = PROT_READ | PROT_WRITE;
	int flags_req = MAP_PRIVATE | MAP_ANONYMOUS;
	size_t off_page;
	size_t len = length;

	/* Do not return same address for different area that has size == 0 */
	if (length == 0) {
		len = 4;
	}

	/* Ignore MAP_STACK */
	flags &= ~MAP_STACK;

	/*
	 * Only support anonymous mmap.
	 *   - PROT_READ || PROT_WRITE
	 *   - MAP_PRIVATE && MAP_ANONYMOUS
	 *   - fd = -1
	 *   - offset = 0
	 */
	if ((prot & prot_req) != 0 ||
	    (flags & flags_req) != flags_req ||
	    fd != -1 || offset != 0) {
		printk("sys_mmap: only support anonymous mmap.\n");
		return INT_TO_PTR(-EINVAL);
	}

	__mmap_lock();
	off_page = alloc_pages(len);
	__mmap_unlock();
	if (off_page == -1) {
		printk("sys_mmap: no enough pages, len:%d.\n", (int)len);
		return INT_TO_PTR(-ENOMEM);
	}
	anon_ptr = heap_area_start() + off_page * __PAGE_SIZE;

	kmemset(anon_ptr, 0, len);

	return anon_ptr;
}

int __sys_munmap(void *addr, size_t length)
{
	if (addr < heap_area_start() || heap_area_end() < addr + length) {
		return -EINVAL;
	}

	__mmap_lock();
	free_pages(addr, length);
	__mmap_unlock();

	return 0;
}

int __sys_mprotect(void *addr, size_t length, int prot)
{
	printk("sys_mprotect: ignore mprotect.\n");

	return 0;
}

long __sys_clone(unsigned long flags, void *child_stack, void *ptid, void *tls, void *ctid)
{
	struct __cpu_device *cpu_cur = __cpu_get_current(), *cpu;
	struct __proc_info *pi = __proc_get_current();
	struct __thread_info *ti;
	int r;

	__smp_lock();

	r = __smp_find_idle_cpu(&cpu);
	if (r) {
		goto err_out;
	}

	/* Init thread info */
	ti = __thread_create(pi);
	if (!ti) {
		r = -ENOMEM;
		goto err_out;
	}

	/* Copy user regs to the initial stack of new thread */
	ti->sp = child_stack;
	kmemcpy(&ti->regs, cpu_cur->regs, sizeof(__arch_user_regs_t));

	/* Return value and stack for new thread */
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_RETVAL, 0);
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_STACK, (uintptr_t)ti->sp);

	r = __thread_run(ti, cpu);
	if (r) {
		goto err_out;
	}

	/* Notify */
	dwmb();

	/* Return value for current thread */
	r = ti->tid;

err_out:
	__smp_unlock();

	return r;
}

long __sys_context_switch(void)
{
	struct __cpu_device *cpu = __cpu_get_current();
	struct __thread_info *ti, *ti_idle, *ti_task;
	uintptr_t v;

	__smp_lock();
	drmb();

	ti = __cpu_get_thread(cpu);
	ti_idle = __cpu_get_thread_idle(cpu);
	ti_task = __cpu_get_thread_task(cpu);

	if (ti == ti_idle) {
		kmemcpy(&ti->regs, cpu->regs, sizeof(__arch_user_regs_t));
	}

	if (ti && ti_task) {
		/* Switch to task */
		kmemcpy(cpu->regs, &ti_task->regs, sizeof(__arch_user_regs_t));
		__cpu_set_thread(cpu, ti_task);
	} else {
		/* Switch to idle */
		kmemcpy(cpu->regs, &ti_idle->regs, sizeof(__arch_user_regs_t));
		__cpu_set_thread(cpu, ti_idle);
	}

	__arch_get_arg(cpu->regs, __ARCH_ARG_TYPE_RETVAL, &v);

	dwmb();
	__smp_unlock();

	return v;
}
