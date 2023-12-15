/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_EVENT_H_
#define BAREMETAL_CRT_EVENT_H_

#if !defined(__ASSEMBLER__)
#include <stddef.h>
#include <stdint.h>
#endif /* !__ASSEMBLER__ */

#include <bmetal/bmetal.h>
#include <bmetal/io.h>
#include <bmetal/sys/errno.h>

#define EVENT_HANDLED            1
#define EVENT_NOT_HANDLED        2

#if !defined(__ASSEMBLER__)

#define for_each_handler(x, head)    for (struct k_event_handler *x = (head); x; x = x->hnd_next)

struct k_event_handler;

typedef int (* k_event_handler_func_t)(int event, struct k_event_handler *hnd);

struct k_event_handler {
	/* Set by driver */
	k_event_handler_func_t func;
	void *priv;

	/* Set by framework */
	int event;
	struct k_event_handler *hnd_next;
};

static inline int k_event_has_next(const struct k_event_handler *handler)
{
	return handler->hnd_next != NULL;
}

int k_event_alloc_handler(struct k_event_handler **handler);
int k_event_free_handler(struct k_event_handler *handler);
int k_event_add_handler(struct k_event_handler *head, struct k_event_handler *handler);
int k_event_remove_handler(struct k_event_handler *head, struct k_event_handler *handler);
int k_event_handle_generic(int event, struct k_event_handler *hnd_head);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_EVENT_H_ */
