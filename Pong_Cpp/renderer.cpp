//This file is excluded from build
/* We don't need this fucntion anymore
internal void 
render_background() {
	//create a current pixel pointer
	u32* pixel = (u32*)render_state.memory;
	//iterate through the height and widith of the pixels
	for (int y = 0; y < render_state.height; y++)
	{
		for (int x = 0; x < render_state.width; x++)
		{
			//change the colour of every pixel to a bice poral(dereferencing here) 
			//and increment it to point to the next pixel
			*pixel++ = x * y; //set this to x*y for a cool effect
			//400 + y + 23 * y * 89 * x - y - 12 is a nice effect
		}
	}

}
*/


internal void 
clear_screen(u32 colour) {
	//create a current pixel pointer
	u32* pixel = (u32*)render_state.memory;
	//iterate through the height and widith of the pixels
	for (int y = 0; y < render_state.height; y++)
	{
		for (int x = 0; x < render_state.width; x++)
		{
			//change the colour of every pixel to a bice poral(dereferencing here) 
			//and increment it to point to the next pixel
			*pixel++ = colour; //set each pixel to the colour
			//400 + y + 23 * y * 89 * x - y - 12 is a nice effect
		}
	}
}
internal void 
draw_rect_in_pixels(int x0,int y0, int x1,int y1, u32 colour) {
	//clamping the x and the y to stop crashes
	x0 = clamp(0,x0,render_state.width);
	x1 = clamp(0, x1, render_state.width);
	y0 = clamp(0, y0, render_state.height);
	y1 = clamp(0, y1, render_state.height);
	//start with y0 ang go to y1
	for (int y = y0; y < y1; y++)
	{//create a current pixel pointer
		u32* pixel = (u32*)render_state.memory + x0 + y*render_state.width; //draw a pixel for every row
		for (int x = x0; x < x1; x++) //go from x0 to x1
		{
			//change the colour of every pixel to a bice poral(dereferencing here) 
			//and increment it to point to the next pixel
			*pixel++ = colour; //set each pixel to the colour
			//400 + y + 23 * y * 89 * x - y - 12 is a nice effect
		}
	}
}

//scale multiplier
global_variable float render_scale = 0.01f; //100 will be one full height

//This function is used to resize everything
internal void
draw_rect(float x, float y, float half_size_x, float half_size_y,u32 colour) {

	// player will see much of the world with this setting
	// but a low resolution would cause the player to lose vision
	x *= render_state.height * render_scale;
	y *= render_state.height * render_scale;
	half_size_x *= render_state.height * render_scale;
	half_size_y *= render_state.height *render_scale;

	//offsetting it to the centre
	x += render_state.width / 2.f;
	y += render_state.height / 2.f;


	//Change to pixels
	//generate the min and max points
	int x0 = x - half_size_x;
	int x1 = x + half_size_x;
	int y0 = y - half_size_y;
	int y1 = y + half_size_y;
	draw_rect_in_pixels(x0,y0,x1,y1, colour);
}