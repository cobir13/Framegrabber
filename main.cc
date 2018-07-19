#include "framegrabber.h"
#include "framegrabber_app.h"
#include <string.h>
#include <stdio.h>
#include "focuser.h"

bool isdone (FramegrabberApp &app) {return app->done;}

struct {
    int wax;
    int way;
    int tint;
} serial_words;

void update_words(&words) {
    char word_str[8];
    int word_val;
    if (scanf(" %7s(%i)", word_str, &word_val) != 2) {
        throw BadFormatStringException(std::string("Error parsing serial word"));
    }
    if (!strcmp(word_str, "WAX")) {
        words->wax = word_val;
    }
    else if (!strcmp(word_str, "WAY")) {
        words->way = word_val;
    }
    else if(!strcmp(word_str, "TINT")) {
        words->tint = word_val;
    }
    else {
        printf("ERORR(Unknown serial word %s. Waiting for ';' to continue)\n", word_str);
    }
    while (c = getchar()) {
        if (c == ';') {
            printf("INFO(Caught ';'. Continuing)");
            return;
        }
    }
}

void newapp(Framegrabber *grabber, std::list<FramegrabberApp> *apps) {
    char app_str[16];
    char arglist[80];
    if (scanf("%15s (%79s) ;") != 2) {
        throw BadFormatStringException(std::string("Error parsing application"));
    }
    try {
        if (!strcmp(app_str, "FOCUSER")) {
            apps->push_front(
                Focuser(grabber, arglist);
            );
        }
        else if (!strcmp(app_str, "FULLFRAME")) {
            apps->push_front(
                FullFrame(grabber, arglist);
            );
        }
        else if (!strcmp(app_str, "WINDOW")) {
            apps->push_front(
                Window(grabber, arglist);
            );
        }
        else {
            printf("ERROR(Unknown application %s)\n");
        }
    } catch (BadFormatStringException e) {
        printf("ERROR(%s)\n", e.what());
    }
}

void FGloop(Framegrabber *grabber) {
    serial_words words = {0,0,202};

    while (true) {
        // Check socket for new apps to add/serial words
        try {
            char command[8];
            while (1 == scanf("%7s", command)) {
                if (!strcmp(command, "WORD")) {
                    update_words(&words);
                } 
                else if (!strcmp(command, "STREAM")) {
                    newapp(grabber, &grabber->apps);
                }
                else if (!strcmp(command, "QUIT;")) {
                    printf("INFO(Quitting)\n");
                    goto QUIT;
                } 
                else {
                    printf("ERROR(Unknown command word %s. Waiting for ';' to continue.)\n", command);
                    char c;
                    // Eat the rest of the malformed commnd until end is reached
                    while (c = getchar()) {
                        if (c == ';') {
                            printf("INFO(Caught ';'. Continuing)");
                            goto LOOP_END;
                        }
                    }
                }
                LOOP_END:
                void;
            }
        } catch (BadFormatStringException e) {
            printf("ERROR(%s)\n", e.what());
        }

        //write serial words

        //Update data for all running apps
        grabber->data_loop();

        // Save and quit any apps that say they're done
        for (auto &app: grabber->apps) {
            app.update();
            if (app.done) {
                app.save()
            }
        }
        grabber->apps.remove_if(isdone);
    }
    QUIT:
    void;
}

int main(int argc, char **argv) {
    Framegrabber grabber;
    grabber.Connect();
    FGloop(&grabber);
    grabber.Disconnect();
}