#ifndef FRAMEGRABBER_H
#define FRAMEGRABBER_H

#include "framegrabber_app.h"
#include "serialwords.h"
#include "ebus.h"
#include <list>
#include <regex>



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
  struct {
    int width;
    int height;
    int font_size;
    int fps;
  } focusergraph;
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

	std::pair<int, int> small_to_large(int x, int y);

private:
	PvDeviceGEV device;
	PvBuffer *buffers;
	PvStreamGEV stream;
	
	PvGenParameterArray *params;
	PvGenInteger *TLLocked;
	PvGenInteger *payload_size;
	PvGenCommand *start;
	PvGenCommand *stop;

	PvDeviceInfoGEV *select_sole_device();

	void stream_info();

	void load_config(const char *configfile);
};

#endif //FRAMEGRABBER_H
