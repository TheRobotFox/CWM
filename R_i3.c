#include "R_i3.h"
#include "BasicWindow.h"
#include "Conscreen/List/List.h"
#include "RR.h"
#include "RR_renderer.h"
#include <stdlib.h>

typedef enum {
VBRANCH=0, HBRANCH=1, LEAF
} I3_type;


typedef struct {
    LIST(I3_node) nodes;
    LIST(float) splits; // n-1 splits
} I3_branch;

typedef struct _I3_context {
    I3_type type;
    union {
        I3_branch branch;
        BasicWindow win;
    };
}* I3_node;

static void render(RR_context ctx, void *data)
{
   
}

RR_renderer R_i3()
{
    RR_renderer r = RR_renderer_create(render);
    I3_node layout = malloc(sizeof(*layout));
    layout->type = LEAF;
    layout->win = NULL;
    RR_renderer_data_set(r, layout);
    return  r;
}

void R_i3_split(RR_renderer r, I3_direction direction)
{
    I3_node layout = RR_renderer_data_get(r);
    if(layout->type==(direction&1)){ // homogenous split

    } else { // Heterogenous split

    }
}
void R_i3_select(RR_renderer r, I3_direction direction)
{

}
