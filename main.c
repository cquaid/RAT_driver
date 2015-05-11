#include <usb.h>

#include <linux/input.h>

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* daemonize */
#include <unistd.h> /* usleep() (deprecated) */
#include <sys/types.h> /* pid_t */
#include <sys/stat.h> /* umask() */

#include "RAT_driver.h"
#include "debug.h"
#include "uinput.h"

static void handle_profile1(RATDriver *rat, enum ButtonValue button, int val);
static void handle_profile2(RATDriver *rat, enum ButtonValue button, int val);
static void handle_profile3(RATDriver *rat, enum ButtonValue button, int val);

static void daemonize(void);

int
main(int argc, char *argv[])
{
	int ret;
	RATDriver rat;

	ret = rat_driver_init(&rat, PRODUCT_ID, VENDOR_ID);
	if (ret != 0) {
		debugln("Failed to initialize driver.");
		return 1;
	}

#ifndef DEBUG
	daemonize();
#endif

	rat_driver_set_profile(&rat, PROFILE_1, handle_profile1);
	rat_driver_set_profile(&rat, PROFILE_2, handle_profile2);
	rat_driver_set_profile(&rat, PROFILE_3, handle_profile3);

	ret = 0;
	while (ret >= 0 && rat.killme == 0) {
		ret = rat_driver_read_data(&rat);
		//usleep(1);
	}

	ret = rat_driver_fini(&rat);

	return ret;
}

static void
handle_profile1(RATDriver *rat, enum ButtonValue button, int val)
{
	rat_handle_profile_default(rat, button, val);
}

static void
handle_profile2(RATDriver *rat, enum ButtonValue button, int val)
{
	rat_handle_profile_default(rat, button, val);
}

static void
handle_profile3(RATDriver *rat, enum ButtonValue button, int val)
{
	switch (button) {
	case BV_SIDE_FRONT:
		if (val != 0) {
			(void)uinput_send_button_press(rat->uinput, KEY_LEFTSHIFT);
			(void)uinput_send_button_click(rat->uinput, KEY_LEFTBRACE);
			(void)uinput_send_button_release(rat->uinput, KEY_LEFTSHIFT);
		}
		break;

	case BV_SIDE_BACK:
		if (val != 0) {
			(void)uinput_send_button_press(rat->uinput, KEY_LEFTSHIFT);
			(void)uinput_send_button_click(rat->uinput, KEY_RIGHTBRACE);
			(void)uinput_send_button_release(rat->uinput, KEY_LEFTSHIFT);
		}
		break;

	default:
		rat_handle_profile_default(rat, button, val);
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
