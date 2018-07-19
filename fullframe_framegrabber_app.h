#ifndef FULLFRAME_FRAMEGRABBER_APP_H
#define FULLFRAME_FRAMEGRABBER_APP_H

#include "framegrabber.h"
#include "framegrabber_app.h"
#include <string>

class FullFrame : FramegrabberApp {
	FullFrame(Framegrabber *grabber, int numframes, std::string dest);
	~FullFrame();

	bool set_frame(uint16_t * data);

private:
	int curframe;
	int numframes;
	std::string dest;
	uint16_t *fbuf;
	uint32_t width, height;
};



#endif //FULLFRAME_FRAMEGRABBER_APP_H