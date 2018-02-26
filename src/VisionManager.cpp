#include "VisionManager.h"
#include "XimeaCamera.h"
#include "Gui.h"
#include "FpsCounter.h"
#include "SignalHandler.h"
#include "Util.h"
#include <algorithm>

VisionManager::VisionManager() :
	frontCamera(nullptr),
	gui(nullptr),
	blobber(nullptr),
	fpsCounter(nullptr),
	running(false), debugVision(false),
	dt(0.01666f), lastStepTime(0.0), totalTime(0.0f),
	debugCameraDir(Dir::FRONT)
{

}

VisionManager::~VisionManager() {
	std::cout << "! Releasing all resources" << std::endl;

	delete gui;
	gui = nullptr;
	delete frontCamera;
	frontCamera = nullptr;
    delete blobber;
	blobber = nullptr;
    delete hubCom;

	std::cout << "! Resources freed" << std::endl;
}

void VisionManager::setup() {
	setupCameras();
	setupVision();
	setupFpsCounter();
	setupSignalHandler();

	if (showGui) {
		setupGui();
	}
}

void VisionManager::run() {
	std::cout << "! Starting main loop" << std::endl;

	running = true;

	if (frontCamera->isOpened()) {
		frontCamera->startAcquisition();
	}

	if (!frontCamera->isOpened()) {
		std::cout << "! Neither of the cameras was opened, running in test mode" << std::endl;

		while (running) {
			Sleep(100);

			if (SignalHandler::exitRequested) {
				running = false;
			}
		}

		return;
	}

	//bool gotFrontFrame, gotRearFrame;
	double time;
	double debugging;

	while (running) {
		//__int64 startTime = Util::timerStart();

		time = Util::millitime();

		if (lastStepTime != 0.0) {
			dt = (float)(time - lastStepTime);
		} else {
			dt = 1.0f / 60.0f;
		}

		/*if (dt > 0.04f) {
			std::cout << "@ LARGE DT: " << dt << std::endl;
		}*/

		totalTime += dt;

		//gotFrontFrame = gotRearFrame = false;
		debugging = debugVision || showGui;

		/*gotFrontFrame = fetchFrame(frontCamera, frontProcessor);
		gotRearFrame = fetchFrame(rearCamera, rearProcessor);

		if (!gotFrontFrame && !gotRearFrame && fpsCounter->frameNumber > 0) {
			//std::cout << "- Didn't get any frames from either of the cameras" << std::endl;

			continue;
		}*/

		fpsCounter->step();

		BaseCamera::Frame *frame = frontCamera->getFrame();

		if (showGui) {
			if (gui == NULL) {
				setupGui();
			}

			gui->setFps(fpsCounter->getFps());

			gui->processFrame(frame);
		}

		//__int64 startTime = Util::timerStart();

		blobber->analyse(frame->data);

		//std::cout << "! Total time: " << Util::timerEnd(startTime) << std::endl;

		if (showGui) {
			gui->update();

			if (gui->isQuitRequested()) {
				running = false;
			}
		}
		
		/*if (fpsCounter->frameNumber % 60 == 0) {
			std::cout << "! FPS: " << fpsCounter->getFps() << std::endl;
		}*/

		lastStepTime = time;

		if (SignalHandler::exitRequested) {
			running = false;
		}

		//std::cout << "! Total time: " << Util::timerEnd(startTime) << std::endl;

		//std::cout << "FRAME" << std::endl;
	}

	std::cout << "! Main loop ended" << std::endl;
}

void VisionManager::setupGui() {
	std::cout << "! Setting up GUI" << std::endl;

	gui = new Gui(
		GetModuleHandle(0),
		blobber,
		Config::cameraWidth, Config::cameraHeight
	);
}

void VisionManager::setupCameras() {
	std::cout << "! Setting up cameras" << std::endl;

	frontCamera = new XimeaCamera(374363729);
	frontCamera->open();

	if (frontCamera->isOpened()) {
		setupXimeaCamera("Front", frontCamera);
	} else {
		std::cout << "- Opening front camera failed" << std::endl;
	}
}

void VisionManager::setupFpsCounter() {
	std::cout << "! Setting up fps counter.. ";

	fpsCounter = new FpsCounter();

	std::cout << "done!" << std::endl;
}

	camera->setGain(Config::cameraGain);
	//camera->setGain(4);
	camera->setExposure(Config::cameraExposure);
void VisionManager::setupXimeaCamera(std::string name, XimeaCamera* camera) {
	camera->setFormat(XI_RAW8);
	camera->setAutoWhiteBalance(false);
	camera->setAutoExposureGain(false);
	//camera->setLuminosityGamma(1.0f);
	//camera->setWhiteBalanceBlue(1.0f); // TODO check
	//camera->setQueueSize(12); // TODO Affects anything?

	std::cout << "! " << name << " camera info:" << std::endl;
	std::cout << "  > Name: " << camera->getName() << std::endl;
	std::cout << "  > Type: " << camera->getDeviceType() << std::endl;
	std::cout << "  > API version: " << camera->getApiVersion() << std::endl;
	std::cout << "  > Driver version: " << camera->getDriverVersion() << std::endl;
	std::cout << "  > Serial number: " << camera->getSerialNumber() << std::endl;
	std::cout << "  > Color: " << (camera->supportsColor() ? "yes" : "no") << std::endl;
	std::cout << "  > Framerate: " << camera->getFramerate() << std::endl;
	std::cout << "  > Available bandwidth: " << camera->getAvailableBandwidth() << std::endl;
}

void VisionManager::setupSignalHandler() {
	SignalHandler::setup();
}

void VisionManager::setupVision() {
	blobber = new Blobber();

    blobber->loadColors("colors.dat");
	blobber->setColorMinArea(1, 100);
	blobber->setColorMinArea(2, 100);
	blobber->setColorMinArea(3, 100);
	blobber->setColorMinArea(4, 100);
	blobber->setColorMinArea(5, 100);
	blobber->setColorMinArea(6, 100);

	blobber->start();
}