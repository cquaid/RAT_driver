#include <linux/input.h>

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <libusb-1.0/libusb.h>

#include "debug.h"
#include "RAT_driver.h"
#include "uinput.h"

#define RAT_INTR_IN  (1 | LIBUSB_ENDPOINT_IN)

#define RAT_CTRL_IN  (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN)
#define RAT_CTRL_OUT (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT)

#define RAT_CTRL_REQ_READ  (0x90)
#define RAT_CTRL_REQ_WRITE (0x91)

#define RAT_CTRL_RD_GET_ACTIVE_DPI_MODE_IDX  ((uint16_t)0x0074)
#define RAT_CTRL_RD_GET_ACTIVE_DPI_MODE_VAL  ((uint16_t)0)
#define RAT_CTRL_RD_GET_ACTIVE_DPI_MODE_LEN  (1)

/* Format:  0xMA73 ; M = mode ; A = axis */
#define RAT_CTRL_RD_GET_DPI_MODE_IDX(mode, axis) \
	((uint16_t)(0x0073 | ((axis & 0x0f) << 8) | ((mode & 0x0f) << 12)))
#define RAT_CTRL_RD_GET_DPI_MODE_VAL  ((uint16_t)0)
#define RAT_CTRL_RD_GET_DPI_MODE_LEN  (2)

/* Format:  0xMAVV ; M = mode ; A = axis ; VV = val */
#define RAT_CTRL_WR_SET_DPI_MODE_VAL(mode, axis, val) \
	((uint16_t)((val & 0x00ff) | ((axis & 0x0f) << 8) | ((mode & 0x0f) << 12)))
#define RAT_CTRL_WR_SET_DPI_MODE_IDX  ((uint32_t)0x73)

#define RAT_CTRL_WR_CONFIRM_VAL  ((uint16_t)0x51)
#define RAT_CTRL_WR_CONFIRM_IDX  ((uint16_t)0x70)

#define RAT_WR_RESET_DPI_MODES_VAL  ((uint16_t)0x00)
#define RAT_WR_RESET_DPI_MODES_IDX  ((uint16_t)0x73)

#define RAT_WR_SET_ACTIVE_DPI_MODE_VAL(mode) \
	((uint16_t)((uint16_t)mode & 0x0f) << 12)
#define RAT_WR_SET_ACTIVE_DPI_MODE_IDX  ((uint16_t)0x74)

#define RAT_DPI_X_AXIS  (1)
#define RAT_DPI_Y_AXIS  (2)


static struct libusb_context *ctx = NULL;

int
rat_driver_init(void)
{
	int ret;

	ret = libusb_init(&ctx);

	if (ret < 0) {
		debug("libusb_init() failed: (%d) %s\n",
			ret, libusb_error_name(ret));
	}

	return ret;
}

void
rat_driver_fini(void)
{
	libusb_exit(ctx);
}

#if DEBUG
static void
debug_print_device_info(struct libusb_device *dev,
	struct libusb_device_descriptor *desc)
{
	uint8_t i;
	struct libusb_config_descriptor *config;

	libusb_get_config_descriptor(dev, 0, &config);

	fprintf(stderr,
		"VendorID: %04" PRIx16 "\n"
		"ProductID: %04" PRIx16 "\n"
		"Device Class: %" PRIu8 "\n"
		"Number of Possible Configurations: %" PRIu8 "\n"
		"Number of Interfaces: %" PRIu8 "\n",
		desc->idVendor,
		desc->idProduct,
		desc->bDeviceClass,
		desc->bNumConfigurations,
		config->bNumInterfaces);

	for (i = 0; i < config->bNumInterfaces; ++i) {
		int j;
		const struct libusb_interface *interface;

		interface = &(config->interface[i]);

		fprintf(stderr,
				"  Interface: %" PRIu8 "\n"
				"  Number of Alternate Settings: %d\n",
				i, interface->num_altsetting);

		for (j = 0; j < interface->num_altsetting; ++j) {
			uint8_t k;
			const struct libusb_interface_descriptor *idesc;

			idesc = &(interface->altsetting[j]);

			fprintf(stderr,
					"    Alternate Setting: %d\n"
					"    Number of End Points: %" PRIu8 "\n",
					j, idesc->bNumEndpoints);

			for (k = 0; k < idesc->bNumEndpoints; ++k) {
				const struct libusb_endpoint_descriptor *epdesc;

				epdesc = &(idesc->endpoint[k]);

				fprintf(stderr,
						"      End Point: %" PRIu8 "\n"
						"      Descriptor Type: 0x%" PRIx8 "\n"
						"      End Point Address: 0x%" PRIx8 "\n"
						"      Length: %" PRIu8 "\n",
						k, epdesc->bDescriptorType,
						epdesc->bEndpointAddress,
						epdesc->bLength);
			}
		}
	}

	libusb_free_config_descriptor(config);
}
#else
#define debug_print_device_info(dev, desc) do{}while(0)
#endif


static struct libusb_device_handle *
grab_device(uint16_t product, uint16_t vendor)
{
	int err;
	ssize_t i;
	ssize_t dev_count;

	struct libusb_device **devs;

	struct libusb_device *dev = NULL;
	struct libusb_device_handle *handle = NULL;

	dev_count = libusb_get_device_list(ctx, &devs);

	if (dev_count < 0) {
		debug("libusb_get_device_list() failed: (%zd) %s\n",
			dev_count, libusb_error_name(dev_count));
		return NULL;
	}

	/* Find device.
	 *
	 * TODO: There can be multiple devices.
	 *       We should grab them all and spawn individual
	 *       threads for each.
	 *       For now, just grab the first occurance.
	 */
	for (i = 0; i < dev_count; ++i) {
		struct libusb_device_descriptor desc;

		err = libusb_get_device_descriptor(devs[i], &desc);

		if (err < 0) {
			debug("libusb_get_device_descriptor() failed for device %zd: "
				  "(%d) %s\n", i, err, libusb_error_name(err));

			/* Ignore the error and continue. */
			continue;
		}

		/* Found a match. */
		if (desc.idVendor == vendor && desc.idProduct == product) {
			dev = devs[i];
			debug_print_device_info(dev, &desc);
			break;
		}
	}

	if (dev == NULL) {
		debug("Failed to find device %04" PRIx16 ":%04" PRIx16 ".\n",
				vendor, product);
		goto out;
	}

	err = libusb_open(dev, &handle);

	if (err < 0) {
		debug("libusb_open() failed: (%d) %s\n",
			err, libusb_error_name(err));
		goto out;
	}

out:
	/* 1 for unrefing the devices in the list. */
	libusb_free_device_list(devs, 1);

	return handle;
}

int
RATDriver_init(RATDriver *rat, uint64_t product, uint64_t vendor)
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

	if (rat->usb_handle == NULL)
		return -ENODEV;

	err = libusb_kernel_driver_active(rat->usb_handle, 0);

	if (err < 0 && err != LIBUSB_ERROR_NOT_SUPPORTED) {
		debug("libusb_kernel_driver_active() failed: (%d) %s\n",
			err, libusb_error_name(err));
		goto out_close;
	}
	else if (err == 1) {
		/* Return of 1 means a kernel driver is attached. */
		err = libusb_detach_kernel_driver(rat->usb_handle, 0);

		if (err < 0) {
			ret = -EIO;
			debug("libusb_detatch_kernel_driver() failed: (%d) %s\n",
				err, libusb_error_name(err));
			goto out_close;
		}
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
RATDriver_interpret_data_default(RATDriver *rat,
	char *buffer, size_t buffer_len)
{
	int16_t x, y;

	if (buffer == NULL || buffer_len != RAT_DATA_LEN)
		return 1;

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


	ret = libusb_interrupt_transfer(rat->usb_handle, RAT_INTR_IN,
				(unsigned char *)buffer, (int)sizeof(buffer),
				&transfered, 0);

	debug("Requested %zd, got %d\n",
			sizeof(buffer), transfered);

	if (transfered != sizeof(buffer))
		ret = LIBUSB_ERROR_IO;

	if (ret != 0) {
		debug("libusb_interrupt_transfer() failed: (%d) %s\n",
			ret, libusb_error_name(ret));
		return -EIO;
	}

	if (rat->interpret_data != NULL)
		return rat->interpret_data(rat, buffer, RAT_DATA_LEN);

	return RATDriver_interpret_data_default(rat, buffer, RAT_DATA_LEN);
}

static int
RATDriver_read_data_ctrl(RATDriver *rat,
	uint16_t val, uint16_t idx,
	unsigned char *buffer, uint16_t buffer_len,
	unsigned int timeout)
{
	int err;

	err = libusb_control_transfer(rat->usb_handle,
			RAT_CTRL_IN, RAT_CTRL_REQ_READ,
			val, idx,
			buffer, buffer_len,
			timeout);

	if (err < 0) {
		debug("libusb_control_transfer() failed: (%d) %s\n",
			err, libusb_error_name(err));
		return -EIO;
	}

	return 0;
}

int
RATDriver_get_dpi(RATDriver *rat, enum RATDPIMode mode,
	uint8_t *X_dpi, uint8_t *Y_dpi)
{
	int err;
	uint16_t X;
	uint16_t Y;
	unsigned char buffer[sizeof(uint16_t)];

	err = RATDriver_read_data_ctrl(rat,
			RAT_CTRL_RD_GET_DPI_MODE_VAL,
			RAT_CTRL_RD_GET_DPI_MODE_IDX((int)mode, RAT_DPI_X_AXIS),
			buffer, sizeof(buffer), 0);

	if (err < 0) {
		debug("Failed to get DPI for X axis for mode %d\n",
			(int)mode);
		return err;
	}

	/* XXX: Endianess */
	X = *(uint16_t *)buffer;

	err = RATDriver_read_data_ctrl(rat,
			RAT_CTRL_RD_GET_DPI_MODE_VAL,
			RAT_CTRL_RD_GET_DPI_MODE_IDX((int)mode, RAT_DPI_Y_AXIS),
			buffer, sizeof(buffer), 0);

	if (err < 0) {
		debug("Failed to get DPI for Y axis for mode %d\n",
			(int)mode);
		return err;
	}

	/* XXX: Endianess */
	Y = *(uint16_t *)buffer;

	/* Byte layout of return:
	 * 0xDDMA - DD = DPI ; M = mode ; A = Axis
	 *
	 * Since the mode and axis are known, they
	 * are useless information.
	 */

	*X_dpi = (uint8_t)((X >> 8) & 0xff);
	*Y_dpi = (uint8_t)((Y >> 8) & 0xff);

	return 0;
}

int
RATDriver_get_active_dpi_mode(RATDriver *rat, enum RATDPIMode *mode)
{
	int err;
	uint8_t imode;
	unsigned char buffer[sizeof(uint8_t)];

	err = RATDriver_read_data_ctrl(rat,
			RAT_CTRL_RD_GET_ACTIVE_DPI_MODE_VAL,
			RAT_CTRL_RD_GET_ACTIVE_DPI_MODE_IDX,
			buffer, sizeof(buffer), 0);

	if (err < 0) {
		debugln("Failed to get active DPI mode.");
		return err;
	}

	/* XXX: Endianess */
	imode = *(uint8_t *)buffer;

	switch (imode) {
	case 0x10:
		*mode = RAT_DPI_MODE_1;
		break;

	case 0x20:
		*mode = RAT_DPI_MODE_2;
		break;

	case 0x30:
		*mode = RAT_DPI_MODE_3;
		break;

	case 0x40:
		*mode = RAT_DPI_MODE_4;
		break;

	default:
		return -EBADMSG;
	}

	return 0;
}
