#include "focuser.h"
#include <stdlib.h>
#include "exceptions.h"
#include "TinyTIFF/tinytiffwriter.h"
#include "iomanager.h"

Focuser::Focuser(Framegrabber *grabber, int numf, std::string dest_str, int savex, int savey) {
	init(grabber, numf, dest_str, savex, savey);
}

//If your path has more than 2048 chars, you need to rethink your filing system
Focuser::Focuser(Framegrabber *grabber, std::vector<std::string> &argstring) {
	if (argstring.size() != 4) {
		throw bad_parameter_exception("Focuser requires four arguments");
	}
	int numf, savex, savey;
	std::string dest;

	try {
		numf = stoi(argstring[0]);
		dest = grabber->iomanager->extract_fp(argstring[1]);
		savex = stoi(argstring[2]);
		savey = stoi(argstring[3]);
	}
	catch (std::invalid_argument &iarg) {
		throw bad_parameter_exception(iarg.what());
	}
	catch (std::out_of_range &oor) {
		throw bad_parameter_exception(oor.what());
	}

	init(grabber, numf, dest, savex, savey);
}

//Generalized constructor
void Focuser::init(Framegrabber *g, int numf, std::string dest_str, int savex, int savey) {
	name = "Focuser";
	grabber = g;
	id = get_id();
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

//Copy data from the stream buffer to the internal buffer
bool Focuser::set_frame(uint16_t *data) {
	if (!done) {
		fbuf[curframe] = *(data + width*y + x);
		curframe++;

		if (curframe >= numframes) {
			done = true;
		}
	}
	return true;
}

// Write the internal buffer to a tif file
bool Focuser::save() {
	grabber->iomanager->info(name, "Saving");
	TinyTIFFFile *tif = TinyTIFFWriter_open(dest.c_str(), 16, 1, 1);
	if (tif) {
		for (int frame = 0; frame < numframes; frame++) {
			uint8_t *bufptr = (uint8_t*)fbuf + 2*frame;
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
