#ifndef FOCUSER_H
#define FOCUSER_H

#include "framegrabber.h"
#include "framegrabber_app.h"
#include <string>
#include <vector>

class Focuser : public FramegrabberApp {
public:
	Focuser(Framegrabber *grabber, int numframes, std::string dest, int savex, int savey);
	Focuser(Framegrabber *grabber, std::vector<std::string> &argstring);
	~Focuser();

	bool set_frame(uint16_t * data);
	bool save();

	//Nothing to do every frame
	void update(){};
	void message(std::vector<std::string> &messageparts);

	FGAppStatus GetStatus() { return status; }

private:
	// Set this to FGAPP_DONE when your code is done and ready to be saved.
	FGAppStatus status = FGAPP_ACQUIRE;
	Framegrabber *grabber;
	void init(Framegrabber *grabber, int numf, std::string dest_str, int savex, int savey);
	int curframe;
	int numframes;
	std::string dest;
	uint16_t *fbuf;
	uint32_t width, height;
	int x, y;
};



#endif //FOCUSER_H