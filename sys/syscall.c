/* SPDX-License-Identifier: Apache-2.0 */

#include <stdatomic.h>
#include <stdint.h>

#include <bmetal/syscall.h>
#include <bmetal/clock.h>
#include <bmetal/comm.h>
#include <bmetal/file.h>
#include <bmetal/init.h>
#include <bmetal/lock.h>
#include <bmetal/printk.h>
#include <bmetal/smp.h>
#include <bmetal/thread.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/futex.h>
#include <bmetal/sys/inttypes.h>
#include <bmetal/sys/mman.h>
#include <bmetal/sys/resource.h>
#include <bmetal/sys/sched.h>
#include <bmetal/sys/string.h>

#if (CONFIG_HEAP_SIZE % __PAGE_SIZE) != 0
#  error Invalid heap size. It should be aligned page size. \
         Please check configs about HEAP_SIZE.
#endif

static struct __spinlock lock_mmap;
static char __section(".noinit") brk_area[CONFIG_BRK_SIZE];
static char *brk_cur = brk_area;
static char __section(".noinit") __aligned(__PAGE_SIZE) heap_area[CONFIG_HEAP_SIZE];
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

intptr_t __sys_uname(struct new_utsname *name)
{
	kmemcpy(name, &uname, sizeof(uname));

	return 0;
}

intptr_t __sys_prlimit64(pid_t pid, int resource, const struct rlimit64 *new_lim, struct rlimit64 *old_lim)
{
	struct rlimit64 old;

	if (new_lim) {
		printk("prlimit64: not support to set new rlimit.\n");
		return -EPERM;
	}

	switch (resource) {
	case RLIMIT_STACK:
		old.rlim_cur = CONFIG_MAIN_STACK_SIZE;
		old.rlim_max = ~0;
		break;
	default:
		printk("prlimit64: not support to resource %d.\n", resource);
		return -EINVAL;
	}

	if (old_lim) {
		*old_lim = old;
	}

	return 0;
}

intptr_t __sys_getuid(void)
{
	return 0;
}

intptr_t __sys_geteuid(void)
{
	return 0;
}

intptr_t __sys_getgid(void)
{
	return 0;
}

intptr_t __sys_getegid(void)
{
	return 0;
}

intptr_t __sys_getpid(void)
{
	return __proc_get_pid();
}

intptr_t __sys_gettid(void)
{
	return __thread_get_tid();
}

intptr_t __sys_clock_gettime(clockid_t clock_id, struct timespec64 *tp)
{
	int r;

	if (!tp) {
		return -EFAULT;
	}

	switch (clock_id) {
	case CLOCK_REALTIME:
		r = __clock_get_realtime(tp);
		break;
	case CLOCK_MONOTONIC:
		r = __clock_get_monotonic(tp);
		break;
	default:
		printk("sys_clock_gettime: not supported clock_id:%d.\n", (int)clock_id);
		return -EINVAL;
	}
	if (r) {
		return r;
	}

	return 0;
}

intptr_t __sys_clock_settime(clockid_t clock_id, const struct timespec64 *tp)
{
	int r;

	if (!tp) {
		return -EFAULT;
	}

	switch (clock_id) {
	case CLOCK_REALTIME:
		r = __clock_set_realtime(tp);
		break;
	default:
		printk("sys_clock_settime: not supported clock_id:%d.\n", (int)clock_id);
		return -EINVAL;
	}
	if (r) {
		return r;
	}

	return 0;
}

intptr_t __sys_gettimeofday(struct timeval *tp, void *tzp)
{
	struct timespec64 tsp;
	int r;

	if (!tp) {
		return -EFAULT;
	}

	r = __clock_get_realtime(&tsp);
	if (r) {
		return r;
	}

	tp->tv_sec = tsp.tv_sec;
	tp->tv_usec = tsp.tv_nsec / 1000L;

	return 0;
}

intptr_t __sys_settimeofday(const struct timeval *tp, const void *tzp)
{
	struct timespec64 tsp;
	int r;

	if (!tp) {
		return -EFAULT;
	}

	tsp.tv_sec = tp->tv_sec;
	tsp.tv_nsec = tp->tv_usec * 1000L;

	r = __clock_set_realtime(&tsp);
	if (r) {
		return r;
	}

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

intptr_t __sys_close(int fd)
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

static ssize_t sys_write_nolock(struct __file_desc *desc, const void *buf, size_t count)
{
	ssize_t ret = 0;

	if (desc->ops && desc->ops->write) {
		ret = desc->ops->write(desc, buf, count);
	}

	return ret;
}

intptr_t __sys_write(int fd, const void *buf, size_t count)
{
	struct __file_desc *desc = get_file_desc(fd);
	ssize_t ret = 0;

	if (!desc || !desc->ops || !desc->ops->write) {
		return -EBADF;
	}
	if (count == 0) {
		return 0;
	}

	//printk("sys_write: fd:%d cnt:%d rd:%d\n", fd, (int)count, (int)ret);

	__spinlock_lock(&desc->lock);
	ret = sys_write_nolock(desc, buf, count);
	__spinlock_unlock(&desc->lock);

	return ret;
}

intptr_t __sys_writev(int fd, const struct iovec *iov, int iovcnt)
{
	struct __file_desc *desc = get_file_desc(fd);
	ssize_t ret = 0, wr;

	if (!desc || !desc->ops || !desc->ops->write) {
		return -EBADF;
	}
	if (iov < 0) {
		return -EINVAL;
	} else if (iov == 0) {
		return 0;
	}

	__spinlock_lock(&desc->lock);
	for (int i = 0; i < iovcnt; i++) {
		wr = sys_write_nolock(desc, iov[i].iov_base, iov[i].iov_len);
		if (wr > 0) {
			ret += wr;
		} else {
			printk("sys_writev: failed at %d, fd:%d len:%d.\n",
				i, fd, (int)iov[i].iov_len);
		}
	}
	__spinlock_unlock(&desc->lock);

	return ret;
}

void *__brk_area_start(void)
{
	return &brk_area[0];
}

void *__brk_area_end(void)
{
	return &brk_area[0] + CONFIG_BRK_SIZE;
}

intptr_t __sys_brk(void *addr)
{
	char *caddr = (char *)addr;

	if (addr == NULL) {
		return PTR_TO_INT(brk_cur);
	}
	if (addr < __brk_area_start() || __brk_area_end() < addr) {
		printk("sys_brk: addr:%p is out of bounds.\n", addr);
		return -ENOMEM;
	}

	if (caddr > brk_cur) {
		/* Expand: should zero clear by specification */
		kmemset(brk_cur, 0, caddr - brk_cur);
	} else {
		/* Shrink: zero clear for security */
		kmemset(addr, 0, brk_cur - caddr);
	}
	brk_cur = addr;

	return PTR_TO_INT(addr);
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

void *__heap_area_start(void)
{
	return &heap_area[0];
}

void *__heap_area_end(void)
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

static ssize_t alloc_pages(size_t len)
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

static int check_pages(void *start, size_t length)
{
	uintptr_t off_addr = start - __heap_area_start();
	size_t off_page = off_addr / __PAGE_SIZE;
	size_t n_page = length / __PAGE_SIZE;
	int failed = 0;

	if (off_addr & (__PAGE_SIZE - 1)) {
		n_page += 1;
	}

	for (size_t i = off_page; i < off_page + n_page; i++) {
		if (!heap_used[i]) {
			printk("check_pages: not allocated at %d.\n", (int)i);
			failed = 1;
		}
	}
	if (failed) {
		dump_heap_pages();

		return -EBADF;
	}

	return 0;
}

static int free_pages(void *start, size_t length)
{
	uintptr_t off_addr = start - __heap_area_start();
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

intptr_t __sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	void *anon_ptr;
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
	 *   - MAP_PRIVATE && MAP_ANONYMOUS
	 *   - fd = -1
	 *   - offset = 0
	 */
	if ((flags & flags_req) != flags_req ||
	    fd != -1 || offset != 0) {
		printk("sys_mmap: only support anonymous mmap.\n");
		return -EINVAL;
	}

	__mmap_lock();
	off_page = alloc_pages(len);
	__mmap_unlock();
	if (off_page == -1) {
		printk("sys_mmap: no enough pages, len:%d.\n", (int)len);
		return -ENOMEM;
	}
	anon_ptr = __heap_area_start() + off_page * __PAGE_SIZE;

	kmemset(anon_ptr, 0, len);

	return PTR_TO_INT(anon_ptr);
}

intptr_t __sys_munmap(void *addr, size_t length)
{
	if (addr < __heap_area_start() || __heap_area_end() < addr + length) {
		return -EINVAL;
	}

	__mmap_lock();
	free_pages(addr, length);
	__mmap_unlock();

	return 0;
}

intptr_t __sys_madvise(void *addr, size_t length, int advice)
{
	int r;

	switch (advice) {
	case MADV_DONTNEED:
		printk("sys_madvise: advice %08"PRIxPTR" - %08"PRIxPTR" do not need.\n",
			(uintptr_t)addr, (uintptr_t)addr + length);

		r = check_pages(addr, length);
		if (r) {
			return r;
		}

		kmemset(addr, length, 0);

		break;
	default:
		printk("sys_madvise: advice %d is not supported.\n", advice);

		return -EINVAL;
	}

	return 0;
}

intptr_t __sys_mprotect(void *addr, size_t length, int prot)
{
	printk("sys_mprotect: ignore mprotect.\n");

	return 0;
}

intptr_t __sys_clone(unsigned long flags, void *child_stack, void *ptid, void *tls, void *ctid)
{
	struct __cpu_device *cpu_cur = __cpu_get_current(), *cpu;
	struct __proc_info *pi = __proc_get_current();
	struct __thread_info *ti;
	size_t pos_intr;
	int need_ctid = 0, need_ptid = 0, need_tls = 0;
	int r;

	if (flags & (CLONE_CHILD_CLEARTID | CLONE_CHILD_SETTID)) {
		if (!ctid) {
			printk("sys_clone: Need ctid but ctid is NULL.\n");
			r = -EFAULT;
			goto err_out;
		}
		need_ctid = 1;
	}
	if (flags & CLONE_PARENT_SETTID) {
		if (!ptid) {
			printk("sys_clone: Need ptid but ptid is NULL.\n");
			r = -EFAULT;
			goto err_out;
		}
		need_ptid = 1;
	}
	if (flags & CLONE_SETTLS) {
		if (!tls) {
			printk("sys_clone: Need tls but tls is NULL.\n");
			r = -EFAULT;
			goto err_out;
		}
		need_tls = 1;
	}

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

	ti->flags = flags;
	ti->ctid = NULL;
	if (need_ctid) {
		ti->ctid = ctid;
	}
	ti->ptid = NULL;
	if (need_ptid) {
		ti->ptid = ptid;
	}
	ti->tls = NULL;
	if (need_tls) {
		ti->tls = tls;
	}

	/* Notify to user space */
	if (flags & CLONE_CHILD_SETTID) {
		*ti->ctid = ti->tid;
	}
	if (flags & CLONE_PARENT_SETTID) {
		*ti->ptid = ti->tid;
	}

	/* Copy user regs to the initial stack of new thread */
	ti->sp = child_stack;
	kmemcpy(&ti->regs, cpu_cur->regs, sizeof(__arch_user_regs_t));

	pos_intr = (cpu->id_cpu + 1) * CONFIG_INTR_STACK_SIZE;

	/* Return value and stack for new thread */
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_RETVAL, 0);
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_STACK, (uintptr_t)ti->sp);
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_STACK_INTR, (uintptr_t)&__stack_intr[pos_intr]);
	__arch_set_arg(&ti->regs, __ARCH_ARG_TYPE_TLS, (uintptr_t)ti->tls);

	r = __thread_run(ti, cpu);
	if (r) {
		goto err_out2;
	}

	dwmb();

	r = __cpu_raise_ipi(ti->cpu, NULL);
	if (r) {
		goto err_out3;
	}

	__smp_unlock();

	/* Return value for current thread */
	return ti->tid;

err_out3:
	__thread_stop(ti);

err_out2:
	__thread_destroy(ti);

err_out:
	__smp_unlock();

	return r;
}

intptr_t __sys_futex(int *uaddr, int op, int val, const struct timespec *timeout, int *uaddr2, int val3)
{
	int cmd = op & FUTEX_MASK;
	int ret = 0, r;

	if (!uaddr) {
		return -EFAULT;
	}

	switch (cmd) {
	case FUTEX_WAIT:
		val3 = FUTEX_BITSET_ANY;
		/* fallthrough */
	case FUTEX_WAIT_BITSET:
		r = __cpu_futex_wait(uaddr, val, val3);
		if (r) {
			if (r != -EWOULDBLOCK) {
				printk("%d: futex err wait %p %d.\n", __arch_get_cpu_id(), uaddr, val);
			}

			ret = r;
			goto err_out;
		}

		break;
	case FUTEX_WAKE:
		val3 = FUTEX_BITSET_ANY;
		/* fallthrough */
	case FUTEX_WAKE_BITSET:
		r = __cpu_futex_wake(uaddr, val, val3);
		if (r < 0) {
			printk("%d: futex err wake %p %d.\n", __arch_get_cpu_id(), uaddr, val);

			ret = r;
			goto err_out;
		}

		ret = r;
		break;
	default:
		printk("sys_futex: cmd %d is not supported.\n", cmd);

		ret = -ENOSYS;
		goto err_out;
	}

err_out:
	return ret;
}

intptr_t __sys_set_tid_address(int *tidptr)
{
	struct __cpu_device *cpu = __cpu_get_current();
	struct __thread_info *ti;

	ti = __cpu_get_thread_task(cpu);
	if (!ti) {
		printk("sys_set_tid_address: cannot get task thread.\n");
		return -EINVAL;
	}

	if (ti->flags & CLONE_CHILD_CLEARTID) {
		ti->ctid = tidptr;
	}

	return 0;
}

intptr_t __sys_exit_group(int status)
{
	/* TODO: kill other threads in process group */
	return __sys_exit(status);
}

intptr_t __sys_exit(int status)
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
		__fini_leader(status);
	} else {
		__fini_child(status);
	}

	/* Notify to user space */
	if (ti->flags & CLONE_CHILD_CLEARTID) {
		*ti->ctid = 0;

		r = __cpu_futex_wake(ti->ctid, 1, FUTEX_BITSET_ANY);
		if (r < 0) {
			printk("sys_exit: futex error %d but ignored.\n", r);
		}
	}

	r = __thread_context_switch_nolock();
	if (r) {
		__smp_unlock();
		return r;
	}

	__smp_unlock();

	__arch_get_arg(cpu->regs, __ARCH_ARG_TYPE_RETVAL, &v);

	return v;
}

intptr_t __sys_context_switch(void)
{
	struct __cpu_device *cpu = __cpu_get_current();
	uintptr_t v;
	int r;

	r = __thread_context_switch();
	if (r) {
		/* TODO: fatal error, panic? */
		printk("sys_context_switch: failed to context_switch.\n");
		return r;
	}

	__arch_get_arg(cpu->regs, __ARCH_ARG_TYPE_RETVAL, &v);

	return v;
}
