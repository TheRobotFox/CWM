#include "R_opacity.h"
#include "Conscreen/Conscreen_ANSI.h"
#include "RR.h"
#include "RR_context.h"
#include "RR_renderer.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


static int char_is_empty(Conscreen_char c){
	switch(c){
	case ' ':
	case '\n':
		return 1;
	default:
		return 0;

	}
}


static Conscreen_color color_blend(Conscreen_color a, Conscreen_color b, float opacity)
{

	Conscreen_color c;
	c.r=a.r*(1-opacity)+b.r*opacity;
	c.g=a.g*(1-opacity)+b.g*opacity;
	c.b=a.b*(1-opacity)+b.b*opacity;
	return c;
}

static void R_opacity_setScreen(RR_context context, int16_t x, int16_t y, RR_pixel pixel)
{
    RR_point size = RR_size_get(context);
    RR_pixel (*screen)[size.y] = RR_data_get(context);
    screen[x][y] = pixel;
}
static RR_pixel R_opacity_getScreen(RR_context context, int16_t x, int16_t y)
{
    RR_point size = RR_size_get(context);
    RR_pixel (*screen)[size.y] = RR_data_get(context);
    return screen[x][y];
}

static void R_opacity_render(RR_context context, void *data)
{
    RR_point size = RR_size_get(context);
    if(!size.x*size.y) return;
    RR_pixel (*screen)[size.y] = malloc(sizeof(RR_pixel[size.x][size.y]));

    for(size_t y=0; y<size.y; y++){
        for(size_t x=0; x<size.x; x++){
            screen[x][y]=RR_get(context, x,y);
        }
    }

    RR_render(context, size, R_opacity_getScreen, R_opacity_setScreen, screen);

    float opacity = *(float*)data;

    for(size_t y=0; y<size.y; y++){
        for(size_t x=0; x<size.x; x++){
            RR_pixel pixel = screen[x][y];
            RR_pixel current = RR_get(context, x, y);
            if(char_is_empty(pixel.character)){
                pixel.character=current.character;
                pixel.style.forground = color_blend(current.style.forground, pixel.style.background, opacity);
            }
            pixel.style.background = color_blend(current.style.background, pixel.style.background, opacity);
            RR_set(context, x, y, pixel);
        }
    }
    free(screen);
}

RR_renderer R_opacity()
{
    RR_renderer r = RR_renderer_create(R_opacity_render);
    float *opacity = malloc(sizeof(float));
    *opacity=0.75;
    RR_renderer_data_set(r, opacity);
    return r;
}

void R_opacity_set(RR_renderer r, float opacity)
{
    float *data = RR_renderer_data_get(r);
    *data=opacity;
}

void R_opacity_free(RR_renderer r)
{
    free(RR_renderer_data_get(r));
    RR_renderer_free(r);
}
