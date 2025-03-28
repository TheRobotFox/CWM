#include "AM.h"
#include "BasicWindow.h"
#include "BasicWindow_internal.h"
#include "CWM.h"
#include "CWM_internal.h"
#include "Conscreen/Conscreen_ANSI.h"
#include "Conscreen/Conscreen_string.h"
#include "Conscreen/List/List.h"
#include "RR.h"
#include "RR_renderer.h"
#include "R_border.h"
#include "R_dwm.h"
#include "R_info.h"
#include "R_list.h"
#include "WM_i3.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _App
{
    _BasicWindow window;
    RR_context chain;
    const char *name;
    void *data;
    AM_callback callback;
} _App;

enum AM_state
{
    INSERT, NAVIGATE
};

typedef struct {
    _BasicWindow win;
    LIST(App) apps;
    I3_context i3;
    enum AM_state state;
} _AM_window;
typedef _AM_window* AM_window;

static void AM_highlight(App app, bool highlight)
{
    BW_renderer_toggle(app, "BORDER", !highlight);
    BW_renderer_toggle(app, "AM_HIGHLIGHT", highlight);
    BW_gen_chain(app);
}

void AM_internal_app_free(App app)
{
    R_border_free(BW_get_renderer(app, "AM_HIGHLIGHT"));
    BW_internal_free(app);
}

static void selector_callback(App al, char key, void *data)
{
    AM_window am = data;
    RR_renderer renderer = BW_get_renderer(al, "AM_SELECT");
    switch (key) {
        case 'j': R_list_down(renderer); break;
        case 'k': R_list_up(renderer); break;
        case '\n': {
            // overwrite self
            _App *app = *(_App**)List_at(am->apps, R_list_get(renderer));
            WM_i3_set(am->i3, (App)app);
            AM_highlight((App)app, true);
            AM_internal_app_free(al);
            R_list_free(renderer);
        }
    }
}

// TODO set app to RR_context
// create context renderer for App Views
// Maybe extract app from App viewer?

static char* app_format(void *element)
{
    _App *app = *(_App**)element;
    app->name = "Test";

    int len = strlen(app->name)+1;
    char *res = malloc(len);
    memcpy(res, app->name, len);
    return res;
}

static void AM_select(AM_window am)
{
    App selector = AM_app_create("Choose App", selector_callback);
    RR_renderer list_render = R_list();
    AM_app_data_set(selector, am);
    R_list_set(list_render, am->apps, app_format);
    BW_add(selector, "AM_SELECT", CR_POST, list_render);
    BW_renderer_toggle(selector, "TEXT", false);
    BW_renderer_toggle(selector, "DWM", false);
    BW_gen_chain(selector);
    WM_i3_set(am->i3, selector);
    AM_highlight(selector, true);
}

AM AM_create()
{
    AM_window am = malloc(sizeof(_AM_window));
    BW_create(&am->win);

    BW_renderer_toggle(&am->win, "TEXT", false);
    BW_renderer_toggle(&am->win, "TITLE", false);
    BW_renderer_toggle(&am->win, "BORDER", false);
    BW_gen_chain(&am->win);

    am->i3 = WM_i3(BW_get_renderer(&am->win, "DWM"));
    am->state = INSERT;
    am->apps = LIST_create(_App*);

    AM_select(am);
    return (BasicWindow)am;
}

void AM_free(AM _am)
{
    AM_window am = (AM_window)_am;
    WM_i3_free(am->i3);
    List_free(am->apps);
    BW_internal_free(_am);
    free(_am);
}

App AM_app_create(const char *name, AM_callback callback)
{
    _App *app = malloc(sizeof(_App));
    BW_create(&app->window);

    RR_renderer highlight = R_border();
    Conscreen_ansi highlight_style = CONSCREEN_ANSI_NORMAL;
    highlight_style.background = CONSCREEN_RGB(128, 128, 255);
    R_border_set_style(highlight, highlight_style);
    BW_add(&app->window, "AM_HIGHLIGHT", CR_PRE, highlight);
    BW_renderer_toggle(&app->window, "AM_HIGHLIGHT", false);
    BW_title_set(&app->window, name, strlen(name));
    BW_gen_chain(&app->window);

    app->data = NULL;
    /* app->magic=69; */
    app->callback = callback;
    return (BasicWindow)app;
}


void* AM_app_data_get(App _app)
{
    _App *app = (_App*)_app;
    return app->data;
}
void AM_app_data_set(App _app, void *data)
{
    _App *app = (_App*)_app;
    app->data = data;
}
void AM_app_free(App app)
{
    AM_internal_app_free(app);
    free(app);
}

void AM_register(AM _am, App app)
{
    AM_window am = (AM_window)_am;
    List_push(am->apps, &app);
}
void AM_unregister(AM _am, App app)
{
    AM_window am = (AM_window)_am;
    List_rme(am->apps, &app);
}

void AM_key_event(AM _am, char key)
{
    if(!key) return;
    AM_window am = (AM_window)_am;
    _App *app = (_App*)WM_i3_get(am->i3);
    if(key==23)
        am->state=NAVIGATE;
    else switch(am->state)
    {
        case INSERT:
        {
            if(app->callback)
                app->callback((App)app, key, app->data);
        } break;
        case NAVIGATE:
        {
            // movement
            switch (key) {
                case 8:
                case 'h': WM_i3_select(am->i3, I3_LEFT); break;
                case 12:
                case 'l': WM_i3_select(am->i3, I3_RIGHT); break;
                case 10:
                case 'j': WM_i3_select(am->i3, I3_DOWN); break;
                case 11:
                case 'k': WM_i3_select(am->i3, I3_UP); break;
                case 19:
                case 's': WM_i3_split(am->i3, I3_DOWN); AM_select(am); break;
                case 22:
                case 'v': WM_i3_split(am->i3, I3_RIGHT); AM_select(am); break;
                case 'c': WM_i3_kill(am->i3); break;
                case 1:
                case 'a': WM_i3_select_parent(am->i3); goto reset;
                case ' ': AM_select(am); goto reset;
                default:
                    BW_error(&am->win, WARNING, "Invalid Command '%c'(%d) (use h,j,k,l,c,s,v,' ')", key, key);
                    goto reset;
            };
            // highlight new App
            AM_highlight((App)app, false);
            App new = WM_i3_get(am->i3);
            AM_highlight(new, true);

            reset:
            am->state=INSERT;
        }
    }
}

void AM_prepare(AM _am)
{
    AM_window am = (AM_window)_am;
    WM_i3_prepare(am->i3);
}
