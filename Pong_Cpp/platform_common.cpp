/*
This file is excluded from the build
*/
struct Button_State {
	bool isDown;
	bool changed;
};

enum {
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_W,
	BUTTON_S,
	BUTTON_ENTER,

	BUTTON_COUNT,// SHould be the last item
};

struct Input {
	Button_State  buttons[BUTTON_COUNT];// change this every time was add a new button
};