#ifndef FULLFRAME_FRAMEGRABBER_APP_H
#define FULLFRAME_FRAMEGRABBER_APP_H

#include "framegrabber.h"
#include "framegrabber_app.h"
#include "TinyTIFF/tinytiffwriter.h"
#include <string>
#include <vector>

class FullFrame : public FramegrabberApp {
public:
	FullFrame(Framegrabber *grabber, int numframes, std::string dest);
	FullFrame(Framegrabber *grabber, std::vector<std::string> &argstring);
	~FullFrame();

	bool set_frame(uint16_t * data);
	bool save();

	void update();

	void message(std::vector<std::string> &messageparts);

	FGAppStatus GetStatus() { return status; }

private:
	// Set this to FGAPP_DONE when your code is done and ready to be saved.
	FGAppStatus status = FGAPP_ACQUIRE;

	Framegrabber *grabber;
	int curframe;
	int numframes;
	std::string dest;
	uint16_t *fbuf;
	uint32_t width, height;
	TinyTIFFFile *tif;
	uint64_t current_save_frame;

  void init(Framegrabber *grabber, int numframes, std::string dest);
};



#endif //FULLFRAME_FRAMEGRABBER_APP_H
