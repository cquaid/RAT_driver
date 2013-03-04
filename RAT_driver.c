#include <usb.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* daemonize */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "debug.h"

/* Saitek R.A.T. 7 Albino ID */
#define VENDOR_ID  (0x06a3)
#define PRODUCT_ID (0x0cce)
#define DATA_SIZE  (7)

enum Profile {
	PROFILE_1 = 0x01,
	PROFILE_2 = 0x02,
	PROFILE_3 = 0x04
};

enum XButtonValue {
	XBTN_LEFT   = 1, /* left click         */
	XBTN_MIDDLE = 2, /* scroll wheel click */
	XBTN_RIGHT  = 3, /* right click        */
	XBTN_SUP    = 4, /* scroll up          */
	XBTN_SDOWN  = 5, /* scroll down        */
	XBTN_SLEFT  = 6, /* side scroll wheel  */
	XBTN_SRIGHT = 7, /* side scroll wheel  */
	XBTN_BTN4   = 8, /* front side button  */
	XBTN_BTN5   = 9, /* rear side button   */
	XBTN_MAX    = 10
};

enum ButtonValue {
	BTN_LEFT         = XBTN_LEFT,
	BTN_RIGHT        = XBTN_RIGHT,
	BTN_MIDDLE       = XBTN_MIDDLE,
	BTN_SIDEF        = XBTN_BTN4,
	BTN_SIDEB        = XBTN_BTN5,
	BTN_SNIPE        = 77,
	BTN_SCROLL_UP    = XBTN_SUP,
	BTN_SCROLL_DOWN  = XBTN_SDOWN,
	BTN_SCROLL_LEFT  = XBTN_SLEFT,
	BTN_SCROLL_RIGHT = XBTN_SRIGHT,
	BTN_CENTER       = 88,
	BTN_SCROLL       = 99
};

/* profile is global to simplify things */
static int profile = PROFILE_1;
static int killme = 0;

struct usb_device* grab_device(void);

void handle_event(Display *display, enum ButtonValue button,  int value);
void get_coords(Display *display, int *x, int *y);
void move_mouse_abs(Display *display, int x, int y);
void move_mouse_rel(Display *display, int x, int y);

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

struct usb_device*
grab_device(void)
{
	struct usb_bus *bus;
	struct usb_device *dev;

	usb_init();
	usb_find_busses();
	usb_find_devices();
	bus = usb_get_busses();

#define DES(x) dev->descriptor.id##x
	for (; bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if ((DES(Vendor) == VENDOR_ID) && (DES(Product) == PRODUCT_ID))
				return dev;
		}
	}
#undef DES
	return NULL;
}

inline void
_mouse_click(Display *display, int button)
{
	XTestFakeButtonEvent(display, button, True, CurrentTime);
	usleep(1);
}

inline void
_mouse_release(Display *display, int button)
{
	XTestFakeButtonEvent(display, button, False, CurrentTime);
	usleep(1);
}

void
_mouse_scroll(Display *display, int value)
{
	if ((value & 0xff) == 0x01)
		_mouse_click(display, BTN_SCROLL_UP);
	else if ((value & 0xff) == 0xff)
		_mouse_click(display, BTN_SCROLL_DOWN);
	else {
		_mouse_release(display, BTN_SCROLL_UP);
		_mouse_release(display, BTN_SCROLL_DOWN);
	}
}

void
handle_profile_default(Display *display, enum ButtonValue button, int value)
{
	if ((int)button < (int)XBTN_MAX) {
		if (value)
			_mouse_click(display, (int)button);
		else
			_mouse_release(display, (int)button);
	}
	else if (button == BTN_SCROLL)
		_mouse_scroll(display, value);
	else if (button == BTN_SNIPE)
		killme = !!value;
	else return;
}

void
handle_profile1(Display *display, enum ButtonValue button, int value)
{
	handle_profile_default(display, button, value);
}

void
handle_profile2(Display *display, enum ButtonValue button, int value)
{
	handle_profile_default(display, button, value);
}

void
handle_profile3(Display *display, enum ButtonValue button, int value)
{
	handle_profile_default(display, button, value);
}

void
handle_event(Display *display, enum ButtonValue button, int value)
{
	switch (profile) {
		case PROFILE_1:
			handle_profile1(display, button, value);
			break;
		case PROFILE_2:
			handle_profile2(display, button, value);
		case PROFILE_3:
			handle_profile3(display, button, value);
		default:
			handle_profile_default(display, button, value);
	}
}

void
get_coords(Display *display, int *x, int *y)
{
	XEvent event;
	
	XQueryPointer(display, DefaultRootWindow(display),
		&event.xbutton.root, &event.xbutton.subwindow,
		&event.xbutton.x_root, &event.xbutton.y_root,
		&event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
	
	*x = event.xbutton.x;
	*y = event.xbutton.y;
}

void
move_mouse_abs(Display *display, int x, int y)
{
	XTestFakeMotionEvent(display, 0, x, y, CurrentTime);
	usleep(1);
}

void
move_mouse_rel(Display *display, int x, int y)
{
	int rx, ry;
	get_coords(display, &rx, &ry);
	XTestFakeMotionEvent(display, 0, rx + x, ry + y, CurrentTime);
	XSync(display, 0);
	usleep(1);
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
