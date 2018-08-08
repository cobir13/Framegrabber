#ifndef FRAMEGRABBER_APP_H
#define FRAMEGRABBER_APP_H
#include <stdint.h>
#include <stdexcept>
#include <vector>
#include <string>

#define FGAPP_INIT uint16_t FramegrabberApp::cur_id = 0


class FramegrabberApp {
public:
	virtual ~FramegrabberApp() {};


	// Set this to true when your app is finished and should be saved
	bool done = false;

	// Override this method to define what to do when new data is received (typically, copy it to an internal buffer)
	virtual bool set_frame(uint16_t *data) = 0;
	// Override this to define the save and quit sequence for your app
	virtual bool save() = 0;
	// Override this to define processing that may take a little long
	// e.g. draw to the screen, or process data
	virtual void update() = 0;
	// Set this with get_id() in your constructor
	uint16_t id;

	virtual void message(std::vector<std::string> &messageparts) = 0;


	// Leave these alone---they should be the same for all apps
	bool operator==(const FramegrabberApp *app) { return this->id == app->id; }
	const char *getname() {
		return name;
	}
	
protected:
	// Leave this alone---it's used to assign IDs to new apps
	static uint16_t cur_id;
	uint16_t get_id() { return cur_id++; }

	// Set this in your constructor---it's used in messages about the app.
	// DO NOT leave this undefined---big crash
	const char *name;
};


#endif //FRAMEGRABBER_APP_H