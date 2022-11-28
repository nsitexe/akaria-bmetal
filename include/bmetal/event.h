/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_EVENT_H_
#define BAREMETAL_CRT_EVENT_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/bmetal.h>
#include <bmetal/io.h>
#include <bmetal/sys/errno.h>

#define EVENT_HANDLED            1
#define EVENT_NOT_HANDLED        2

#define for_each_handler(x, head)    for (struct __event_handler *x = (head); x; x = x->hnd_next)

struct __event_handler;

typedef int (* __event_handler_func_t)(int event, struct __event_handler *hnd);

struct __event_handler {
	/* Set by driver */
	__event_handler_func_t func;
	void *priv;

	/* Set by framework */
	int event;
	struct __event_handler *hnd_next;
};

static inline int __event_has_next(const struct __event_handler *handler)
{
	return handler->hnd_next != NULL;
}

int __event_alloc_handler(struct __event_handler **handler);
int __event_free_handler(struct __event_handler *handler);
int __event_add_handler(struct __event_handler *head, struct __event_handler *handler);
int __event_remove_handler(struct __event_handler *head, struct __event_handler *handler);
int __event_handle_generic(int event, struct __event_handler *hnd_head);

#endif /* BAREMETAL_CRT_EVENT_H_ */
