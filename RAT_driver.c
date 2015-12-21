#include <linux/input.h>

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <libusb-1.0/libusb.h>

#include "debug.h"
#include "RAT_driver.h"
#include "uinput.h"

#define RAT_ENDPOINT_IN  (1 | LIBUSB_ENDPOINT_IN)
#define RAT_ENDPOINT_OUT (0 | LIBUSB_ENDPOINT_OUT) /* XXX */

int
rat_driver_init(void)
{
	int ret;

	ret = libusb_init(NULL);

	if (ret < 0) {
		debug("libusb_init() failed: (%d) %s\n",
			ret, libusb_error_name(ret));
	}

	return ret;
}

void
rat_driver_fini(void)
{
	libusb_exit(NULL);
}


static struct libusb_device_handle *
grab_device(int product, int vendor)
{
	struct libusb_device_handle *handle;

	/* XXX: Not the best way to do this.  See LibUSB Documentation. */
	handle = libusb_open_device_with_vid_pid(NULL, vendor, product);

	return handle;
}

int
RATDriver_init(RATDriver *rat, int product, int vendor)
{
	int ret, err;

	if (rat == NULL)
		return -EINVAL;

	memset(rat, 0, sizeof(*rat));

	rat->killme = false;
	rat->profile = 1;
	rat->dpi_mode = 1;

	rat->vendor_id = vendor;
	rat->product_id = product;

	rat->profile1 = RATDriver_handle_profile_default;
	rat->profile2 = RATDriver_handle_profile_default;
	rat->profile3 = RATDriver_handle_profile_default;

	rat->interpret_data = RATDriver_interpret_data_default;

	rat->usb_handle = grab_device(product, vendor);

	if (rat->usb_handle == NULL) {
		debug("Failed to find device %04x:%04x.\n",
			vendor, product);
		return -ENODEV;
	}

	err = libusb_claim_interface(rat->usb_handle, 0);

	if (err < 0) {
		ret = -EIO;
		debug("libusb_claim_interface failed: (%d) %s\n",
			err, libusb_error_name(err));
		goto out_close;
	}

	rat->uinput = calloc(1, sizeof(*(rat->uinput)));

	if (rat->uinput == NULL) {
		ret = -ENOMEM;
		goto out_release;
	}

	err = uinput_init(rat->uinput);

	if (err != 0) {
		(void)uinput_fini(rat->uinput);
		free(rat->uinput);
		ret = -EIO;
		goto out_release;
	}

	return 0;

out_release:

	libusb_release_interface(rat->usb_handle, 0);

out_close:

	libusb_close(rat->usb_handle);

	return ret;
}

int
RATDriver_fini(RATDriver *rat)
{
	if (rat == NULL)
		return -EINVAL;

	(void)uinput_fini(rat->uinput);
	free(rat->uinput);

	libusb_release_interface(rat->usb_handle, 0);
	libusb_close(rat->usb_handle);

	return 0;
}

void
RATDriver_set_profile(RATDriver *rat, int profile, rat_profile_callback pc)
{
	if (rat == NULL || pc == NULL)
		return;

	switch (profile) {
	case RAT_PROFILE_1:
		rat->profile1 = pc;
		break;

	case RAT_PROFILE_2:
		rat->profile2 = pc;
		break;

	case RAT_PROFILE_3:
		rat->profile3 = pc;
		break;

	default:
		break;
	}
}


void
RATDriver_mouse_click(RATDriver *rat, int button)
{
	(void)uinput_send_button_press(rat->uinput, button);
}

void
RATDriver_mouse_release(RATDriver *rat, int button)
{
	(void)uinput_send_button_release(rat->uinput, button);
}

void
RATDriver_mouse_scroll(RATDriver *rat, int value)
{
	if ((value & 0xff) == 0x01)
		(void)uinput_send_mouse_scroll(rat->uinput, 1);
	else if ((value & 0xff) == 0xff)
		(void)uinput_send_mouse_scroll(rat->uinput, -1);
	else
		(void)uinput_send_mouse_scroll(rat->uinput, 0);
}

void
RATDriver_handle_profile_default(RATDriver *rat, enum RATButtonValue button, int val)
{
	if (button == RAT_BV_SCROLL)
		RATDriver_mouse_scroll(rat, val);
#ifdef KILL_ON_SNIPE
	else if (button == RAT_BV_SNIPE)
		rat->killme = val ? true : false;
#endif
	else {
		if (val != 0)
			RATDriver_mouse_click(rat, (int)button);
		else
			RATDriver_mouse_release(rat, (int)button);
	}
}

void
RATDriver_handle_event(RATDriver *rat, enum RATButtonValue button, int val)
{
	rat_profile_callback call = RATDriver_handle_profile_default;

	switch (rat->profile) {
	case RAT_PROFILE_1:
		if (rat->profile1 != NULL)
			call = rat->profile1;
		break;

	case RAT_PROFILE_2:
		if (rat->profile2 != NULL)
			call = rat->profile2;
		break;

	case RAT_PROFILE_3:
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
RATDriver_mouse_move_rel(RATDriver *rat, int x, int y)
{
	(void)uinput_send_mouse_move(rat->uinput, x, y);
}

int
RATDriver_interpret_data_default(RATDriver *rat, char buffer[RAT_DATA_LEN])
{
	int16_t x, y;

	rat->profile  = (int)(buffer[1] & 0x07);
	rat->dpi_mode = (int)(buffer[1] & 0x70);

	RATDriver_handle_event(rat, RAT_BV_LEFT,         buffer[0] & 0x01);
	RATDriver_handle_event(rat, RAT_BV_RIGHT,        buffer[0] & 0x02);
	RATDriver_handle_event(rat, RAT_BV_MIDDLE,       buffer[0] & 0x04);
	RATDriver_handle_event(rat, RAT_BV_SIDE_BACK,    buffer[0] & 0x08);
	RATDriver_handle_event(rat, RAT_BV_SIDE_FRONT,   buffer[0] & 0x10);
	RATDriver_handle_event(rat, RAT_BV_SCROLL_RIGHT, buffer[0] & 0x20);
	RATDriver_handle_event(rat, RAT_BV_SCROLL_LEFT,  buffer[0] & 0x40);
	RATDriver_handle_event(rat, RAT_BV_SNIPE,        buffer[0] & 0x80);

	RATDriver_handle_event(rat, RAT_BV_CENTER,       buffer[1] & 0x10);
	RATDriver_handle_event(rat, RAT_BV_SCROLL,       buffer[6] & 0xFF);

	/* Move the mouse.
	 * XXX: Endianess? */
	x = *(int16_t *)(buffer + 2);
	y = *(int16_t *)(buffer + 4);

	RATDriver_mouse_move_rel(rat, (int)x, (int)y);

	return 0;
}


int
RATDriver_read_data(RATDriver *rat)
{
	int ret;
	int transfered;
	char buffer[RAT_DATA_LEN];


	ret = libusb_bulk_transfer(rat->usb_handle, RAT_ENDPOINT_IN,
				buffer, sizeof(buffer), &transfered, 0);

	debug("Requested %zd, got %d\n",
			sizeof(buffer), transfered);

	if (transfered != sizeof(buffer))
		ret = LIBUSB_ERROR_IO;

	if (ret != 0) {
		debug("libusb_bulk_transfer failed: (%d) %s\n",
			ret, libusb_error_name(ret));
		return ret;
	}

	if (rat->interpret_data != NULL)
		return rat->interpret_data(rat, buffer);

	return RATDriver_interpret_data_default(rat, buffer);
}
