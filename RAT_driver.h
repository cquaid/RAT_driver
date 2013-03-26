#ifndef _H_RAT_DRIVER
#define _H_RAT_DRIVER

#include <usb.h>

#include <linux/input.h>

/* Saitek R.A.T. 7 Albino ID */
#ifndef PRODUCT_ID
# ifdef ALBINO7
#  define PRODUCT_ID (0x0cce)
# elif defined(RAT7)
#  define PRODUCT_ID (0x0ccb)
# else /* regular R.A.T. 7 */
#  define PRODUCT_ID (0x0ccb)
# endif
#endif
#ifndef VENDOR_ID
# define VENDOR_ID (0x06a3)
#endif
#define DATA_SIZE  (7)

enum Profile {
	PROFILE_1 = 0x01,
	PROFILE_2 = 0x02,
	PROFILE_3 = 0x04
};

enum ButtonValue {
	BV_LEFT         = BTN_LEFT,
	BV_RIGHT        = BTN_RIGHT,
	BV_MIDDLE       = BTN_MIDDLE,
	BV_SIDEF        = BTN_SIDE,
	BV_SIDEB        = BTN_EXTRA,
	BV_SNIPE        = 77,
	BV_SCROLL_UP    = BTN_GEAR_UP,
	BV_SCROLL_DOWN  = BTN_GEAR_DOWN,
	BV_SCROLL_LEFT  = BTN_FORWARD,
	BV_SCROLL_RIGHT = BTN_BACK,
	BV_CENTER       = 88,
	BV_SCROLL       = 99
};

typedef void(*profile_callback)(enum ButtonValue, int);

extern int profile;
extern int killme;

extern struct usb_device* grab_device(void);

extern void mouse_click(int button);
extern void mouse_release(int button);
extern void mouse_scroll(int button);

extern void handle_event(enum ButtonValue button,  int value);
extern void move_mouse_rel(int x, int y);

extern void set_profile1_callback(profile_callback pc);
extern void set_profile2_callback(profile_callback pc);
extern void set_profile3_callback(profile_callback pc);

extern void handle_profile_default(enum ButtonValue button, int value);

#endif /* _H_RAT_DRIVER */
