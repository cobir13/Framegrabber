#ifndef QUERY_H
#define QUERY_H
#include <string>
#include <vector>

#include "framegrabber_app.h"
#include "framegrabber.h"
class PixelQuery : public FramegrabberApp {
public:
	PixelQuery(Framegrabber *g);
	PixelQuery(Framegrabber *grabber, std::vector<std::string> &arglist);
	~PixelQuery();

	bool set_frame(uint16_t * data);
	bool save();
	void update();
	void message(std::vector<std::string> &messageparts);
	FGAppStatus GetStatus() { return status; }
private:
	FGAppStatus status = FGAPP_ACQUIRE;
	Framegrabber *grabber;
	void init(Framegrabber *g);
	int x, y;
	bool replyneeded;
	bool dataupdated;
	uint16_t returnval;
};

#endif //QUERY_H