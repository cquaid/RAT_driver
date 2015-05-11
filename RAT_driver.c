#include <linux/input.h>

#include <errno.h>
#include <stddef.h>
#include <string.h>

#include <usb.h>

#include "debug.h"
#include "RAT_driver.h"
#include "uinput.h"

static struct usb_device*
grab_device(int product, int vendor)
{
	struct usb_bus *bus;
	struct usb_device *dev;

	usb_init();
	usb_find_busses();
	usb_find_devices();

	debug("Looking for %04x:%04x\n", vendor, product);

#define DES(x) dev->descriptor.id##x
	for (bus = usb_get_busses(); bus != NULL; bus = bus->next) {
		for (dev = bus->devices; dev != NULL; dev = dev->next) {
			debug("Checking device %04x:%04x\n", DES(Vendor), DES(Product));
			if ((DES(Vendor) == vendor) && (DES(Product) == product))
				return dev;
		}
	}
#undef DES

	return NULL;
}

int
rat_driver_init(RATDriver *rat, int product, int vendor)
{
	int ret, err;

	if (rat == NULL)
		return -EINVAL;

	memset(rat, 0, sizeof(*rat));

	rat->killme = 0;
	rat->profile = 1;
	rat->dpi_mode = 1;

	rat->vendor_id = vendor;
	rat->product_id = product;

	rat->profile1 = rat_handle_profile_default;
	rat->profile2 = rat_handle_profile_default;
	rat->profile3 = rat_handle_profile_default;

	rat->interpret_data = rat_interpret_data_default;

	rat->usb_dev = grab_device(product, vendor);

	if (rat->usb_dev == NULL)
		return -ENODEV;

	rat->usb_handle = usb_open(rat->usb_dev);

	if (rat->usb_handle == NULL)
		return -EACCES;

#ifdef LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP
	err = usb_detach_kernel_driver_np(rat->usb_handle, 0);

#if 0
	if (err < 0) {
		ret = -EACCES;
		goto out;
	}
#endif
#endif

	err = usb_set_configuration(rat->usb_handle, 1);

	if (err < 0) {
		ret = -EIO;
		goto out;
	}

	err = usb_claim_interface(rat->usb_handle, 0);

	if (err < 0) {
		ret = -EIO;
		goto out;
	}

	rat->uinput = calloc(1, sizeof(*(rat->uinput)));

	if (rat->uinput == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	err = uinput_init(rat->uinput);

	if (err != 0) {
		(void)uinput_fini(rat->uinput);
		free(rat->uinput);
		ret = -EIO;
		goto out;
	}

	ret = 0;

out:

	if (ret != 0)
		usb_close(rat->usb_handle);

	return ret;
}

int
rat_driver_fini(RATDriver *rat)
{
	if (rat == NULL)
		return -EINVAL;

	(void)uinput_fini(rat->uinput);
	free(rat->uinput);

	usb_release_interface(rat->usb_handle, 0);
	usb_close(rat->usb_handle);

	return 0;
}

void
rat_driver_set_profile(RATDriver *rat, int profile, profile_callback pc)
{
	if (rat == NULL || pc == NULL)
		return;

	switch (profile) {
	case PROFILE_1:
		rat->profile1 = pc;
		break;

	case PROFILE_2:
		rat->profile2 = pc;
		break;

	case PROFILE_3:
		rat->profile3 = pc;
		break;

	default:
		break;
	}
}


void
rat_mouse_click(RATDriver *rat, int button)
{
	(void)uinput_send_button_press(rat->uinput, button);
}

void
rat_mouse_release(RATDriver *rat, int button)
{
	(void)uinput_send_button_release(rat->uinput, button);
}

void
rat_mouse_scroll(RATDriver *rat, int value)
{
	if ((value & 0xff) == 0x01)
		(void)uinput_send_mouse_scroll(rat->uinput, 1);
	else if ((value & 0xff) == 0xff)
		(void)uinput_send_mouse_scroll(rat->uinput, -1);
	else
		(void)uinput_send_mouse_scroll(rat->uinput, 0);
}

void
rat_handle_profile_default(RATDriver *rat, enum ButtonValue button, int val)
{
	if (button == BV_SCROLL)
		rat_mouse_scroll(rat, val);
#ifdef KILL_ON_SNIPE
	else if (button == BV_SNIPE)
		rat_rat->killme = !!val;
#endif
	else {
		if (val != 0)
			rat_mouse_click(rat, (int)button);
		else
			rat_mouse_release(rat, (int)button);
	}
}

void
rat_handle_event(RATDriver *rat, enum ButtonValue button, int val)
{
	profile_callback call = rat_handle_profile_default;

	switch (rat->profile) {
	case PROFILE_1:
		if (rat->profile1 != NULL)
			call = rat->profile1;
		break;

	case PROFILE_2:
		if (rat->profile2 != NULL)
			call = rat->profile2;
		break;

	case PROFILE_3:
		if (rat->profile3 != NULL)
			call = rat->profile3;
		break;

	default:
		break;
	}

	/* not possible... ever */
	if (call == NULL)
		return;

	call(rat, button, val);
}

void
rat_mouse_move_rel(RATDriver *rat, int x, int y)
{
	(void)uinput_send_mouse_move(rat->uinput, x, y);
}

int
rat_interpret_data_default(RATDriver *rat, char buffer[RAT_DATA_LEN])
{
	int16_t x, y;

	rat->profile  = (int)(buffer[1] & 0x07);
	rat->dpi_mode = (int)(buffer[1] & 0x70);

	rat_handle_event(rat, BV_LEFT,         buffer[0] & 0x01);
	rat_handle_event(rat, BV_RIGHT,        buffer[0] & 0x02);
	rat_handle_event(rat, BV_MIDDLE,       buffer[0] & 0x04);
	rat_handle_event(rat, BV_SIDE_BACK,    buffer[0] & 0x08);
	rat_handle_event(rat, BV_SIDE_FRONT,   buffer[0] & 0x10);
	rat_handle_event(rat, BV_SCROLL_RIGHT, buffer[0] & 0x20);
	rat_handle_event(rat, BV_SCROLL_LEFT,  buffer[0] & 0x40);
	rat_handle_event(rat, BV_SNIPE,        buffer[0] & 0x80);

	rat_handle_event(rat, BV_CENTER,       buffer[1] & 0x10);
	rat_handle_event(rat, BV_SCROLL,       buffer[6] & 0xFF);

	/* Move the mouse.
	 * XXX: Endianess? */
	x = *(int16_t *)(buffer + 2);
	y = *(int16_t *)(buffer + 4);

	rat_mouse_move_rel(rat, (int)x, (int)y);

	return 0;
}


int
rat_driver_read_data(RATDriver *rat)
{
	int ret;
	char buffer[RAT_DATA_LEN];

	ret = usb_interrupt_read(rat->usb_handle, 0x81, buffer, RAT_DATA_LEN, 0);

	if (ret < 0) {
		if (ret == -EAGAIN)
			ret = 0;
		return ret;
	}

	if (rat->interpret_data != NULL)
		return rat->interpret_data(rat, buffer);

	return rat_interpret_data_default(rat, buffer);
}
