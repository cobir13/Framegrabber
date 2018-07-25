#ifndef FRAMEGRABBER_APP_H
#define FRAMEGRABBER_APP_H
#include <stdint.h>
#include <stdexcept>

#define FGAPP_INIT uint16_t FramegrabberApp::cur_id = 0


class FramegrabberApp {
public:
	bool done;
	virtual bool set_frame(uint16_t *data) = 0;
	virtual bool save() = 0;
	virtual void update() = 0;

	bool operator==(const FramegrabberApp *app) { return this->id == app->id; }
	uint16_t id;

	const char *getname() {
		return name;
	}
	
protected:
	static uint16_t cur_id;
	uint16_t get_id() { return cur_id++; }
	const char *name;
};


#endif //FRAMEGRABBER_APP_H