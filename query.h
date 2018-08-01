#ifndef QUERY_H
#define QUERY_H
#include <string>
#include <vector>

#include "framegrabber_app.h"
#include "framegrabber.h"
class PixelQuery : public FramegrabberApp {
public:
	PixelQuery(Framegrabber *g, int x, int y);
	PixelQuery(Framegrabber *grabber, std::vector<std::string> &arglist);
	~PixelQuery();

	bool set_frame(uint16_t * data);
	bool save();
	void update() {};
private:
	Framegrabber *grabber;
	void init(Framegrabber *g, int savex, int savey);
	int x, y;

	uint16_t pixval;
};

#endif //QUERY_H