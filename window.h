#ifndef WINDOW_H
#define WINDOW_H
#include "framegrabber_app.h"
#include "framegrabber.h"
#include <stdint.h>
#include <SDL.h>
#undef main

#define DEFAULT_FPS (60)
#define DEFAULT_MSPF (1000/DEFAULT_FPS)

class Window : public FramegrabberApp {
public:
	Window(Framegrabber *grabber);
	Window(Framegrabber *grabber, const char *input);
	~Window();

	bool set_frame(uint16_t *data);
	bool save() { return true; }
	void update();

private:
	uint32_t ms_per_frame;
	Framegrabber *framegrabber;
	SDL_Window *window;
	SDL_Surface *surface;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	uint32_t last_update;
	bool should_update();
	bool updated;
	void init(Framegrabber *grabber);
};

#endif // WINDOW_H
