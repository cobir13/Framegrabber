#include "query.h"
#include "iomanager.h"
#include "exceptions.h"



PixelQuery::PixelQuery(Framegrabber *g, int x, int y) {

}

PixelQuery::PixelQuery(Framegrabber * grabber, std::vector<std::string>& arglist) {
	if (arglist.size() != 2) {
		throw bad_parameter_exception("2 parameters required!");
	}
	int savex, savey;

	try {
		savex = stoi(arglist[0]);
		savey = stoi(arglist[1]);
	}
	catch (std::invalid_argument &iarg) {
		throw bad_parameter_exception(iarg.what());
	}
	catch (std::overflow_error &oe) {
		throw bad_parameter_exception(oe.what());
	}
	init(grabber, savex, savey);
}


PixelQuery::~PixelQuery()
{
}

bool PixelQuery::set_frame(uint16_t * data)
{
	pixval = *(data + grabber->width * y + x);
	done = true;
	return true;
}

bool PixelQuery::save()
{
	grabber->iomanager->success(name, std::to_string(pixval));
	return true;
}

void PixelQuery::init(Framegrabber * g, int savex, int savey) {
	name = "PixelQuery";
	grabber = g;
	x = savex;
	y = savey;
	reportsuccess = false;
	done = false;
}
