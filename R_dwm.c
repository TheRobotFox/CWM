#include "R_dwm.h"
#include "Conscreen/List/List.h"
#include "RR.h"
#include "RR_context.h"
#include "RR_renderer.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "Conscreen/List/Heap.h"

typedef struct{
	float x,y;
} f32vec2;


enum DWM_window_point_type{
	DWM_ABSOLUTE,
	DWM_RELATIVE
};

typedef struct{
	enum DWM_window_point_type type;
	union{
		RR_point absolute;
		f32vec2 relative;
	};
} DWM_point;


typedef struct _DWM_window {
    DWM_point pos;
    DWM_point size;
    int z_depth;
    RR_context render_chain;
} Window;

static RR_point DWM_window_pos_get(RR_context ctx, DWM_window w)
{
	RR_point pos;

    switch (w->pos.type)
    {

    case DWM_ABSOLUTE:
        pos.x = w->pos.absolute.x;
        pos.y = w->pos.absolute.y;
        break;

    case DWM_RELATIVE: {

            RR_point parrent_size = RR_size_get(ctx);
            pos.x = parrent_size.x * w->pos.relative.x;
            pos.y = parrent_size.y * w->pos.relative.y;
            break;
        }

    }
	return pos;
}

static int16_t min(int16_t a, int16_t b)
{
    if(a<b) return a;
    return b;
}
static RR_point DWM_window_size_get(RR_context ctx, DWM_window w)
{
	RR_point size;

    RR_point parrent_size = RR_size_get(ctx);
    RR_point pos=DWM_window_pos_get(ctx, w);
    switch (w->size.type)
    {
    case DWM_ABSOLUTE:
        size.x = w->size.absolute.x;
        size.y = w->size.absolute.y;
        break;
    case DWM_RELATIVE:
        size.x = ceilf(w->size.relative.x*parrent_size.x);
        size.y = ceilf(w->size.relative.y*parrent_size.y);
        break;
    }

    size.x = min(size.x, parrent_size.x-pos.x);
    size.y = min(size.y, parrent_size.y-pos.y);
	return size;
}

struct DWM_render_info {
    RR_context origin;
    RR_point pos;
};
static void dwm_forward_set(RR_context ctx, int16_t x, int16_t y, RR_pixel p)
{
    struct DWM_render_info *info = RR_data_get(ctx);
    RR_set(info->origin, x+info->pos.x, y+info->pos.y, p);
}
static RR_pixel dwm_forward_get(RR_context ctx, int16_t x, int16_t y)
{
    struct DWM_render_info *info = RR_data_get(ctx);
    return RR_get(info->origin, x+info->pos.x, y+info->pos.y);
}

static void DWM_internal_window_render(RR_context ctx, DWM_window w)
{
	if(!w->render_chain) return;
    struct DWM_render_info info = {ctx, DWM_window_pos_get(ctx, w)};

    RR_render(w->render_chain,
              DWM_window_size_get(ctx, w),
              dwm_forward_get,
              dwm_forward_set,
              &info);
}

static void RR_renderer_dynamicWM(RR_context context, void *data){

	HEAP_LOOP(DWM_window, data, ptr){
        DWM_internal_window_render(context, *ptr);
    }
}

static int DWM_internal_window_depth_cmp(void* a, void* b)
{
	DWM_window _a=*(DWM_window*)a, _b=*(DWM_window*)b;
	return _a->z_depth < _b->z_depth;
}

RR_renderer R_dwm(){
    RR_renderer renderer = RR_renderer_create(RR_renderer_dynamicWM);
    RR_renderer_data_set(renderer,
                         Heap_create(sizeof(DWM_window), DWM_internal_window_depth_cmp));
    return renderer;
}

void R_dwm_free(RR_renderer r)
{
    Heap_free(RR_renderer_data_get(r));
    RR_renderer_free(r);
}

void DWM_register(RR_renderer dwm, DWM_window w, int z_pos) {
    w->z_depth = z_pos;
    Heap_add(RR_renderer_data_get(dwm), &w);
}

void DWM_unregister(RR_renderer dwm, DWM_window w) {
    Heap_rme(RR_renderer_data_get(dwm), &w);
}

DWM_window DWM_window_create() {

    DWM_window w = malloc(sizeof(struct _DWM_window));
    w->render_chain=NULL;

    w->pos = (DWM_point) {
      .type = DWM_RELATIVE,
      .relative = {0,0}};

    w->size = (DWM_point) {
      .type = DWM_RELATIVE,
      .relative = {1,1}};

    w->z_depth=0;
    return w;
}

void DWM_window_pos_set_abs(DWM_window w, int16_t x, int16_t y) {
  w->pos.type = DWM_ABSOLUTE;
  w->pos.absolute.x = x;
  w->pos.absolute.y = y;
}
void DWM_window_pos_set_rel(DWM_window w, float x, float y) {
  w->pos.type = DWM_RELATIVE;
  w->pos.relative.x = x;
  w->pos.relative.y = y;
}
void DWM_window_size_set_abs(DWM_window w, int16_t x, int16_t y) {
  w->size.type = DWM_ABSOLUTE;
  w->size.absolute.x = x;
  w->size.absolute.y = y;
}
void DWM_window_size_set_rel(DWM_window w, float x, float y) {
  w->size.type = DWM_RELATIVE;
  w->size.relative.x = x;
  w->size.relative.y = y;
}
void DWM_window_renderer_set(DWM_window w, RR_context ctx) {
    w->render_chain = ctx;
}
