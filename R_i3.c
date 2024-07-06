#include "R_i3.h"
#include "BasicWindow.h"
#include "Conscreen/List/List.h"
#include "RR.h"
#include "RR_renderer.h"
#include <stdint.h>
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

static void render(RR_context ctx, void *data)
{
   
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
    I3_node new;
    I3_node selected = RR_renderer_data_get(r);
    if(selected->type!=(direction&1)){ // herterogenous split
        List nodes = LIST_create(_I3_node);
        I3_node child = List_push(nodes, selected);
        child->parent = selected;
        selected->type=direction&1;
        selected->nodes = nodes;
    }
    new = direction&2 ? List_push(selected->nodes, NULL) : List_insert(selected->nodes, 0, NULL);
    new->type=LEAF;
    new->leaf = (I3_leaf){.win=NULL, .fracs=1};
}

static float get_visual_offset(LIST(_I3_node) nodes, int index)
{
    int target=0, total=0;
    LIST_LOOP(_I3_node, nodes, node){
       
    }
}

static I3_node descend(I3_node from, float visual_offset, I3_direction closest)
{
        // decent
        // if parralel -> calculate visual next
        // else select closest ex. this.TOP->next.BOTTOM
        while(ctx->selected->type!=LEAF){

        }
}
bool R_i3_select(RR_renderer r, I3_direction direction)
{
    I3_context ctx = RR_renderer_data_get(r);
    I3_node parent = descend(ctx->selected, 0, I3_LEFT);

    while ((parent = ctx->selected->parent)) {
        if((direction&1)!=parent->type) continue;

        int index = List_contains(parent->nodes, ctx->selected);
        int next = index+(direction&2 ? 1 : -1);

        if(!(ctx->selected = List_at(parent->nodes, next)))
            continue;

        float visual_offset;
        ctx->selected = descend(ctx->selected, 0, I3_LEFT);

        return  true;
    }
    return false;
}
bool R_i3_select_parent(RR_renderer r)
{
    I3_context ctx = RR_renderer_data_get(r);
    if(!ctx->selected->parent) return false;
    ctx->selected=ctx->selected->parent;
    return true;
}
