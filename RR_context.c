#include "Conscreen/List/List.h"
#include "RR.h"
#include "RR_context.h"
#include "RR_renderer.h"
#include <stdarg.h>
#include <stddef.h>

typedef struct _RR_context_data
{
    RR_point size;
    RR_set_function setScreen;
    RR_get_function getScreen;
    RR_context last_context;
    void *data;

} _RR_context_data;

struct _RR_context
{
    LIST(RR_renderer) chain;
    LIST(RR_renderer) context_data;
    size_t index;
};

void RR_context_free(RR_context context)
{
    List_free(context->chain);
    List_free(context->context_data);
    free(context);
}

RR_context RR_context_create() {

    RR_context context = malloc(sizeof(struct _RR_context));
    context->chain = NULL;
    context->context_data = LIST_create(_RR_context_data);
    context->index=0;
    return context;
}


void RR_context_chain_set(RR_context context, LIST(RR_renderer) renderers) {
    if(context->chain) List_free(context->chain);
    context->chain = renderers;
    List_resize(context->context_data, List_size(renderers));
}

void RR_context_make_chain(RR_context context, int count, ...) {

    LIST(RR_renderer) chain = LIST_create(RR_renderer);
    va_list renderers;
    va_start(renderers, count);
    while (count--){
        RR_renderer r = va_arg(renderers, RR_renderer);
        List_push(chain, &r);
    }
    va_end(renderers);
    List_reverse(chain);
    RR_context_chain_set(context, chain);
}
void RR_render(RR_context context, RR_point new_size, RR_get_function getScreen,
               RR_set_function setScreen, void *data) {

    RR_renderer *next = List_at(context->chain, context->index);
    if(!next) return;

    // write new context data to stack
    _RR_context_data *cData = List_at(context->context_data, context->index);

    cData->size = new_size;
    cData->setScreen=setScreen;
    cData->getScreen=getScreen;
    cData->data=data;

    context->index++;
    // Execute new Renderer
    RR_renderer_render(*next, context);
    context->index--;
}

void* RR_data_get(RR_context context) {

    _RR_context_data *cData = List_at(context->context_data, context->index);
    if(!cData) return NULL;
    return cData->data;
}
RR_point RR_size_get(RR_context context) {
    _RR_context_data *cData = List_at(context->context_data, context->index-1);
    return cData->size;
}

void RR_set(RR_context context, int16_t x, int16_t y, RR_pixel pixel) {
    _RR_context_data *cData = List_at(context->context_data, --context->index);
    cData->setScreen(context, x, y, pixel);
    context->index++;
}
RR_pixel RR_get(RR_context context, int16_t x, int16_t y) {
    _RR_context_data *cData = List_at(context->context_data, --context->index);
    RR_pixel res = cData->getScreen(context, x, y);
    context->index++;
    return res;
}


void RR_set_default(RR_context context, int16_t x, int16_t y, RR_pixel pixel)
{
// DEBUG
    /* RR_point size = RR_size_get(context); */
    /* if(x>=size.x) printf("WARNUNG! from %ld with %hd/%hd\n", context->index, x, size.x); */
    /* if(y>=size.y) printf("WARNUNG! from %ld with %hd/%hd\n", context->index, y, size.y); */

    RR_set(context, x, y, pixel);
}
RR_pixel RR_get_default(RR_context context, int16_t x, int16_t y)
{
    return RR_get(context, x, y);
}

void RR_set_offset(RR_context ctx, int16_t x, int16_t y, RR_pixel p)
{
    RR_point *pos = RR_data_get(ctx);
    RR_set_default(ctx, pos->x+x, pos->y+y, p);
}
RR_pixel RR_get_offset(RR_context ctx, int16_t x, int16_t y)
{
    RR_point *pos = RR_data_get(ctx);
    return RR_get_default(ctx, pos->x+x, pos->y+y);
}
