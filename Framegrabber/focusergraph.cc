//
//  focusergraph.cpp
//  Framegrabber
//
//  Created by Thomas Malthouse on 8/2/18.
//  Copyright © 2018 Thomas Malthouse. All rights reserved.
//

#include "focusergraph.h"

void FocuserGraph::calculate_plotsize() {
  plot_x = window_x - (margin_r+margin_l);
  plot_y = window_y - (margin_t+margin_b);
}

void FocuserGraph::draw_plotbox() {
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
    FC_Draw(font, renderer, xpos-5, xaxis-10, "%d", i);
  }
  
  // Draw ticks along the y axis
  int yaxis = margin_l;
  int tick_left = yaxis - ticksize/2;
  int tick_right = yaxis + ticksize/2;
  int diff = max-min;
  for (uint16_t i=0; i<diff; i++) {
    int ypos = (window_y-margin_b) - i*(plot_y/(diff-1));
    SDL_RenderDrawLine(renderer, tick_left, ypos, tick_right, ypos);
    FC_Draw(font, renderer, yaxis-10, ypos, "%d", min + i);
  }
}
