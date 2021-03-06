#include "DebugRenderer.h"
#include "CameraTranslator.h"
#include "Canvas.h"
#include "Maths.h"
#include "Vision.h"
#include "Util.h"

void DebugRenderer::renderFPS(unsigned char* image, int fps, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	canvas.drawText(20, 20, "FPS: " + Util::toString(fps));
}

void DebugRenderer::renderBlobs(unsigned char* image, Blobber* blobber, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	for (int colorIndex = 0; colorIndex < blobber->getColorCount(); colorIndex++) {
		Blobber::ColorClassState* color = blobber->getColor(colorIndex);

		if (color == nullptr || color->name == nullptr) {
			continue;
		}

		if (
				strcmp(color->name, "green") != 0
				&& strcmp(color->name, "blue") != 0
				&& strcmp(color->name, "magenta") != 0
				) {
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

				canvas.drawBoxCentered(
					(int)blob.centerX, (int)blob.centerY,
					blob.x2 - blob.x1, blob.y2 - blob.y1,
					r, g, b
				);
			}
		}
	}
}

void DebugRenderer::renderBalls(unsigned char* image, Vision* vision, const ObjectList& balls, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	Object* ball = NULL;
	char buf[256];
	//int correctedX, correctedY;

	for (ObjectListItc it = balls.begin(); it != balls.end(); it++) {
		ball = *it;

		canvas.drawBoxCentered(ball->x, ball->y, ball->width, ball->height);
		//canvas.drawLine(ball->x - ball->width / 2, ball->y - ball->height / 2, ball->x + ball->width / 2, ball->y + ball->height / 2);
		//canvas.drawLine(ball->x - ball->width / 2, ball->y + ball->height / 2, ball->x + ball->width / 2, ball->y - ball->height / 2);

		//sprintf(buf, "%.2fm x %.2fm  %.1f deg", ball->distanceX, ball->distanceY, Math::radToDeg(ball->angle));
		sprintf(buf, "%.2fm %.2fm  %.1f deg", ball->distanceX, ball->distanceY, Math::radToDeg(ball->angle));

		if (ball->y + ball->height / 2 < Config::cameraHeight - 50) {
			canvas.drawText(ball->x - ball->width / 2 + 2, ball->y + ball->height / 2 + 4, buf);
		} else {
			canvas.drawText(ball->x - ball->width / 2 + 2, ball->y - ball->height / 2 - 10, buf);
		}

		//correctedX = ball->x;
		//correctedY = ball->y + ball->height / 2;

		CameraTranslator::CameraPosition undistortedPos = vision->getCameraTranslator()->undistort(ball->x, ball->y + ball->height / 2);

		//Util::correctCameraPoint(correctedX, correctedY);

		//sprintf(buf, "%d x %d - %d x %d", ball->x, ball->y + ball->height / 2, undistortedPos.x, undistortedPos.y);
		//canvas.drawText(ball->x - ball->width / 2 + 2, ball->y + ball->height / 2 + 14, buf);

		//int boxArea = ball->width * ball->height;

		/*if (boxArea == 0) {
			continue;
		}

        int density = ball->area * 100 / boxArea;

        sprintf(buf, "%d - %d%%", ball->area, density);
        canvas.drawText(ball->x - ball->width / 2 + 2, ball->y - ball->height / 2 - 9, buf);*/
	}

	// TEMP - draw centerline
	//canvas.drawLine(canvas.width / 2, 0, canvas.width / 2, canvas.height);
	//canvas.fillCircleCentered(Config::cameraWidth / 2, Config::cameraHeight / 2, 100, 0, 0, 255);

	/*Blobber::Blob* blob = blobber->getBlobs("ball");

    while (blob != NULL) {
        image->drawBoxCentered(blob->centerX, blob->centerY, blob->x2 - blob->x1, blob->y2 - blob->y1);

        blob = blob->next;
    }*/
}

void DebugRenderer::renderGoals(unsigned char* image, const ObjectList& goals, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	Object* goal = NULL;
	char buf[256];
	int r, g, b;

	for (ObjectListItc it = goals.begin(); it != goals.end(); it++) {
		goal = *it;

		if (goal->type == Side::YELLOW) {
			r = 200;
			g = 200;
			b = 0;
		} else {
			r = 0;
			g = 0;
			b = 200;
		}

		canvas.drawBoxCentered(goal->x, goal->y, goal->width, goal->height, r, g, b);
		//canvas.drawLine(goal->x - goal->width / 2, goal->y - goal->height / 2, goal->x + goal->width / 2, goal->y + goal->height / 2, r, g, b);
		//canvas.drawLine(goal->x - goal->width / 2, goal->y + goal->height / 2, goal->x + goal->width / 2, goal->y - goal->height / 2, r, g, b);

		int offsetY = 20;

		sprintf(buf, "%.2fm %.1f deg", goal->distance, Math::radToDeg(goal->angle));
		canvas.drawText(goal->x - goal->width / 2 + 2, goal->y + goal->height / 2 + offsetY, buf, r, g, b);

		sprintf(buf, "%d x %d, %d", goal->x, goal->y + goal->height / 2, goal->area);
		canvas.drawText(goal->x - goal->width / 2 + 2, goal->y + goal->height / 2 + offsetY + 10, buf, r, g, b);

		/*int boxArea = goal->width * goal->height;

        if (boxArea == 0) {
            continue;
        }

        int density = goal->area * 100 / boxArea;

        sprintf(buf, "%d - %d%%", goal->area, density);
        canvas.drawText(goal->x - goal->width / 2 + 2, goal->y - goal->height / 2 - 9, buf);*/
	}
}

void DebugRenderer::renderBrush(unsigned char* image, int x, int y, int radius, bool active, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	canvas.drawCircle(x, y, radius, active ? 255 : 0, 0, active ? 0 : 255);
}

/*void DebugRenderer::renderObstructions(unsigned char* image, Obstruction obstruction, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	if (obstruction == Obstruction::BOTH || obstruction == Obstruction::LEFT) {
		canvas.fillBox(width / 2 - 20, height - 80, 20, 40, 200, 0, 0);
	} else {
		canvas.fillBox(width / 2 - 20, height - 80, 20, 40, 0, 200, 0);
	}

	if (obstruction == Obstruction::BOTH || obstruction == Obstruction::RIGHT) {
		canvas.fillBox(width / 2, height - 80, 20, 40, 200, 0, 0);
	} else {
		canvas.fillBox(width / 2, height - 80, 20, 40, 0, 200, 0);
	}
}*/

void DebugRenderer::renderObjectHighlight(unsigned char* image, Object* object, int red, int green, int blue, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	canvas.drawBoxCentered(object->x, object->y, object->width + 4, object->height + 4, red, green, blue);
	canvas.drawLine(object->x - object->width / 2, object->y - object->height / 2, object->x + object->width / 2, object->y + object->height / 2, red, green, blue);
	canvas.drawLine(object->x - object->width / 2, object->y + object->height / 2, object->x + object->width / 2, object->y - object->height / 2, red, green, blue);
}

void DebugRenderer::renderGrid(unsigned char* image, Vision* vision, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	float maxDistanceY = 8.0f;
	float stepX = 0.01f;
	float minDistanceX = -4.0f;
	float maxDistanceX = 4.0f;
	float distanceX = 0.0f, distanceY = 0.0f;
	float distanceStartY = 0.125f;
	int counter = 0;
	int lastTextY = -1;
	int xOverflow = 500;
	Math::Point screenCoords;
	CameraTranslator::CameraPosition pos;
	CameraTranslator::CameraPosition distorted;
	CameraTranslator::CameraPosition undistorted;

	for (distanceY = distanceStartY; distanceY <= maxDistanceY; distanceY *= 2.0f) {
		for (distanceX = minDistanceX; distanceX <= maxDistanceX; distanceX += stepX) {
			pos = vision->getCameraTranslator()->getCameraPosition(distanceX, distanceY);

			canvas.setPixelAt(pos.x, pos.y, 0, 0, 128);

			/*for (int x = -xOverflow; x < Config::cameraWidth + xOverflow; x += 3) {
				distorted = vision->getCameraTranslator()->distort(x, pos.y);
				//undistorted = vision->getCameraTranslator()->undistort(distorted.x, distorted.y);

				canvas.setPixelAt(x, pos.y, 128, 0, 0);
				canvas.setPixelAt(distorted.x, distorted.y, 0, 0, 128);
				//canvas.setPixelAt(undistorted.x, undistorted.y, 128, 0, 0);
			}*/

			/*for (int y = 0; y < Config::cameraHeight; y += 3) {
				distorted = vision->getCameraTranslator()->distort(pos.x, y);
				//undistorted = vision->getCameraTranslator()->undistort(distorted.x, distorted.y);

				//canvas.setPixelAt(x, y, 0, 0, 128);
				canvas.setPixelAt(distorted.x, distorted.y, 0, 0, 128);
				//canvas.setPixelAt(undistorted.x, undistorted.y, 128, 0, 0);
			}*/

			//px = 10 + (counter % 10) * 30;


			/*for (distanceX = minDistanceX; distanceX < maxDistanceX; distanceX += stepX) {
				for (int y = 0; y < Config::cameraHeight; y++) {
					screenCoords = vision->getScreenCoords(vision->getDir(), distanceX, distanceY);
				}
			}*/

			counter++;
		}

		int x = Config::cameraWidth / 2 - 15;

		distorted = vision->getCameraTranslator()->getCameraPosition(0, distanceY);

		//if (lastTextY == -1 || lastTextY - distorted.y >= 8) {
		canvas.drawText(distorted.x, distorted.y, Util::toString(distanceY) + "m", 0, 0, 0);

		lastTextY = distorted.y;
		//}
	}

	// draw vertical dots at each 10x increment
	for (distanceX = minDistanceX; distanceX <= maxDistanceX; distanceX += stepX * 10.0f) {
		for (distanceY = 0.0f; distanceY < maxDistanceY; distanceY += stepX) {
			pos = vision->getCameraTranslator()->getCameraPosition(distanceX, distanceY);

			canvas.setPixelAt(pos.x, pos.y, 0, 0, 128);
		}
	}
}

void DebugRenderer::renderMapping(unsigned char* image, Vision* vision, int width, int height) {
	Canvas canvas = Canvas();

	canvas.data = image;
	canvas.width = width;
	canvas.height = height;

	CameraTranslator* translator = vision->getCameraTranslator();

	int step = 5;

	int x, y;
	CameraTranslator::CameraPosition pos;

	for (x = 0; x < Config::cameraWidth; x += step) {
		for (y = 0; y < Config::cameraHeight; y += step) {
			pos = translator->undistort(x, y);
			//pos = translator->distort(x, y);

			canvas.setPixelAt(pos.x, pos.y, 0, 0, 0);
		}
	}
}