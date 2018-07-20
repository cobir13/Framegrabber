#ifndef FOCUSER_H
#define FOCUSER_H

#include "framegrabber.h"
#include "framegrabber_app.h"
#include <string>

class Focuser : FramegrabberApp {
	Focuser(Framegrabber *grabber, int numframes, std::string dest, int savex, int savey);
	Focuser(Framegrabber *grabber, const char *input);
	~Focuser();

	bool set_frame(uint16_t * data);
	bool save();

	//Nothing to do every frame
	bool update(){};

private:
	void init();
	int curframe;
	int numframes;
	std::string dest;
	uint16_t *fbuf;
	uint32_t width, height;
	int x, y;
};



#endif //FOCUSER_H