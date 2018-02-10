#ifndef GUI_H
#define GUI_H

#define _WINSOCKAPI_
#include <windows.h>

#include "DisplayWindow.h"
#include "BaseCamera.h"
#include <vector>
#include <string>

class Canvas;
class CameraTranslator;
class Command;
class Vision;
class Blobber;
class ParticleFilterLocalizer;

class Gui {

public:
	class Element {
		public:
			Element();
			virtual void draw(unsigned char* image, int imageWidth, int imageHeight) = 0;
			virtual bool contains(int x, int y) { return false; };
			Canvas canvas;
			double lastInteractionTime;
	};

	Gui(HINSTANCE instance, int width, int height);
    ~Gui();

	DisplayWindow* createWindow(int width, int height, std::string name);
	void drawElements(unsigned char* image, int width, int height);
    bool update();
	bool isQuitRequested() { return quitRequested; }
	void setFps(int fps) { this->fps = fps; };
	void setFrontImages(unsigned char* rggb);


private:
	HINSTANCE instance;
	MSG msg;
	std::vector<DisplayWindow*> windows;
	DisplayWindow* frontRGB;
	std::vector<Element*> elements;
	std::string selectedColorName;
	int width;
	int height;
	int fps;
	int mouseX;
	int mouseY;
	bool mouseDown;
	bool quitRequested;
};

#endif // GUI_H
