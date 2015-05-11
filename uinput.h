#ifndef _H_UINPUT_RAT
#define _H_UINPUT_RAT

#ifndef UINPUT_PATH
# define UINPUT_PATH "/dev/uinput"
#endif

struct uinput {
	int fd;
};

extern int uinput_init(struct uinput *u);
extern int uinput_fini(struct uinput *u);

extern int uinput_push_key(struct uinput *u, int key, int state);
extern int uinput_push_key_end(struct uinput *u);

extern int uinput_send_key_press(struct uinput *u, int key);
extern int uinput_send_key_release(struct uinput *u, int key);
extern int uinput_send_key_repeate(struct uinput *u, int key);

extern int uinput_send_button_click(struct uinput *u, int btn);
extern int uinput_send_button_press(struct uinput *u, int btn);
extern int uinput_send_button_release(struct uinput *u, int btn);
extern int uinput_send_button_repeat(struct uinput *u, int btn);

extern int uinput_send_mouse_scroll(struct uinput *u, int val);
extern int uinput_send_mouse_x(struct uinput *u, int x);
extern int uinput_send_mouse_y(struct uinput *u, int y);
extern int uinput_send_mouse_move(struct uinput *u, int x, int y);

extern int uinput_flush(struct uinput *u);

#endif /* _H_UINPUT_RAT */
