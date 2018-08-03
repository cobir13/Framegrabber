//
//  focusergraph.hpp
//  Framegrabber
//
//  Created by Thomas Malthouse on 8/2/18.
//  Copyright © 2018 Thomas Malthouse. All rights reserved.
//

#ifndef focusergraph_hpp
#define focusergraph_hpp
#pragma clang diagnostic ignored "-Wdocumentation"

#include <stdio.h>
#include <stdint.h>
#include <SDL.h>
#include "SDL_FontCache.h"
#include "framegrabber_app.h"
#include "framegrabber.h"

#define PIXEL_COUNT (9)


class FocuserGraph : public FramegrabberApp {
  uint16_t window_x, window_y;
  uint16_t center_x, center_y;
  uint16_t min, max;
  
  uint16_t plot_x, plot_y;
  uint16_t margin_t, margin_b, margin_l, margin_r;
  
  uint32_t ms_per_frame;
  uint16_t pixels[PIXEL_COUNT];
  Framegrabber *grabber;
  SDL_Window *window;
  SDL_Surface *surface;
  SDL_Renderer *renderer;
  uint32_t last_update;
  bool should_update();
  bool updated;
  
  uint8_t ticksize;
  
  FC_Font *font;

public:
  FocuserGraph(Framegrabber *g, int x_center, int y_center, int min, int max);
  FocuserGraph(Framegrabber *g, std::vector<std::string> &args);
  
  bool set_frame(uint16_t *data);
  bool save();
  void update();
  
private:
  void init(Framegrabber *g, int x_center, int y_center, int min, int maxs);
  void calculate_plotsize();
  void draw_plotbox();
  void draw_x_ticks();
  
};

#endif /* focusergraph_hpp */
