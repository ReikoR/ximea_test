#ifndef SOCCERBOT_H
#define SOCCERBOT_H

#include "Vision.h"
#include <string>

class XimeaCamera;
class Blobber;
class Gui;

class SoccerBot {

public:
	SoccerBot();
	~SoccerBot();

	void setup();
	void run();

	void setupCameras();
	void setupSignalHandler();
	void setupGui();
	void setupVision();

	bool debugVision;
	bool showGui;

private:
	void setupXimeaCamera(std::string name, XimeaCamera* camera);

	XimeaCamera* frontCamera;
	Gui* gui;
	Blobber* blobber;

	bool running;
	float dt;
	double lastStepTime;
	float totalTime;
	Dir debugCameraDir;
	unsigned char* rgb;
	unsigned char* rgbData;
};

#endif // SOCCERBOT_H