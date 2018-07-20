#include "window.h"

static inline uint32_t grey16_to_rgba32(uint16_t pix) {
	uint8_t pix8 = pix / 256;
	uint8_t result[4];
	result[0] = SDL_ALPHA_OPAQUE;
	result[1] = pix8;
	result[2] = pix8;
	result[3] = pix8;
	return *(uint32_t*)(&result);
}

Window::Window(Framegrabber *grabber) { init(grabber); }
Window::Window(Framegrabber *grabber, const char *input) { init(grabber); }


void Window::init(Framegrabber *grabber) {
	framegrabber = grabber;
	done = false;
	
	ms_per_frame = DEFAULT_MSPF;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		throw std::runtime_error("Could not initialize SDL2");
	}

	window = SDL_CreateWindow(
		"Framegrabber Display",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		grabber->width * 2,
		grabber->height * 2,
		SDL_WINDOW_SHOWN);
	if (!window) {
		throw std::runtime_error("Could not create SDL window");
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		throw std::runtime_error("Could not create SDL renderer");
	}
	
	//Set background color to black
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);

	texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_STREAMING,
		grabber->width,
		grabber->height
	);
	if (!texture) {
		throw std::runtime_error("Could not create SDL texture");
	}
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
	if (!should_update()) {
		return true;
	}


	uint32_t *vidbuf;
	int pitch;
	SDL_LockTexture(texture, NULL, (void**)&vidbuf, &pitch);
	for (uint32_t i = 0; i < (framegrabber->width * framegrabber->height); i++) {
		vidbuf[i] = grey16_to_rgba32(data[i]);
	}
	SDL_UnlockTexture(texture);

	last_update = SDL_GetTicks();
	updated = true;
	return true;
}

void Window::update() {
	if (!updated) return;

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT) { done = true; }
	}
}