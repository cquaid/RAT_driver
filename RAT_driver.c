#include <usb.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "RAT_driver.h"
#include "debug.h"

/* profile is global to simplify things */
int profile = PROFILE_1;
int killme = 0;

static profile_callback pc_1 = handle_profile_default;
static profile_callback pc_2 = handle_profile_default;
static profile_callback pc_3 = handle_profile_default;
static profile_callback pc_default = handle_profile_default;

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

void
mouse_click(Display *display, int button)
{
	XTestFakeButtonEvent(display, button, True, CurrentTime);
	usleep(1);
}

void
mouse_release(Display *display, int button)
{
	XTestFakeButtonEvent(display, button, False, CurrentTime);
	usleep(1);
}

void
mouse_scroll(Display *display, int value)
{
	if ((value & 0xff) == 0x01)
		mouse_click(display, BTN_SCROLL_UP);
	else if ((value & 0xff) == 0xff)
		mouse_click(display, BTN_SCROLL_DOWN);
	else {
		mouse_release(display, BTN_SCROLL_UP);
		mouse_release(display, BTN_SCROLL_DOWN);
	}
}

void
handle_profile_default(Display *display, enum ButtonValue button, int value)
{
	if ((int)button < (int)XBTN_MAX) {
		if (value)
			mouse_click(display, (int)button);
		else
			mouse_release(display, (int)button);
	}
	else if (button == BTN_SCROLL)
		mouse_scroll(display, value);
	else if (button == BTN_SNIPE)
		killme = !!value;
	else return;
}

void
handle_event(Display *display, enum ButtonValue button, int value)
{
	profile_callback call = handle_profile_default;
	switch (profile) {
		case PROFILE_1:
			if (pc_1 != NULL)
				call = pc_1;
			break;
		case PROFILE_2:
			if (pc_2 != NULL)
				call = pc_2;
			break;
		case PROFILE_3:
			if (pc_3 != NULL)
				call = pc_3;
			break;
		default:
			if (pc_default != NULL)
				call = pc_default;
			break;
	}

	/* not possible... ever */
	if (call == NULL)
		return;
	
	call(display, button, value);
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

void
set_profile1_callback(profile_callback pc)
{
	if (pc != NULL)
		pc_1 = pc;
}

void
set_profile2_callback(profile_callback pc)
{
	if (pc != NULL)
		pc_2 = pc;
}

void
set_profile3_callback(profile_callback pc)
{
	if (pc != NULL)
		pc_3 = pc;
}
