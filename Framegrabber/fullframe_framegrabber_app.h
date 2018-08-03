#ifndef FULLFRAME_FRAMEGRABBER_APP_H
#define FULLFRAME_FRAMEGRABBER_APP_H

#include "framegrabber.h"
#include "framegrabber_app.h"
#include <string>
#include <vector>

class FullFrame : public FramegrabberApp {
public:
	FullFrame(Framegrabber *grabber, int numframes, std::string dest);
	FullFrame(Framegrabber *grabber, std::vector<std::string> &argstring);
	~FullFrame();

	bool set_frame(uint16_t * data);
	bool save();

	//Nothing to do every update.
	void update(){};

private:
	Framegrabber *grabber;
	int curframe;
	int numframes;
	std::string dest;
	uint16_t *fbuf;
	uint32_t width, height;

  void init(Framegrabber *grabber, int numframes, std::string dest);
};



#endif //FULLFRAME_FRAMEGRABBER_APP_H
