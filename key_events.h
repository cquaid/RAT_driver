#ifndef _H_KEY_EVENTS
#define _H_KEY_EVENTS

#include <X11/Xlib.h>
#include <X11/keysym.h>

enum ModMask {
	MASK_SHIFT   = ShiftMask,   /* Shift_L, Shift_R */
	MASK_CONTROL = ControlMask, /* Control_L */
	MASK_LOCK    = LockMask,    /* Caps_Lock */ 
	MASK_ALT     = Mod1Mask,    /* Alt_L, Alt_R, Meta_L */
	MASK_NUMLOCK = Mod2Mask,    /* Num_Lock */
	MASK_MOD3    = Mod3Mask,    /* (None) */
	MASK_SUPER   = Mod4Mask,    /* Super_L, Super_R, Hyper_L, Hyper_R */
	MASK_MOD5    = Mod5Mask,    /* ISO_Level3_Shift Mode_Switch */
};

extern void send_key(Display *display, KeySym keysym, int modmask, int state);
extern void send_key_typed(Display *display, char *key, int modmask, int state);

#endif /* _H_KEY_EVENTS */
