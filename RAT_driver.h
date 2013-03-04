#ifndef _H_RAT_DRIVER
#define _H_RAT_DRIVER

#include <usb.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

/* Saitek R.A.T. 7 Albino ID */
#ifdef ALBINO7
# define PRODUCT_ID (0x0cce)
#else /* regular R.A.T. 7 */
# define PRODUCT_ID (0x0ccb)
#endif
#define VENDOR_ID (0x06a3)
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

typedef void(*profile_callback)(Display*, enum ButtonValue, int);

extern int profile;
extern int killme;

extern struct usb_device* grab_device(void);

extern void mouse_click(Display *display, int button);
extern void mouse_release(Display *display, int button);
extern void mouse_scroll(Display *display, int button);

extern void handle_event(Display *display, enum ButtonValue button,  int value);
extern void get_coords(Display *display, int *x, int *y);
extern void move_mouse_abs(Display *display, int x, int y);
extern void move_mouse_rel(Display *display, int x, int y);

extern void set_profile1_callback(profile_callback pc);
extern void set_profile2_callback(profile_callback pc);
extern void set_profile3_callback(profile_callback pc);

extern void handle_profile_default(Display *display, enum ButtonValue button, int value);

#endif /* _H_RAT_DRIVER */
