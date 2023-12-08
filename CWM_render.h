#pragma once
#include "CWM.h"
typedef i16vec2 (*CWM_render_function)(CWM_window w, Conscreen_pixel *content, void *data);
typedef struct CWM_renderer* CWM_renderer;

CWM_renderer CWM_renderer_create(CWM_render_function func);
void CWM_renderer_data_set(CWM_renderer renderer, void *data);
