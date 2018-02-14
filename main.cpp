#include <iostream>
#include <chrono>
#include <Util.h>
#include <Blobber.h>
#include "XimeaCamera.h"
#include "Gui.h"
#include "FpsCounter.h"
#include "SignalHandler.h"
#include "xiApi.h"
#include "SoccerBot.h"

/** Use to init the clock */
#define TIMER_INIT \
    LARGE_INTEGER frequency; \
    LARGE_INTEGER t1,t2; \
    double elapsedTime; \
    QueryPerformanceFrequency(&frequency);


/** Use to start the performance timer */
#define TIMER_START QueryPerformanceCounter(&t1);

/** Use to stop the performance timer and output the result to the standard stream. Less verbose than \c TIMER_STOP_VERBOSE */
#define TIMER_STOP \
    QueryPerformanceCounter(&t2); \
    elapsedTime=(float)(t2.QuadPart-t1.QuadPart)/frequency.QuadPart;

FpsCounter *fpsCounter = nullptr;
XimeaCamera *ximeaCamera = nullptr;
Blobber *blobber = nullptr;

unsigned char* rgb = new unsigned char[1280 * 1024 * 3];
unsigned char* bgr = new unsigned char[640 * 512 * 3];

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

/*void setupGui() {
    gui = new Gui(GetModuleHandle(0), 1280, 1024);
}*/

void run() {
    TIMER_INIT

    std::cout << "! Starting main loop" << std::endl;

    Gui *gui = new Gui(GetModuleHandle(nullptr), blobber, 1280, 1024);

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
            /*if (gui == nullptr) {
                setupGui();
            }*/

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

            //TIMER_START

            for (int y = 0; y < halfHeight; y++) {
                for (int x = 0; x < halfWidth; x++) {
                    unsigned char b = *(frame->data + (2 * y + 1) * width + 2 * x + 1);
                    //unsigned char g = (*(frame->data + 2 * y * width + 2 * x + 1) + *(frame->data + (2 * y + 1) * width + 2 * x)) / 2;
                    unsigned char g = *(frame->data + 2 * y * width + 2 * x + 1);
                    //unsigned char g = *(frame->data + (2 * y + 1) * width + 2 * x);
                    unsigned char r = *(frame->data + 2 * y * width + 2 * x);

                    rgb[(y * width + x) * 3] = b;
                    rgb[(y * width + x) * 3 + 1] = g;
                    rgb[(y * width + x) * 3 + 2] = r;

                    /*bgr[(y * halfWidth + x) * 3] = b;
                    bgr[(y * halfWidth + x) * 3 + 1] = g;
                    bgr[(y * halfWidth + x) * 3 + 2] = r;*/

                    //rgb[(y * width + x) * 3] = 255;
                    //rgb[(y * width + x) * 3 + 1] = 0;
                    //rgb[(y * width + x) * 3 + 2] = 255;
                }
            }

            /*for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
//                    unsigned char b = *(blobber->segmented + (2 * y + 1) * width + 2 * x + 1);
//                    unsigned char g = *(blobber->segmented + 2 * y * width + 2 * x + 1);
//                    unsigned char r = *(blobber->segmented + 2 * y * width + 2 * x);

                    unsigned char p = *(blobber->segmented + (y * width + x));

                    //int drawX = x + halfWidth;
                    int drawX = x;

                    rgb[(y * width + drawX) * 3] = 0;
                    rgb[(y * width + drawX) * 3 + 1] = 0;

                    if (p == 1) {
                        rgb[(y * width + drawX) * 3 + 2] = 255;
                    } else {
                        rgb[(y * width + drawX) * 3 + 2] = 0;
                    }
                }
            }*/

            TIMER_START

            //blobber->analyse(bgr);
            blobber->analyse(f);

            TIMER_STOP

            //std::wcout << "rle " << blobber->rle[1000].x << std::endl;

            std::wcout << elapsedTime << L" sec" << std::endl;

            //running = false;

            Blobber::BlobInfo* blobInfo = blobber->getBlobs(1);

            std::cout << "blobCount " << blobInfo->count << std::endl;



            if (blobInfo->count > 0) {
                for (int i = 0; i < blobInfo->count; i++) {
                    //std::cout << "blob " << blobInfo->blobs[0].area << " " << blobInfo->blobs[0].centerX << " "
                    //         << blobInfo->blobs[0].centerY << std::endl;

                    int x = blobInfo->blobs[i].centerX / 2;
                    int y = blobInfo->blobs[i].centerY / 2;
                    int x1 = blobInfo->blobs[i].x1 / 2;
                    int x2 = blobInfo->blobs[i].x2 / 2;
                    int y1 = blobInfo->blobs[i].y1 / 2;
                    int y2 = blobInfo->blobs[i].y2 / 2;

                    for (int lineX = x1; lineX <= x2; lineX++) {
                        rgb[(y1 * width + lineX) * 3] = 0;
                        rgb[(y1 * width + lineX) * 3 + 1] = 0;
                        rgb[(y1 * width + lineX) * 3 + 2] = 255;

                        rgb[(y2 * width + lineX) * 3] = 0;
                        rgb[(y2 * width + lineX) * 3 + 1] = 0;
                        rgb[(y2 * width + lineX) * 3 + 2] = 255;
                    }

                    for (int lineY = y1; lineY <= y2; lineY++) {
                        rgb[(lineY * width + x1) * 3] = 0;
                        rgb[(lineY * width + x1) * 3 + 1] = 0;
                        rgb[(lineY * width + x1) * 3 + 2] = 255;

                        rgb[(lineY * width + x2) * 3] = 0;
                        rgb[(lineY * width + x2) * 3 + 1] = 0;
                        rgb[(lineY * width + x2) * 3 + 2] = 255;
                    }

                    for (int subX = x - 1; subX <= x + 1; subX++) {
                        for (int subY = y - 1; subY <= y + 1; subY++) {
                            rgb[(subY * width + subX) * 3] = 0;
                            rgb[(subY * width + subX) * 3 + 1] = 0;
                            rgb[(subY * width + subX) * 3 + 2] = 255;
                        }
                    }
                }
            }

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

            //TIMER_STOP

            //std::wcout<<elapsedTime<<L" sec"<<std::endl;

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

            //gui->setFrontImages(rgb);

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

    delete ximeaCamera;

    Sleep(10000);

    std::cout << "! Main loop ended" << std::endl;
}

/*int main() {
    fpsCounter = new FpsCounter();

    blobber = new Blobber();

    unsigned char* blobberColors = new unsigned char[0x1000000];

    for (int r = 0; r < 255; r++) {
        for (int g = 0; g < 255; g++) {
            for (int b = 0; b < 255; b++) {
                if (r >= 200 && r <= 255 && g >= 80 && g <= 110 && b >= 35 && b <= 60) {
                    blobberColors[b + (g << 8) + (r << 16)] = 1;
                } else {
                    blobberColors[b + (g << 8) + (r << 16)] = 0;
                }
            }
        }
    }

    blobber->setColors(blobberColors);
    blobber->setColorMinArea(1, 100);

    blobber->start();

    //ximeaCamera = new XimeaCamera(857769553);
    ximeaCamera = new XimeaCamera(374363729);

    ximeaCamera->open();

    if (ximeaCamera->isOpened()) {
        setupXimeaCamera(ximeaCamera);
    } else {
        std::cout << "- Opening camera failed - configured serial: " << std::endl;

    }

    run();

    return 0;
}*/

int main() {
    SoccerBot* soccerBot = new SoccerBot();

    soccerBot->showGui = true;

    soccerBot->setup();
    soccerBot->run();

    delete soccerBot;
    soccerBot = NULL;
}

