#include "framegrabber.h"
#include "framegrabber_app.h"
#include <string.h>
#include <stdio.h>
#include "focuser.h"
#include "fullframe_framegrabber_app.h"
#include "window.h"

bool isdone (FramegrabberApp *app) {return app->done;}



void update_words(serial_words *words) {
    char word_str[8];
    int word_val;
	
	if (scanf(" %7[A-Z]s ( %i ) ; ", word_str, &word_val) != 2) {
		printf("word_str: '%s', word_val: '%d'\n", word_str, word_val);
        throw BadFormatStringException(std::string("Error parsing serial word"));
    }
	printf("word_str: '%s', word_val: '%d'\n", word_str, word_val);
    if (!strcmp(word_str, "WAX")) {
        words->wax = word_val;
		words->WAXY_updated = true;
    }
    else if (!strcmp(word_str, "WAY")) {
        words->way = word_val;
		words->WAXY_updated = true;
    }
    else if(!strcmp(word_str, "TINT")) {
        words->tint = word_val;
		words->tint_updated = true;
    }
    else {
        printf("ERORR(Unknown serial word %s. Waiting for ';' to continue)\n", word_str);
    }
	char c;
    while (c = getchar()) {
        if (c == ';') {
            printf("INFO(Caught ';'. Continuing)\n");
            return;
        }
    }
}

void newapp(Framegrabber *grabber) {
    char app_str[16];
    char arglist[80];
    if (scanf("%15s (%79s) ;") != 2) {
        throw BadFormatStringException(std::string("Error parsing application"));
    }
    try {
        if (!strcmp(app_str, "FOCUSER")) {
            grabber->apps.push_front(
                new Focuser(grabber, arglist)
            );
        }
        else if (!strcmp(app_str, "FULLFRAME")) {
            grabber->apps.push_front(
                new FullFrame(grabber, arglist)
            );
        }
        else if (!strcmp(app_str, "WINDOW")) {
            grabber->apps.push_front(
                new Window(grabber, arglist)
            );
        }
        else {
            printf("ERROR(Unknown application %s)\n");
        }
    } catch (BadFormatStringException e) {
        printf("ERROR(%s (%s, %s))\n", e.what(), __FILE__, __LINE__);
    }
}

void prompt() { printf(">"); fflush(stdout); }

char commandbuf[2048];
void FGloop(Framegrabber *grabber) {
    serial_words words = {0,0,202};

	prompt();
    while (fgets(commandbuf, 2048, stdin)) {
		//Remove the trailing newline
		int buflen = strlen(commandbuf);
		if (buflen > 0) commandbuf[buflen - 1] = 0;
		printf("Got command '%s'\n", commandbuf);
        try {
			static char command[16];
			static char subcommand[16];
			static char args[2048];
			if (scanf(commandbuf, " %15s %15s ( %[^)]2047s ) ; ", command, subcommand, args) == 3) {
				printf("Full command\n");
			}
			else if (scanf(commandbuf, " %15s ; ", command) == 1) {
				printf("stub command\n");
			}
			else {
				printf("Unhandled command\n");
			}
			printf("command: '%s', subcommand: '%s', args: '%s'\n", command, subcommand, args);
        } catch (BadFormatStringException e) {
            printf("ERROR(%s)\n", e.what());
        }
		prompt();

        //write serial words

        //Update data for all running apps
        grabber->data_loop();

        // Save and quit any apps that say they're done
        for (FramegrabberApp *app: grabber->apps) {
            app->update();
            if (app->done) {
				app->save();
				delete app;

            }
        }
        grabber->apps.remove_if(isdone);
    }
    QUIT:
    void;
}

int main(int argc, char **argv) {
    Framegrabber grabber;
    //grabber.Connect();
    FGloop(&grabber);
    grabber.Disconnect();
}