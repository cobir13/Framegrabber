#ifndef FRAMEGRABBER_H
#define FRAMEGRABBER_H

#include "framegrabber_app.h"
#include "serialwords.h"
#include <PvDevice.h>
#include <PvGenParameterArray.h>
#include <PvStream.h>
#include <PvBuffer.h>
#include <PvResult.h>
#include <list>
#include <regex>

//#define FOO

typedef struct {
	struct {
		std::string ip_addr;
		int port;
		std::string logfile;
	} communications;
	struct {
		int img_h;
		int img_w;
		int maxapps;
		int bufcount;
	} fg_config;
	struct {
		int fps;
		int scaling;
		int text_height;
	} window;
} FramegrabberConfig;

class IOManager;

class Framegrabber {
public:
	Framegrabber();
	~Framegrabber();

	bool Connect();
	bool Disconnect();
	void data_loop();

	IOManager *iomanager;
	
	uint32_t width, height;
	std::list<FramegrabberApp *> apps;
	SerialWords words;
	FramegrabberConfig config;

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

	void stream_info();

	void load_config(const char *configfile);
};

#endif //FRAMEGRABBER_H