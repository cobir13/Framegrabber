#ifndef FRAMEGRABBER_H
#define FRAMEGRABBER_H

#include "framegrabber_app.h"
#include <PvDevice.h>
#include <PvGenParameterArray.h>
#include <PvStream.h>
#include <PvBuffer.h>
#include <PvResult.h>
#include <list>

class Framegrabber {
public:
	
	

private:
	PvDevice device;
	PvBuffer *buffers;

	PvDeviceInfo *select_sole_device();
	std::list<FramegrabberApp> apps;

	void data_loop();
};

#endif //FRAMEGRABBER_H