#pragma once
#include "RR_context.h"

typedef void (*RR_render_function)(RR_context context, void *data);


RR_renderer RR_renderer_create(RR_render_function func);
void RR_renderer_data_set(RR_renderer renderer, void *data);
void* RR_renderer_data_get(RR_renderer renderer);
void RR_renderer_free(RR_renderer r);
void RR_renderer_render(RR_renderer r, RR_context context);
