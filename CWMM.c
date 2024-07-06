#ifdef AWDA
#include "CWMM.h"
#include "CWM.h"
#include "CWM_internal.h"
#include "Conscreen/Conscreen_ANSI.h"
#include "Conscreen/List/List.h"
#include "RR.h"
#include "RR_renderer.h"
#include "R_border.h"
#include "R_dwm.h"
#include <stdlib.h>

typedef struct _App
{
    CWMM_window window;
    CWMM_callback callback;
} _App;
typedef _App* App;

enum CWMM_pane_type
{
    VERT_SPLIT,HORI_SPLIT,
    CONTENT
};

typedef struct {float x,y;} CWMM_point;
union CWMM_pane_content
{
    struct {
        LIST(_CWMM_pane) panes;
        CWMM_point pos, size;
    };
    CWMM_buffer buffer;
};

typedef struct _CWMM_pane
{
    struct _CWMM_pane *parent;
    enum CWMM_pane_type type;
    union CWMM_pane_content content;
} _CWMM_pane;
typedef _CWMM_pane* CWMM_pane;


enum CWMM_state
{
    INSERT, NAVIGATE
};

typedef struct _CWMM_render_info
{
    enum CWMM_state state;
    _CWMM_pane tree;
    CWMM_pane selected;
    LIST(CWMM_buffer) buffers;
} _CWMM_render_info;

typedef _CWMM_render_info* CWMM_render_info ;


static void CWMM_renderer(RR_context ctx, void *data)
{
    CWMM_render_info info = data;
}

static CWMM_render_info CWMM_get_info(CWMM_window manager)
{
    return RR_renderer_data_get(CWM_window_get_renderer(manager, "CWMM"));
}

CWM_window CWMM_manager_create(CWM_window parent)
{
    CWM_window m = CWM_window_push(parent);

    RR_renderer r = RR_renderer_create(CWMM_renderer);

    CWMM_render_info info = malloc(sizeof(_CWMM_render_info));
    info->buffers=LIST_create(CWMM_buffer);

    RR_renderer_data_set(r, info);

    CWM_window_add(m, "CWMM", CR_POST, r);
    CWM_window_renderer_toggle(m, "TEXT", false);
    CWM_window_renderer_toggle(m, "DWM", false);
    CWM_window_gen_chain(m);

    return m;
}

void CWMM_manager_key_press(CWMM_manager m, char key)
{
    if(key==23)
        m->state=NAVIGATE;
    else switch(m->state)
    {
        case INSERT:
        {
            CWMM_window w=m->selected;
            if(w && w->callback)
                w->callback(key, w->data);
        }
        case NAVIGATE:
        {
            // movement
            // Maybe info border on error
            m->state=INSERT;
        }
    }
}

static void CWMM_window_highlight(CWMM_window w, bool highlight)
{
    CWM_window_renderer_toggle(w, "BORDER", !highlight);
    CWM_window_renderer_toggle(w, "CWMM_HIGHLIGHT", highlight);
}

static void CWMM_views_highlight(CWMM_pane pane,
                                        CWMM_pane selected,
                                        bool highlight)
{
    if(!highlight && pane==selected) highlight=true;
    if(pane->type==CONTENT) CWMM_window_highlight(pane->content.buffer->window, highlight);
    else
        LIST_LOOP(_CWMM_pane, pane->content.panes, p){
            CWMM_views_highlight(p, selected, highlight);
        }
}

static CWMM_buffer CWMM_views_build(CWMM_pane pane,
                           CWMM_point pos,
                           CWMM_point size)
{
    if(pane->type==CONTENT){
        DWM_window frame = pane->content.buffer->window->frame;
        DWM_window_pos_set_rel(frame, pos.x, pos.y);
        DWM_window_size_set_rel(frame, size.x, size.y);
        return pane->content.buffer;
    }

    LIST(CWMM_pane) panes = pane->content.panes;
    size_t div = List_size(panes);

    // if only one member merge up
    if(div==1) return CWMM_views_build(List_start(panes), pos, size);

    pane->content.pos=pos;
    pane->content.size=size;

    float *pos_axies, *size_axies;
    switch(pane->type)
    {
        case HORI_SPLIT:
            pos_axies=&pos.y;
            size_axies=&size.y;
            break;
        case VERT_SPLIT:
            pos_axies=&pos.x;
            size_axies=&size.x;
            break;
        default:
            return NULL;
    }
    *size_axies/=div;
    LIST_LOOP(_CWMM_pane, panes, p){
        CWMM_buffer merge = CWMM_views_build(p, pos, size);

        // merge if possible
        if(merge && p->type!=CONTENT){
            List_free(p->content.panes);
            p->type=CONTENT;
            p->content.buffer=merge;
            p->parent=pane;
        }

        *pos_axies+=*size_axies;
    }
    return NULL;
}
// move up tree until win==manager -> error
// if child in right direction move down

void CWMM_manager_move_up()
{

}
void CWMM_manager_move_down()
{

}
void CWMM_manager_move_left()
{

}
void CWMM_manager_move_right()
{

}

void CWMM_manager_select_up()
{

}
void CWMM_manager_select_down()
{

}
void CWMM_manager_select_left()
{

}
void CWMM_manager_select_right()
{

}

void CWMM_manager_kill()
{

}

void CWMM_manager_split(CWMM_window m, enum CWMM_split split)
{
    CWMM_render_info info = CWMM_get_info(m);
}

CWMM_window CWMM_window_push(CWMM_window manager, CWMM_callback callback)
{
    // setup window
    CWMM_window w = CWM_internal_window_create();

    RR_renderer highlight = R_border();
    Conscreen_ansi highlight_style = CONSCREEN_ANSI_NORMAL;
    highlight_style.background = CONSCREEN_RGB(128, 255, 255);
    R_border_set_style(highlight, highlight_style);
    CWM_window_add(w, "CWMM_HIGHLIGHT", CR_PRE, highlight);
    CWM_window_gen_chain(w);


    // register Buffer
    CWMM_render_info info = CWMM_get_info(manager);

    CWMM_buffer buffer = malloc(sizeof(_CWMM_buffer));
    buffer->window=w;
    buffer->callback=callback;

    List_push(info->buffers, &buffer);
    return w;
}

void CWMM_window_unregister(CWMM_manager m, CWMM_window w)
{
    if(w==m->selected) {
        m->selected = NULL;
    }
    List_rme(m->views, &w);
}

void* CWMM_window_data_get(CWMM_window w)
{
    return w->data;
}
void CWMM_window_data_set(CWMM_window w, void *data)
{
    w->data=data;
}

void CWMM_window_callback_set(CWMM_window w, CWMM_callback callback)
{
    w->callback=callback;
}

static void CWMM_views_free(CWMM_pane pane)
{
    if(pane->type==CONTENT) return; // freed via buffer_list
    LIST_LOOP(_CWMM_pane, pane->content.panes, p){
        CWMM_views_free(p);
    }
    List_free(pane->content.panes);
}

void CWMM_manager_free(CWMM_window m)
{
    CWMM_render_info info = CWMM_get_info(m);
    CWMM_views_free(&info->tree)
    LIST_LOOP(CWMM_buffer, info->buffers, p){
        R_border_free(CWM_window_get_renderer((*p)->window, "CWMM_HIGHLIGHT"));
        CWM_window_remove((*p)->window);
        free(*p);
    }
}

// Renderer ref to child list?
#endif
