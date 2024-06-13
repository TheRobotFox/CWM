#include "CWMM.h"
#include "CWM.h"
#include "CWM_internal.h"
#include "Conscreen/List/List.h"
#include <cstring>
#include <stdlib.h>

enum CWMM_type
{
    NO_SPLIT,VERT_SPLIT,HORI_SPLIT,
    CONTENT
};

enum CWMM_state
{
    INSERT, NAVIGATE
};

struct _CWMM_manager
{
    _CWM_window window;
    enum CWMM_state state;
    LIST(CWMM_window) views;
    CWMM_window selected;
};

struct _CWMM_window
{
    _CWM_window window;
    enum CWMM_type kind;
    CWMM_callback callback;
    CWMM_window content;
};


static CWMM_window CWMM_pane_create()
{
    CWM_window win = CWM_window_create();
    CWMM_window mwin = realloc(win, sizeof(struct _CWMM_window));
    mwin->callback=NULL;
    mwin->kind=NO_SPLIT;
    return mwin;
}

static CWMM_window CWMM_wrapper_create(CWM_window w)
{

}


CWMM_manager CWMM_manager_create()
{
    CWM_window win = CWM_window_create();
    CWMM_manager mwin = realloc(win, sizeof(struct _CWMM_manager));
    mwin->views=LIST_create(CWMM_window);
    mwin->state=INSERT;
    mwin->selected=CWMM_window_create();
    CWM_internal_window_push((CWM_window)mwin, (CWM_window)mwin->selected);
    return mwin;
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

bool f_cmp_pointer(void **a, void **b)
{
    return *a==*b;
}

void CWMM_manager_split(CWMM_manager m, enum CWMM_type split)
{
    CWMM_window s = m->selected;
    CWMM_window parent = s->window.parent;
    CWMM_window new = CWMM_window_create();

    // Normal Split
    if(parent->kind==split || parent->kind==NO_SPLIT) {
        List_push(parent->window.child_windows, &new);
        parent->kind = split;
    } else {
        CWMM_window new_parent = CWMM_window_create();
        CWMM_window *child = List_finde(parent->window.child_window, f_cmp_pointer, s);
        *child = new_parent;
        CWM_internal_window_push(new_parent, s);
        CWM_internal_window_push(new_parent, new);
        new_parent->kind=split;

    }
}

CWMM_window CWMM_window_register(CWMM_manager m, CWM_window w)
{
    CWMM_window mwin = CWMM_window_create();
    List_push(m->views, &mwin);
    return mwin;
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

// pane -> window

// Manager must not be selected
