#include "libchron.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void callback(chron_timer_t* this, void* data) {
	printf("data = %s\n", (char*)data);
}

int main(int argc, char* argv[]) {
	char *str = "demo";

	chron_timer_t* timer = chron_timer_init(
		callback,
		str,
		1000,
		1000,
		0,
		false
	);

	assert(timer != NULL);

	chron_timer_start(timer);

	puts("\t\tTimer Demo\n\n");
	puts("(1) pause\n(2) resume\n(3) restart\n(4) reschedule\n(5) delete\n(6) cancel\n(7) show time remaining\n(8) show timer state\n");
	int selection = 0;

	while (1) {
		scanf("%d", &selection);

		switch (selection) {
			case 1:
				chron_timer_pause(timer);
				break;
			case 2:
				chron_timer_resume(timer);
				break;
			case 3:
				chron_timer_restart(timer);
				break;
			case 4:
				chron_timer_reschedule(
					timer,
					timer->exp_time,
					timer->exp_interval
				);
				break;
			case 5:
				chron_timer_delete(timer);
				break;
			case 6:
				chron_timer_cancel(timer);
				break;
			case 7:
				printf("Time remaining = %lu\n", chron_timer_get_ms_remaining(timer));
				break;
			case 8:
				chron_timer_print(timer);
				break;
			default: continue;
		}

		// pause();
	}
	return EXIT_SUCCESS;
}
