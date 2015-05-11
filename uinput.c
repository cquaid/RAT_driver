#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <linux/input.h>
#include <linux/uinput.h>

#include <sys/ioctl.h>
#include <sys/time.h>

#include "uinput.h"
#include "debug.h"

/*
EV_KEY key press (check usr/include/linux/input.h)
EV_REL rel mov
EV_ABS abs mov

*/


static int
uinput_open(struct uinput *u)
{
	/* if it's initted... just keep it open */
	if (u->fd > 0)
		return 0;

	u->fd = open(UINPUT_PATH, O_WRONLY | O_NONBLOCK);
	if (u->fd < 0) {
		debug("Failed to open %s: %s\n", UINPUT_PATH, strerror(errno));
		return 1;
	}

	return 0;
}

int
uinput_init(struct uinput *u)
{
	int ret, i;
	struct uinput_user_dev uidev;

	if (uinput_open(u))
		return 1;

	memset(&uidev, 0, sizeof(uidev));

	/* max len of uidev.name is 20 */
	memcpy(uidev.name, "RAT Driver", sizeof("RAT Driver"));
	uidev.id.version = 0; /* ?? */
	uidev.id.bustype = BUS_USB;

	/* keys and buttons */
	if (ioctl(u->fd, UI_SET_EVBIT, EV_KEY)) {
		debug("Failed ioctl EV_KEY: %s\n", strerror(errno));
		return 1;
	}

	/* relative movement */
	if (ioctl(u->fd, UI_SET_EVBIT, EV_REL)) {
		debug("Failed ioctl EV_REL: %s\n", strerror(errno));
		return 1;
	}

	/* key repetition */
	if (ioctl(u->fd, UI_SET_EVBIT, EV_REP)) {
		debug("Failed ioctl EV_REP: %s\n", strerror(errno));
		return 1;
	}

	/* syncronous */
	if (ioctl(u->fd, UI_SET_EVBIT, EV_SYN)) {
		debug("Failed ioctl EV_SYN: %s\n", strerror(errno));
		return 1;
	}

	if (ioctl(u->fd, UI_SET_RELBIT, REL_X)) {
		debug("Failed ioctl REL_X: %s\n", strerror(errno));
		return 1;
	}

	if (ioctl(u->fd, UI_SET_RELBIT, REL_Y)) {
		debug("Failed ioctl REL_Y: %s\n", strerror(errno));
		return 1;
	}

	if (ioctl(u->fd, UI_SET_RELBIT, REL_WHEEL)) {
		debug("Failed ioctl REL_WHEEL: %s\n", strerror(errno));
		return 1;
	}

	/* turn on various buttons */
#define btn(x) \
	do { \
		if (ioctl(u->fd, UI_SET_KEYBIT, BTN_##x)) { \
			debug("Failed ioctl BTN_" #x ": %s\n", strerror(errno)); \
			return 1; \
		} \
	} while (0)

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
		if (ioctl(u->fd, UI_SET_KEYBIT, i)) {
			debug("Failed ioctl UI_SET_KEYBIT of %d: %s\n", i, strerror(errno));
			return 1;
		}
	}

	ret = write(u->fd, &uidev, sizeof(uidev));
	if (ret != sizeof(uidev)) {
		debug("Failed to write to uinput: %s\n", strerror(errno));
		return 1;
	}

	if (ioctl(u->fd, UI_DEV_CREATE)) {
		debug("Failed to UI_DEV_CREATE: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}

static int
uinput_close(struct uinput *u)
{
	if (u->fd < 0)
		return 0;

	if (ioctl(u->fd, UI_DEV_DESTROY)) {
		debug("Failed ioctl UI_DEV_DESTROY: %s\n", strerror(errno));
		return 1;
	}

	close(u->fd);
	u->fd = -1;

	return 0;
}

int
uinput_fini(struct uinput *u)
{
	if (uinput_close(u))
		return 1;

	return 0;
}

static int
uinput_push_event(struct uinput *u, int type, int code, int value)
{
	int ret;
	struct input_event event;

	if (u->fd < 0) {
		debugln("Uinput not open.");
		return -1;
	}

	memset(&event, 0, sizeof(event));
	gettimeofday(&event.time, NULL);
	event.type  = type;
	event.code  = code;
	event.value = value;
	ret = write(u->fd, &event, sizeof(event));
	if (ret != sizeof(event)) {
		debug("Failed to write to uinput: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}

int
uinput_push_key(struct uinput *u, int key, int state)
{
	return uinput_push_event(u, EV_KEY, key, state);
}

static int
uinput_push_syn(struct uinput *u)
{
	return uinput_push_event(u, EV_SYN, SYN_REPORT, 0);
}

int
uinput_push_key_end(struct uinput *u)
{
	return uinput_push_syn(u);
}

static int
uinput_send_key(struct uinput *u, int key, int state)
{
	if (uinput_push_key(u, key, state))
		return 1;

	return uinput_push_syn(u);
}

int
uinput_send_key_press(struct uinput *u, int key)
{
	return uinput_send_key(u, key, 1);
}

int
uinput_send_key_release(struct uinput *u, int key)
{
	return uinput_send_key(u, key, 0);
}

int
uinput_send_key_repeate(struct uinput *u, int key)
{
	return uinput_send_key(u, key, 2);
}

int
uinput_send_button_press(struct uinput *u, int btn)
{
	return uinput_send_key_press(u, btn);
}

int
uinput_send_button_release(struct uinput *u, int btn)
{
	return uinput_send_key_release(u, btn);
}

int
uinput_send_button_repeat(struct uinput *u, int btn)
{
	return uinput_send_key_repeate(u, btn);
}

int
uinput_send_button_click(struct uinput *u, int btn)
{
	if (uinput_send_button_press(u, btn))
		return 1;

	return uinput_send_button_release(u, btn);
}

int
uinput_send_mouse_scroll(struct uinput *u, int val)
{
	if (uinput_push_event(u, EV_REL, REL_WHEEL, val))
		return 1;

	return uinput_push_syn(u);
}

int
uinput_send_mouse_x(struct uinput *u, int x)
{
	if (uinput_push_event(u, EV_REL, REL_X, x))
		return 1;

	return uinput_push_syn(u);
}

int
uinput_send_mouse_y(struct uinput *u, int y)
{
	if (uinput_push_event(u, EV_REL, REL_Y, y))
		return 1;

	return uinput_push_syn(u);
}

int
uinput_send_mouse_move(struct uinput *u, int x, int y)
{
	if (uinput_push_event(u, EV_REL, REL_X, x))
		return 1;

	if (uinput_push_event(u, EV_REL, REL_Y, y))
		return 1;

	return uinput_push_syn(u);
}

int
uinput_flush(struct uinput *u)
{
	return uinput_push_syn(u);
}
