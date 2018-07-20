#include "focuser.h"
#include <stdlib.h>
#include "TinyTIFF/tinytiffwriter.h"

void Focuser::Focuser(Framegrabber *grabber, int numf, std::string dest_str, int savex, int savey) {
	init(grabber, numf, dest_str, savex, savey);
}

//If your path has more than 2048 chars, you need to rethink your filing system
static char fname_buf[2048];
Focuser::Focuser(Framegrabber *grabber, const char *input) {
	int numf, savex, savey;
	if (sscanf(input, " %i , %2047s , %i , %i ", &numf, fname_buf, &savex, &savey) != 4) {
		throw BadFormatStringException(std::string("Error in creating Focuser app"));
	}
	init(grabber, numf, std::string(fname_buf), savex, savey);
}

void Focuser::init(Framegrabber *grabber, int numf, std::string dest_str, int savex, int savey) {
	numframes = numf;
	curframe = 0;
	dest = dest_str;
	done = false;
	x = savex;
	y = savey;

	width = grabber->width;
	height = grabber->height;
	fbuf = (uint16_t*)calloc(sizeof(uint16_t), numframes);
}

Focuser::~Focuser() {
	free(fbuf);
}

bool Focuser::set_frame(uint16_t *data) {
	if (!done) {
		fbuf[curframe] = *(data + width*y + x);
		curframe++;

		if (curframe >= numframes) {
			done = true;
		}
	}
}

bool Focuser::save() {
	TinyTIFFFile *tif = TinyTIFFWriter_open(dest.c_str(), 16, width, height);
	if (tif) {
		for (int frame = 0; frame < numframes; frame++) {
			uint8_t *bufptr = (uint8_t*)fbuf + 2 * frame;
			TinyTIFFWriter_writeImage(tif, bufptr);
		}
		TinyTIFFWriter_close(tif);
		return true;
	}
	else {
		return false;
	}
}