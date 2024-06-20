#pragma once

#include "RR.h"
#include "Conscreen/Conscreen.h"
#include <stddef.h>

struct _DWM_window;
typedef struct _DWM_window *DWM_window;

RR_renderer R_dwm();
void R_dwm_free(RR_renderer r);

DWM_window DWM_window_create();

void DWM_register(RR_renderer dwm, DWM_window w, int z_pos);
void DWM_unregister(RR_renderer dwm, DWM_window w);

void DWM_window_pos_set_abs(DWM_window w, int16_t x, int16_t y);
void DWM_window_pos_set_rel(DWM_window w, float x, float y);
void DWM_window_size_set_abs(DWM_window w, int16_t x, int16_t y);
void DWM_window_size_set_rel(DWM_window w, float x, float y);
void DWM_window_renderer_set(DWM_window w, RR_context ctx);
