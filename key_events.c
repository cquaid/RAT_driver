#include "key_events.h"

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

static void
send_key_backend(Display *display, KeyCode kc, int state)
{
	XTestGrabControl(display, True);

	if (state)
		XTestFakeKeyEvent(display, kc, True, 0);
	else
		XTestFakeKeyEvent(display, kc, False, 0);
	
	XSync(display, False);
	XTestGrabControl(display, False);
}

void
send_key(Display *display, KeySym keysym, int state)
{
	KeyCode kc;

	kc = XKeysymToKeycode(display, keysym);
	if (kc == 0)
		return;
	
	send_key_backend(display, kc, state);
}

void
send_key_typed(Display *display, char *keyname, int state)
{
	KeySym keysym;

	keysym = XStringToKeysym(keyname);
	if (keysym == 0)
		return;

	send_key(display, keysym, state);
}

