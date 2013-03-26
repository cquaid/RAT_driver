#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <linux/input.h>
#include <linux/uinput.h>

#include <sys/ioctl.h>

#include "uinput.h"

static int uinput_fd = 0;

/*
EV_KEY key press (check usr/include/linux/input.h)
EV_REL rel mov
EV_ABS abs mov

*/


static int
uinput_open(void)
{
	/* if it's initted... just keep it open */
	if (uinput_fd)
		return 0;

	uinput_fd = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
	if (uinput_fd < 0)
		return errno;
}

static inline int
init_rel(void)
{		
	return ioctl(uinput_fd, UI_SET_EVBIT, EV_REL);
}

static inline int
init_keys(void)
{
	int ret;

	/* turn on key and button events */
	ret = ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY);
	if (ret)
		return 1;

	/* turn on various buttons */
#define btn(x) do { ret = ioctl(uinput_fd, UI_SET_KEYBIT, BTN_##x); \
		    if (ret) return 1; } while(0)
	btn(0);
	btn(1);
	btn(2);
	btn(3);
	btn(4);
	btn(5);
	btn(6);
	btn(7);
	btn(8);
	btn(9);

	btn(LEFT);
	btn(RIGHT);
	btn(MIDDLE);
	btn(SIDE);
	btn(EXTRA);
	btn(FORWARD);
	btn(BACK);
	btn(GEAR_DOWN); /* wheel down */
	btn(GEAR_UP); /* wheel up */
#undef btn

	return 0;
}

int
uinput_init(void)
{
	if (uinput_open())
		return 1;

	if (init_keys())
		return 1;

	if (init_rel())
		return 1;

}

