#pragma once

#include "BasicWindow.h"
typedef enum {
I3_LEFT=0,I3_UP=1,I3_RIGHT=2,I3_DOWN=3
} I3_direction;

struct _I3_context;
typedef struct _I3_context* I3_context;

I3_context WM_i3(RR_renderer dwm);
void WM_i3_split(I3_context ctx, I3_direction direction);
bool WM_i3_select(I3_context ctx, I3_direction direction);
bool WM_i3_kill(I3_context ctx);
bool WM_i3_select_parent(I3_context ctx);
void WM_i3_set(I3_context ctx, BasicWindow win);
BasicWindow WM_i3_get(I3_context ctx);
void WM_i3_free(I3_context ctx);

void WM_i3_prepare(I3_context ctx);
