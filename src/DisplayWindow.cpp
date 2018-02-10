#include "DisplayWindow.h"
#include "Gui.h"

#include <iostream>
#include <Windowsx.h>

DisplayWindow::DisplayWindow(HINSTANCE instance, int width, int height, std::string name, Gui* gui) : instance(instance), gui(gui), width(width), height(height), name(name), firstDraw(true) {
    windowHandle = CreateWindowEx(
		0,
		"Window Class",
		name.c_str(),
		WS_OVERLAPPEDWINDOW,
		200,
		200,
		width + 16,
		height + 38,
		HWND_DESKTOP,
		nullptr,
		instance,
		(void*)this
	);

	if (!windowHandle) {
		int nResult = GetLastError();

		MessageBox(NULL,
			"Window creation failed",
			"Window Creation Failed",
			MB_ICONERROR);
	}

	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = width;
	bitmapInfo.bmiHeader.biHeight = height;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 24;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
	bitmapInfo.bmiHeader.biSizeImage = 0;
	bitmapInfo.bmiHeader.biClrUsed = 0;
	bitmapInfo.bmiHeader.biClrImportant = 0;

	windowDeviceHandle = GetDC(windowHandle);
	bitmapDeviceHandle = CreateCompatibleDC(windowDeviceHandle);
	bitmap = CreateCompatibleBitmap(windowDeviceHandle, width, height);

	canvas = new Canvas();
	canvas->width = width;
	canvas->height = height;
}

DisplayWindow::~DisplayWindow() {
	DeleteObject(SelectObject(bitmapDeviceHandle, bitmap));
	DeleteDC(bitmapDeviceHandle);
	DeleteObject(bitmap);

	delete canvas;
}

void DisplayWindow::setImage(unsigned char* image, bool rgb2bgr) {
	if (firstDraw) {
		ShowWindow(windowHandle, SW_SHOWNORMAL);

		firstDraw = false;
	}

	/*if (!IsWindowVisible(windowHandle)) {
		std::cout << "! Window not visible" << std::endl;

		return;
	}*/

	canvas->data = image;

	if (rgb2bgr) {
		// BGR to RGB..
		unsigned char blue;

		for (int i = 0; i < width * height * 3 - 3; i += 3) {
			blue = image[i];
			image[i] = image[i + 2];
			image[i + 2] = blue;
		}
	}

    SetDIBits(windowDeviceHandle, bitmap, 0, height, image, &bitmapInfo, DIB_RGB_COLORS);
	bitmap = (HBITMAP) SelectObject(bitmapDeviceHandle, bitmap);
	//BitBlt (windowDeviceHandle, 0, 0, width, height, bitmapDeviceHandle, 0, 0, SRCCOPY);
	StretchBlt(windowDeviceHandle, 0, height, width, -height, bitmapDeviceHandle, 0, 0, width, height, SRCCOPY);

	//RedrawWindow(windowHandle, NULL, NULL, RDW_INVALIDATE);
}

LRESULT DisplayWindow::handleMessage(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM lParam) {
	return DefWindowProc(windowHandle, msg, wParam, lParam);
}