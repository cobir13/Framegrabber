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
	bool Connect();
	bool Disconnect();
	uint32_t width, height;
	
	

private:
	PvDevice device;
	PvBuffer *buffers;
	PvStream stream;
	
	PvGenParameterArray *params;
	PvGenInteger *TLLocked;
	PvGenInteger *payload_size;
	PvGenCommand *start;
	PvGenCommand *stop;

	PvDeviceInfo *select_sole_device();
	std::list<FramegrabberApp*> apps;

	void data_loop();
};

#endif //FRAMEGRABBER_H