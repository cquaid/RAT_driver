#ifndef H_RAT_DRIVER
#define H_RAT_DRIVER

#include <stdbool.h>
#include <stdint.h>

#include "uinput.h"

/* Defined by the EndPoint, for the RAT7 and Albino7, this is correct. */
#define RAT_DATA_LEN  (7)

enum RATProfile {
	RAT_PROFILE_1 = 0x01,
	RAT_PROFILE_2 = 0x02,
	RAT_PROFILE_3 = 0x04
};

enum RATDPIMode {
	RAT_DPI_MODE_1 = 0x01,
	RAT_DPI_MODE_2 = 0x02,
	RAT_DPI_MODE_3 = 0x03,
	RAT_DPI_MODE_4 = 0x04
};

enum RATButtonValue {
	RAT_BV_LEFT         = BTN_LEFT,
	RAT_BV_RIGHT        = BTN_RIGHT,
	RAT_BV_MIDDLE       = BTN_MIDDLE,
	RAT_BV_SIDE_FRONT   = BTN_SIDE,
	RAT_BV_SIDE_BACK    = BTN_EXTRA,
	RAT_BV_SNIPE        = 77,
	RAT_BV_SCROLL_UP    = BTN_GEAR_UP,
	RAT_BV_SCROLL_DOWN  = BTN_GEAR_DOWN,
	RAT_BV_SCROLL_LEFT  = BTN_FORWARD,
	RAT_BV_SCROLL_RIGHT = BTN_BACK,
	RAT_BV_CENTER       = 88,
	RAT_BV_SCROLL       = 99
};

struct rat_driver;
typedef struct rat_driver RATDriver;

typedef void(*rat_profile_callback)(RATDriver *, enum RATButtonValue, int);
typedef int(*rat_interpret_data_callback)(RATDriver *,
			char *buffer, size_t buffer_len);

struct rat_driver {
	struct libusb_device_handle *usb_handle;

	struct uinput *uinput;

	bool killme;

	int profile;
	int dpi_mode;

	uint64_t product_id;
	uint64_t vendor_id;

	rat_profile_callback profile1;
	rat_profile_callback profile2;
	rat_profile_callback profile3;

	rat_interpret_data_callback interpret_data;
};

int rat_driver_init(void);
void rat_driver_fini(void);

int RATDriver_init(RATDriver *rat, uint64_t product, uint64_t vendor);
int RATDriver_fini(RATDriver *rat);

void RATDriver_set_profile(RATDriver *rat, int profile,
		rat_profile_callback pc);



void RATDriver_mouse_click(RATDriver *rat, int button);
void RATDriver_mouse_release(RATDriver *rat, int button);
void RATDriver_mouse_scroll(RATDriver *rat, int button);

void RATDriver_handle_event(RATDriver *rat,
		enum RATButtonValue button,  int value);
void RATDriver_mouse_move_rel(RATDriver *rat, int x, int y);

void RATDriver_handle_profile_default(RATDriver *rat,
	enum RATButtonValue button, int val);

int RATDriver_interpret_data_default(RATDriver *rat,
		char *buffer, size_t buffer_len);

int RATDriver_read_data(RATDriver *rat);

int RATDriver_get_dpi(RATDriver *rat, enum RATDPIMode mode,
	uint8_t *X_dpi, uint8_t *Y_dpi);

int RATDriver_get_active_dpi_mode(RATDriver *rat, enum RATDPIMode *mode);

#endif /* H_RAT_DRIVER */
