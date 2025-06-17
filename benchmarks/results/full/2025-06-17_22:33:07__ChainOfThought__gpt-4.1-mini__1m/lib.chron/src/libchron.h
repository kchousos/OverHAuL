#ifndef LIB_CHRON_H
#define LIB_CHRON_H

#include "lib.cartilage/libcartilage.h"

#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>

#include <pthread.h>

/* Timer Base */

/**
 * @brief Timer state
 */
typedef enum {
	TIMER_INIT,
	TIMER_RUNNING,
	TIMER_CANCELLED,
	TIMER_DELETED,
	TIMER_PAUSED,
	TIMER_RESUMED
} chron_timer_state;

/**
 * @brief Represents a compound timer
 */
typedef struct chron_timer {
	/* A POSIX timer */
	timer_t timer;

	/* Timer callback function */
	void(*callback)(struct chron_timer*, void*);

	/* Timer callback argument */
	void* callback_arg;

	/* Expiration time in ms */
	unsigned long exp_time;

	/* Interval in ms */
	unsigned long exp_interval;

	/* Threshold at which we no longer invoke */
	uint32_t threshold;

	/* Indicates whether the timer is exponential */
	bool is_exponential;

	/* Persists time remaining  */
	unsigned long time_remaining;

	/* Counts callback invocations */
	uint32_t invocation_count;

	/* Expiration time */
	struct itimerspec ts;

	/* Time val for exponent p*/
	unsigned long exponential_backoff_time;

	/* Current state of timer */
	chron_timer_state timer_state;
} chron_timer_t;

/* Hierarchical Timer Wheel */

typedef enum {
	TW_CREATE,
	TW_RESCHEDULED,
	TW_DELETE,
	TW_SCHEDULED,
	TW_UNKNOWN,
} chron_tw_opcode;

/**
 * @brief Generic timer wheel callback
 */
typedef void (*chron_tw_callback)(void* arg, int arg_size);

/**
 * @brief Represents a single slot on the ring buffer
 */
typedef struct ring_buffer_slot {
	glthread_t linked_list;
	pthread_mutex_t mutex;
} chron_tw_slot;

/**
 * @brief Represents a single element in a slot on the ring buffer.
 */
typedef struct tw_slot_el {
	chron_tw_opcode opcode;

	/* interval after which the event needs to be invoked */
	int interval;

	int new_interval;

	/* revolution number at which the element's event must be invoked */
	int r;

	/* numeric identifier of the slot to which this el belongs */
	int slot_n;

	/* the event callback */
	chron_tw_callback callback;

	/* the event callback argument */
	void* callback_arg;

	/* the event callback argument size */
	int arg_size;

	/* is the event recurring? i.e. if 1, the event must be triggered at ea interval */
	int is_recurring;

	/* el's linked list node delegate */
	glthread_t linked_list_node;

	glthread_t waitlist_node;

	/* pointer to the head node address of the slot to which this el belongs */
	chron_tw_slot* slot_head;

	/* counter of how many times this el has been scheduled */
	unsigned int n_scheduled;
} chron_tw_slot_el_t;

/**
 * @brief Represents a Hierarchical Timer Wheel
 */
typedef struct timer_wheel {
	/* current tick and also the slot number currently pointed to */
	int current_tick;

	/* tick interval e.g. 1ms, 1s, 1m... */
	int tick_interval;

	/* number of slots in the wheel */
	int ring_size;

	/* aka R; the number of full revolutions completed */
	int n_revolutions;

	/* the thread on which the wheel is invoked */
	pthread_t thread;

	/* slots holding linked lists */
	chron_tw_slot slots[0];

	chron_tw_slot waitlist;

	/* total number of slots in the wheel */
	unsigned int n_slots;
} chron_timer_wheel_t;

/* Methods */

chron_timer_wheel_t* chron_timer_wheel_init(int size, int tick_interval);

bool chron_timer_wheel_start(chron_timer_wheel_t* tw);

int chron_timer_wheel_get_time_remaining(chron_timer_wheel_t* tw, chron_tw_slot_el_t* el);

void chron_timer_wheel_reset(chron_timer_wheel_t* tw);

void chron_timer_wheel_reschedule_ev(
	chron_timer_wheel_t* tw,
	chron_tw_slot_el_t* el,
	int next_interval
);

void chron_timer_wheel_unregister_ev(
	chron_timer_wheel_t* tw,
	chron_tw_slot_el_t* el
);

chron_tw_slot_el_t* chron_timer_wheel_register_ev(
	chron_timer_wheel_t* tw,
	chron_tw_callback callback,
	void* arg,
	int arg_size,
	int interval,
	int recurring
);

chron_timer_t* chron_timer_init(
	void (*callback)(chron_timer_t* timer, void* arg),
	void* callback_arg,
	unsigned long expiry_ms,
	unsigned long interval_ms, // subsequent to initial expiry
	uint32_t max_expirations, // 0 for infinite
	bool is_exponential
);

bool chron_timer_toggle(chron_timer_t* timer);

void chron_timer_start(chron_timer_t* timer);

unsigned long chron_timer_get_ms_remaining(chron_timer_t* timer);

bool chron_timer_pause(chron_timer_t* timer);

bool chron_timer_resume(chron_timer_t* timer);

bool chron_timer_restart(chron_timer_t* timer);

bool chron_timer_cancel(chron_timer_t* timer);

void chron_timer_print(chron_timer_t* timer);

bool chron_timer_reschedule(
	chron_timer_t* timer,
	unsigned long exp_time,
	unsigned long exp_interval
);

bool chron_timer_delete(chron_timer_t* timer);

#endif /* LIB_CHRON_H */
