#ifndef H_RAT_DRIVER
#define H_RAT_DRIVER

#include <usb.h>

#include "uinput.h"

#define RAT_DATA_LEN  (9)

enum Profile {
	PROFILE_1 = 0x01,
	PROFILE_2 = 0x02,
	PROFILE_3 = 0x04
};

enum DPIMode {
	DPI_MODE_1 = 0x01,
	DPI_MODE_2 = 0x02,
	DPI_MODE_3 = 0x03,
	DPI_MODE_4 = 0x04
};

enum ButtonValue {
	BV_LEFT         = BTN_LEFT,
	BV_RIGHT        = BTN_RIGHT,
	BV_MIDDLE       = BTN_MIDDLE,
	BV_SIDE_FRONT   = BTN_SIDE,
	BV_SIDE_BACK    = BTN_EXTRA,
	BV_SNIPE        = 77,
	BV_SCROLL_UP    = BTN_GEAR_UP,
	BV_SCROLL_DOWN  = BTN_GEAR_DOWN,
	BV_SCROLL_LEFT  = BTN_FORWARD,
	BV_SCROLL_RIGHT = BTN_BACK,
	BV_CENTER       = 88,
	BV_SCROLL       = 99
};

struct rat_driver;
typedef struct rat_driver RATDriver;

typedef void(*profile_callback)(RATDriver *, enum ButtonValue, int);
typedef int(*interpret_data_callback)(RATDriver *, char buffer[RAT_DATA_LEN]);

struct rat_driver {
	struct usb_device *usb_dev;
	struct usb_dev_handle *usb_handle;

	struct uinput *uinput;

	int profile;
	int killme;
	int dpi_mode;
	int product_id;
	int vendor_id;

	profile_callback profile1;
	profile_callback profile2;
	profile_callback profile3;

	interpret_data_callback interpret_data;
};

int rat_driver_init(RATDriver *rat, int product, int vendor);
int rat_driver_fini(RATDriver *rat);

void rat_driver_set_profile(RATDriver *rat, int profile, profile_callback pc);



void rat_mouse_click(RATDriver *rat, int button);
void rat_mouse_release(RATDriver *rat, int button);
void rat_mouse_scroll(RATDriver *rat, int button);

void rat_handle_event(RATDriver *rat, enum ButtonValue button,  int value);
void rat_mouse_move_rel(RATDriver *rat, int x, int y);

void rat_handle_profile_default(RATDriver *rat,
	enum ButtonValue button, int val);

int rat_interpret_data_default(RATDriver *rat, char buffer[RAT_DATA_LEN]);

int rat_driver_read_data(RATDriver *rat);

#endif /* H_RAT_DRIVER */
