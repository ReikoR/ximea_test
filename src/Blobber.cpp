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
#include <cstdio>
#include <memory.h>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include "Blobber.h"

Blobber::Blobber() {
	bpp = 1;
	width = 0;
	height = 0;
	segmented = NULL;
	bgr = NULL;
	pout = (unsigned short *) malloc(10000 * 9 * sizeof(unsigned short));
	run_c = 0;
	region_c = 0;
	max_area = 0;

    rle = new BlobberRun[MAX_RUNS];
	
	int i;
	for(i=0; i<COLOR_COUNT; i++) {
		colors[i].list = NULL;
		colors[i].num	= 0;
		colors[i].min_area = MAX_INT;
		colors[i].color = i;
	}
	
	for (i=0; i<MAX_WIDTH * MAX_HEIGHT; i++) {
		pixel_active[i] = 1;
	}
}

Blobber::~Blobber() {
	//exit, free resources
    if (segmented != NULL) {
        free(segmented);
    }
    if (bgr != NULL) {
        free(bgr);
    }
    if (pout != NULL) {
        free(pout);
    }
}

void Blobber::setColorMinArea(int color, int min_area) {
	//set min blob size

	if (color < COLOR_COUNT) {
		colors[color].min_area = min_area;
	}
}

void Blobber::setColors(unsigned char *data) {
	//set colortable
	//unsigned long size = min(0x1000000, (unsigned long)PyArray_NBYTES(lookup));
	unsigned long size = 0x1000000;
	memcpy(colors_lookup, data, size);
}

void Blobber::setActivePixels(unsigned char *data) {
	//set colortable
	//unsigned long size = min(MAX_WIDTH * MAX_HEIGHT, (unsigned long)PyArray_NBYTES(pixels));
	unsigned long size = MAX_WIDTH * MAX_HEIGHT;
	memcpy(pixel_active, data, size);
}

void Blobber::refreshSize() {
	//set cam size and allocate buffers
	
	//self->width = (int)self->image.width;
	//self->height = (int)self->image.height;

    width = 1280;
    height = 1024;

	int size = width * width;
	
	if (segmented != NULL) {
		free(segmented);
	}
	segmented = (unsigned char *)calloc(size, sizeof(unsigned char));
	
	if (bgr != NULL) {
		free(bgr);
	}
	bgr = (unsigned char *)malloc(size * sizeof(unsigned char) * 3);
}

void Blobber::start() {
	//start capture
	refreshSize();
}

void Blobber::segEncodeRuns() {
// Changes the flat array version of the thresholded image into a run
// length encoded version, which speeds up later processing since we
// only have to look at the points where values change.
	unsigned char m, save;
	unsigned char *row = NULL;
	int x, y, j, l;
	BlobberRun r;
	unsigned char *map = segmented;
	BlobberRun *rle = rle;
	
	int w = width;
	int h = height;

	r.next = 0;

	// initialize terminator restore
	save = map[0];

	j = 0;
	for(y = 0; y < h; y++){
		row = &map[y * w];

		// restore previous terminator and store next
		// one in the first pixel on the next row
		row[0] = save;
		save = row[w];
		row[w] = 255;
		
		r.y = y;

		x = 0;
		while(x < w){
			m = row[x];
			r.x = x;

			l = x;
			while(row[x] == m) x++;

			if(colors[m].min_area < MAX_INT || x >= w ) {
				r.color = m;
				r.width = x - l;
				r.parent = j;
				rle[j++] = r;

				if(j >= MAX_RUNS) {
					row[w] = save;
					run_c = j;
					return;
				}
			}
		}
	}

	run_c = j;
}

void Blobber::segConnectComponents() {
// Connect components using four-connecteness so that the runs each
// identify the global parent of the connected region they are a part
// of.	It does this by scanning adjacent rows and merging where
// similar colors overlap.	Used to be union by rank w/ path
// compression, but now it just uses path compression as the global
// parent index, a simpler rank bound in practice.
// WARNING: This code is complicated.	I'm pretty sure it's a correct
//	 implementation, but minor changes can easily cause big problems.
//	 Read the papers on this library and have a good understanding of
//	 tree-based union find before you touch it
	int l1, l2;
	BlobberRun r1, r2;
	int i, j, s;
	int num = run_c;
	BlobberRun *map = rle;

	// l2 starts on first scan line, l1 starts on second
	l2 = 0;
	l1 = 1;
	while(map[l1].y == 0) l1++; // skip first line

	// Do rest in lock step
	r1 = map[l1];
	r2 = map[l2];
	s = l1;
	while(l1 < num){
		if(r1.color==r2.color && colors[r1.color].min_area < MAX_INT){
			if((r2.x<=r1.x && r1.x<r2.x+r2.width) || (r1.x<=r2.x && r2.x<r1.x+r1.width)){
				if(s != l1){
					// if we didn't have a parent already, just take this one
					map[l1].parent = r1.parent = r2.parent;
					s = l1;
				} else if(r1.parent != r2.parent) {
					// otherwise union two parents if they are different

					// find terminal roots of each path up tree
					i = r1.parent;
					while(i != map[i].parent) i = map[i].parent;
					j = r2.parent;
					while(j != map[j].parent) j = map[j].parent;

					// union and compress paths; use smaller of two possible
					// representative indicies to preserve DAG property
					if(i < j) {
						map[j].parent = i;
						map[l1].parent = map[l2].parent = r1.parent = r2.parent = i;
					} else {
						map[i].parent = j;
						map[l1].parent = map[l2].parent = r1.parent = r2.parent = j;
					}
				}
			}
		}

		// Move to next point where values may change
		i = (r2.x + r2.width) - (r1.x + r1.width);
		if(i >= 0) r1 = map[++l1];
		if(i <= 0) r2 = map[++l2];
	}

	// Now we need to compress all parent paths
	for(i=0; i<num; i++){
		j = map[i].parent;
		map[i].parent = map[j].parent;
	}
}

int Blobber::rangeSum(int x, int w) {
	//foo bar
	return(w*(2*x + w-1) / 2);
}

void Blobber::segExtractRegions() {
// Takes the list of runs and formats them into a region table,
// gathering the various statistics along the way.	num is the number
// of runs in the rmap array, and the number of unique regions in
// reg[] (bounded by max_reg) is returned.	Implemented as a single
// pass over the array of runs.
	int b, i, n, a;
	int num = run_c;
	BlobberRun *rmap = rle;
	BlobberRegion *reg = regions;
	BlobberRun r;
	n = 0;

	for(i=0; i<num; i++){
		if( colors[rmap[i].color].min_area < MAX_INT){
			r = rmap[i];
			if(r.parent == i){
				// Add new region if this run is a root (i.e. self parented)
				rmap[i].parent = b = n;	// renumber to point to region id
				reg[b].color = r.color;
				reg[b].area = r.width;
				reg[b].x1 = r.x;
				reg[b].y1 = r.y;
				reg[b].x2 = r.x + r.width;
				reg[b].y2 = r.y;
				reg[b].cen_x = rangeSum(r.x,r.width);
				reg[b].cen_y = r.y * r.width;
				reg[b].run_start = i;
				reg[b].iterator_id = i; // temporarily use to store last run
				n++;
				if(n >= MAX_REG) {
					printf( "Regions buffer exceeded.\n" );
					region_c = MAX_REG;
					return;
				}
			} else {
				// Otherwise update region stats incrementally
				b = rmap[r.parent].parent;
				rmap[i].parent = b; // update parent to identify region id
				reg[b].area += r.width;
				reg[b].x2 = max(r.x + r.width,reg[b].x2);
				reg[b].x1 = min((int)r.x,reg[b].x1);
				reg[b].y2 = r.y; // last set by lowest run
				reg[b].cen_x += rangeSum(r.x,r.width);
				reg[b].cen_y += r.y * r.width;
				// set previous run to point to this one as next
				rmap[reg[b].iterator_id].next = i;
				reg[b].iterator_id = i;
			}
		}
	}

	// calculate centroids from stored sums
	for(i=0; i<n; i++){
		a = reg[i].area;
		reg[i].cen_x = (float)reg[i].cen_x / a;
		reg[i].cen_y = (float)reg[i].cen_y / a;
		rmap[reg[i].iterator_id].next = 0; // -1;
		reg[i].iterator_id = 0;
		reg[i].x2--; // change to inclusive range
	}
	region_c = n;
}

void Blobber::segSeparateRegions() {
// Splits the various regions in the region table a separate list for
// each color.	The lists are threaded through the table using the
// region's 'next' field.	Returns the maximal area of the regions,
// which can be used later to speed up sorting.
	BlobberRegion *p = NULL;
	int i;
	int c;
	int area;
	int num = region_c;
	BlobberRegion *reg = regions;
	color_class_state *color = colors;

	// clear out the region list head table
	for(i=0; i<COLOR_COUNT; i++) {
		color[i].list = NULL;
		color[i].num	= 0;
	}
	// step over the table, adding successive
	// regions to the front of each list
	max_area = 0;
	for(i=0; i<num; i++){
		p = &reg[i];
		c = p->color;
		area = p->area;

		if(area >= color[c].min_area){
			if(area > max_area) max_area = area;
			color[c].num++;
			p->next = color[c].list;
			color[c].list = p;
		}
	}
}

BlobberRegion* Blobber::segSortRegions(BlobberRegion *list, int passes ) {
// Sorts a list of regions by their area field.
// Uses a linked list based radix sort to process the list.
	BlobberRegion *tbl[CMV_RADIX]={NULL}, *p=NULL, *pn=NULL;
	int slot, shift;
	int i, j;

	// Handle trivial cases
	if(!list || !list->next) return(list);

	// Initialize table
	for(j=0; j<CMV_RADIX; j++) tbl[j] = NULL;

	for(i=0; i<passes; i++){
		// split list into buckets
		shift = CMV_RBITS * i;
		p = list;
		while(p){
			pn = p->next;
			slot = ((p->area) >> shift) & CMV_RMASK;
			p->next = tbl[slot];
			tbl[slot] = p;
			p = pn;
		}

		// integrate back into partially ordered list
		list = NULL;
		for(j=0; j<CMV_RADIX; j++){
			p = tbl[j];
			tbl[j] = NULL; // clear out table for next pass
			while(p){
				pn = p->next;
				p->next = list;
				list = p;
				p = pn;
			}
		}
	}

	return(list);
}

void Blobber::analyse(unsigned char *frame) {
	//get new frame and find blobs
	
	/*
	//use CameraRefreshFrame to convert RGGB to BGR. slower, but shorter code
	CameraRefreshFrame(self);
	int w = width;
	int h = height;
	
	unsigned char *f;
	f = bgr;
	int y, x;
	for (y=1; y<h-1; y++) {//threshold
		for (x=1; x<w-1; x++) {
			segmented[y*w+x] = colors_lookup[f[y*w*3+x*3] + (f[y*w*3+x*3+1] << 8) + (f[y*w*3+x*3+2] << 16)];
		}
	}
	*/

    unsigned char *f = frame;

	int w = width;
	int h = height;
	int y, x;
	for (y=1; y < h-1; y += 2) {//threshold RGGB Bayer matrix
		for (x = 1; x < w-1; x+=2) {//ignore sides
			//http://en.wikipedia.org/wiki/Bayer_filter
			//current block is BGGR
			//blue f[y*w+x],green1 f[y*w+x+1],green2 f[y*w+x+w],red f[y*w+x+w+1]
			int xy = y*w+x;
			int b;//blue
			int g;//green
			int r;//red
			
			if (pixel_active[xy]) {
				b = f[xy];
				g = (f[xy-1]+f[xy+1]+f[xy-w]+f[xy+w]+2) >> 2;//left,right,up,down
				r = (f[xy-w-1]+f[xy-w+1]+f[xy+w-1]+f[xy+w+1]+2) >> 2;//diagonal
				segmented[xy] = colors_lookup[b + (g << 8) + (r << 16)];
			}
			
			xy += 1;
			if (pixel_active[xy]) {
				b = (f[xy-1]+f[xy+1]+1) >> 1;//left,right
				g = f[xy];
				r = (f[xy-w]+f[xy+w]+1) >> 1;//up,down
				segmented[xy] = colors_lookup[b + (g << 8) + (r << 16)];
			}
			
			xy += w - 1;
			if (pixel_active[xy]) {
				b = (f[xy-w] + f[xy+w]+1) >> 1;//up,down
				g = f[xy];
				r = (f[xy-1]+f[xy+1]+1) >> 1;//left,right
				segmented[xy] = colors_lookup[b + (g << 8) + (r << 16)];
			}
			
			xy += 1;
			if (pixel_active[xy]) {
				b = (f[xy-w-1]+f[xy-w+1]+f[xy+w-1]+f[xy+w+1]+2) >> 2;//diagonal
				g = (f[xy-1]+f[xy+1]+f[xy-w]+f[xy+w]+2) >> 2;//left,right,up,down
				r = f[xy];
				segmented[xy] = colors_lookup[b + (g << 8) + (r << 16)];
			}

			//std::cout << +segmented[xy] << std::endl;
		}
	}
	
	segEncodeRuns();
	segConnectComponents();
	segExtractRegions();
	segSeparateRegions();

	// do minimal number of passes sufficient to touch all set bits
	y = 0;
	while( max_area != 0 ) {
		max_area >>= CMV_RBITS;
		y++;
	}
	passes = y;
}

unsigned short* Blobber::getBlobs(int color) {
	//get blobs for color, return numpy array [[distance,angle,area,cen_x,cen_y,x1,x2,y1,y2],...]

	BlobberRegion *list = segSortRegions(colors[color].list, passes);
	int rows = colors[color].num;
	int cols = 7;
	int i;
	int n = 0;
	int w = width;
	int xy;
	unsigned short cen_x, cen_y;
	unsigned short *pout = (unsigned short *) malloc(rows * cols * sizeof(unsigned short));

	if (rows > 0) {
		std::cout << "rows " << rows << std::endl;
	}

	for (i=0; i<rows; i++) {
		cen_x = (unsigned short)round(list[i].cen_x);
		cen_y = (unsigned short)round(list[i].cen_y);
		xy = cen_y * w + cen_x;

		pout[n++] = (unsigned short)min(65535 , list[i].area);
		pout[n++] = cen_x;
		pout[n++] = cen_y;
		pout[n++] = (unsigned short)list[i].x1;
		pout[n++] = (unsigned short)list[i].x2;
		pout[n++] = (unsigned short)list[i].y1;
		pout[n++] = (unsigned short)list[i].y2;
	}

	return pout;
}