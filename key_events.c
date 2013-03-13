#include "key_events.h"

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

static void
send_key_backend(Display *display, KeyCode keycode, int modmask, int state)
{
	KeyCode kc;
	XTestGrabControl(display, True);

	if (state) {
		if (modmask & MASK_SHIFT) {
			kc = XKeysymToKeycode(display, XK_Shift_L);
			XTestFakeKeyEvent(display, kc, True, 0);
		}
		
		if (modmask & MASK_CONTROL) {
			kc = XKeysymToKeycode(display, XK_Control_L);
			XTestFakeKeyEvent(display, kc, True, 0);
		}
		
		if (modmask & MASK_LOCK) {
			kc = XKeysymToKeycode(display, XK_Caps_Lock);
			XTestFakeKeyEvent(display, kc, True, 0);
		}
		
		if (modmask & MASK_ALT) {
			kc = XKeysymToKeycode(display, XK_Alt_L);
			XTestFakeKeyEvent(display, kc, True, 0);
		}
		
		if (modmask & MASK_NUMLOCK) {
			kc = XKeysymToKeycode(display, XK_Num_Lock);
			XTestFakeKeyEvent(display, kc, True, 0);
		}
	
		if (modmask & MASK_SUPER) {
			kc = XKeysymToKeycode(display, XK_Super_L);
			XTestFakeKeyEvent(display, kc, True, 0);
		}

		XTestFakeKeyEvent(display, keycode, True, 0);
	}
	else {
		XTestFakeKeyEvent(display, keycode, False, 0);
	
		if (modmask & MASK_SHIFT) {
			kc = XKeysymToKeycode(display, XK_Shift_L);
			XTestFakeKeyEvent(display, kc, False, 0);
		}
		
		if (modmask & MASK_CONTROL) {
			kc = XKeysymToKeycode(display, XK_Control_L);
			XTestFakeKeyEvent(display, kc, False, 0);
		}
		
		if (modmask & MASK_LOCK) {
			kc = XKeysymToKeycode(display, XK_Caps_Lock);
			XTestFakeKeyEvent(display, kc, False, 0);
		}
		
		if (modmask & MASK_ALT) {
			kc = XKeysymToKeycode(display, XK_Alt_L);
			XTestFakeKeyEvent(display, kc, False, 0);
		}
		
		if (modmask & MASK_NUMLOCK) {
			kc = XKeysymToKeycode(display, XK_Num_Lock);
			XTestFakeKeyEvent(display, kc, False, 0);
		}
		
		if (modmask & MASK_SUPER) {
			kc = XKeysymToKeycode(display, XK_Super_L);
			XTestFakeKeyEvent(display, kc, False, 0);
		}
	}
	
	XSync(display, False);
	XTestGrabControl(display, False);
}

void
send_key(Display *display, KeySym keysym, int modmask, int state)
{
	KeyCode kc;

	kc = XKeysymToKeycode(display, keysym);
	if (kc == 0)
		return;
	
	send_key_backend(display, kc, modmask, state);
}

void
send_key_typed(Display *display, char *keyname, int modmask, int state)
{
	KeySym keysym;

	keysym = XStringToKeysym(keyname);
	if (keysym == 0)
		return;

	send_key(display, keysym, modmask, state);
}

