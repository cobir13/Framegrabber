#include <stdlib.h>
#include <stdexcept>
#include "fullframe_framegrabber_app.h"
#include "iomanager.h"
#include "exceptions.h"
#include "TinyTIFF/tinytiffwriter.h"

void FullFrame::init(Framegrabber *g, int numf, std::string dest_str) {
	name = "FullFrame";
	grabber = g;
	id = get_id();
	numframes = numf;
	curframe = 0;
	dest = dest_str;
	done = false;

	width = g->width;
	height = g->height;
	fbuf = (uint16_t*)calloc(sizeof(uint16_t), numframes*width*height);
	if (!fbuf) {
		throw std::bad_alloc();
	}
}

FullFrame::FullFrame(Framegrabber *grabber, int numf, std::string dest_str) {
	init(grabber, numf, dest_str);
}

FullFrame::FullFrame(Framegrabber *grabber, std::vector<std::string> &argstring) {
	if (argstring.size() != 2) {
		throw bad_parameter_exception("FullFrame requires two arguments");
	}
	int numf;
	std::string dest;

	try {
		numf = stoi(argstring[0]);
		dest = grabber->iomanager->extract_fp(argstring[1]);
	}
	catch (std::invalid_argument &iarg) {
		throw bad_parameter_exception(iarg.what());
	}
	catch (std::overflow_error &oe) {
		throw bad_parameter_exception(oe.what());
	}

	if (dest.empty()) {
		throw bad_parameter_exception("Could not extract path");
	}

	init(grabber, numf, dest);
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
	grabber->iomanager->info(name, "Saving");
	TinyTIFFFile *tif = TinyTIFFWriter_open(dest.c_str(), 16, width, height);
	if (tif) {
		for (int frame = 0; frame < numframes; frame++) {
			uint8_t *bufptr = (uint8_t*)fbuf + width*height * 2 * frame;
			TinyTIFFWriter_writeImage(tif, bufptr);
		}
		TinyTIFFWriter_close(tif);
		grabber->iomanager->info(name, "Saved");
    return true;
	}
	else {
		char warnbuf[48];
		snprintf(warnbuf, 48, "w=%d, h=%d, tif=%p, dest='%s'", width, height, (void*)tif, dest.c_str());
		grabber->iomanager->warning(name, warnbuf);
		grabber->iomanager->warning(name, "Save error!");
		grabber->iomanager->fatal("Exiting on save failure");
		return false;
	}
}
