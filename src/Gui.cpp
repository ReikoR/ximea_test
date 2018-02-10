#include "Gui.h"
#include "DebugRenderer.h"
#include "BaseCamera.h"
#include "Util.h"

LRESULT CALLBACK WinProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

Gui::Gui(HINSTANCE instance, int width, int height) : instance(instance), width(width), height(height), quitRequested(false) {
	WNDCLASSEX wClass;
	ZeroMemory(&wClass, sizeof(WNDCLASSEX));

	wClass.cbClsExtra = 0;
	wClass.cbSize = sizeof(WNDCLASSEX);
	wClass.cbWndExtra = 0;
	wClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wClass.hInstance = instance;
	wClass.lpfnWndProc = (WNDPROC)WinProc;
	wClass.lpszClassName = "Window Class";
	wClass.lpszMenuName = NULL;
	wClass.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wClass)) {
		int nResult = GetLastError();

		MessageBox(
			NULL,
			"Window class creation failed",
			"Window Class Failed",
			MB_ICONERROR
		);
	}

	ZeroMemory(&msg, sizeof(MSG));

	mouseX = 0;
	mouseY = 0;
	mouseDown = false;

	frontRGB = createWindow(width, height, "Camera 1 RGB");

	selectedColorName = "";
}

Gui::~Gui() {
	for (std::vector<DisplayWindow*>::const_iterator i = windows.begin(); i != windows.end(); i++) {
		delete *i;
	}

	windows.clear();

	for (std::vector<Element*>::const_iterator i = elements.begin(); i != elements.end(); i++) {
		delete *i;
	}

	elements.clear();
}

DisplayWindow* Gui::createWindow(int width, int height, std::string name) {
	DisplayWindow* window = new DisplayWindow(instance, width, height, name, this);

	windows.push_back(window);

	return window;
}

void Gui::drawElements(unsigned char* image, int width, int height) {
	for (std::vector<Element*>::const_iterator i = elements.begin(); i != elements.end(); i++) {
		(*i)->draw(image, width, height);
	}
}

bool Gui::update() {
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT) {
			return false;
		}
	}

	return true;
}

void Gui::setFrontImages(unsigned char* rgb) {
	//DebugRenderer::renderFPS(rgb, fps);

	//drawElements(rggb, width, height);
	
	frontRGB->setImage(rgb, false);
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_CREATE:
			SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
		break;

		case WM_DESTROY:
			PostQuitMessage(0);
			printf("Destroy\n");

			return 0;
		break;

		default:
			DisplayWindow* displayWindow = (DisplayWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

			if (displayWindow != NULL) {
				return displayWindow->handleMessage(hWnd, msg, wParam, lParam);
			} else {
				return DefWindowProc(hWnd, msg, wParam, lParam);
			}
		break;
	}

	return 0;
}
