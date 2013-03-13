#include "key_events.h"

#include <X11/Xlib.h>
#include <unistd.h>

static XKeyEvent
build_event(Display *display)
{
	int rev;
	Window root;
	Window focus;
	XKeyEvent ret;

	root = XDefaultRootWindow(display);
	
	/* get window with current keyboard focus */
	XGetInputFocus(display, &focus, &rev);

	ret.display     = display;
	ret.window      = focus;
	ret.root        = root;
	ret.subwindow   = None;
	ret.time        = CurrentTime;
	ret.x           = 1;
	ret.y           = 1;
	ret.x_root      = 1;
	ret.y_root      = 1;
	ret.same_screen = True;

	return ret;
}

static void
send_key_backend(Display *display, KeyCode keycode, int modmask, int state)
{
	XKeyEvent event;

	event = build_event(display);

	if (state)
		event.type = KeyPress;
	else
		event.type = KeyRelease;

	event.keycode = keycode;
	event.state = modmask;

	XSendEvent(display, event.window, True, KeyPressMask, (XEvent *)&event); 

//	XSync(display, False);

	usleep(1);
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

