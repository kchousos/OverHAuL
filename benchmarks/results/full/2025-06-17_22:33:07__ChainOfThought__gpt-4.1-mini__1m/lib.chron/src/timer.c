#include "libchron.h"

#include <stdarg.h>
#include <stdio.h>

/**
 * @brief Set the chron_timer state flag
 *
 * @param timer
 * @param state
 */
void __timer_setstate(chron_timer_t* timer, chron_timer_state state) {
	timer->timer_state = state;
}

chron_timer_state __timer_getstate(chron_timer_t* timer) {
	return timer->timer_state;
}

/**
 * @brief Opaque helper. Set the itimerspec ms and ns
 *
 * @param ts
 * @param ms
 */
void __set_itimerspec(struct timespec* ts, unsigned long ms) {
	memset(ts, 0, sizeof(struct timespec));

	if (!ms) return;

	unsigned long s = ms / 1000;
	ts->tv_sec = s;

	unsigned long remaining_ms = ms % 1000;

	ts->tv_nsec = remaining_ms * (1000000);
}

/**
 * @brief Opaque helper. Checks if the timer occupies any
 * of the provided states
 *
 * @param nargs
 * @param ...
 * @return bool
 */
bool __timer_has_state(chron_timer_t* timer, int nargs,...) {
	va_list args;

	va_start(args, nargs);

	for (int i = 0; i < nargs; i++) {
		chron_timer_state state = va_arg(args, chron_timer_state);
		if (__timer_getstate(timer) == state) return true;
	}

	va_end(args);

	return false;
}

/**
 * @brief Opaque helper. Convert timespec to ms
 *
 * @param time
 * @return unsigned
 */
unsigned long __timespec_to_ms(struct timespec* time) {
	unsigned long ms = 0;
	ms = time->tv_sec * 1000;

  ms += time->tv_nsec / 1000000;

	return ms;
}

/**
 * @brief Opaque helper. Timer callback wrapper
 *
 * @param arg
 */
void __callback_wrapper(union sigval arg) {
	chron_timer_t* timer = (chron_timer_t*)(arg.sival_ptr);

	// if the state is TIMER_RESUMED, the timer was *just* resumed, so
	// we can set the state to TIMER_RUNNING...
	// provided the timer still has running time (otherwise, we would be stuck in a loop)
	// if (__timer_getstate(timer) == TIMER_RESUMED && timer->exp_time != 0) {
	// 	__timer_setstate(timer, TIMER_RUNNING);
	// }

	timer->invocation_count++;

	if (timer->threshold && timer->invocation_count > timer->threshold) {
		chron_timer_cancel(timer);
		return;
	}

	(timer->callback)(timer, timer->callback_arg);

	if (timer->is_exponential && timer->exponential_backoff_time) {
		chron_timer_reschedule(timer, timer->exponential_backoff_time *= 2, 0);
	} else if (timer->timer_state == TIMER_RESUMED) {
		chron_timer_reschedule(timer, timer->exp_time, timer->exp_interval);
	}
}


/*****************************
 *
 * 				Public API
 *
 ****************************/


/**
 * @brief Initialize a new chron_timer
 *
 * @param callback
 * @param callback_arg
 * @param expiry_ms
 * @param interval_ms
 * @param max_expirations
 * @param is_exponential
 * @return chron_timer_t*
 */
chron_timer_t* chron_timer_init(
	void (*callback)(chron_timer_t* timer, void* arg),
	void* callback_arg,
	unsigned long expiry_ms,
	unsigned long interval_ms, // subsequent to initial expiry
	uint32_t max_expirations, // 0 for infinite
	bool is_exponential
) {
	chron_timer_t* timer = malloc(sizeof(chron_timer_t));

	if (!timer) return NULL;

	timer->callback = callback;
	timer->callback_arg = callback_arg;
	timer->exp_time = expiry_ms;
	timer->exp_interval = interval_ms;
	timer->is_exponential = is_exponential;
	timer->threshold = max_expirations;

	__timer_setstate(timer, TIMER_INIT);

	struct sigevent evp;
	memset(&evp, 0, sizeof(struct sigevent));

	evp.sigev_value.sival_ptr = (void*)(timer);
	evp.sigev_notify = SIGEV_THREAD;
	evp.sigev_notify_function = __callback_wrapper;

	if (timer_create(CLOCK_REALTIME, &evp, &timer->timer) < 0) {
		return NULL;
	}

	__set_itimerspec(&timer->ts.it_value, timer->exp_time);

	if (!timer->is_exponential) {
		__set_itimerspec(&timer->ts.it_interval, timer->exp_interval);
		timer->exponential_backoff_time = 0;
	} else {
		timer->exponential_backoff_time = __timespec_to_ms(&timer->ts.it_value);
		__set_itimerspec(&timer->ts.it_interval, 0);
	}

	return timer;
}

/**
 * @brief Resurrect a timer that is not running; stop a running timer.
 * To stop the timer, the itimerspec members (ts)
 * must be set to zero.
 *
 * @param timer
 * @return bool true only if toggle succeeded
 */
bool chron_timer_toggle(chron_timer_t* timer) {
	if (timer_settime(timer->timer, 0, &timer->ts, NULL) < 0) {
		return false;
	}

	return true;
}

/**
 * @brief Start a stopped timer
 *
 * @param timer
 */
void chron_timer_start(chron_timer_t* timer) {
	chron_timer_toggle(timer);
	__timer_setstate(timer, TIMER_RUNNING);
}

/**
 * @brief Get the remaining time of the given chron_timer in ms
 *
 * @param timer
 * @return unsigned long
 */
unsigned long chron_timer_get_ms_remaining(chron_timer_t* timer) {
	struct itimerspec time_remaining;

	switch (timer->timer_state){
		case TIMER_INIT:
		case TIMER_PAUSED:
		case TIMER_RUNNING:
			break;
		case TIMER_DELETED:
		case TIMER_CANCELLED:
			return -1;
		default:
			break;
	}

	memset(&time_remaining, 0, sizeof(struct itimerspec));

	timer_gettime(timer->timer, &time_remaining);

	return __timespec_to_ms(&time_remaining.it_value);
}

/**
 * @brief Pause a running timer
 *
 * @param timer
 * @return bool true only if pause succeeded
 */
bool chron_timer_pause(chron_timer_t* timer) {
	if (__timer_has_state(
		timer,
		2,
		TIMER_PAUSED,
		TIMER_DELETED
	)) return false;

	timer->time_remaining = chron_timer_get_ms_remaining(timer);

	__set_itimerspec(&timer->ts.it_value, 0);
	__set_itimerspec(&timer->ts.it_interval, 0);

	if (!chron_timer_toggle(timer)) return false;

	__timer_setstate(timer, TIMER_PAUSED);

	return true;
}

/**
 * @brief Resume a paused timer
 *
 * @param timer
 * @return bool true only if resume succeeded
 */
bool chron_timer_resume(chron_timer_t* timer) {
	if (__timer_has_state(
		timer,
		2,
		TIMER_RESUMED,
		TIMER_DELETED
	)) return false;

	__set_itimerspec(&timer->ts.it_value, timer->time_remaining);
	__set_itimerspec(&timer->ts.it_interval, timer->exp_interval);
	timer->time_remaining = 0;

	if (!chron_timer_toggle(timer)) return false;

	__timer_setstate(timer, TIMER_RESUMED);

	return true;
}

/**
 * @brief Restart the given chron_timer
 *
 * @param timer
 * @return bool
 */
bool chron_timer_restart(chron_timer_t* timer) {
	if (__timer_has_state(
		timer,
		1,
		TIMER_DELETED
	)) return false;

	if (!chron_timer_cancel(timer)) return false;

	__set_itimerspec(&timer->ts.it_value, timer->exp_time);

	if (!timer->is_exponential) {
		__set_itimerspec(&timer->ts.it_interval, timer->exp_interval);
	} else {
		__set_itimerspec(&timer->ts.it_interval, 0);
	}

	timer->invocation_count = 0;
	timer->time_remaining = 0;
	timer->exponential_backoff_time = timer->exp_time;
	if (!chron_timer_toggle(timer)) return false;
	__timer_setstate(timer, TIMER_RUNNING);

	return true;
}

/**
 * @brief Cancel the timer
 *
 * @param timer
 * @return bool
 */
bool chron_timer_cancel(chron_timer_t* timer) {
	if (__timer_has_state(timer, 2, TIMER_INIT, TIMER_DELETED)) {
		return false;
	}

	__set_itimerspec(&timer->ts.it_value, 0);
	__set_itimerspec(&timer->ts.it_interval, 0);

	timer->time_remaining = 0;
	timer->invocation_count = 0;

	if (!chron_timer_toggle(timer)) return false;

	__timer_setstate(timer, TIMER_CANCELLED);

	return true;
}

/**
 * @brief Reschedule the chron_timer
 *
 * @param timer
 * @param exp_time
 * @param exp_interval
 * @return bool
 */
bool chron_timer_reschedule(
	chron_timer_t* timer,
	unsigned long exp_time,
	unsigned long exp_interval
) {
	uint32_t invocation_counter;
	chron_timer_state state = __timer_getstate(timer);

	if (state == TIMER_DELETED) return false;

	invocation_counter = timer->invocation_count;

	if (state != TIMER_CANCELLED) {
		if (!chron_timer_cancel(timer)) return false;
	}

	timer->invocation_count = invocation_counter;

	__set_itimerspec(&timer->ts.it_value, exp_time);

	if (!timer->is_exponential) {
		__set_itimerspec(&timer->ts.it_interval, exp_interval);
	} else {
		__set_itimerspec(&timer->ts.it_interval, 0);
		timer->exponential_backoff_time = exp_time;
	}

	timer->time_remaining = 0;
	if (!chron_timer_toggle(timer)) return false;

	__timer_setstate(timer, TIMER_RUNNING);

	return true;
}

/**
 * @brief Delete the timer
 * The caller must free `callback_arg`
 *
 * @param timer
 * @return bool
 */
bool chron_timer_delete(chron_timer_t* timer) {
	if (timer_delete(timer->timer) < 0) {
		return false;
	}

	timer->callback_arg = NULL;
	__timer_setstate(timer, TIMER_DELETED);

	return true;
}

const char* getEnumStr(chron_timer_state state) {

}

/**
 * @brief Print timer details to stdout
 *
 * @param timer
 */
void chron_timer_print(chron_timer_t* timer) {
	char* readable_state;

	switch (__timer_getstate(timer)) {
		case TIMER_INIT:
			readable_state = "TIMER_INIT";
			break;
		case TIMER_RUNNING:
			readable_state = "TIMER_RUNNING";
			break;
		case TIMER_CANCELLED:
			readable_state = "TIMER_CANCELLED";
			break;
		case TIMER_DELETED:
			readable_state = "TIMER_DELETED";
			break;
		case TIMER_PAUSED:
			readable_state = "TIMER_PAUSED";
			break;
		case TIMER_RESUMED:
			readable_state = "TIMER_RESUMED";
			break;
		// we should never get here, but why not
		default:
			readable_state = "unknown - this is an error; please notify the maintainer";
			break;
	}

	printf(
		"counter = %u | time remaining = %lu | state = %s\n",
		timer->invocation_count,
		chron_timer_get_ms_remaining(timer),
		readable_state
	);
}
