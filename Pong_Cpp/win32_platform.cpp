#include <windows.h>

void* buffer_memory; //void pointer to somewhere in memory, we don't care about the type
int buffer_width;
int buffer_height;
//bitmap info struct stores: what our pixel looks like,compresion.etc
BITMAPINFO buffer_bitmap_info;
bool running=true;
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
			buffer_width = rect.right - rect.left;
			buffer_height = rect.bottom - rect.top;

			//Buffer space for the window, height *width * size of pixel(32 bits, 8,8,8 for RGB)
			int buffer_size = buffer_width * buffer_height * sizeof(unsigned int);
			/*Every time we run our program, we are allocated memory, called stack. The remainder
			is called Heap. When we create a variable, that variable is inserted to the stack, and 
			deleted whenever it goes out of scope, the variable is popped from the stack.But in this
			case, we want our buffer to persist in the stack.We also need to free it and relocate it
			depending on the window size. In this case, we're gonna ask windows for a new memory from the heap.*/


			//what if the player change the size again? Our buffer already has allocated memory, but it maybe too big or too small
			//check if we already have some valid memory and free it before allocation new memory
			if (buffer_memory) VirtualFree(buffer_memory,0,MEM_RELEASE); //address, size, mem_release. We get black screen cuz this is set to 0

			//some people use malloc() for this, but since we're on the windows platform layer, we're using VirtualAlloc(0)
			buffer_memory = VirtualAlloc(0, //Adress we want to creat, in this case it doesn't matter
				buffer_size, //size
				MEM_COMMIT| MEM_RESERVE,//types
				PAGE_READWRITE); //we want to read and write to this page(memory?)
			buffer_bitmap_info.bmiHeader.biSize = sizeof(buffer_bitmap_info.bmiHeader);
			buffer_bitmap_info.bmiHeader.biWidth = buffer_width;
			buffer_bitmap_info.bmiHeader.biHeight = buffer_height;
			buffer_bitmap_info.bmiHeader.biPlanes = 1; //No idea why it has to be 1
			buffer_bitmap_info.bmiHeader.biBitCount = 32;// 32 for our RGB
			buffer_bitmap_info.bmiHeader.biCompression = BI_RGB;//Compression type
			

		}break;

		default: {
			result = DefWindowProc(hwnd, uMsg, wParam, lParam); // Keep the window running
		}
	}
	//processing everything and returning them
	return result;
}

int WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{
	//Create a window class
	WNDCLASS window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW; // makes sure we redraw the window horizontally and vertically whenever we need to
	window_class.lpszClassName = L"Game Window Class"; //The 'L' is doing a conversion to do with Unicode
	window_class.lpfnWndProc = window_callback; //Window callback function- the way windows use to pass messages down to us
	
	//register it
	RegisterClass(&window_class);
	
	//create it
	HWND window =CreateWindow(
		window_class.lpszClassName, // the name of the window class
		L"Pong!", // Title that would  be displayed, again we're doing that unicode conversion
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, // stylizing the window to make it overlapped and visible
		CW_USEDEFAULT, CW_USEDEFAULT, // x and y values, using default so that windows will use what it thinks is best
		1280, 720,//Width and height
		0, 0,//window parent and menu -  we don't really care rn
		hInstance, //The instance variable we recieved at the entry point
		0//we don't care
		);
	//Device Context: Reference which windows use to draw to it
	HDC hdc = GetDC(window);
	while (running) {
		//Input
		//Ask windows if there was any message for us
		MSG message;
		//take all the messages we didn't read so far and give us a chance to process it
		while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {
			//dispatch that message to our callback
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		//Simulate
		//create a current pixel pointer
		unsigned int* pixel = (unsigned int*)buffer_memory;
		//iterate through the height and widith of the pixels
		for (int x = 0; x < buffer_height; x++)
		{
			for (int y = 0; y < buffer_width; y++) 
			{
				//change the colour of every pixel to a bice poral(dereferencing here) 
				//and increment it to point to the next pixel
				*pixel++= 0xff00ff; //set this to x*y for a cool effect
				//400 + y + 23 * y * 89 * x - y - 12 is a nice effect
			}
		}

		//Render
		//now that we have the memory, we're gonna send to windows and ask to use it.
		StretchDIBits( hdc,//Handle to our device context
			0,0, // x and y dest
			buffer_width,buffer_height, //width and height
			0,0, buffer_width, buffer_height,//source x,source y,source width,source height
			buffer_memory, //memory that we're gonna use 
			&buffer_bitmap_info,//pointer to our bitmap info struct
			DIB_RGB_COLORS, //usage will be DIB RGB colours
			SRCCOPY //operation will be via source copy
		);
	}
}