#include <iostream>
#include <Util.h>
#include "XimeaCamera.h"
#include "Gui.h"
#include "FpsCounter.h"
#include "SignalHandler.h"
#include "xiApi.h"

FpsCounter *fpsCounter = nullptr;
XimeaCamera *ximeaCamera = nullptr;
Gui *gui = nullptr;

unsigned char* rgb = new unsigned char[1280 * 1024 * 3];

bool showGui = true;

void setupXimeaCamera(XimeaCamera* camera) {
    camera->setGain(1);
    camera->setExposure(10000);
    camera->setFormat(XI_RAW8);
    camera->setAutoWhiteBalance(false);
    camera->setAutoExposureGain(false);
    //camera->setLuminosityGamma(1.0f);
    //camera->setWhiteBalanceBlue(1.0f); // TODO check
    //camera->setQueueSize(12); // TODO Affects anything?

    std::cout << "! " << " Camera info:" << std::endl;
    std::cout << "  > Name: " << camera->getName() << std::endl;
    std::cout << "  > Type: " << camera->getDeviceType() << std::endl;
    std::cout << "  > API version: " << camera->getApiVersion() << std::endl;
    std::cout << "  > Driver version: " << camera->getDriverVersion() << std::endl;
    std::cout << "  > Serial number: " << camera->getSerialNumber() << std::endl;
    std::cout << "  > Color: " << (camera->supportsColor() ? "yes" : "no") << std::endl;
    std::cout << "  > Framerate: " << camera->getFramerate() << std::endl;
    std::cout << "  > Available bandwidth: " << camera->getAvailableBandwidth() << std::endl;
}

void setupGui() {
    gui = new Gui(GetModuleHandle(0), 1280, 1024);
}

void run() {
    std::cout << "! Starting main loop" << std::endl;

    bool running = true;

    if (ximeaCamera->isOpened()) {
        ximeaCamera->startAcquisition();
    }

    //bool gotFrontFrame, gotRearFrame;
    double time;
    double debugging;
    double lastStepTime;
    float dt;
    float totalTime;

    while (running) {
        time = Util::millitime();

        if (lastStepTime != 0.0) {
            dt = (float)(time - lastStepTime);
        } else {
            dt = 1.0f / 60.0f;
        }

        totalTime += dt;

        fpsCounter->step();

        if (showGui) {
            if (gui == nullptr) {
                setupGui();
            }

            //gui->setFps(fpsCounter->getFps());

            BaseCamera::Frame *frame = ximeaCamera->getFrame();

            unsigned char pixel = *frame->data;

            //std::cout << "Frame number: " << frame->number << std::endl;
            //std::cout << "Frame timestamp: " << frame->timestamp << std::endl;
            //std::cout << "Frame size: " << frame->size << std::endl;
            //std::cout << "Frame width: " << frame->width << std::endl;
            //std::cout << "Frame height: " << frame->height << std::endl;
            //std::cout << "first pixel: " << pixel << std::endl;
            //printf("first pixel: %d\n", pixel);

            int width = 1280;
            int height = 1024;

            int w = width;
            int h = height;

            unsigned char* f = frame->data;
            unsigned char* p = rgb;

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

                    //rgb[(y * width + x) * 3] = 255;
                    //rgb[(y * width + x) * 3 + 1] = 0;
                    //rgb[(y * width + x) * 3 + 2] = 255;
                }
            }*/

            /*for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    unsigned char r = *(frame->data + y * width + x + width + 1);
                    unsigned char g = *(frame->data + y * width + x + 1);
                    unsigned char b = *(frame->data + y * width + x);

                    rgb[(y * width + x) * 3] = r;
                    rgb[(y * width + x) * 3 + 1] = g;
                    rgb[(y * width + x) * 3 + 2] = b;

                    //rgb[(y * width + x) * 3] = 255;
                    //rgb[(y * width + x) * 3 + 1] = 0;
                    //rgb[(y * width + x) * 3 + 2] = 255;
                }
            }*/

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

            /*for (int y=1; y < h-1; y += 2) {//ignore sides
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
            }*/

            /*for (int y=0; y < h; y++) {//ignore sides
                for (int x = 0; x < w; x++) {
                    //http://en.wikipedia.org/wiki/Bayer_filter
                    //current block is BGGR
                    //blue f[y*w+x],green1 f[y*w+x+1],green2 f[y*w+x+w],red f[y*w+x+w+1]
                    int xy = y*w+x;
                    int txy = xy*3;

                    p[txy] = f[xy];
                    p[txy + 1] = f[xy + 1];
                    p[txy + 2] = f[xy + w + 1];
                }
            }*/

            gui->setFrontImages(rgb);

            gui->update();

            if (gui->isQuitRequested()) {
                running = false;
            }
        }

        /*lastStepTime = time;

        if (SignalHandler::exitRequested) {
            running = false;
        }*/
    }

    Sleep(10000);

    std::cout << "! Main loop ended" << std::endl;
}

int main() {
    fpsCounter = new FpsCounter();

    ximeaCamera = new XimeaCamera(857769553);

    ximeaCamera->open();

    if (ximeaCamera->isOpened()) {
        setupXimeaCamera(ximeaCamera);
    } else {
        std::cout << "- Opening camera failed - configured serial: " << std::endl;

    }

    run();

    return 0;
}

