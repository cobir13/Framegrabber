#include <stdlib.h>
#include <stdexcept>
#include <chrono>
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
	status = FGAPP_ACQUIRE;
	current_save_frame = 0;

	

	width = g->width;
	height = g->height;
	size_t bufsize = numframes*width*height;
	if (bufsize > UINT32_MAX) {
		throw std::runtime_error("TIFF format only supports files of up to 4GB");
	}
	fbuf = (uint16_t*)calloc(sizeof(uint16_t), bufsize);
	if (!fbuf) {
		throw std::runtime_error("Could not allocate buffer");
	}

	tif = TinyTIFFWriter_open(dest.c_str(), 16, width, height);
	printf("Opened TIF\n");
	if (!tif) {
		throw std::runtime_error("Could not get lock on TIF save file");
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
	if (status == FGAPP_ACQUIRE) {
		printf("Acquiring frame %d/%d\n", curframe, numframes);
		uint16_t *current_buf = fbuf + (width*height*curframe);
		memcpy(current_buf, data, width*height * sizeof(uint16_t));
		curframe++;

		if (curframe >= numframes) {
			status = FGAPP_SAVING;
			grabber->iomanager->info(name, "Starting save");
		}
	}
	return true;
}

bool FullFrame::save() {
	grabber->iomanager->info(name, "Done saving");
	TinyTIFFWriter_close(tif);
	return true;
}

void FullFrame::update() {
	if (status != FGAPP_SAVING) {
		return;
	}
	typedef std::chrono::high_resolution_clock Time;
	typedef std::chrono::microseconds us;
	typedef std::chrono::duration<uint64_t, us> duration;
	auto t0 = Time::now();
	auto t1 = Time::now();

	for ((void)0;
	  std::chrono::duration_cast<us>(t1 - t0) < us(grabber->config.fg_config.us_per_frame) &&
		current_save_frame < numframes;
	  current_save_frame++) {
		uint8_t *bufptr = (uint8_t*)fbuf + width*height * 2 * current_save_frame;
		TinyTIFFWriter_writeImage(tif, bufptr);
		t1 = Time::now();
	}
	if (current_save_frame >= numframes) {
		status = FGAPP_DONE;
	}
}

void FullFrame::message(std::vector<std::string>& messageparts) {
	if (messageparts[0] == "status") {
		std::string state;
		switch (status) {
		case FGAPP_ACQUIRE:
			state = "Acquiring frames";
			break;
		case FGAPP_SAVING:
			state = "Saving frames";
			break;
		case FGAPP_DONE:
			state = "Done";
			break;
		default:
			grabber->iomanager->error(name, "Internal status error---corrupted status");
			return;
		}
		grabber->iomanager->success(name, state);
	}
	else {
		grabber->iomanager->error(name, "Unahndled message");
	}
}
