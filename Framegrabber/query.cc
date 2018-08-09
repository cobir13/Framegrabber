#include "query.h"
#include "iomanager.h"
#include "exceptions.h"



PixelQuery::PixelQuery(Framegrabber *g) {
	init(grabber);
}

PixelQuery::PixelQuery(Framegrabber * grabber, std::vector<std::string>& arglist) {
	if (arglist.size() != 0) {
		throw bad_parameter_exception("No parameters required! Use update method to query pixel");
	}
	init(grabber);
}


PixelQuery::~PixelQuery() { }

bool PixelQuery::set_frame(uint16_t * data)
{
	if (replyneeded) {
		returnval = data[y*grabber->width + y];
		dataupdated = true;
	}
	return true;
}

bool PixelQuery::save()
{
	return true;
}

void PixelQuery::update() {
	if (replyneeded && dataupdated) {
		grabber->iomanager->success(name, std::to_string(returnval));
		replyneeded = false;
		dataupdated = false;
	}
}

void PixelQuery::message(std::vector<std::string>& messageparts) {
	try {
		x = std::stoi(messageparts[0]);
		y = std::stoi(messageparts[1]);
		if (x < 0 || x > (int)grabber->width || y < 0 || y > (int)grabber->height) {
			throw std::out_of_range("");
		}
	}
	catch (std::invalid_argument) {
		grabber->iomanager->error(__FUNCTION__, "Invalid argument");
		return;
	}
	catch (std::out_of_range) {
		grabber->iomanager->error(__FUNCTION__, "Argument out of range");
		return;
	}
	replyneeded = true;
}

void PixelQuery::init(Framegrabber * g) {
	name = "PixelQuery";
	grabber = g;
	status = FGAPP_ACQUIRE;
	id = get_id();
}
