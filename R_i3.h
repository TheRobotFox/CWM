#pragma once

#include "BasicWindow.h"
typedef enum {
I3_LEFT=0,I3_UP=1,I3_RIGHT=2,I3_DOWN=3
} I3_direction;

RR_renderer R_i3();
void R_i3_split(RR_renderer r, I3_direction direction);
bool R_i3_select(RR_renderer r, I3_direction direction);
bool R_i3_kill(RR_renderer r);
bool R_i3_select_parent(RR_renderer r);
void R_i3_set(RR_renderer r, BasicWindow win);
BasicWindow R_i3_get(RR_renderer r);
void R_i3_free(RR_renderer r);
