#include "R_info.h"
#include "CWM_internal.h"
#include "Conscreen/Conscreen_ANSI.h"
#include "Conscreen/Conscreen_screen.h"
#include "Conscreen/Conscreen_string.h"
#include "RR.h"
#include "RR_renderer.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

struct Error_display
{
	Conscreen_char *name;
	Conscreen_ansi style;
};

static struct Error_display errors[] = {
[NONE]={},
[INFO] = {STR("INFO"), CONSCREEN_ANSI_DEFAULT(0,128,255)},
	[WARNING] = {STR("WARNING"), CONSCREEN_ANSI_DEFAULT(255,180,0)},
	[ERROR] = {STR("ERROR"), CONSCREEN_ANSI_DEFAULT(255,50,50)},
	[FATAL] = {STR("FATAL"), CONSCREEN_ANSI_DEFAULT(255,0,0)}
};


const Conscreen_char* clamps[4] = {
    STR(">|%|<"),
    STR("<[%]>"),
    #ifdef CONSCREEN_CHAR
    STR("v%^"),
    STR("v%^")
    #elifdef CONSCREEN_WCHAR
    STR("∨—%—∧"),
    STR("∨—%—∧")
    #endif
};

static void print_side(RR_context ctx,
                       enum INFO_side side,
                       size_t offset,
                       const Conscreen_char*const text,
                       size_t length,
                       Conscreen_ansi *style)
{
    RR_point pos={0},
        size = RR_size_get(ctx);
    int16_t *axies,
        direction;

    switch (side) {
        case INFO_TOP:
            direction=1;
            axies = &pos.x;
            break;
        case INFO_LEFT:
            direction=1;
            axies = &pos.y;
            break;
        case INFO_BOTTOM:
            pos.y=size.y-1;
            direction=1;
            axies = &pos.x;
            break;
        case INFO_RIGHT:
            pos.x=size.x-1;
            direction=1;
            axies = &pos.y;
    }
    *axies +=offset*direction;

	Conscreen_pixel p;
    if(style) p.style=*style;
    for(size_t i=0; i<length; i++) {
        p.character = text[i];
        Conscreen_pixel current = RR_get(ctx, pos.x, pos.y);
        if(!style)
            p.style=current.style;
        /* else */
        /*     p.style.background=current.style.background; */
        RR_set(ctx, pos.x,  pos.y, p);
        *axies+=direction;
    }
}

typedef struct {
    CWM_string string;
    BW_error_level level;
    enum INFO_side side;
    enum INFO_align align;
    const char* clamp;
} _INFO_info;
typedef _INFO_info* INFO_info;


static int16_t get_offset(enum INFO_side side, enum INFO_align align, size_t length, RR_point size)
{
    int16_t offset;
    int16_t *space;
    switch (side) {
        case INFO_TOP:
        case INFO_BOTTOM:
            space = &size.x; break;
        case INFO_LEFT:
        case INFO_RIGHT:
            space = &size.y; break;
    }
    switch (align) {
        case INFO_START:
            offset = mini16(1, *space-length-2); break;
        case INFO_CENTER:
            offset = mini16((*space-length-2)/2, *space-length-2); break;
        case INFO_END:
            offset = *space-length-1;
            if(offset<0) offset=*space-length-2; break;
    }
    return offset;
}

static void render_info(RR_context ctx, void *data)
{
    RR_point size = RR_size_get(ctx);
    RR_render(ctx, size, RR_get_default, RR_set_default, NULL);

    INFO_info info = data;

    int clamp_start_len,
             clamp_end_len,
            tag_len=0,
            trunc=0;


    // eval clamp
    const char *clamp = info->clamp;
    if(!clamp) clamp = clamps[info->side];

    size_t i=0;
    while(i<strlen(clamp) && clamp[i]!='%')
        i++;
    clamp_start_len = i;
    clamp_end_len = maxi16((int)strlen(clamp)-i-1, 0);

    // eval tag
    struct Error_display tag;
    if(info->level!=NONE){
        tag = errors[info->level];
        tag_len = strlen(tag.name)+3; // strlen+3 -> '['+tag+']'+' '
    }

    // calc offset and truncate
    double offset = get_offset(info->side,
                                info->align,
                                info->string.len
                                            +clamp_start_len
                                            +clamp_end_len
                                            +tag_len,
                                size);
    if(offset<0){
        trunc = -1*offset;
        offset = 1;
        if(trunc>=info->string.len)
            return; // stop if too little space
    }

    // draw info
    print_side(ctx, info->side, offset, clamp, clamp_start_len, NULL);
    offset+=clamp_start_len;
    if(info->level!=NONE){
        Conscreen_ansi normal = CONSCREEN_ANSI_NORMAL;
        print_side(ctx, info->side, offset, "[", 1, &normal);
        print_side(ctx, info->side, offset+1, tag.name, strlen(tag.name), &tag.style);
        print_side(ctx, info->side, offset+1+strlen(tag.name), "] ", 2, &normal);
        offset+=tag_len;
    }
    print_side(ctx, info->side, offset, info->string.str, info->string.len-trunc, &info->string.style);
    offset+=info->string.len-trunc;
    print_side(ctx, info->side, offset, clamp+clamp_start_len+1, clamp_end_len, NULL);
}
RR_renderer R_info()
{
    RR_renderer r = RR_renderer_create(render_info);
    INFO_info info = malloc(sizeof(_INFO_info));
    info->string = (CWM_string){0};
    info->string.style = CONSCREEN_ANSI_NORMAL;
    info->clamp=NULL;
    info->side=INFO_TOP;
    info->align=INFO_CENTER;
    info->level=NONE;
    RR_renderer_data_set(r, info);
    return r;
}

void R_info_free(RR_renderer r)
{
    INFO_info info = RR_renderer_data_get(r);
    if(info->string.str) free(info->string.str);
    free(info);
    RR_renderer_free(r);
}

void R_info_set_text(RR_renderer r, const char*const name, size_t length)
{
    INFO_info info = RR_renderer_data_get(r);
    CWM_internal_string_set(&info->string, name, length);
}

void R_info_set_level(RR_renderer r, BW_error_level level)
{
    INFO_info info = RR_renderer_data_get(r);
    info->level = level;
}

void R_info_set_style(RR_renderer r, Conscreen_ansi style)
{
    INFO_info info = RR_renderer_data_get(r);
    info->string.style=style;
}

void R_info_set_clamp(RR_renderer r, const char*const clamp)
{
    INFO_info info = RR_renderer_data_get(r);
    info->clamp=clamp;
}
void R_info_set_side(RR_renderer r, enum INFO_side side)
{
    INFO_info info = RR_renderer_data_get(r);
    info->side=side;
}
void R_info_set_align(RR_renderer r, enum INFO_align align)
{
    INFO_info info = RR_renderer_data_get(r);
    info->align=align;
}

void R_info_vprintf(RR_renderer r, const Conscreen_char*const format, va_list arg)
{
    INFO_info info = RR_renderer_data_get(r);
	Conscreen_string string = Conscreen_string_create();
	Conscreen_string_vsprintf(string, format, arg);
	CWM_internal_string_set(&info->string,Conscreen_string_start(string),Conscreen_string_length(string));
    Conscreen_string_free(string);
}
void R_info_printf(RR_renderer r, const Conscreen_char*const format, ...)
{
	va_list arg;
	va_start(arg, format);
	R_info_vprintf(r, format, arg);
	va_end(arg);
}
void R_info_text_get(RR_renderer r, const char** out, int *len)
{
    INFO_info info = RR_renderer_data_get(r);
    *out = info->string.str;
    *len = info->string.len;
}
