#include "utility.cpp"

internal void render_background() {
	u32* pixel = (u32*)render_state.memory;
	for (int y = 0; y < render_state.height; y++) {
		for (int x = 0; x < render_state.width; x++) {
			*pixel++ = 0xfff123;
		}
	}
}

internal void clear_screen(u32 colour) {
	u32* pixel = (u32*)render_state.memory;
	for (int y = 0; y < render_state.height; y++) {
		for (int x = 0; x < render_state.width; x++) {
			*pixel++ = colour;
		}
	}
}

/* Updates the pixel values in memory before rendering to the screen. */
internal void draw_rect_in_pixels(int x0, int y0, int x1, int y1, u32 colour) {

	x0 = clamp(0, x0, render_state.width);
	x1 = clamp(0, x1, render_state.width);
	y0 = clamp(0, y0, render_state.height);
	y1 = clamp(0, y1, render_state.height);

	for (int y = y0; y < y1; y++) {
		u32* pixel = (u32*)render_state.memory + x0 + y * render_state.width;
		for (int x = x0; x < x1; x++) {
			*pixel++ = colour;
		}
	}
}

global_variable float render_scale = 0.01f;

/* Handles the conversion of coordinates to centered pixel coordinates. */
internal void draw_rect(float x, float y, float half_size_x, float half_size_y, u32 colour) {

	x *= render_state.height * render_scale;
	y *= render_state.height * render_scale;
	half_size_x *= render_state.height * render_scale;
	half_size_y *= render_state.height * render_scale;

	x += render_state.width / 2.f;
	y += render_state.height / 2.f;

	int x0 = x - half_size_x;
	int x1 = x + half_size_x;
	int y0 = y - half_size_y;
	int y1 = y + half_size_y;

	draw_rect_in_pixels(x0, y0, x1, y1, colour);
}

/* Draws a sprite based on a given array of 0s. */
internal void draw_sprite(const char** sprite, int rows, float x, float y, float size, u32 colour) {
	float half_size = size * .5f;
	float original_x = x;

	for (int i = 0; i < rows; i++) {
		const char* row = sprite[i];
		while (*row) {
			if (*row == '0') {
				draw_rect(x, y, half_size, half_size, colour);
			}
			x += size;
			row++;
		}
		y -= size;
		x = original_x;
	}
}