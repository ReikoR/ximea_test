#include "DebugRenderer.h"
#include "Canvas.h"
#include "Util.h"

void DebugRenderer::renderFPS(unsigned char* image, int fps, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	canvas.drawText(20, 20, "FPS: " + Util::toString(fps));
}