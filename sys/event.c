/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/event.h>
#include <bmetal/printk.h>

static struct __event_handler evt_handlers[CONFIG_MAX_EVENT_HANDLERS];
static int evt_handler_stat[CONFIG_MAX_EVENT_HANDLERS];

int __event_alloc_handler(struct __event_handler **handler)
{
	int found = 0;
	int i;

	if (handler == NULL) {
		return -EINVAL;
	}

	for (i = 0; i < CONFIG_MAX_EVENT_HANDLERS; i++) {
		if (evt_handler_stat[i] == 0) {
			found = 1;
			break;
		}
	}
	if (!found) {
		pri_err("No more interrupt handlers.\n");
		return -ENOMEM;
	}

	*handler = &evt_handlers[i];
	evt_handler_stat[i] = 1;

	return 0;
}

int __event_free_handler(struct __event_handler *handler)
{
	int found = 0;
	int i;

	if (handler == NULL) {
		return -EINVAL;
	}

	for (i = 0; i < CONFIG_MAX_EVENT_HANDLERS; i++) {
		if (handler == &evt_handlers[i]) {
			found = 1;
			break;
		}
	}
	if (!found) {
		pri_err("handler %p is not found.\n", handler);
		return -EINVAL;
	}

	evt_handler_stat[i] = 0;

	return 0;
}

int __event_add_handler(struct __event_handler *head, struct __event_handler *handler)
{
	struct __event_handler *last = NULL;

	if (head == NULL || handler == NULL) {
		return -EINVAL;
	}

	for_each_handler(h, head) {
		last = h;
	}

	last->hnd_next = handler;
	handler->hnd_next = NULL;

	return 0;
}

int __event_remove_handler(struct __event_handler *head, struct __event_handler *handler)
{
	int found = 0;

	if (head == NULL || handler == NULL) {
		return -EINVAL;
	}

	for_each_handler(h, head) {
		if (h->hnd_next == handler) {
			found = 1;
			h->hnd_next = handler->hnd_next;
			break;
		}
	}
	if (!found) {
		pri_err("handler %p is not found.\n", handler);
		return -EINVAL;
	}

	return 0;
}

int __event_handle_generic(int event, struct __event_handler *hnd_head)
{
	int r, res = EVENT_NOT_HANDLED;

	for_each_handler (h, hnd_head) {
		/* h->func is not NULL because checked at add function */
		if (h->event == event) {
			r = h->func(event, h);
			if (r == EVENT_HANDLED) {
				res = EVENT_HANDLED;
			}
		}
	}

	return res;
}


