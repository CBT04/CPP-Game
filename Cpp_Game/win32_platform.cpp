#include <windows.h>

bool running = true;

struct Render_State {
	int height, width;
	void* memory;

	BITMAPINFO bitmap_info;
};

Render_State render_state;

#include "platform_common.cpp"
#include "render_file.cpp"
#include "game.cpp"

/* Callback function used to return the operation based on the message sent by the window. */
LRESULT CALLBACK window_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	LRESULT result = 0;

	switch (uMsg) {
		// Special case for closing the window.
		case WM_CLOSE:
		case WM_DESTROY: {
			running = false;
		} break;
		
		// Case which returns the buffer size so that the correct amount of memory is reserved
		// to render the graphics when the window size is changed by the user.
		case WM_SIZE: {
			RECT rect;
			GetClientRect(hwnd, &rect);
			render_state.width = rect.right - rect.left;
			render_state.height = rect.bottom - rect.top;

			// Size is the number of pixels multiplied by the number of bytes per pixel.
			int size = render_state.width * render_state.height * sizeof(unsigned int);

			if (render_state.memory) VirtualFree(render_state.memory, 0, MEM_RELEASE);
			render_state.memory = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			render_state.bitmap_info.bmiHeader.biSize = sizeof(render_state.bitmap_info.bmiHeader);
			render_state.bitmap_info.bmiHeader.biWidth = render_state.width;
			render_state.bitmap_info.bmiHeader.biHeight = render_state.height;
			render_state.bitmap_info.bmiHeader.biPlanes = 1;
			render_state.bitmap_info.bmiHeader.biBitCount = 32;
			render_state.bitmap_info.bmiHeader.biCompression = BI_RGB;

		} break;

		default: {
			result = DefWindowProc(hwnd, uMsg, wParam, lParam);
		}

	}
	return result;
}

/* Windows API is used for access to windows features. This function uses the API to create a 
window and send messages to the callback function so that window features such as minimise and 
quit can be processed. The game logic is also contained in this function. */
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	// Create Window Class
	WNDCLASS window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpszClassName = L"Game Window Class";
	window_class.lpfnWndProc = window_callback;

	// Register Class
	RegisterClass(&window_class);

	// Create Window
	HWND window = CreateWindow(window_class.lpszClassName, L"New Game", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, 0, 0, hInstance, 0);
	HDC hdc = GetDC(window);

	Input input = {};
	int y = 0;
	int x = 0;

	float delta_time = 0.016666f;
	LARGE_INTEGER frame_begin_time;
	QueryPerformanceCounter(&frame_begin_time);

	float performance_frequency;
	{
		LARGE_INTEGER perf;
		QueryPerformanceFrequency(&perf);
		performance_frequency = (float)perf.QuadPart;
	}

	init_platforms();

	while (running) {
		// Input
		MSG message;

		for (int i = 0; i < BUTTON_COUNT; i++) {
			input.buttons[i].changed = false;
		}
		while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {
			switch (message.message) {
				case WM_KEYUP:
				case WM_KEYDOWN: {
					u32 vk_code = (u32)message.wParam;
					bool is_down = ((message.lParam & (1 << 31)) == 0);

					#define process_button(b, vk)\
					case vk: {\
						input.buttons[b].is_down = is_down;\
						input.buttons[b].changed = true;\
					} break;

					switch (vk_code) {
						process_button(BUTTON_JUMP, 'W');
						process_button(BUTTON_JUMP, VK_SPACE);
						process_button(BUTTON_LEFT, 'A');
						process_button(BUTTON_RIGHT, 'D');
						process_button(BUTTON_DASH, 'E');
						process_button(BUTTON_RESET, 'R');
						process_button(BUTTON_STOMP, 'S');
					}
				} break;

		        default: {
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
			}
		}
		// Simulate
		movement_simulation(&input, delta_time);

		// Render
		StretchDIBits(hdc, 0, 0, render_state.width, render_state.height, 0, 0, render_state.width, render_state.height, render_state.memory, &render_state.bitmap_info, DIB_RGB_COLORS, SRCCOPY);

		LARGE_INTEGER frame_end_time;
		QueryPerformanceCounter(&frame_end_time);

		// Delta time is used to process movement independently of frame rate.
		delta_time = (float)(frame_end_time.QuadPart - frame_begin_time.QuadPart) / performance_frequency;
		if (delta_time > .02f) {
			delta_time = 0.016666f;
		}
		frame_begin_time = frame_end_time;
	}
}

