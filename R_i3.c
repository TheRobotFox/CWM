#include "R_i3.h"
#include "BasicWindow.h"
#include "BasicWindow_internal.h"
#include "Conscreen/List/List.h"
#include "RR.h"
#include "RR_context.h"
#include "RR_renderer.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
VBRANCH=0, HBRANCH=1, LEAF
} I3_type;


typedef struct _I3_node {
    I3_type type;
    uint16_t fracs; // how many fractions the window is big
    struct _I3_node *parent;
    union {
        BasicWindow win;
        LIST(I3_node) nodes;
    };
} _I3_node;
typedef _I3_node* I3_node;

typedef struct {
    _I3_node layout;
    I3_node selected;
} _I3_context;
typedef _I3_context* I3_context;

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

struct DWM_render_info {
    RR_context origin;
    RR_point pos;
};
// TODO Bake i3 to dwm
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

static void render_node(RR_context ctx, I3_node node, I3_rect rect)
{
    if(node->type==LEAF){
        if(!node->win) return;
        RR_point size = RR_size_get(ctx),
            new_size = {
                ceilf(rect.size_x*size.x),
                ceilf(rect.size_y*size.y)
            },
            position = {
                ceilf(rect.pos_x*size.x),
                ceilf(rect.pos_y*size.y)
        };
        if(position.x+new_size.x>size.x) new_size.x--;
        if(position.y+new_size.y>size.y) new_size.y--;

        struct DWM_render_info info = {.origin=ctx, .pos=position};
        RR_render(
            BW_get_context(node->win),
            new_size,
            dwm_forward_get,
            dwm_forward_set,
            &info);
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
            render_node(ctx, child, new_rect);
        }
    }
}

static void render(RR_context ctx, void *data)
{
    I3_context i3 = data;
    render_node(ctx, &i3->layout, (I3_rect){.pos_x=0, .pos_y=0, .size_x=1, .size_y=1});
}

RR_renderer R_i3()
{
    RR_renderer r = RR_renderer_create(render);
    I3_context ctx = malloc(sizeof(*ctx));
    ctx->layout.parent = NULL;
    ctx->layout.type = LEAF;
    ctx->layout.fracs = 1;
    ctx->layout.win = NULL;
    ctx->selected=&ctx->layout;
    RR_renderer_data_set(r, ctx);
    return  r;
}

void R_i3_split(RR_renderer r, I3_direction direction)
{
    I3_context ctx = RR_renderer_data_get(r);
    I3_node new;
    if(ctx->selected->type!=(direction&1)){ // herterogenous split
        List nodes = LIST_create(_I3_node);
        I3_node child = List_push(nodes, ctx->selected);
        child->parent = ctx->selected;
        ctx->selected->type=direction&1;
        ctx->selected->nodes = nodes;
    }
    new = direction&2 ? List_push(ctx->selected->nodes, NULL) : List_insert(ctx->selected->nodes, 0, NULL);
    *new = (_I3_node){.type=LEAF, .win=NULL, .fracs=1, .parent=ctx->selected};
    // select new window
    ctx->selected=new;
}

static I3_node fit_visual_offset(LIST(_I3_node) nodes, float visual_offset)
/*
** Get index of node which is on same level as [visual_offset]
*/
{
    // calc fracs
    float target=visual_offset*get_fracs(nodes, -1),
        min_dist=target;
    I3_node closest = List_start(nodes);

    LIST_LOOP(_I3_node, nodes, node){
        if(target<0) break;
        target-=node->fracs;
        float dist = target < 0 ? -target : target;
        if(dist<min_dist){
            min_dist=dist;
            closest = node;
        }
    }
    return closest;
}

static I3_node descend(I3_node current, float visual_offset, I3_direction direction)
/*
** Select closest window in next fram
**
** Used in R_i3_select
*/
{
    while(current->type!=LEAF){
        if(current->type==(direction&1)){
            // go into closest next (direction)
            current = List_at(current->nodes, direction&2 ? 0 : -1);
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

bool R_i3_select(RR_renderer r, I3_direction direction)
/*
** Select next (closest) window in [direction]
*/
{
    I3_context ctx = RR_renderer_data_get(r);
    I3_node current = ctx->selected,
        parent;
    // keep track of visual_offset
    // start with half window size
    float visual_offset=current->fracs/2;

    // try find current with children in ["direction"]
    while ((parent = current->parent)) {

        if((direction&1)!=parent->type) goto next; // wrong orientation

        int index = List_contains(parent->nodes, current);
        int selected = index+(direction&2 ? 1 : -1);

        // calc new absolute offset (to most outer frame)
        int total_frac = get_fracs(parent->nodes, -1);
        float frame_offset = get_fracs(parent->nodes, index)/total_frac;

        visual_offset*=current->fracs/total_frac; // scale current offset
        visual_offset+=frame_offset;

        if(!(current = List_at(parent->nodes, selected)))
            goto next; // if no other children, continue search

        // frame present in [direction]
        // descend into the frame
        // mirror (XOR) the direction cause move left -> rightmost (closest) in next frame
        ctx->selected = descend(current, visual_offset, direction^2);
        return  true;

        next:
        current=parent;
    }
    return false;
}

static void nodes_free(LIST(I3_nodes) nodes)
{
    LIST_LOOP(_I3_node, nodes, node){
        if(node->type!=LEAF)
            nodes_free(node->nodes);
    }
    List_free(nodes);
}

bool R_i3_kill(RR_renderer r)
/*
** Kill currently selected node
** return error false if selected node is root
*/
{
    I3_context ctx = RR_renderer_data_get(r);
    I3_node parent = ctx->selected;
    if(!parent) return false;
    List_rme(parent->nodes, ctx->selected);

    // collapse node
    if(List_size(parent->nodes)<2){
        nodes_free(parent->nodes);
        parent->type=ctx->selected->type;
        if(parent->type==LEAF)
            parent->win = ctx->selected->win;
        else
            parent->nodes = ctx->selected->nodes;
    }
    return true;
}

bool R_i3_select_parent(RR_renderer r)
{
    I3_context ctx = RR_renderer_data_get(r);
    if(!ctx->selected->parent) return false;
    ctx->selected=ctx->selected->parent;
    return true;
}

void R_i3_set(RR_renderer r, BasicWindow win)
/*
** Set currently selected node to win
** collapse node if not LEAF
*/
{
    I3_context ctx = RR_renderer_data_get(r);
    if(ctx->selected->type!=LEAF)
        nodes_free(ctx->selected->nodes);
    ctx->selected->type=LEAF;
    ctx->selected->win = win;
}

BasicWindow R_i3_get(RR_renderer r)
/*
** Return currently selected window.
** can return NULL if no window has been set
** or if selection is not a LEAF
*/
{
    I3_context ctx = RR_renderer_data_get(r);
    if(ctx->selected->type!=LEAF) return  NULL;
    return ctx->selected->win;
}

void R_i3_free(RR_renderer r)
{
    I3_context ctx = RR_renderer_data_get(r);
    if(ctx->layout.type!=LEAF)
        nodes_free(ctx->layout.nodes);
    free(ctx);
    RR_renderer_free(r);
}
