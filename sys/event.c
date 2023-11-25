/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/event.h>
#include <bmetal/printk.h>

static struct k_event_handler k_evt_handlers[CONFIG_MAX_EVENT_HANDLERS];
static int k_evt_handler_stat[CONFIG_MAX_EVENT_HANDLERS];

int k_event_alloc_handler(struct k_event_handler **handler)
{
	int found = 0;
	int i;

	if (handler == NULL) {
		return -EINVAL;
	}

	for (i = 0; i < CONFIG_MAX_EVENT_HANDLERS; i++) {
		if (k_evt_handler_stat[i] == 0) {
			found = 1;
			break;
		}
	}
	if (!found) {
		k_pri_err("No more interrupt handlers.\n");
		return -ENOMEM;
	}

	*handler = &k_evt_handlers[i];
	k_evt_handler_stat[i] = 1;

	return 0;
}

int k_event_free_handler(struct k_event_handler *handler)
{
	int found = 0;
	int i;

	if (handler == NULL) {
		return -EINVAL;
	}

	for (i = 0; i < CONFIG_MAX_EVENT_HANDLERS; i++) {
		if (handler == &k_evt_handlers[i]) {
			found = 1;
			break;
		}
	}
	if (!found) {
		k_pri_err("handler %p is not found.\n", handler);
		return -EINVAL;
	}

	k_evt_handler_stat[i] = 0;

	return 0;
}

int k_event_add_handler(struct k_event_handler *head, struct k_event_handler *handler)
{
	struct k_event_handler *last = NULL;

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

int k_event_remove_handler(struct k_event_handler *head, struct k_event_handler *handler)
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
		k_pri_err("handler %p is not found.\n", handler);
		return -EINVAL;
	}

	return 0;
}

int k_event_handle_generic(int event, struct k_event_handler *hnd_head)
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
