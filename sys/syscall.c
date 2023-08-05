/* SPDX-License-Identifier: Apache-2.0 */

#include <stdatomic.h>
#include <stdint.h>

#include <bmetal/syscall.h>
#include <bmetal/clock.h>
#include <bmetal/comm.h>
#include <bmetal/file.h>
#include <bmetal/fini.h>
#include <bmetal/init.h>
#include <bmetal/lock.h>
#include <bmetal/memory.h>
#include <bmetal/printk.h>
#include <bmetal/smp.h>
#include <bmetal/thread.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/futex.h>
#include <bmetal/sys/inttypes.h>
#include <bmetal/sys/mman.h>
#include <bmetal/sys/random.h>
#include <bmetal/sys/reboot.h>
#include <bmetal/sys/resource.h>
#include <bmetal/sys/sched.h>
#include <bmetal/sys/string.h>

static define_brk(brk_area, CONFIG_BRK_SIZE);
static char *brk_cur = brk_area;

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
	pri_info("%d: unknown syscall %"PRIdPTR"\n", __cpu_get_current_id_phys(), number);

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
		pri_warn("prlimit64: not support to set new rlimit.\n");
		return -EPERM;
	}

	switch (resource) {
	case RLIMIT_STACK:
		old.rlim_cur = CONFIG_MAIN_STACK_SIZE;
		old.rlim_max = ~0;
		break;
	default:
		pri_warn("prlimit64: not support to resource %d.\n", resource);
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

intptr_t __sys_clock_gettime32(clockid_t clock_id, struct timespec32 *tp)
{
	struct timespec64 *tp64 = NULL, tmp;
	int r;

	if (tp) {
		tmp.tv_sec = tp->tv_sec;
		tmp.tv_nsec = tp->tv_nsec;
		tp64 = &tmp;
	}

	r = __sys_clock_gettime64(clock_id, tp64);

	if (tp) {
		tp->tv_sec = tmp.tv_sec;
		tp->tv_nsec = tmp.tv_nsec;
	}

	return r;
}

intptr_t __sys_clock_settime32(clockid_t clock_id, const struct timespec32 *tp)
{
	struct timespec64 *tp64 = NULL, tmp;

	if (tp) {
		tmp.tv_sec = tp->tv_sec;
		tmp.tv_nsec = tp->tv_nsec;
		tp64 = &tmp;
	}

	return __sys_clock_settime64(clock_id, tp64);
}

intptr_t __sys_clock_gettime64(clockid_t clock_id, struct timespec64 *tp)
{
	int r = 0;

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
		pri_warn("sys_clock_gettime: not supported clock_id:%d.\n", (int)clock_id);
		return -EINVAL;
	}

	return r;
}

intptr_t __sys_clock_settime64(clockid_t clock_id, const struct timespec64 *tp)
{
	int r = 0;

	if (!tp) {
		return -EFAULT;
	}

	switch (clock_id) {
	case CLOCK_REALTIME:
		r = __clock_set_realtime(tp);
		break;
	default:
		pri_warn("sys_clock_settime: not supported clock_id:%d.\n", (int)clock_id);
		return -EINVAL;
	}

	return r;
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

intptr_t __sys_getrandom(void *buf, size_t buflen, unsigned int flags)
{
	if (!buf || buflen == 0) {
		return -EINVAL;
	}

	char *p = buf;

	/* TODO: randomize */
	for (size_t i = 0; i < buflen; i++) {
		p[i] = i;
	}

	return 0;
}

intptr_t __sys_close(int fd)
{
	struct __file_desc *desc = __file_get_desc(fd);
	int ret = 0;

	if (!desc) {
		return -EBADF;
	}

	if (desc->ops && desc->ops->close) {
		ret = desc->ops->close(desc);
	}

	__file_set_desc(fd, NULL);

	return ret;
}

intptr_t __sys_read(int fd, void *buf, size_t count)
{
	struct __file_desc *desc = __file_get_desc(fd);

	if (!desc || !desc->ops || !desc->ops->read) {
		return -EBADF;
	}

	pri_dbg("sys_read: fd:%d cnt:%d\n", fd, (int)count);

	return __file_read(desc, buf, count);
}

intptr_t __sys_write(int fd, const void *buf, size_t count)
{
	struct __file_desc *desc = __file_get_desc(fd);

	if (!desc || !desc->ops || !desc->ops->write) {
		return -EBADF;
	}

	pri_dbg("sys_write: fd:%d cnt:%d\n", fd, (int)count);

	return __file_write(desc, buf, count);
}

intptr_t __sys_writev(int fd, const struct iovec *iov, int iovcnt)
{
	struct __file_desc *desc = __file_get_desc(fd);
	ssize_t ret = 0, wr;

	if (!desc || !desc->ops || !desc->ops->write) {
		return -EBADF;
	}
	if (!iov) {
		return -EINVAL;
	}
	if (iovcnt < 0) {
		return -EINVAL;
	} else if (iovcnt == 0) {
		return 0;
	}

	__spinlock_lock(&desc->lock);
	for (int i = 0; i < iovcnt; i++) {
		if (iov[i].iov_len == 0) {
			continue;
		}

		wr = __file_write_nolock(desc, iov[i].iov_base, iov[i].iov_len);
		if (wr > 0) {
			ret += wr;
		} else if (wr < iov[i].iov_len) {
			break;
		} else {
			pri_warn("sys_writev: failed at %d, fd:%d len:%d.\n",
				i, fd, (int)iov[i].iov_len);
			ret = wr;
			break;
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
		pri_info("sys_brk: addr:%p is out of bounds.\n", addr);
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

intptr_t __sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
#ifdef CONFIG_HEAP
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
		pri_warn("sys_mmap: only support anonymous mmap.\n");
		return -EINVAL;
	}

	__mem_lock();
	off_page = __mem_alloc_pages(len);
	__mem_unlock();
	if (off_page == -1) {
		pri_warn("sys_mmap: no enough pages, len:%d.\n", (int)len);
		return -ENOMEM;
	}
	anon_ptr = __mem_heap_area_start() + off_page * __PAGE_SIZE;

	kmemset(anon_ptr, 0, len);

	return PTR_TO_INT(anon_ptr);
#else
	pri_warn("sys_mmap: heap is disabled.\n");

	return -ENOTSUP;
#endif /* CONFIG_HEAP */
}

intptr_t __sys_munmap(void *addr, size_t length)
{
#ifdef CONFIG_HEAP
	if (addr < __mem_heap_area_start() || __mem_heap_area_end() < addr + length) {
		return -EINVAL;
	}

	__mem_lock();
	__mem_free_pages(addr, length);
	__mem_unlock();

	return 0;
#else
	pri_warn("sys_munmap: heap is disabled.\n");

	return -ENOTSUP;
#endif /* CONFIG_HEAP */
}

intptr_t __sys_madvise(void *addr, size_t length, int advice)
{
	switch (advice) {
	case MADV_DONTNEED:
#ifdef CONFIG_HEAP
		pri_info("sys_madvise: %08"PRIxPTR" - %08"PRIxPTR" do not need.\n",
			(uintptr_t)addr, (uintptr_t)addr + length);

		int r = __mem_check_pages(addr, length);
		if (r) {
			return r;
		}

		kmemset(addr, length, 0);

		break;
#else
		pri_warn("sys_madvise: heap is disabled.\n");

		return -ENOTSUP;
#endif /* CONFIG_HEAP */
	default:
		pri_warn("sys_madvise: advice %d is not supported.\n", advice);

		return -EINVAL;
	}

	return 0;
}

intptr_t __sys_mprotect(void *addr, size_t length, int prot)
{
	pri_info("sys_mprotect: ignore mprotect.\n");

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
			pri_warn("sys_clone: Need ctid but ctid is NULL.\n");
			return -EFAULT;
		}
		need_ctid = 1;
	}
	if (flags & CLONE_PARENT_SETTID) {
		if (!ptid) {
			pri_warn("sys_clone: Need ptid but ptid is NULL.\n");
			return -EFAULT;
		}
		need_ptid = 1;
	}
	if (flags & CLONE_SETTLS) {
		if (!tls) {
			pri_warn("sys_clone: Need tls but tls is NULL.\n");
			return -EFAULT;
		}
		need_tls = 1;
	}

	__smp_lock();

	r = __smp_find_idle_cpu(&cpu);
	if (r) {
		__smp_unlock();
		return r;
	}

	__smp_unlock();

	__cpu_lock(cpu);

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

	r = __cpu_raise_ipi(ti->cpu, NULL);
	if (r) {
		goto err_out3;
	}

	dwmb();
	__cpu_unlock(cpu);

	/* Return value for current thread */
	return ti->tid;

err_out3:
	__thread_stop(ti);

err_out2:
	__thread_destroy(ti);

err_out:
	__cpu_unlock(cpu);

	return r;
}

intptr_t __sys_futex32(int *uaddr, int op, int val, const struct timespec32 *timeout, int *uaddr2, int val3)
{
	int cmd = op & FUTEX_MASK;
	const struct timespec64 *ts = NULL;
	struct timespec64 tmp;

	if (cmd == FUTEX_WAIT && timeout) {
		tmp.tv_sec = timeout->tv_sec;
		tmp.tv_nsec = timeout->tv_nsec;
		ts = &tmp;
	}

	return __sys_futex64(uaddr, op, val, ts, uaddr2, val3);
}

intptr_t __sys_futex64(int *uaddr, int op, int val, const struct timespec64 *timeout, int *uaddr2, int val3)
{
	int cmd = op & FUTEX_MASK;
	int ret = 0, r;

	if (!uaddr) {
		return -EFAULT;
	}

	switch (cmd) {
	case FUTEX_WAIT:
		if (timeout) {
			pri_warn("sys_futex: timeout %d.%09d is not support.\n",
				(int)timeout->tv_sec, (int)timeout->tv_nsec);
		}

		val3 = FUTEX_BITSET_ANY;
		/* fallthrough */
	case FUTEX_WAIT_BITSET:
		r = __cpu_futex_wait(uaddr, val, val3);
		if (r) {
			if (r != -EWOULDBLOCK) {
				pri_warn("%d: futex err wait %p %d.\n", __cpu_get_current_id_phys(), uaddr, val);
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
			pri_warn("%d: futex err wake %p %d.\n", __cpu_get_current_id_phys(), uaddr, val);

			ret = r;
			goto err_out;
		}

		ret = r;
		break;
	case FUTEX_REQUEUE:
		r = __cpu_futex_wake(uaddr, val, FUTEX_BITSET_ANY);
		if (r < 0) {
			pri_warn("%d: futex err requeue %p %d.\n", __cpu_get_current_id_phys(), uaddr, val);

			ret = r;
			goto err_out;
		}

		ret = r;
		break;
	default:
		pri_warn("sys_futex: cmd %d is not supported.\n", cmd);

		ret = -ENOSYS;
		goto err_out;
	}

err_out:
	return ret;
}

intptr_t __sys_set_robust_list(void *head, size_t len)
{
	struct __cpu_device *cpu = __cpu_get_current();
	struct __thread_info *ti;

	if (len == 0) {
		return -EINVAL;
	}

	ti = __cpu_get_thread_task(cpu);
	if (!ti) {
		pri_err("sys_robust_list: cannot get task thread.\n");
		return -EINVAL;
	}

	ti->robust_list = head;
	ti->robust_len = len;

	return 0;
}

intptr_t __sys_set_tid_address(int *tidptr)
{
	struct __cpu_device *cpu = __cpu_get_current();
	struct __thread_info *ti;

	ti = __cpu_get_thread_task(cpu);
	if (!ti) {
		pri_err("sys_set_tid_address: cannot get task thread.\n");
		return -EINVAL;
	}

	if (ti->flags & CLONE_CHILD_CLEARTID) {
		ti->ctid = tidptr;
	}

	return ti->tid;
}

intptr_t __sys_exit_group(int status)
{
	struct __proc_info *pi = __proc_get_current();
	struct __cpu_device *cur = __cpu_get_current();
	struct __thread_info *ti;

	__cpu_lock(cur);

	ti = __cpu_get_thread_task(cur);
	if (!ti) {
		pri_err("sys_exit_group: cannot get current task thread.\n");
		__cpu_unlock(cur);
		return -EINVAL;
	}

	if (__proc_get_leader(pi) != ti) {
		__cpu_unlock(cur);
		return __sys_exit(status);
	}

	__cpu_unlock(cur);

	/* TODO: kill other threads in process group */

	/* Wait for children */
	for (int i = 1; i < CONFIG_NUM_CORES; i++) {
		struct __cpu_device *cpu = __cpu_get(i);

		if (!cpu) {
			continue;
		}

		/* TODO: avoid busy loop */
		while (__cpu_get_thread_task(cpu)) {
			drmb();
		}
	}

	return __sys_exit(status);
}

intptr_t __sys_exit(int status)
{
	struct __proc_info *pi = __proc_get_current();
	struct __cpu_device *cpu = __cpu_get_current();
	struct __thread_info *ti;
	uintptr_t v;
	int f_wake = 0, r;

	__cpu_lock(cpu);

	ti = __cpu_get_thread_task(cpu);
	if (!ti) {
		pri_err("sys_exit: cannot get task thread.\n");

		__cpu_unlock(cpu);
		return -EINVAL;
	}

	r = __thread_stop(ti);
	if (r) {
		__cpu_unlock(cpu);
		return r;
	}

	r = __thread_destroy(ti);
	if (r) {
		__cpu_unlock(cpu);
		return r;
	}

	/* Notify to host when leader exit */
	if (__proc_get_leader(pi) == ti) {
		__fini_leader(status);
	} else {
		__fini_child(status);
	}

	/* Notify to user space */
	if (ti->flags & CLONE_CHILD_CLEARTID) {
		*ti->ctid = 0;
		f_wake = 1;
	}

	r = __thread_context_switch_nolock();
	if (r) {
		__cpu_unlock(cpu);
		return r;
	}

	__arch_get_arg(cpu->regs, __ARCH_ARG_TYPE_RETVAL, &v);

	dwmb();
	__cpu_unlock(cpu);

	if (f_wake) {
		r = __cpu_futex_wake(ti->ctid, 1, FUTEX_BITSET_ANY);
		if (r < 0) {
			pri_warn("sys_exit: futex error %d but ignored.\n", r);
		}
	}

	return v;
}

intptr_t __sys_reboot(int magic1, int magic2, int cmd)
{
	if (magic1 != REBOOT_MAGIC1 ||
	    magic2 != REBOOT_MAGIC2) {
		return -EINVAL;
	}

	switch (cmd) {
	case RB_AUTOBOOT:
		pri_err("Restarting system.\n");
		__fini_reboot();
		break;
	case RB_POWER_OFF:
		pri_err("Power down.\n");
		__fini_power_off();
		break;
	default:
		pri_warn("sys_reboot: cmd %d is not supported.\n", cmd);
		return -EINVAL;
	}

	return 0;
}

intptr_t __sys_context_switch(void)
{
	struct __cpu_device *cpu = __cpu_get_current();
	uintptr_t v;
	int r;

	r = __thread_context_switch();
	if (r) {
		/* TODO: fatal error, panic? */
		pri_err("sys_context_switch: failed to context_switch.\n");
		return r;
	}

	__arch_get_arg(cpu->regs, __ARCH_ARG_TYPE_RETVAL, &v);

	return v;
}
