#ifndef WINDOW_H
#define WINDOW_H
#include "framegrabber_app.h"
#include "framegrabber.h"
#include <stdint.h>
#include <vector>
#include <SDL.h>
#undef main



class Window : public FramegrabberApp {
public:
	Window(Framegrabber *grabber);
	Window(Framegrabber *grabber, std::vector<std::string> &argstring);
	~Window();

	bool set_frame(uint16_t *data);
	bool save();
	void update();

private:
	uint32_t ms_per_frame;
	uint16_t *framebuf;
	Framegrabber *framegrabber;
	SDL_Window *window;
	SDL_Surface *surface;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	uint32_t last_update;
	bool should_update();
	bool updated;
	void init(Framegrabber *grabber);
	uint32_t grey16_to_rgba32(uint16_t pix);

	int img_h, img_w;
	SDL_Rect img_viewport;
	SDL_Rect text_viewport;
};

#endif // WINDOW_H
