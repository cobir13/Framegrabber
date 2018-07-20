#include <stdlib.h>
#include <stdexcept>
#include "fullframe_framegrabber_app.h"
#include "TinyTIFF/tinytiffwriter.h"

void FullFrame::init(Framegrabber *grabber, int numf, std::string dest_str) {
	numframes = numf;
	curframe = 0;
	dest = dest_str;
	done = false;

	width = grabber->width;
	height = grabber->height;
	fbuf = (uint16_t*)calloc(sizeof(uint16_t), numframes*width*height);
	if (!fbuf) {
		throw std::bad_alloc();
	}
}

FullFrame::FullFrame(Framegrabber *grabber, int numf, std::string dest_str) {
	init(grabber, numf, dest_str);
}

static char fname_buf[2048];
FullFrame::FullFrame(Framegrabber *grabber, const char *input) {
	int numf;
	if (scanf(input, " %i , %2047s ", &numf, fname_buf) != 2) {
		throw BadFormatStringException(std::string("Error in creating FullFrame app"));
	}
	init(grabber, numf, std::string(fname_buf));
}

FullFrame::~FullFrame() {
	free(fbuf);
}

bool FullFrame::set_frame(uint16_t *data) {
	if (!done) {
		uint16_t *current_buf = fbuf + (width*height*curframe);
		memcpy(current_buf, data, width*height * sizeof(uint16_t));
		curframe++;

		if (curframe >= numframes) {
			done = true;
		}
	}
	return true;
}

bool FullFrame::save() {
	TinyTIFFFile *tif = TinyTIFFWriter_open(dest.c_str(), 16, width, height);
	if (tif) {
		for (int frame = 0; frame < numframes; frame++) {
			uint8_t *bufptr = (uint8_t*)fbuf + width*height * 2 * frame;
			TinyTIFFWriter_writeImage(tif, bufptr);
		}
		TinyTIFFWriter_close(tif);
		return true;
	}
	else {
		return false;
	}
}