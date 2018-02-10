#ifndef DEBUGRENDERER_H
#define DEBUGRENDERER_H

class Canvas;
class Vision;

class DebugRenderer {

public:
	static void renderFPS(unsigned char* image, int fps, int width = 1280, int height = 1024);
};

#endif // DEBUGRENDERER_H