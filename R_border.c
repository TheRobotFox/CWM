#include "R_border.h"
#include "Conscreen/Conscreen_ANSI.h"
#include "RR.h"
#include "RR_context.h"
#include "RR_renderer.h"
#include "CWM_internal.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    Conscreen_ansi border_style;
} Border_info;

static void render_border(RR_context ctx, void *data)
{
	RR_point pos  = {0,0},
		size = RR_size_get(ctx);

    Border_info *info = data;

	if(size.x && size.y){

		Conscreen_ansi attr=info->border_style;
		Conscreen_pixel corners = {CHR('+'), attr}, vertical = {CHR('|'), attr}, horizontal = {CHR('-'), attr};

		RR_set(ctx, pos.x,pos.y, corners);
		RR_set(ctx, pos.x+size.x-1, pos.y, corners);
		RR_set(ctx, pos.x, pos.y+size.y-1, corners);
		RR_set(ctx, pos.x+size.x-1, pos.y+size.y-1, corners);

		for(int i=1; i<size.x-1; i++){
			RR_set(ctx, pos.x+i,pos.y+size.y-1, horizontal);
			RR_set(ctx, pos.x+i,pos.y, horizontal);
		}

		for(int i=1; i<size.y-1; i++){
			RR_set(ctx, pos.x,pos.y+i, vertical);
			RR_set(ctx, pos.x+size.x-1,pos.y+i, vertical);
		}
	}
	RR_point start = {1,1};
	RR_render(ctx, (RR_point){size.x-2, size.y-2}, RR_get_offset, RR_set_offset , &start);
}

RR_renderer R_border()
{
	RR_renderer r = RR_renderer_create(render_border);
	Border_info *data = malloc(sizeof(Border_info));
	data->border_style = CONSCREEN_ANSI_DEFAULT(255, 0, 255);
	data->border_style.background = CONSCREEN_RGB(0, 255, 255);
	RR_renderer_data_set(r, data);
	return r;
}
void R_border_free(RR_renderer r)
{
	free(RR_renderer_data_get(r));
	RR_renderer_free(r);
}

void R_border_set_style(RR_renderer r, Conscreen_ansi style)
{
	Border_info *info = RR_renderer_data_get(r);
	info->border_style=style;
}
