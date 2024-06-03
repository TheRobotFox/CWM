#pragma once
#include "CWM.h"
typedef void (*CWM_render_function)(Conscreen_pixel *content, i16vec2 size, void *data);

struct _CWM_renderer;
typedef struct _CWM_renderer* CWM_renderer;

CWM_renderer CWM_renderer_create(CWM_render_function func);
void CWM_renderer_data_set(CWM_renderer renderer, void *data);
