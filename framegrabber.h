#ifndef FRAMEGRABBER_H
#define FRAMEGRABBER_H

#include "framegrabber_app.h"
#include <PvDevice.h>
#include <PvGenParameterArray.h>
#include <PvStream.h>
#include <PvBuffer.h>
#include <PvResult.h>
#include <list>

typedef struct {
	int wax;
	int way;
	int tint;
	bool WAXY_updated;
	bool tint_updated;
} serial_words;

class Framegrabber {
public:
	bool Connect();
	bool Disconnect();
	void data_loop();
	
	uint32_t width, height;
	std::list<FramegrabberApp *> apps;
	serial_words words;
	
	

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


};

#endif //FRAMEGRABBER_H