#include "R_context.h"
#include "RR.h"
#include "RR_context.h"
#include "RR_renderer.h"

static void render(RR_context ctx, void* data)
{
    if(data)
        RR_render(data, RR_size_get(ctx), RR_get_default, RR_set_default, NULL);
    RR_render(ctx, RR_size_get(ctx), RR_get_default, RR_set_default, NULL);
}
RR_renderer R_context()
{
    RR_renderer r = RR_renderer_create(render);
    RR_renderer_data_set(r, NULL);
    return r;
}

void R_context_set(RR_renderer r, RR_context ctx)
{
    RR_renderer_data_set(r, ctx);
}
