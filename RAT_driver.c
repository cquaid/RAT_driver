#include <usb.h>
#include <stddef.h>

#include <linux/input.h>

#include "RAT_driver.h"
#include "uinput.h"

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
mouse_click(int button)
{
	(void)uinput_send_button_press(button);
}

void
mouse_release(int button)
{
	(void)uinput_send_button_release(button);
}

void
mouse_scroll(int value)
{
	if ((value & 0xff) == 0x01)
		(void)uinput_send_mouse_scroll(1);
	else if ((value & 0xff) == 0xff)
		(void)uinput_send_mouse_scroll(-1);
	else
		(void)uinput_send_mouse_scroll(0);
}

void
handle_profile_default(enum ButtonValue button, int value)
{
	if (button == BV_SCROLL)
		mouse_scroll(value);
#ifdef KILL_ON_SNIPE
	else if (button == BV_SNIPE)
		killme = !!value;
#endif
	else {
		if (value)
			mouse_click((int)button);
		else
			mouse_release((int)button);
	}
}

void
handle_event(enum ButtonValue button, int value)
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
	
	call(button, value);
}

void
move_mouse_rel(int x, int y)
{
	(void)uinput_send_mouse_move(x, y);
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
