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

    // Save and quit any apps that say they're done
    for (FramegrabberApp *app: grabber->apps) {
        app->update();
        if (app->done) {
			app->save();
			delete app;

        }
    }
    grabber->apps.remove_if([](FramegrabberApp *app) { return app->done; });

}

int main(int argc, char **argv) {
    Framegrabber grabber;
	grabber.Connect();
	while (1) FGloop(&grabber);
    grabber.Disconnect();

	while (1);
}