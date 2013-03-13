#include <usb.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

#include <stdint.h>
#include <stdio.h>

/* daemonize */
#include <unistd.h> /* usleep() (deprecated) */
#include <sys/types.h> /* pid_t */
#include <sys/stat.h> /* umask() */

#include "key_events.h"
#include "RAT_driver.h"
#include "debug.h"

static void handle_profile1(Display *display, enum ButtonValue button, int value);
static void handle_profile2(Display *display, enum ButtonValue button, int value);
static void handle_profile3(Display *display, enum ButtonValue button, int value);

static void daemonize(void);

int
main(int argc, char *argv[])
{
	int ret;
	int x, y;
	char data[DATA_SIZE];
	struct usb_device *dev;
	struct usb_dev_handle *handle;
	Display *display;	

#ifndef DEBUG
	daemonize();
#endif

	dev = grab_device();
	if (dev == NULL) {
		edebug("device not found.\n");
		return 1;
	}

	handle = usb_open(dev);
	if (handle == NULL) {
		edebug("failed to open USB device\n");
		return 1;
	}
#ifdef LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP
	ret = usb_detach_kernel_driver_np(handle, 0);
	if (ret < 0)
		edebug("failed to detatch kernel driver");
#endif
	ret = usb_set_configuration(handle, 1);
	if (ret < 0) {
		edebug("failed to set configuration\n");
		usb_close(handle);
		return 1;
	}

	ret = usb_claim_interface(handle, 0);
	if (ret < 0) {
		edebug("failed to claim interface\n");
		usb_close(handle);
		return 1;
	}

	display = XOpenDisplay(NULL);
	if (display == NULL) {
		edebug("failed to open display.\n");
		usb_close(handle);
		return 1;
	}

	set_profile1_callback(handle_profile1);
	set_profile2_callback(handle_profile2);
	set_profile3_callback(handle_profile3);

	ret = 0;
	while (ret >= 0 && !killme) {
		ret = usb_interrupt_read(handle, 0x81, data, DATA_SIZE, 0);
#ifdef SUPER_DEBUG
		debug("%i: %.2X%.2X%.2X%.2X %.2X%.2X%.2X\n", ret,
			(int)(data[0] & 0xff),
			(int)(data[1] & 0xff),
			(int)(data[2] & 0xff),
			(int)(data[3] & 0xff),
			(int)(data[4] & 0xff),
			(int)(data[5] & 0xff),
			(int)(data[6] & 0xff));
#endif
		profile = (int)(data[1] & 0x07);

		handle_event(display, BTN_LEFT, data[0] & 0x01);
		handle_event(display, BTN_RIGHT, data[0] & 0x02);
		handle_event(display, BTN_MIDDLE, data[0] & 0x04);
		handle_event(display, BTN_SIDEF, data[0] & 0x10);
		handle_event(display, BTN_SIDEB, data[0] & 0x08);
		handle_event(display, BTN_SCROLL_RIGHT, data[0] & 0x20);
		handle_event(display, BTN_SCROLL_LEFT, data[0] & 0x40);
		handle_event(display, BTN_CENTER, data[1] & 0x10);
		handle_event(display, BTN_SCROLL, data[6]);
		handle_event(display, BTN_SNIPE, data[0] & 0x80);

		/* move the mouse */
		x = (int)(*(int16_t*)(data + 2));
		y = (int)(*(int16_t*)(data + 4));
		move_mouse_rel(display, x, y);
		usleep(1);
	}

	XCloseDisplay(display);

	usb_release_interface(handle, 0);
	usb_close(handle);
	return 0;
}

static void
handle_profile1(Display *display, enum ButtonValue button, int value)
{
	handle_profile_default(display, button, value);
}

static void
handle_profile2(Display *display, enum ButtonValue button, int value)
{
	handle_profile_default(display, button, value);
}

static void
handle_profile3(Display *display, enum ButtonValue button, int value)
{
	switch (button) {
	case BTN_SIDEF:
		send_key(display, XK_Shift_L, value);
		send_key(display, XK_braceleft, value);
		break;
	case BTN_SIDEB:
		send_key(display, XK_Shift_R, value);
		send_key(display, XK_braceright, value);
		break;
	default:
		handle_profile_default(display, button, value);
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

	/* here we are executing as the child process */
	/* change the file mode mask */
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
