#include "CWM_internal.h"
#include "Conscreen/Conscreen_ANSI.h"
#include "Conscreen/List/List.h"
#include "RR_context.h"
#include "RR_renderer.h"
#include "R_list.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int selection;
    List list;
    R_list_format format;
} _Info;
typedef _Info* Info;

int max(int a, int b)
{
    return a>b ? a : b;
}
int min(int a, int b)
{
    return a<b ? a : b;
}

static List format_visible_lines(Info info, int n_lines, int *selected)
{
    List lines = LIST_create(List);
    int start = max(0, min(info->selection-n_lines/2, List_size(info->list)-n_lines)),
        count = min(n_lines, List_size(info->list)-start);
    *selected = info->selection-start;
    List_reserve(lines, count);
    for(int i=start; i<count; i++){
        List line = LIST_create(char*);
        char *str = info->format(List_at(info->list, i));
        char *start = strtok(str, "\t");
        while (start){
            List_push(line, &start);
            start = strtok(NULL, "\t");
        }
        List_push(lines, &line);
    }
    return lines;
}

static void render(RR_context ctx, void *data)
{
    // draw lines with supplied renderer
    // -> also works with params :)
    Info info = data;
    if(!info->list) goto end;

    RR_point size = RR_size_get(ctx);

    int highlight;
    LIST(LIST(char*)) lines = format_visible_lines(info, size.y, &highlight);

    // calc col lengths
    List col_len = LIST_create(size_t);
    size_t col;
    LIST_LOOP(LIST(char*), lines, line){
        col=0;
        LIST_LOOP(char*, *line, str){
            size_t *len, str_len=0;
            if(List_size(col_len)<=col) len = List_push(col_len, &str_len);
            else len = List_at(col_len, col);
            str_len = strlen(*str);
            if(str_len>*len) *len = str_len;
            col++;
        }
    }

    // TODO do basic scaling on collums

    int line = 0;
    RR_pixel pix = {.character='#', .style=CONSCREEN_ANSI_NORMAL};
    RR_set(ctx, 0, 0, pix);
    LIST_LOOP(LIST(char*), lines, pline){
        size_t *len = LIST_start(size_t)(col_len),
            offset=0;
        pix.style = CONSCREEN_ANSI_NORMAL;
        if(line==highlight){
            pix.style.background = CONSCREEN_RGB(255, 255, 255);
            pix.style.forground = CONSCREEN_RGB(0, 0, 0);
        }else{
            pix.style.background = CONSCREEN_RGB(0, 0, 0);
            pix.style.forground = CONSCREEN_RGB(255, 255, 255);
        }

        LIST_LOOP(char*, *pline, str){
            size_t write = max(0,min(*len+5, size.x-offset));
            size_t i=0;
            for(; i<write; i++){
                if(i<strlen(*str)) pix.character = (*str)[i];
                else pix.character = '.';

                RR_set(ctx, offset+i, line, pix);
            }
            offset+=i;
            len++;
        }
        free(*LIST_start(char*)(*pline));
        List_free(*pline);
        line++;
    }
    List_free(lines);

end:
    /* RR_render(ctx, RR_size_get(ctx), RR_get_default, RR_set_default, NULL); */
}

RR_renderer R_list()
{
    RR_renderer r = RR_renderer_create(render);
    Info info = malloc(sizeof(_Info));
    info->list = NULL;
    info->format = NULL;
    info->selection = 0;

    RR_renderer_data_set(r, info);

    return r;
}

void R_list_down(RR_renderer r)
{
    Info info = RR_renderer_data_get(r);
    if(info->selection+1<List_size(info->list))
        info->selection++;
}
void R_list_up(RR_renderer r)
{
    Info info = RR_renderer_data_get(r);
    if(info->selection>0)
        info->selection--;
}

size_t R_list_get(RR_renderer r)
{
    Info info = RR_renderer_data_get(r);
    return info->selection;
}

void R_list_set(RR_renderer r, List l, R_list_format format)
{
    Info info = RR_renderer_data_get(r);
    info->list = l;
    info->format = format;
}

void R_list_free(RR_renderer r)
{
    free(RR_renderer_data_get(r));
    RR_renderer_free(r);
}
