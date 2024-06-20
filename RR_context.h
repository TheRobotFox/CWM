#pragma once
#include "RR.h"
#include "Conscreen/List/List.h"

#include "Conscreen/Conscreen_screen.h"
typedef Conscreen_pixel RR_pixel;

typedef void (*RR_set_function)(RR_context context, int16_t x, int16_t y, RR_pixel pixel);
typedef RR_pixel (*RR_get_function)(RR_context context, int16_t x, int16_t y);
void RR_set(RR_context context, int16_t x, int16_t y, RR_pixel pixel);
RR_pixel RR_get(RR_context context, int16_t x, int16_t y);

void RR_data_set(RR_context context, void* data);
void* RR_data_get(RR_context context);

RR_point RR_size_get(RR_context context);

void RR_render(RR_context context, RR_point new_size, RR_get_function getScreen,
               RR_set_function setScreen, void *data);

RR_context RR_context_create();
void RR_context_free(RR_context context);
void RR_context_chain_set(RR_context context, LIST(RR_renderer) renderers);
void RR_context_make_chain(RR_context context, int count, ...);

void RR_set_default(RR_context context, int16_t x, int16_t y, RR_pixel pixel);
RR_pixel RR_get_default(RR_context context, int16_t x, int16_t y);

RR_pixel RR_get_offset(RR_context ctx, int16_t x, int16_t y);
void RR_set_offset(RR_context ctx, int16_t x, int16_t y, RR_pixel p);
