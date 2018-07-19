#ifndef FRAMEGRABBER_APP_H
#define FRAMEGRABBER_APP_H
#include <stdint.h>

class FramegrabberApp {
	static int curID;
public:
	bool done;
	virtual bool set_frame(uint16_t *data) = 0;
	virtual bool save() = 0;
};

#endif //FRAMEGRABBER_APP_H