//
//  focusergraph.cpp
//  Framegrabber
//
//  Created by Thomas Malthouse on 8/2/18.
//  Copyright © 2018 Thomas Malthouse. All rights reserved.
//

#include "focusergraph.h"
#include "exceptions.h"
#include "iomanager.h"
#include <stdlib.h>

FocuserGraph::FocuserGraph(Framegrabber *g, int x_c, int y_c, int min, int max) {
  init(g, x_c, y_c, min, max);
}

FocuserGraph::FocuserGraph(Framegrabber *g, std::vector<std::string> &arglist) {
  int x_c, y_c, min, max;
  if (arglist.size() != 4) {
    throw bad_parameter_exception("FocuserGraph requires four arguments");
  }
  
  try {
    x_c = stoi(arglist[0]);
    y_c = stoi(arglist[1]);
    min = stoi(arglist[2]);
    max = stoi(arglist[3]);
  } catch (std::invalid_argument &iarg) {
    throw bad_parameter_exception(iarg.what());
  }
  catch (std::out_of_range &oor) {
    throw bad_parameter_exception(oor.what());
  }
  
  init(g, x_c, y_c, min, max);
}

FocuserGraph::~FocuserGraph() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
}

void FocuserGraph::init(Framegrabber *g, int x_c, int y_c, int min_a, int max_a) {
  name = "FocuserGraph";
  id = get_id();
  grabber = g;
  center_x = x_c;
  center_y = y_c;
  min = min_a;
  max = max_a;
  font = FC_CreateFont();
  ms_per_frame = 50;
  margin_b = 80;
  margin_t = 80;
  margin_l = 80;
  margin_r = 80;
  window_x = 600;
  window_y = 600;
  ticksize = 10;
  
  if (center_x < PIXEL_COUNT/2 || center_x > (grabber->width-PIXEL_COUNT/2)) {
    throw bad_parameter_exception("Center_x must not be within 4 pixels of the edge");
  }
  
  auto &graphconfig = grabber->config.focusergraph;
  
  if (!SDL_WasInit(SDL_INIT_VIDEO) && SDL_Init(SDL_INIT_VIDEO) < 0) {
    
    throw std::runtime_error("Could not initialize SDL2");
  }
  
  //No antialiasing for us (not that it matters for this one
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
  
  window = SDL_CreateWindow("Focuser Graph", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_x, window_y, SDL_WINDOW_SHOWN);
  if (!window) {
    throw std::runtime_error("Could not create FocuserGraph window!");
  }
  
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    throw std::runtime_error("Could not create SDL renderer");
  }
  
  if (!FC_LoadFont(font, renderer, grabber->config.fg_config.font.c_str(),
                   graphconfig.font_size, FC_MakeColor(0, 0, 0, 255), TTF_STYLE_NORMAL)) {
    throw bad_parameter_exception("Could not load font");
  }
  
  //Set background color to white
  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
  

}

bool FocuserGraph::set_frame(uint16_t *data) {
  size_t offset = center_y * grabber->width + center_x - PIXEL_COUNT/2;
  memcpy(pixels, data+offset, PIXEL_COUNT * sizeof(uint16_t));
  return true;
}

bool FocuserGraph::save() {
  return true;
}

void FocuserGraph::update() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
      case SDL_WINDOWEVENT:
        switch (e.window.event) {
          case SDL_WINDOWEVENT_CLOSE:
            status = FGAPP_DONE;
            break;
            
          default:
            break;
        }
        break;
        
      default:
        break;
    }
  }
  
  SDL_RenderClear(renderer);
  calculate_plotsize();
  draw_plotbox();
  draw_points();
  SDL_RenderPresent(renderer);
}

void FocuserGraph::message(std::vector<std::string> &msg_parts) {
	std::string &dest = msg_parts[0];
	int arg;
	try {
		arg = std::stoi(msg_parts[1]);
	}
	catch (std::invalid_argument) {
		grabber->iomanager->error(name, "Invalid argument value");
		return;
	}
	catch (std::out_of_range) {
		grabber->iomanager->error(name, "Argument value out of range");
		return;
	}

	if (dest == "min") {
		if (0 <= arg && arg < max) {
			min = arg;
			grabber->iomanager->success(name, "Set min");
		}
		else {
			grabber->iomanager->error(name, "new min must be between 0 and max");
		}
	}
	else if (dest == "max") {
		if (min < arg && arg <= UINT16_MAX) {
			max = arg;
			grabber->iomanager->success(name, "Set max");
		}
		else {
			grabber->iomanager->error(name, "New max must be between 0 and UINT16_MAX");
		}
	}
	else {
		grabber->iomanager->error(name, "Could not parse command.");
	}
}

void FocuserGraph::calculate_plotsize() {
  plot_x = window_x - (margin_r+margin_l) + 1;
  plot_y = window_y - (margin_t+margin_b) + 1;
}

void FocuserGraph::draw_plotbox() {
  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
  // Draw the box the plot will be put in
  SDL_Rect plotbox = {margin_l, margin_t, plot_x, plot_y};
  SDL_RenderDrawRect(renderer, &plotbox);
  
  // Draw ticks along the x axis
  int xaxis = window_y - margin_b;
  int tick_top = xaxis - ticksize/2;
  int tick_bottom = xaxis + ticksize/2;
  
  for (uint16_t i=0; i<PIXEL_COUNT; i++) {
    int xpos = margin_l + i*(plot_x/(PIXEL_COUNT-1));
    SDL_RenderDrawLine(renderer, xpos, tick_top, xpos, tick_bottom);
    FC_Draw(font, renderer, xpos-5, xaxis+15, "%d", i - (PIXEL_COUNT/2));
  }
  
  // Draw ticks along the y axis
  int yaxis = margin_l;
  int tick_left = yaxis - ticksize/2;
  int tick_right = yaxis + ticksize/2;
  int diff = max-min;
  for (uint16_t i=0; i<diff; i++) {
    int ypos = (window_y-margin_b) - i*(plot_y/(diff-1));
    SDL_RenderDrawLine(renderer, tick_left, ypos, tick_right, ypos);
    FC_Draw(font, renderer, yaxis-40, ypos - 10, "%d", min + i);
  }
  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
}

void FocuserGraph::draw_points() {
	pixel_outofrange = false;
	SDL_Point points[PIXEL_COUNT];
	for (uint32_t i = 0; i < PIXEL_COUNT; i++) {
		bool thispix_oor = false;
		int xpos = margin_l + i*(plot_x / (PIXEL_COUNT - 1));
		
		int diff = max - min;
		uint16_t rawval = pixels[i];
		int raw_min_dist = rawval - min;
		int ypos = window_y - margin_b - raw_min_dist * (plot_y / diff);
		if (ypos < 0) {
			thispix_oor = true;
			ypos = 0;
		}
		else if (ypos >= window_y) {
			thispix_oor = true;
			ypos = window_y;
		}
		points[i] = { xpos, ypos };

		if (i / (PIXEL_COUNT / 2) == 0 && thispix_oor) {
			pixel_outofrange = true;
		}
	}

	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawLines(renderer, points, PIXEL_COUNT);
	if (pixel_outofrange) {
		FC_DrawColor(font, renderer, 0, 0, { 255, 10, 10, SDL_ALPHA_OPAQUE }, "warning---center pixel out of range");
	}
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
} 

