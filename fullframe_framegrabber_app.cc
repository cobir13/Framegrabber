#include "fullframe_framegrabber_app.h"
#include <stdlib.h>

FullFrame::FullFrame(Framegrabber *grabber, int numf, std::string dest_str) {
	numframes = numf;
	curframe = 0;
	dest = dest_str;
	done = false;

	width = grabber->width;
	height = grabber->height;
	fbuf = (uint16_t*)calloc(sizeof(uint16_t), numframes*width*height);
}

FullFrame::~FullFrame() {
	free(fbuf);
}

bool FullFrame::set_frame(uint16_t *data) {
	if (!done) {
		uint16_t *current_buf = fbuf + (width*height*curframe);
		curframe++;

		if (curframe >= numframes) {
			done = true;
		}
	}
}