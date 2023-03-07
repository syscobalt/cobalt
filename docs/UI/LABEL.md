# Labels
When creating GUIs, you may want to display some text. This is where labels come in handy.
To create labels,  simply define them using their type, set their text & size, and them add the label to the window.
Here is a more detailed example:
```c
#include <dxui.h>

static dxui_content* context;
static dxui_label* label;

int main(void)
{
	atexit(shutdown);
	context = dxui_initialize(DXUI_INIT_CURSOR);
	if (!context) dxui_panic(NULL, "Failed to initialize dxui.");

	if (!createWindow()) {
		dxui_panic(context, "Failed to create about window.");
	}
	dxui_pump_events(context, DXUI_PUMP_WHILE_WINDOWS_EXIST, -1);
}

static bool createWindow(void) {
	dxui_rect rect = {{ -1, -1, 400, 200 }};
	dxui_window* window = dxui_create_window(context, rect, "Window with label", DXUI_WINDOW_NO_RESIZE);
	
	rect = (dxui_rect) {{ 10, 10, 390, 25 }};
	label = dxui_create_label(rect, "I am a label!");
	if (!label) return false;
	dxui_set_background(label, COLOR_WHITE);
	dxui_add_control(window, label);

	dxui_show(window);
	return true;
}

static void shutdown(void)
{
	dxui_shutdown(context);
}
```
Let me explain each line bit by bit. The third and fourth lines create the context object, and the label object. The first line in the main function sets the window close event function to the function named `shutdown`, which is the last function in the code. We then initialize the context, check if it's working, and then create the window and verify that it's working.
In the `createWindow` function, a `rect` type is created (to store the position & size of UI elements to have them created). Then a window is created with the dimensions from `rect`. Notice how the position in the `rect` object are (-1, -1). Then, the `rect` object is set to a different size (which is going to be the size of the label). The label is created, with the given size (argument 1), and then the text in the label is also set (argument 2). The background color of the label is set to white (see the 7th line in the `createWindow` function), and then the label is added to the window. Then, the window is rendered.
