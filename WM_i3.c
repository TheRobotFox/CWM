#include "WM_i3.h"
#include "BasicWindow.h"
#include "BasicWindow_internal.h"
#include "Conscreen/List/List.h"
#include "RR.h"
#include "RR_context.h"
#include "RR_renderer.h"
#include "R_dwm.h"
#include "WM_i3.h"
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

typedef enum {
VBRANCH=0, HBRANCH=1, LEAF
} I3_type;

typedef struct _Leaf {
    BasicWindow win;
    DWM_window frame;
} _Leaf;

typedef struct _I3_node {
    I3_type type;
    uint16_t fracs; // how many fractions the window is big
    struct _I3_node *parent;
    union {
        _Leaf leaf;
        LIST(I3_node) nodes;
    };
} _I3_node;
typedef _I3_node* I3_node;

typedef struct _I3_context {
    _I3_node layout;
    I3_node selected;
    bool dirty;
    RR_renderer dwm;
} _I3_context;

static int get_fracs(LIST(_I3_node) nodes, int to)
/*
** sum up I3_node->fracs in list [nodes] up to index [to]
**
** Used to calculate next selection and window sizes
*/
{
    int total=0;

    I3_node node = List_start(nodes), end=List_at(nodes, to);
    for(; node<=end; node++)
        total+=node->fracs;
    return total;
}

typedef struct {
    float pos_x, pos_y, size_x, size_y;
} I3_rect;

// TODO add abolute coords bake?
// would be required for smart scaling

static void node_bake(I3_context i3, I3_node node, I3_rect rect)
{
    if(node->type==LEAF){
        DWM_window_pos_set_rel(node->leaf.frame, rect.pos_x, rect.pos_y);
        DWM_window_size_set_rel(node->leaf.frame, rect.size_x, rect.size_y);
    } else {
        int fracs_total = get_fracs(node->nodes, -1), current=0;
        LIST_LOOP(_I3_node, node->nodes, child){
            I3_rect new_rect = rect;
            if(node->type==VBRANCH){
                new_rect.pos_x += (float)current/fracs_total*rect.size_x;
                new_rect.size_x = (float)child->fracs/fracs_total*rect.size_x;
                current+=child->fracs;
            } else {
                new_rect.pos_y += (float)current/fracs_total*rect.size_y;
                new_rect.size_y = (float)child->fracs/fracs_total*rect.size_y;
                current+=child->fracs;
            }
            node_bake(i3, child, new_rect);
        }
    }
}

void WM_i3_prepare(I3_context ctx)
/*
** Bake the Node Tree to DWM_frames
*/
{
    if(!ctx->dirty) return;
    node_bake(ctx, &ctx->layout, (I3_rect){.pos_x=0, .pos_y=0, .size_x=1, .size_y=1});
    ctx->dirty=false;
}

I3_context WM_i3(RR_renderer dwm)
{
    I3_context ctx = malloc(sizeof(*ctx));
    ctx->layout.parent = NULL;
    ctx->layout.type = LEAF;
    ctx->layout.fracs = 1;
    ctx->layout.leaf = (_Leaf){.win = NULL, .frame = DWM_window_create()};
    ctx->dwm = dwm;
    ctx->dirty = true;
    DWM_register(dwm, ctx->layout.leaf.frame, 0);
    ctx->selected=&ctx->layout;
    return ctx;
}

void WM_i3_split(I3_context ctx, I3_direction direction)
{
    I3_node new;
    I3_node node = ctx->selected->parent;
    if(!node || node->type!=(direction&1)){ // herterogenous split
        node = ctx->selected;
        List nodes = LIST_create(_I3_node);
        I3_node child = List_push(nodes, ctx->selected);
        child->parent = ctx->selected;
        ctx->selected->type=direction&1;
        ctx->selected->nodes = nodes;
    }
    new = direction&2 ? List_push(node->nodes, NULL) : List_insert(node->nodes, 0, NULL);

    *new = (_I3_node){.type=LEAF, .fracs=1, .parent=node};
    new->leaf.win = NULL;
    new->leaf.frame = DWM_window_create();
    DWM_register(ctx->dwm, new->leaf.frame, 0);

    // select new window
    ctx->selected=new;

    ctx->dirty = true;
}

static I3_node fit_visual_offset(LIST(_I3_node) nodes, float visual_offset)
/*
** Get index of node which is on same level as [visual_offset]
*/
{
    // calc fracs
    float target=visual_offset*get_fracs(nodes, -1),
        min_dist=FLT_MAX;
    I3_node closest = List_start(nodes);

    LIST_LOOP(_I3_node, nodes, node){
        target-=(float)node->fracs/2;
        float dist = target < 0 ? -target : target;
        if(dist<min_dist){
            min_dist=dist;
            closest = node;
        }
        if(target<0) break;
        target-=(float)node->fracs/2;
    }
    return closest;
}

static I3_node descend(I3_node current, float visual_offset, I3_direction direction)
/*
** Select closest window in next fram
**
** Used in WM_i3_select
*/
{
    while(current->type!=LEAF){
        if(current->type==(direction&1)){
            // go into closest next (direction)
            current = List_at(current->nodes, direction&2 ? 0 : -1); // 0 : -1 -> reversed direction
        }else{
            // fit visual center
            I3_node next = fit_visual_offset(current->nodes, visual_offset);

            // rescale visual_offset
            float frame_size = next->fracs/get_fracs(current->nodes, -1);
            visual_offset/=frame_size;
            current = next;
        }
    }
    return current;
}

extern int max(int, int);
bool WM_i3_select(I3_context ctx, I3_direction direction)
/*
** Select next (closest) window in [direction]
*/
{
    I3_node current = ctx->selected,
        parent;
    // keep track of visual_offset
    // start with half window size
    float visual_offset=0.5;

    // try find current with children in ["direction"]
    while ((parent = current->parent)) {

        int index = List_contains(parent->nodes, current);

        if((direction&1)!=parent->type){ // wrong orientation
            int total_frac = get_fracs(parent->nodes, -1);
            float frame_offset = 0;
            if(index>0) frame_offset = get_fracs(parent->nodes, index-1)/total_frac;

            visual_offset*=(float)current->fracs/total_frac; // scale current offset
            visual_offset+=frame_offset;
          goto next;
        }

        int selected = index+(direction&2 ? 1 : -1);

        // calc new absolute offset (to most outer frame)

        if(selected<0 || !(current = List_at(parent->nodes, selected)))
            goto next; // if no other children, continue search

        // frame present in [direction]
        // descend into the frame
        ctx->selected = descend(current, visual_offset, direction);
        return  true;

        next:
        current=parent;
    }
    return false;
}

static void node_free(I3_context ctx, I3_node node)
{
    if(node->type==LEAF){
        if(node->leaf.frame){
            DWM_unregister(ctx->dwm, node->leaf.frame);
            DWM_window_free(node->leaf.frame);
        }

    } else {
        LIST_LOOP(_I3_node, node->nodes, child){
            node_free(ctx, child);
        }
        List_free(node->nodes);
    }
}
static bool find_node(const void *a, const void *b)
{
    const _I3_node * A=a, *B=b;
    return A->nodes==B->nodes;
}
bool WM_i3_kill(I3_context ctx)
/*
** Kill currently selected node
** return error false if selected node is root
*/
{
    I3_node parent = ctx->selected->parent;
    if(!parent) return false;

    int index = List_findi(parent->nodes, find_node, ctx->selected);

    // collapse node
    if(List_size(parent->nodes)==2){
        I3_node merge = LIST_at(_I3_node)(parent->nodes, !index);

        ctx->selected = parent;

        if(merge->type==LEAF)
            WM_i3_set(ctx, merge->leaf.win);
        else{
            List chilren = merge->nodes;
            LIST_LOOP(_I3_node, chilren, node)
                node->parent = parent->parent;
            List_free(parent->nodes);
            parent->nodes = chilren;
        }
            
    } else {
        node_free(ctx, ctx->selected);
        List_rmi(parent->nodes, index);
        if(ctx->selected>=LIST_end(_I3_node)(parent->nodes))
            ctx->selected=List_at(parent->nodes, -1);
    }
    ctx->dirty = true;

    return true;
}

bool WM_i3_select_parent(I3_context ctx)
{
    if(!ctx->selected->parent) return false;
    ctx->selected=ctx->selected->parent;
    return true;
}

void WM_i3_set(I3_context ctx, BasicWindow win)
/*
** Set currently selected node to win
** collapse node if not LEAF
*/
{
    if(ctx->selected->type!=LEAF){
        node_free(ctx, ctx->selected);
        DWM_window frame = DWM_window_create();
        DWM_register(ctx->dwm, frame, 0);
        ctx->selected->leaf.frame = frame;
        ctx->dirty = true;
    }
    ctx->selected->type=LEAF;
    ctx->selected->leaf.win = win;
    DWM_window_renderer_set(ctx->selected->leaf.frame, BW_get_context(win));
}

BasicWindow WM_i3_get(I3_context ctx)
/*
** WMeturn currently selected window.
** can return NULL if no window has been set
** or if selection is not a LEAF
*/
{
    if(ctx->selected->type!=LEAF) return  NULL;
    return ctx->selected->leaf.win;
}

void WM_i3_free(I3_context ctx)
{
    if(ctx->layout.type!=LEAF)
        node_free(ctx, &ctx->layout);
    free(ctx);
}
