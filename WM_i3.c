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
    I3_node layout;
    I3_node selected;
    bool dirty;
    RR_renderer dwm;
} _I3_context;

static int get_fracs(LIST(I3_node) nodes, int to)
/*
** sum up I3_node->fracs in list [nodes] up to index [to]
**
** Used to calculate next selection and window sizes
*/
{
    int total=0;

    I3_node *node = LIST_start(I3_node)(nodes), *end=LIST_at(I3_node)(nodes, to);
    for(; node<=end; node++)
        total+=(*node)->fracs;
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
        LIST_LOOP(I3_node, node->nodes, pchild){
            I3_node child = *pchild;
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
    node_bake(ctx, ctx->layout, (I3_rect){.pos_x=0, .pos_y=0, .size_x=1, .size_y=1});
    ctx->dirty=false;
}

I3_context WM_i3(RR_renderer dwm)
{
    I3_context ctx = malloc(sizeof(*ctx));
    I3_node layout = malloc(sizeof(_I3_node));
    layout->parent = NULL;
    layout->type = LEAF;
    layout->fracs = 1;
    layout->leaf = (_Leaf){.win = NULL, .frame = DWM_window_create()};
    DWM_register(dwm, layout->leaf.frame, 0);
    ctx->layout = layout;
    ctx->dwm = dwm;
    ctx->dirty = true;
    ctx->selected=ctx->layout;
    return ctx;
}

static void WM_i3_replace(I3_context ctx, I3_node old, I3_node new)
{
    I3_node parent = old->parent;
    if(!parent){
     ctx->layout = new;
     new->parent =0;
    } else {
        int index = List_contains(parent->nodes, &old);
        List_GET_REF(I3_node, parent->nodes, index) = new;
        new->parent = parent;
    }
}

void WM_i3_split(I3_context ctx, I3_direction direction)
{
    I3_node parent = ctx->selected->parent;
    int index = 0;
    if(!parent || parent->type!=(direction&1)){ // herterogenous split

        I3_node node = malloc(sizeof(_I3_node));
        node->nodes = LIST_create(I3_node);
        node->fracs = 1;
        node->type = direction&1;
        I3_node *child = List_push(node->nodes, &ctx->selected);

        WM_i3_replace(ctx, ctx->selected, node);
        parent = node;

        (*child)->parent = node;


    } else {
        index = List_contains(parent->nodes, &ctx->selected);
    }

    I3_node new = malloc(sizeof(_I3_node));
    List_insert(parent->nodes, index+((direction&2)!=0), &new);

    *new = (_I3_node){.type=LEAF, .fracs=1, .parent=parent};

    new->leaf.win = NULL;
    new->leaf.frame = DWM_window_create();
    DWM_register(ctx->dwm, new->leaf.frame, 0);

    // select new window
    ctx->selected=new;

    ctx->dirty = true;
}

static I3_node fit_visual_offset(LIST(I3_node) nodes, float visual_offset)
/*
** Get index of node which is on same level as [visual_offset]
*/
{
    // calc fracs
    float target=visual_offset*get_fracs(nodes, -1),
        min_dist=FLT_MAX;
    I3_node closest = *LIST_start(I3_node)(nodes);

    LIST_LOOP(I3_node, nodes, pnode){
        I3_node node = *pnode;
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
            current = *LIST_at(I3_node)(current->nodes, direction&2 ? 0 : -1); // 0 : -1 -> reversed direction
        }else{
            // fit visual center
            I3_node next = fit_visual_offset(current->nodes, visual_offset);

            // rescale visual_offset
            float frame_size = (float)next->fracs/get_fracs(current->nodes, -1);
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

        int index = List_contains(parent->nodes, &current);

        if((direction&1)!=parent->type){ // wrong orientation
            int total_frac = get_fracs(parent->nodes, -1);
            float frame_offset = 0;
            if(index>0) frame_offset = (float)get_fracs(parent->nodes, index-1)/total_frac;

            visual_offset*=(float)current->fracs/(float)total_frac; // scale current offset
            visual_offset+=frame_offset;
          goto next;
        }

        int selected = index+(direction&2 ? 1 : -1);

        // calc new absolute offset (to most outer frame)

        if(selected<0 || !(current = *LIST_at(I3_node)(parent->nodes, selected)))
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
        LIST_LOOP(I3_node, node->nodes, child){
            node_free(ctx, *child);
        }
        List_free(node->nodes);
    }
    free(node);
}
bool WM_i3_kill(I3_context ctx)
/*
** Kill currently selected node
** return error false if selected node is root
*/
{
    I3_node parent = ctx->selected->parent;
    if(!parent) return false;

    node_free(ctx, ctx->selected);
    int index = List_contains(parent->nodes, &ctx->selected);
    List_rmi(parent->nodes, index);

    // collapse node
    if(List_size(parent->nodes)==1){
        I3_node sole = *LIST_start(I3_node)(parent->nodes);

        WM_i3_replace(ctx, parent, sole);

        List_free(parent->nodes);
        ctx->selected = sole;

    } else {
        if(index>=(int)List_size(parent->nodes)) index--;
            ctx->selected=*LIST_at(I3_node)(parent->nodes, index);
    }
    ctx->selected = descend(ctx->selected, 0, I3_LEFT);
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
    if(ctx->selected->type!=LEAF) return NULL;
    return ctx->selected->leaf.win;
}

void WM_i3_free(I3_context ctx)
{
    node_free(ctx, ctx->layout);
    free(ctx);
}
