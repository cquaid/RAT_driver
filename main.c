#include <linux/input.h>

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* daemonize */
#include <sys/types.h> /* pid_t */
#include <sys/stat.h> /* umask() */

#include "RAT_driver.h"
#include "debug.h"
#include "uinput.h"

static void handle_profile1(RATDriver *rat,
			enum RATButtonValue button, int val);
static void handle_profile2(RATDriver *rat,
			enum RATButtonValue button, int val);
static void handle_profile3(RATDriver *rat,
			enum RATButtonValue button, int val);

static void daemonize(void);
static void print_dpi_modes(RATDriver *rat);


int
main(int argc, char *argv[])
{
	int ret;
	RATDriver rat;

	ret = rat_driver_init();
	if (ret < 0) {
		debugln("Failed to initialize driver.");
		return 1;
	}


	ret = RATDriver_init(&rat, PRODUCT_ID, VENDOR_ID);

	if (ret != 0) {
		debugln("Failed to initialize driver instance.");
		return 1;
	}

#if DEBUG
	print_dpi_modes(&rat);
#endif

#if ! DEBUG
	daemonize();
#endif

	RATDriver_set_profile(&rat, RAT_PROFILE_1, handle_profile1);
	RATDriver_set_profile(&rat, RAT_PROFILE_2, handle_profile2);
	RATDriver_set_profile(&rat, RAT_PROFILE_3, handle_profile3);

	ret = 0;
	while (ret >= 0 && !rat.killme)
		ret = RATDriver_read_data(&rat);

	ret = RATDriver_fini(&rat);

	rat_driver_fini();

	return ret;
}

static void
handle_profile1(RATDriver *rat, enum RATButtonValue button, int val)
{
	RATDriver_handle_profile_default(rat, button, val);
}

static void
handle_profile2(RATDriver *rat, enum RATButtonValue button, int val)
{
	RATDriver_handle_profile_default(rat, button, val);
}

static void
handle_profile3(RATDriver *rat, enum RATButtonValue button, int val)
{
	switch (button) {
	case RAT_BV_SIDE_FRONT:
		if (val != 0) {
			(void)uinput_send_button_press(rat->uinput, KEY_LEFTSHIFT);
			(void)uinput_send_button_click(rat->uinput, KEY_LEFTBRACE);
			(void)uinput_send_button_release(rat->uinput, KEY_LEFTSHIFT);
		}
		break;

	case RAT_BV_SIDE_BACK:
		if (val != 0) {
			(void)uinput_send_button_press(rat->uinput, KEY_LEFTSHIFT);
			(void)uinput_send_button_click(rat->uinput, KEY_RIGHTBRACE);
			(void)uinput_send_button_release(rat->uinput, KEY_LEFTSHIFT);
		}
		break;

	default:
		RATDriver_handle_profile_default(rat, button, val);
		break;
	}
}

static void
daemonize(void)
{
	pid_t pid;
	pid_t sid;

	/* already a daemon */
	if (getppid() == 1)
		return;

	/* fork parent process */
	pid = fork();
	if (pid < 0)
		exit(EXIT_FAILURE);

	/* if a good pid, exit the parent process */
	if (pid > 0)
		exit(EXIT_SUCCESS);

	/* here we are executing as the child process
	 * change the file mode mask */
	umask(0);

	/* create a new sid for the child process */
	sid = setsid();
	if (sid < 0)
		exit(EXIT_FAILURE);

	/* redirect standard files to /dev/null */
	freopen("/dev/null", "r", stdin);
	freopen("/dev/null", "w", stdout);
	freopen("/dev/null", "w", stderr);
}

static void
print_dpi_modes(RATDriver *rat)
{
	int err;

	size_t i;

	enum RATDPIMode mode;
	enum RATDPIMode modes[4] = {
		RAT_DPI_MODE_1,
		RAT_DPI_MODE_2,
		RAT_DPI_MODE_3,
		RAT_DPI_MODE_4
	};

	err = RATDriver_get_active_dpi_mode(rat, &mode);

	if (err == 0)
		printf("Active DPI Mode: %d\n", (int)mode);
	else
		printf("Failed to get active DPI mode.\n");

	for (i = 0; i < sizeof(modes)/sizeof(modes[0]); ++i) {
		uint8_t X;
		uint8_t Y;

		err = RATDriver_get_dpi(rat, modes[i], &X, &Y);

		if (err != 0) {
			printf("Failed to get DPI Mode %zu\n", i+1);
			continue;
		}

		printf("DPI Mode %zu: X %" PRIu8 " Y %" PRIu8 "\n",
				i+1, X, Y);
	}
}
