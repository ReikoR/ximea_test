#ifndef XIMEA_TEST_BLOBBER_H
#define XIMEA_TEST_BLOBBER_H

/*
Python wrapper for XIMEA camera, search for colored blobs
Lauri Hamarik 2015
Blob finding methods (starts with Seg) taken from CMVision

Needs xiAPI library:
http://www.ximea.com/support/wiki/apis/XIMEA_Linux_Software_Package

xiAPI installation:
wget http://www.ximea.com/downloads/recent/XIMEA_Linux_SP.tgz
tar xzf XIMEA_Linux_SP.tgz
cd package
./install -cam_usb30

USAGE
Grab image:
	import numpy as np
	import pyXiQ
	cam = pyXiQ.Camera()
	cam.start()
	image = cam.image()
Get blobs:
	import numpy as np
	import pyXiQ

	cam = pyXiQ.Camera()
	cam.setInt("exposure", 10000)
	colors = np.zeros((256,256,256), dtype=np.uint8)
	colors[0:200,1:201,2:202] = 1#select colors where blue=0..200, green=1..201, red=2..202
	cam.setColors(colors)
	cam.setColorMinArea(1, 100)#show only blobs larger than 100
	cam.start()
	cam.analyse()
	blobs = cam.getBlobs(1)
*/

#define MAX_WIDTH 1280
#define MAX_HEIGHT 1024
#define MAX_INT 2147483647
#define COLOR_COUNT 10
#define CMV_RBITS 6
#define CMV_RADIX (1 << CMV_RBITS)
#define CMV_RMASK (CMV_RADIX-1)
#define MAX_RUNS MAX_WIDTH * MAX_HEIGHT / 4
#define MAX_REG MAX_WIDTH * MAX_HEIGHT / 16

#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; })

#define min(a,b) \
	({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a < _b ? _a : _b; })

typedef struct {
    short x, y, width;
    unsigned char color;
    int parent, next;
} BlobberRun;

typedef struct BlobberRegion {
    int color;
    int x1, y1, x2, y2;
    float cen_x, cen_y;
    int area;
    int run_start;
    int iterator_id;
    struct BlobberRegion* next;
} BlobberRegion;

typedef struct {
    BlobberRegion *list;
    int num;
    int min_area;
    unsigned char color;
    char *name;
} color_class_state;

/*typedef struct {
    unsigned char colors_lookup[0x1000000];//all possible bgr combinations lookup table/
    unsigned char pixel_active[MAX_WIDTH * MAX_HEIGHT];//0=ignore in segmentation, 1=use pixel
    unsigned char *segmented;//segmented image buffer 0-9
    unsigned char *bgr;//BGR buffer
    unsigned short *pout;//Temp out buffer (for blobs)
    int width, height, bpp;

    BlobberRun rle[MAX_RUNS];
    BlobberRegion regions[MAX_REG];
    color_class_state colors[COLOR_COUNT];
    int run_c;
    int region_c;
    int max_area;
    int passes;
} Camera;*/

class Blobber {
public:
	Blobber();
	~Blobber();

	void setColorMinArea(int color, int min_area);
	void setColors(unsigned char *data);
	void setActivePixels(unsigned char *data);
	void refreshSize();
	void start();
	void segEncodeRuns();
	void segConnectComponents();



	void segExtractRegions();
	void segSeparateRegions();
	BlobberRegion* segSortRegions(BlobberRegion *list, int passes );
	void analyse(unsigned char *bgr);
	unsigned short *getBlobs(int color);

private:
	int rangeSum(int x, int w);

	unsigned char colors_lookup[0x1000000];//all possible bgr combinations lookup table/
	unsigned char pixel_active[MAX_WIDTH * MAX_HEIGHT];//0=ignore in segmentation, 1=use pixel
	unsigned char *segmented;//segmented image buffer 0-9
	unsigned char *bgr;//BGR buffer
	unsigned short *pout;//Temp out buffer (for blobs)
	int width, height, bpp;

	BlobberRun rle[MAX_RUNS];
	BlobberRegion regions[MAX_REG];
	color_class_state colors[COLOR_COUNT];
	int run_c;
	int region_c;
	int max_area;
	int passes;
};

#endif //XIMEA_TEST_BLOBBER_H