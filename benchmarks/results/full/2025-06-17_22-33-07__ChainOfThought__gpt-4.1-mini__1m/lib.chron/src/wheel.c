#include "libchron.h"

#include <unistd.h>

/* MACROS (opaque) */

/* Getters */
#define CHRON_TW_GET_SLOT(tw, idx) (&(tw->slots[idx]))

#define CHRON_TW_GET_SLOT_HEAD(tw, idx) (&(tw->slots[idx].linked_list))

#define CHRON_TW_GET_SLOT_MUTEX(tw, idx) (&(tw->slots[idx].mutex))

#define CHRON_TW_GET_UPTIME(tw) (CHRON_TW_GET_ABS_SLOT_N(tw) * tw->tick_interval)

#define CHRON_TW_GET_SLOTS_AT_IDX(tw, idx) (&(tw->slots[idx]))

// the *absolute* slot number since the wheel routine began
#define CHRON_TW_GET_ABS_SLOT_N(tw)	((tw->n_revolutions * tw->ring_size) + tw->current_tick)

#define CHRON_TW_GET_WAITLIST(tw) (&(tw->waitlist))

#define CHRON_TW_GET_WAITLIST_HEAD(tw) (&((CHRON_TW_GET_WAITLIST(tw))->linked_list))

#define CHRON_TW_GET_SLOT_EMPTY(slot) (IS_GLTHREAD_EMPTY(&(slot->linked_list)))

/* Setters */
#define CHRON_TW_SET_LOCK_SLOT(slot) pthread_mutex_lock(&(slot->mutex))

#define CHRON_TW_SET_UNLOCK_SLOT(slot) pthread_mutex_unlock(&(slot->mutex))

/* HELPERS */

/**
 * @brief Insert a new event element in a given timer wheel slot
 *
 * @param data1
 * @param data2
 * @return int
 */
int __insert_el_in_slot(void* data1, void* data2) {
   chron_tw_slot_el_t* el1 = (chron_tw_slot_el_t* )data1;
   chron_tw_slot_el_t* el2 = (chron_tw_slot_el_t* )data2;

   if (el1->r < el2->r) return -1;

   if (el1->r > el2->r) return 1;

   return 0;
}

void __reschedule_ev(
	chron_timer_wheel_t* tw,
	chron_tw_slot_el_t* el,
	int next_interval,
	chron_tw_opcode opcode
) {

switch(opcode){
	case TW_CREATE:
	case TW_RESCHEDULED:
	case TW_DELETE:

		el->new_interval = next_interval;

		CHRON_TW_SET_LOCK_SLOT(CHRON_TW_GET_WAITLIST(tw));
		el->opcode = opcode;

		glthread_remove(&el->waitlist_node);
		glthread_insert_after(
			CHRON_TW_GET_WAITLIST_HEAD(tw),
			&el->waitlist_node
		);

		CHRON_TW_SET_UNLOCK_SLOT(CHRON_TW_GET_WAITLIST(tw));
		break;

	default:
		break;
	}

}

/**
 * @brief Apply the offset node data in the glthread
 *
 * @param glthread
 * @return chron_tw_slot_el_t*
 */
chron_tw_slot_el_t* __glthread_to_el(glthread_t* glthread) {
	return (chron_tw_slot_el_t*)((char*)(glthread) - (char*)&(((chron_tw_slot_el_t*)0)->waitlist_node));
}

/**
 * @brief
 *
 * @param tw
 */
void __reschedule_slot(chron_timer_wheel_t* tw) {
	glthread_t* current_node;
	chron_tw_slot_el_t* el;

	// lock the waitlist
	CHRON_TW_SET_LOCK_SLOT(CHRON_TW_GET_WAITLIST(tw));

	if (CHRON_TW_GET_SLOT_EMPTY(CHRON_TW_GET_WAITLIST(tw))) {
		CHRON_TW_SET_UNLOCK_SLOT(CHRON_TW_GET_WAITLIST(tw));
		return;
	}

	ITERATE_GLTHREAD_BEGIN(CHRON_TW_GET_WAITLIST_HEAD(tw), current_node) {
		el = __glthread_to_el(current_node);
		glthread_remove(&el->linked_list_node);
		el->slot_head = NULL;

		switch (el->opcode) {
			case TW_CREATE:
			case TW_RESCHEDULED:
				{
					el->interval  = el->new_interval;

					int abs_slot_n = CHRON_TW_GET_ABS_SLOT_N(tw);
					int next_abs_slot_n = abs_slot_n + (el->interval / tw->tick_interval);
					int next_rev_n = next_abs_slot_n / tw->ring_size;
					int next_slot_n = next_abs_slot_n & tw->ring_size;

					el->r = next_rev_n;
					el->slot_n = next_slot_n;

					glthread_priority_insert(
						CHRON_TW_GET_SLOT_HEAD(tw, el->slot_n),
						&el->linked_list_node,
						__insert_el_in_slot,
						(unsigned long)&((chron_tw_slot_el_t*)0)->linked_list_node
					);

					el->slot_head = CHRON_TW_GET_SLOT(tw, el->slot_n);

					glthread_remove(&el->waitlist_node);
					el->n_scheduled++;

					if (el->opcode == TW_CREATE){
						tw->n_slots++;
					}

					el->opcode = TW_SCHEDULED;
				}
				break;

			case TW_DELETE:
				glthread_remove(&el->waitlist_node);
				el->slot_head = NULL;
				free(el);

				tw->n_slots--;
				break;

			default:
				break;
		}
	} ITERATE_GLTHREAD_END(CHRON_TW_GET_WAITLIST_HEAD(tw), current_node);

	CHRON_TW_SET_UNLOCK_SLOT(CHRON_TW_GET_WAITLIST(tw));
}

/**
 * @brief Opaque helper. The thread routine on which the timer wheel runs
 *
 * @param arg
 * @return void*
 */
void* __timer_routine(void* arg) {
	chron_timer_wheel_t* tw = (chron_timer_wheel_t*)arg;
	chron_tw_slot_el_t* el = NULL;

	int abs_slot_n = 0;

	chron_tw_slot* slot = NULL;
	glthread_t* current_node;

	while (true) {
		tw->current_tick++;

		// start a new revolution, if necessary
		if (tw->current_tick == tw->ring_size) {
			tw->current_tick = 0;
			tw->n_revolutions++;
		}

		sleep(tw->tick_interval);

		// retrieve the current slot (linked list of event els)
		slot = CHRON_TW_GET_SLOT(tw, tw->current_tick);

		// and the absolute slot number
		abs_slot_n = CHRON_TW_GET_ABS_SLOT_N(tw);

		/* now, we iterate the slot's linked list and
			1) invoke those events which satisfy the criterion (the *r* value)
			2) reschedule events that are marked as interval, or periodic
				a) this process entails calculating the next slot for said events
		*/
		ITERATE_GLTHREAD_BEGIN(&slot->linked_list, current_node) {
			el = __glthread_to_el(current_node);

			// evaluate whether the current slot element's r variable == n revolutions
			if (tw->n_revolutions == el->r) {
				// if so, invoke the event...
				el->callback(el->callback_arg, el->arg_size);

				// ...and check whether it needs to be rescheduled
				if (el->is_recurring) {
					// reschedule in the next slot

					// get the next available slot
					int next_abs_slot_n = abs_slot_n + (el->interval / tw->tick_interval);

					// determine the next revolution
					int next_revolution_n = next_abs_slot_n / tw->ring_size;

					// finally, the next slot number
					int next_slot_n = next_abs_slot_n % tw->ring_size;

					el->r = next_revolution_n;

					// remove old linked list node and insert a new one
					glthread_remove(&el->linked_list_node);
					glthread_priority_insert(
						CHRON_TW_GET_SLOT_HEAD(tw, next_slot_n),
						&el->linked_list_node,
						__insert_el_in_slot,
						(unsigned long)&((chron_tw_slot_el_t*)0)->linked_list_node
					);

					// assign head of the slot to which the element belongs
					el->slot_head = CHRON_TW_GET_SLOT(tw, next_slot_n);
					el->slot_n = next_slot_n;

					el->n_scheduled++;
				}
			} else break;
		} ITERATE_GLTHREAD_END(slot, current_node);

	__reschedule_slot(tw);
	}

	return NULL;
}

/* PUBLIC API */

/**
 * @brief Initialize a timer wheel, allocating memory for its slots
 *
 * @param size Size of the internal ring buffer aka num of slots
 * @param tick_interval
 * @return chron_timer_wheel_t*
 */
chron_timer_wheel_t* chron_timer_wheel_init(int size, int tick_interval) {
	chron_timer_wheel_t* tw = malloc(
		// we want to calculate using the size of the linked list (slot) pointer
		sizeof(chron_timer_wheel_t) + size * sizeof(chron_tw_slot*)
	);

	if (!tw) return NULL;

	tw->tick_interval = tick_interval;
	tw->ring_size = size;
	tw->n_revolutions = 0;

	memset(&(tw->thread), 0, sizeof(chron_timer_wheel_t));

	// for each slot in the ring buffer...
	for (int i = 0; i < size; i++) {
		glthread_init(CHRON_TW_GET_SLOT_HEAD(tw, i)); // initialize the slot's linked list
		pthread_mutex_init(CHRON_TW_GET_SLOT_MUTEX(tw, i), NULL);
	}

	tw->n_slots = 0;

	return tw;
}

/**
 * @brief Start the timer wheel on a separate thread
 *
 * @param tw
 * @return bool
 */
bool chron_timer_wheel_start(chron_timer_wheel_t* tw) {
	if (pthread_create(&tw->thread, NULL, __timer_routine, (void*)tw)) {
		return false;
	}

	return true;
}

/**
 * @brief Reset the timer wheel
 *
 * @param tw
 */
void chron_timer_wheel_reset(chron_timer_wheel_t* tw) {
	tw->current_tick = 0;
	tw->n_revolutions = 0;
}

/**
 * @brief Register a new event
 *
 * @param tw
 * @param callback
 * @param arg
 * @param arg_size
 * @param interval
 * @param recurring
 * @return chron_tw_slot_el_t*
 */
chron_tw_slot_el_t* chron_timer_wheel_register_ev(
	chron_timer_wheel_t* tw,
	chron_tw_callback callback,
	void* arg,
	int arg_size,
	int interval,
	int recurring
) {
	if (!tw || !callback) return NULL;

	chron_tw_slot_el_t* el = malloc(sizeof(chron_tw_slot_el_t));

	el->callback = callback;
	if (arg && arg_size){
		el->callback_arg = arg;
		el->arg_size = arg_size;
  }

	el->is_recurring = recurring;
	glthread_init(&el->linked_list_node);
	glthread_init(&el->waitlist_node);

	el->n_scheduled = 0;
	__reschedule_ev(tw, el, interval, TW_CREATE);

	return el;
}

/**
 * @brief Unregister an event
 *
 * @param tw
 * @param el
 */
void chron_timer_wheel_unregister_ev(
	chron_timer_wheel_t* tw,
	chron_tw_slot_el_t* el
) {
  __reschedule_ev(tw, el, 0, TW_DELETE);
}

/**
 * @brief Reschedule an event
 *
 * @param tw
 * @param el
 * @param next_interval
 */
void chron_timer_wheel_reschedule_ev(
	chron_timer_wheel_t* tw,
	chron_tw_slot_el_t* el,
	int next_interval
) {
	__reschedule_ev(tw, el, next_interval, TW_RESCHEDULED);
}
