#include "SoccerBot.h"
#include "XimeaCamera.h"
#include "CameraTranslator.h"
#include "Vision.h"
#include "DebugRenderer.h"
#include "Gui.h"
#include "FpsCounter.h"
#include "SignalHandler.h"
#include "Config.h"
#include "Util.h"
#include "ImageProcessor.h"

#include <iostream>
#include <algorithm>
#include <Blobber.h>

SoccerBot::SoccerBot() :
	frontCamera(NULL),
	gui(NULL),
	blobber(NULL),
	running(false), debugVision(false),
	dt(0.01666f), lastStepTime(0.0), totalTime(0.0f),
	debugCameraDir(Dir::FRONT)
{

}

SoccerBot::~SoccerBot() {
	std::cout << "! Releasing all resources" << std::endl;

	if (gui != NULL) delete gui; gui = NULL;
	if (frontCamera != NULL) delete frontCamera; frontCamera = NULL;
    if (blobber != NULL) delete blobber; blobber = NULL;

	std::cout << "! Resources freed" << std::endl;
}

void SoccerBot::setup() {
	setupCameras();
	setupVision();
	setupSignalHandler();

	if (showGui) {
		setupGui();
	}
}

void SoccerBot::run() {
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

		// update goal path obstruction metric

		if (showGui) {
			if (gui == NULL) {
				setupGui();
			}

			BaseCamera::Frame *frame = frontCamera->getFrame();

			int width = 1280;
			int height = 1024;

			unsigned char* f = frame->data;

			int halfWidth = width / 2;
			int halfHeight = height / 2;

			/*for (int y = 0; y < halfHeight; y++) {
				for (int x = 0; x < halfWidth; x++) {
					unsigned char b = *(frame->data + (2 * y + 1) * width + 2 * x + 1);
					//unsigned char g = (*(frame->data + 2 * y * width + 2 * x + 1) + *(frame->data + (2 * y + 1) * width + 2 * x)) / 2;
					unsigned char g = *(frame->data + 2 * y * width + 2 * x + 1);
					//unsigned char g = *(frame->data + (2 * y + 1) * width + 2 * x);
					unsigned char r = *(frame->data + 2 * y * width + 2 * x);

					rgb[(y * width + x) * 3] = b;
					rgb[(y * width + x) * 3 + 1] = g;
					rgb[(y * width + x) * 3 + 2] = r;
				}
			}*/

			int w = width;
			int h = height;
			unsigned char* p = rgb;

			for (int y=1; y < h-1; y += 2) {//ignore sides
				for (int x = 1; x < w-1; x+=2) {
					//http://en.wikipedia.org/wiki/Bayer_filter
					//current block is BGGR
					//blue f[y*w+x],green1 f[y*w+x+1],green2 f[y*w+x+w],red f[y*w+x+w+1]
					int xy = y*w+x;
					int txy = xy*3;

					p[txy++] = f[xy];
					p[txy++] = (f[xy-1]+f[xy+1]+f[xy-w]+f[xy+w]+2) >> 2;//left,right,up,down
					p[txy++] = (f[xy-w-1]+f[xy-w+1]+f[xy+w-1]+f[xy+w+1]+2) >> 2;//diagonal

					xy += 1;
					p[txy++] = (f[xy-1]+f[xy+1]+1) >> 1;//left,right
					p[txy++] = f[xy];
					p[txy++] = (f[xy-w]+f[xy+w]+1) >> 1;//up,down

					xy += w - 1;
					txy = xy * 3;
					p[txy++] = (f[xy-w] + f[xy+w]+1) >> 1;//up,down
					p[txy++] = f[xy];
					p[txy++] = (f[xy-1]+f[xy+1]+1) >> 1;//left,right

					xy += 1;
					p[txy++] = (f[xy-w-1]+f[xy-w+1]+f[xy+w-1]+f[xy+w+1]+2) >> 2;//diagonal
					p[txy++] = (f[xy-1]+f[xy+1]+f[xy-w]+f[xy+w]+2) >> 2;//left,right,up,down
					p[txy]   = f[xy];
				}
			}

			memcpy(rgbData, rgb, static_cast<size_t>(3 * width * height));

			blobber->analyse(f);

            for (int colorIndex = 0; colorIndex < blobber->getColorCount(); colorIndex++) {
                Blobber::ColorClassState* color = blobber->getColor(colorIndex);

                if (color->name == nullptr) {
                    continue;
                }

                Blobber::BlobInfo* blobInfo = blobber->getBlobs(colorIndex);

                if (blobInfo->count > 0) {
                    unsigned char r = color->r;
                    unsigned char g = color->g;
                    unsigned char b = color->b;

                    for (int i = 0; i < blobInfo->count; i++) {
                        //std::cout << "blob " << blobInfo->blobs[0].area << " " << blobInfo->blobs[0].centerX << " "
                        //         << blobInfo->blobs[0].centerY << std::endl;

                        Blobber::Blob blob = blobInfo->blobs[i];

                        int x = blob.centerX;
                        int y = blob.centerY;
                        int x1 = blob.x1;
                        int x2 = blob.x2;
                        int y1 = blob.y1;
                        int y2 = blob.y2;

                        for (int lineX = x1; lineX <= x2; lineX++) {
                            rgb[(y1 * width + lineX) * 3] = b;
                            rgb[(y1 * width + lineX) * 3 + 1] = g;
                            rgb[(y1 * width + lineX) * 3 + 2] = r;

                            rgb[(y2 * width + lineX) * 3] = b;
                            rgb[(y2 * width + lineX) * 3 + 1] = g;
                            rgb[(y2 * width + lineX) * 3 + 2] = r;
                        }

                        for (int lineY = y1; lineY <= y2; lineY++) {
                            rgb[(lineY * width + x1) * 3] = b;
                            rgb[(lineY * width + x1) * 3 + 1] = g;
                            rgb[(lineY * width + x1) * 3 + 2] = r;

                            rgb[(lineY * width + x2) * 3] = b;
                            rgb[(lineY * width + x2) * 3 + 1] = g;
                            rgb[(lineY * width + x2) * 3 + 2] = r;
                        }

                        for (int subX = x - 1; subX <= x + 1; subX++) {
                            for (int subY = y - 1; subY <= y + 1; subY++) {
                                rgb[(subY * width + subX) * 3] = b;
                                rgb[(subY * width + subX) * 3 + 1] = g;
                                rgb[(subY * width + subX) * 3 + 2] = r;
                            }
                        }
                    }
                }
            }

			//std::cout << "blobCount " << blobInfo->count << std::endl;

			gui->setFrontImages(rgb, rgbData);

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

void SoccerBot::setupGui() {
	std::cout << "! Setting up GUI" << std::endl;

	gui = new Gui(
		GetModuleHandle(0),
		blobber,
		Config::cameraWidth, Config::cameraHeight
	);
}

void SoccerBot::setupCameras() {
	std::cout << "! Setting up cameras" << std::endl;

	frontCamera = new XimeaCamera(374363729);
	frontCamera->open();

	if (frontCamera->isOpened()) {
		setupXimeaCamera("Front", frontCamera);
	} else {
		std::cout << "- Opening front camera failed" << std::endl;
	}
}

void SoccerBot::setupXimeaCamera(std::string name, XimeaCamera* camera) {
	camera->setGain(Config::cameraGain);
	//camera->setGain(4);
	camera->setExposure(Config::cameraExposure);
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

void SoccerBot::setupSignalHandler() {
	SignalHandler::setup();
}

void SoccerBot::setupVision() {
	rgbData = new unsigned char[1280 * 1024 * 3];
	rgb = new unsigned char[1280 * 1024 * 3];

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