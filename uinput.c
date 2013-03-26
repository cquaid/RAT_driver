#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <linux/input.h>
#include <linux/uinput.h>

#include <sys/ioctl.h>

#include "uinput.h"

static int uinput_fd = 0;

/*
EV_KEY key press (check usr/include/linux/input.h)
EV_REL rel mov
EV_ABS abs mov

*/


static int
uinput_open(void)
{
	/* if it's initted... just keep it open */
	if (uinput_fd)
		return 0;

	uinput_fd = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
	if (uinput_fd < 0)
		return errno;
}

static inline int
init_rel(void)
{		
	return ioctl(uinput_fd, UI_SET_EVBIT, EV_REL);
}

static inline int
init_keys(void)
{
	int ret;

	/* turn on key and button events */
	ret = ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY);
	if (ret)
		return 1;

	/* turn on various buttons */
#define btn(x) do { ret = ioctl(uinput_fd, UI_SET_KEYBIT, BTN_##x); \
		    if (ret) return 1; } while(0)
	btn(0);
	btn(1);
	btn(2);
	btn(3);
	btn(4);
	btn(5);
	btn(6);
	btn(7);
	btn(8);
	btn(9);

	btn(LEFT);
	btn(RIGHT);
	btn(MIDDLE);
	btn(SIDE);
	btn(EXTRA);
	btn(FORWARD);
	btn(BACK);
	btn(GEAR_DOWN); /* wheel down */
	btn(GEAR_UP); /* wheel up */
#undef btn
#define key(x) do { ret = ioctl(uinput_fd, UI_SET_KEYBIT, KEY_##x); \
		    if (ret) return 1; } while(0)
	key(ESC);
	key(1);
	key(2);
	key(3);
	key(4);
	key(5);
	key(6);
	key(7);
	key(8);
	key(9);
	key(0);
	key(MINUS);
	key(EQUAL);
	key(BACKSPACE);
	key(TAB);
	key(Q);
	key(W);
	key(E);
	key(R);
	key(T);
	key(Y);
	key(U);
	key(I);
	key(O);
	key(P);
	key(LEFTBRACE);
	key(RIGHTBRACE);
	key(ENTER);
	key(LEFTCTRL);
	key(A);
	key(S);
	key(D);
	key(F);
	key(G);
	key(H);
	key(J);
	key(K);
	key(L);
	key(SEMICOLON);
	key(APOSTROPHE);
	key(GRAVE);
	key(LEFTSHIFT);
	key(BACKSLASH);
	key(Z);
	key(X);
	key(C);
	key(V);
	key(B);
	key(N);
	key(M);
	key(COMMA);
	key(DOT);
	key(SLASH);
	key(RIGHTSHIFT);
	key(KPASTERISK);
	key(LEFTALT);
	key(SPACE);
	key(CAPSLOCK);
	key(F1);
	key(F2);
	key(F3);
	key(F4);
	key(F5);
	key(F6);
	key(F7);
	key(F8);
	key(F9);
	key(F10);
	key(NUMLOCK);
	key(SCROLLLOCK);
	key(KP7);
	key(KP8);
	key(KP9);
	key(KPMINUS);
	key(KP4);
	key(KP5);
	key(KP6);
	key(KPPLUS);
	key(KP1);
	key(KP2);
	key(KP3);
	key(KP0);
	key(KPDOT);
	key(F11);
	key(F12);
	key(KPJPCOMMA);
	key(KPENTER);
	key(RIGHTCTRL);
	key(KPSLASH);
	key(SYSRQ);
	key(RIGHTALT);
	key(LINEFEED);
	key(HOME);
	key(UP);
	key(PAGEUP);
	key(LEFT);
	key(RIGHT);
	key(END);
	key(DOWN);
	key(PAGEDOWN);
	key(INSERT);
	key(DELETE);
	key(KPEQUAL);
	key(KPPLUSMINUS);
#undef key

	return 0;
}

int
uinput_init(void)
{
	if (uinput_open())
		return 1;

	if (init_keys())
		return 1;

	if (init_rel())
		return 1;

}

