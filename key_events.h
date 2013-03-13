#ifndef _H_KEY_EVENTS
#define _H_KEY_EVENTS

#include <X11/Xlib.h>

extern void send_key(Display *display, KeySym keysym, int state);
extern void send_key_typed(Display *display, char *key, int state);

#endif /* _H_KEY_EVENTS */
