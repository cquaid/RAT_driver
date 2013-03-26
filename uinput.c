#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <linux/input.h>
#include <linux/uinput.h>

#include <sys/ioctl.h>
#include <sys/time.h>

#include "uinput.h"
#include "debug.h"

static int uinput_fd = -1;

/*
EV_KEY key press (check usr/include/linux/input.h)
EV_REL rel mov
EV_ABS abs mov

*/


static int
uinput_open(void)
{
	/* if it's initted... just keep it open */
	if (uinput_fd < 0)
		return 0;

	uinput_fd = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
	if (uinput_fd < 0) {
		edebug("failed to open uinput: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}

int
uinput_init(void)
{
	int ret, i;
	struct uinput_user_dev uidev;
	
	if (uinput_open())
		return 1;

	memset(&uidev, 0, sizeof(uidev));

	/* max len of uidev.name is 20 */
	strncpy(uidev.name, "RAT Driver", 20);
	uidev.id.version = 0; /* ?? */
	uidev.id.bustype = BUS_USB;

	/* keys and buttons */
	if (ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY)) {
		edebug("EV_KEY\n");
		return 1;
	}

	/* relative movement */
	if (ioctl(uinput_fd, UI_SET_EVBIT, EV_REL)) {
		edebug("EV_REL\n");
		return 1;
	}

	/* key repetition */
	if (ioctl(uinput_fd, UI_SET_EVBIT, EV_REP)) {
		edebug("EV_REP\n");
		return 1;
	}

	/* syncronous */
	if (ioctl(uinput_fd, UI_SET_EVBIT, EV_SYN)) {
		edebug("EV_SYN\n");
		return 1;
	}

	if (ioctl(uinput_fd, UI_SET_RELBIT, REL_X)) {
		edebug("REL_X\n");
		return 1;
	}
	
	if (ioctl(uinput_fd, UI_SET_RELBIT, REL_Y)) {
		edebug("REL_Y\n");
		return 1;
	}

	/* turn on various buttons */
#define btn(x) do { \
					if (ioctl(uinput_fd, UI_SET_KEYBIT, BTN_##x)) { \
						edebug("BTN_" #x "\n"); \
						return 1; \
					} \
			   } while(0)

	btn(0); btn(1); btn(2);
	btn(3); btn(4); btn(5);
	btn(6); btn(7); btn(8);
	btn(9);

	btn(LEFT);
	btn(RIGHT);
	btn(MIDDLE);
	btn(SIDE);
	btn(EXTRA);
	btn(FORWARD);
	btn(BACK);
	btn(GEAR_DOWN); /* wheel down */
	btn(GEAR_UP);   /* wheel up */
#undef btn

	/* turn on all the keys */
	for (i = 0; i < 256; ++i) {
		ret = ioctl(uinput_fd, UI_SET_KEYBIT, i);
		if (ret) {
			edebug("KEY_%d\n", i);
			return 1;
		}
	}

	ret = write(uinput_fd, &uidev, sizeof(uidev));
	if (ret != sizeof(uidev)) {
		edebug("%s: write(): %s\n", __func__, strerror(errno));
		return 1;
	}

	ret = ioctl(uinput_fd, UI_DEV_CREATE);
	if (ret) {
		edebug("Failed to UI_DEV_CREATE\n");
		return 1;
	}

	return 0;
}

static int
uinput_close(void)
{
	int ret;

	if (uinput_fd < 0)
		return 0;

	ret = ioctl(uinput_fd, UI_DEV_DESTROY);
	if (ret)
		return 1;
	
	close(uinput_fd);
	uinput_fd = -1;

	return 0;
}

int
uinput_fini(void)
{
	if (uinput_close())
		return 1;
	
	return 0;
}

static int
uinput_push_event(int type, int code, int value)
{
	int ret;
	struct input_event event;

	if (uinput_fd < 0)
		return -1;
	
	memset(&event, 0, sizeof(event));
	gettimeofday(&event.time, NULL);
	event.type  = type;
	event.code  = code;
	event.value = value;
	ret = write(uinput_fd, &event, sizeof(event));
	if (ret != sizeof(event)) {
		fprintf(stderr, "write(): %s\n", strerror(errno));
		return 1;
	}

	return 0;
}

int
uinput_push_key(int key, int state)
{
	return uinput_push_event(EV_KEY, key, state);
}

static int
uinput_push_syn(void)
{
	return uinput_push_event(EV_SYN, SYN_REPORT, 0);
}

int
uinput_push_key_end(void)
{
	return uinput_push_syn();
}

static int
uinput_send_key(int key, int state)
{
	if (uinput_fd < 0)
		return 1;
	
	if (uinput_push_key(key, state))
		return 1;

	return uinput_push_syn();
}

int
uinput_send_key_press(int key)
{
	return uinput_send_key(key, 1);
}

int
uinput_send_key_release(int key)
{
	return uinput_send_key(key, 0);
}

int
uinput_send_key_repeate(int key)
{
	return uinput_send_key(key, 2);
}

int
uinput_send_button_press(int btn)
{
	return uinput_send_key_press(btn);
}

int
uinput_send_button_release(int btn)
{
	return uinput_send_key_release(btn);
}

int
uinput_send_button_repeat(int btn)
{
	return uinput_send_key_repeate(btn);
}

int
uinput_send_button_click(int btn)
{
	if (uinput_send_button_press(btn))
		return 1;
	
	return uinput_send_button_release(btn);
}

int
uinput_send_mouse_x(int x)
{
	if (uinput_push_event(EV_REL, REL_X, x))
		return 1;
	
	return uinput_push_syn();
}

int
uinput_send_mouse_y(int y)
{
	if (uinput_push_event(EV_REL, REL_Y, y))
		return 1;
	
	return uinput_push_syn();
}

int
uinput_send_mouse_move(int x, int y)
{
	if (uinput_push_event(EV_REL, REL_X, x))
		return 1;
	
	if (uinput_push_event(EV_REL, REL_Y, y))
		return 1;
	
	return uinput_push_syn();
}

int
uinput_flush(void)
{
	return uinput_push_syn();
}
