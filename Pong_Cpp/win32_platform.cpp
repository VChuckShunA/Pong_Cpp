#include "utils.cpp"
#include <windows.h>
struct Render_State {
	int height, width;
	void* memory; //void pointer to somewhere in memory, we don't care about the type
	
	//bitmap info struct stores: what our pixel looks like,compresion.etc
	BITMAPINFO bitmap_info;
};

Render_State render_state;
#include "renderer.cpp" //make sure this is called after the render state
#include "platform_common.cpp"
#include "game.cpp"
global_variable bool running=true;
LRESULT CALLBACK window_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT result=0; // pass it whatever windows wants
	//Switch on the Message
	switch (uMsg)
	{
	//Window Close or Window Destroy Message
		case WM_CLOSE:
		case WM_DESTROY: {
			running = false;//set running to false, the game will quit
		}break;

		case WM_SIZE: {
			//This function takes a window, and a pointer to a rect struct that contains the left,right, and bottom values
			RECT rect;
			GetClientRect(hwnd, &rect);
			//calculating the width and the height
			render_state.width = rect.right - rect.left;
			render_state.height = rect.bottom - rect.top;

			//Buffer space for the window, height *width * size of pixel(32 bits, 8,8,8 for RGB)
			int size = render_state.width * render_state.height * sizeof(u32);
			/*Every time we run our program, we are allocated memory, called stack. The remainder
			is called Heap. When we create a variable, that variable is inserted to the stack, and 
			deleted whenever it goes out of scope, the variable is popped from the stack.But in this
			case, we want our buffer to persist in the stack.We also need to free it and relocate it
			depending on the window size. In this case, we're gonna ask windows for a new memory from the heap.*/


			//what if the player change the size again? Our buffer already has allocated memory, but it maybe too big or too small
			//check if we already have some valid memory and free it before allocation new memory
			if (render_state.memory) VirtualFree(render_state.memory,0,MEM_RELEASE); //address, size, mem_release. We get black screen cuz this is set to 0

			//some people use malloc() for this, but since we're on the windows platform layer, we're using VirtualAlloc(0)
			render_state.memory = VirtualAlloc(0, //Adress we want to creat, in this case it doesn't matter
				size, //size
				MEM_COMMIT| MEM_RESERVE,//types
				PAGE_READWRITE); //we want to read and write to this page(memory?)
			render_state.bitmap_info.bmiHeader.biSize = sizeof(render_state.bitmap_info.bmiHeader);
			render_state.bitmap_info.bmiHeader.biWidth = render_state.width;
			render_state.bitmap_info.bmiHeader.biHeight = render_state.height;
			render_state.bitmap_info.bmiHeader.biPlanes = 1; //No idea why it has to be 1
			render_state.bitmap_info.bmiHeader.biBitCount = 32;// 32 for our RGB
			render_state.bitmap_info.bmiHeader.biCompression = BI_RGB;//Compression type
			

		}break;

		default: {
			result = DefWindowProc(hwnd, uMsg, wParam, lParam); // Keep the window running
		}
	}
	//processing everything and returning them
	return result;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	//Hide cursor
	ShowCursor(FALSE);
	//Create a window class
	WNDCLASS window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW; // makes sure we redraw the window horizontally and vertically whenever we need to
	window_class.lpszClassName = L"Game Window Class"; //The 'L' is doing a conversion to do with Unicode
	window_class.lpfnWndProc = window_callback; //Window callback function- the way windows use to pass messages down to us

	//register it
	RegisterClass(&window_class);

	//create it
	HWND window = CreateWindow(
		window_class.lpszClassName, // the name of the window class
		L"Pong!", // Title that would  be displayed, again we're doing that unicode conversion
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, // stylizing the window to make it overlapped and visible
		CW_USEDEFAULT, CW_USEDEFAULT, // x and y values, using default so that windows will use what it thinks is best
		1280, 720,//Width and height
		0, 0,//window parent and menu -  we don't really care rn
		hInstance, //The instance variable we recieved at the entry point
		0//we don't care
	);
	{
		//Setting the window to FullScreen
		SetWindowLong(window, GWL_STYLE, GetWindowLong(window, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW);
		MONITORINFO mi = { sizeof(mi) }; //player's monitor info
		GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &mi);
		//Setting the Window to be on top with the monitor size as the new width and height
		SetWindowPos(window, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, 
		mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
	//Device Context: Reference which windows use to draw to it
	HDC hdc = GetDC(window);

	//Input struct
	Input input = {};

	//Delta time and related variables to accomdate for resolution/fps/refresh rate
	float delta_time = 0.016666f;//60fps in miliseconds
	LARGE_INTEGER frame_begin_time;
	QueryPerformanceCounter(&frame_begin_time); //No idea what this is doing

	//This function returns how many cycles you can run in one second
	float performance_frequency; {
		LARGE_INTEGER perf;
		QueryPerformanceFrequency(&perf);
		performance_frequency = (float)perf.QuadPart;
	}
	while (running) {
		//Input
		//Ask windows if there was any message for us
		MSG message;

		//reset the change in the begining of every frame
		//we only want the change to last one frame
		for (int i = 0; i < BUTTON_COUNT; i++) {
			//setting the change to false for every button
			input.buttons[i].changed = false;
		}

		//take all the messages we didn't read so far and give us a chance to process it
		while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {
			//checking our messages
			switch (message.message) {
				case WM_KEYUP: {
				}

				case WM_KEYDOWN: {//if keys are arepressed down
				u32 vk_code = (u32)message.wParam;//what key was pressed
				//key up or key down event?
				bool is_down = ((message.lParam & (1 << 31)) == 0); //extract it from the message struct. No idea what this is doing

/*Defining another macro to avoid retyping everything*/
//setting is_down to true only if the current is_down is different from the recorded is_down
#define process_button(b,vk)\
case vk: {\
input.buttons[b].changed = is_down!=input.buttons[b].isDown;\
input.buttons[b].isDown = is_down; \
}break;
					switch (vk_code) {//depending on the key
						process_button(BUTTON_UP, VK_UP);
						process_button(BUTTON_DOWN, VK_DOWN);
						process_button(BUTTON_LEFT, VK_LEFT);
						process_button(BUTTON_RIGHT, VK_RIGHT);
						process_button(BUTTON_W, 'W');
						process_button(BUTTON_S, 'S');
						process_button(BUTTON_ENTER, VK_RETURN);
					}
				}break;

			default: {
				//dispatch that message to our callback
				TranslateMessage(&message);
				DispatchMessage(&message);
				}
			}
		}
		//Simulate
		simulate_game(&input,delta_time);


		//Render
		//now that we have the memory, we're gonna send to windows and ask to use it.
		StretchDIBits(hdc,//Handle to our device context
			0, 0, // x and y dest
			render_state.width, render_state.height, //width and height
			0, 0, render_state.width, render_state.height,//source x,source y,source width,source height
			render_state.memory, //memory that we're gonna use 
			&render_state.bitmap_info,//pointer to our bitmap info struct
			DIB_RGB_COLORS, //usage will be DIB RGB colours
			SRCCOPY //operation will be via source copy
		);

		//End of frame
		LARGE_INTEGER frame_end_time;
		QueryPerformanceCounter(&frame_end_time);
		delta_time = (float)(frame_end_time.QuadPart - frame_begin_time.QuadPart)/performance_frequency;
		frame_begin_time = frame_end_time;
	}
}