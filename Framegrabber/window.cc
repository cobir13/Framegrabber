#include "window.h"
#include "iomanager.h"
#include "exceptions.h"
#include <utility>


 uint32_t Window::grey16_to_rgba32(uint16_t pix) {
	// We need to convert from a 14 bit number (in a 16 bit shell) to an 8-bit value.
	uint8_t pix8 = (uint8_t)(pix); // pix / (2^14/2^8)
	uint8_t result[4];
	result[0] = SDL_ALPHA_OPAQUE;
	result[1] = pix8;
	result[2] = pix8;
	result[3] = pix8;
	return *(uint32_t*)(&result);
}

Window::Window(Framegrabber *grabber) { init(grabber); }
Window::Window(Framegrabber *grabber, std::vector<std::string> &argstring) { init(grabber); }


void Window::init(Framegrabber *grabber) {
	name = "Window";
	id = get_id();
	framegrabber = grabber;
	done = false;
	auto &windowconfig = grabber->config.window;
  mouse_x = 0;
  mouse_y = 0;

	font = FC_CreateFont();
	
	
	ms_per_frame = 1000/windowconfig.fps;

	if (!SDL_WasInit(SDL_INIT_VIDEO) && SDL_Init(SDL_INIT_VIDEO) < 0) {

		throw std::runtime_error("Could not initialize SDL2");
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

	framebuf = (uint16_t*)malloc(2 * grabber->width * grabber->height);

	img_w = grabber->width * windowconfig.scaling;
	img_h = grabber->height * windowconfig.scaling;

	window = SDL_CreateWindow(
		"Framegrabber Display",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		img_w,
		img_h + windowconfig.text_height,
		SDL_WINDOW_SHOWN);
	if (!window) {

		throw std::runtime_error("Could not create SDL window");
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {

		throw std::runtime_error("Could not create SDL renderer");
	}

	if (!FC_LoadFont(font, renderer, grabber->config.fg_config.font.c_str(),
		windowconfig.text_height-4, FC_MakeColor(0, 0, 0, 255), TTF_STYLE_NORMAL)) {
		throw bad_parameter_exception("Could not load font");
	}

	
	//Set background color to black
	SDL_SetRenderDrawColor(renderer, 0xCF, 0xCF, 0xCF, SDL_ALPHA_OPAQUE);

	texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_STREAMING,
		grabber->width,
		grabber->height
	);
	if (!texture) {
		grabber->iomanager->error(name, SDL_GetError());
		throw std::runtime_error("Could not create SDL texture");
	}

	img_viewport = { 0,0,img_w, img_h };
	text_viewport = { 0, img_h, img_w, windowconfig.text_height };
}


Window::~Window() {
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}

bool Window::should_update() {
	if (SDL_GetTicks() - last_update < ms_per_frame) {
		return false;
	}
	return true;
}

bool Window::set_frame(uint16_t *data) {
	if (done || !should_update()) {
		return true;
	}

	memcpy(framebuf, data, 2 * framegrabber->width*framegrabber->height);
	last_update = SDL_GetTicks();
	updated = true;
	return true;
}

void Window::update() {
	int &scaling = framegrabber->config.window.scaling;
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_WINDOWEVENT:
			switch (e.window.event) {
			case SDL_WINDOWEVENT_CLOSE:
				done = true;
				break;
			}
			break;

		case SDL_MOUSEMOTION:
			mouse_x = e.motion.x/scaling;
			mouse_y = e.motion.y/scaling;
			break;
		}
	}

	// If there's no new data, there's no need to handle the screen update
	if (!updated) return;
	updated = false;

	uint32_t *vidbuf;
	int pitch;
	SDL_LockTexture(texture, NULL, (void**)&vidbuf, &pitch);
	for (uint32_t i = 0; i < (framegrabber->width * framegrabber->height); i++) {
		vidbuf[i] = grey16_to_rgba32(framebuf[i]);
	}
	SDL_UnlockTexture(texture);

	

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, &img_viewport);
	if (mouse_y < (int)framegrabber->height) {
		uint16_t mouseval = framebuf[framegrabber->width*mouse_y + mouse_x];
		auto lcoords = framegrabber->small_to_large(mouse_x, mouse_y);
		FC_DrawBox(font, renderer, text_viewport,
			" | X=%d Y=%d | x=%d y=%d | (%d) | ",
			lcoords.first, lcoords.second, mouse_x, mouse_y, mouseval);
	}
	SDL_RenderPresent(renderer);	
}

void Window::message(std::vector<std::string>& messageparts) {
	framegrabber->iomanager->error(name, "message not implemented");
}

bool Window::save() {
	return true;
}
