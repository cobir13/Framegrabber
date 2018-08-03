#include "framegrabber.h"
#include "iomanager.h"
#include "framegrabber_app.h"
#include <string.h>
#include <stdio.h>
#include "focuser.h"
#include "fullframe_framegrabber_app.h"
#include "window.h"

FGAPP_INIT;

void FGloop(Framegrabber *grabber) {
	IOManager *manager = grabber->iomanager;

	manager->ManageInput();
    

    //write serial words

    //Update data for all running apps
	try {
		grabber->data_loop();
	}
	catch (std::runtime_error &e) {
		manager->fatal(e.what());
	}

	auto app = grabber->apps.begin();
	while (app != grabber->apps.end()) {
		(*app)->update();
		if ((*app)->done) {
			(*app)->save();
			delete *app;
			grabber->apps.erase(app++);
		}
		else {
			app++;
		}
	}

}


int main(int argc, char **argv) {
    Framegrabber grabber;
	if (!grabber.Connect()) {
		grabber.iomanager->fatal("Could not connect");
	}
	try {
		while (1) FGloop(&grabber);
	}
	catch (std::exception) {}
    
}
