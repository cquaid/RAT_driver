#ifndef _H_UINPUT_RAT
#define _H_UINPUT_RAT

extern int uinput_init(void);
extern int uinput_fini(void);

extern int uinput_push_key(int key, int state);
extern int uinput_push_key_end(void);

extern int uinput_send_key_press(int key);
extern int uinput_send_key_release(int key);
extern int uinput_send_key_repeate(int key);

extern int uinput_send_button_press(int btn);
extern int uinput_send_button_release(int btn);
extern int uinput_send_button_repeat(int btn);

extern int uinput_send_mouse_x(int x);
extern int uinput_send_mouse_y(int y);
extern int uinput_send_mouse_move(int x, int y);

extern int uinput_flush(void);

#endif /* _H_UINPUT_RAT */
