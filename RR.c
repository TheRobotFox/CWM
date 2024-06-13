#include "CWM.h"
#include "Conscreen/List/List.h"
#include <stdint.h>
#include <stdlib.h>
#include "RR.h"
// Just Layers of renderers

typedef struct _RR_context_data
{
    i16vec2 size;
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

struct _RR_renderer
{
	RR_render_function func;
	LIST(RR_context) users;
	void* data;
};

void RR_set(RR_context context, int16_t x, int16_t y, void *data) {
    context->index--;
    context->setScreen(context, x, y, data);
    context->index++;
}
void* RR_get(RR_context context, int16_t x, int16_t y) {

    return context->getScreen(context, x, y);
}

RR_context RR_make_chain(int count, ...) {
    // collect all rendereers (later with Macro)
    // push in reverse order for list_pop

}
int RR_render(RR_context context, i16vec2 new_size, RR_set_function setScreen,
               RR_get_function getScreen, void *data) {

    RR_renderer next = *(RR_renderer*)List_pop(context->chain);
    if(!next) return -1;

    // write new context data to stack
    _RR_context_data *cData = List_push(context->context_data, NULL);

    cData->size = new_size;
    cData->setScreen=setScreen;
    cData->getScreen=getScreen;
    cData->data=data;

    context->index++;

    // Execute new Renderer
    next->func(context, next->data);
    return 0;
}
void *RR_data_get(RR_context context) {

    _RR_context_data *cData = List_at(context->context_data, context->index);
    if(!cData) return NULL;
    return cData->data;
}

CWM_renderer CWM_renderer_create(CWM_render_function func)
{
	CWM_renderer renderer = (CWM_renderer)malloc(sizeof(struct _CWM_renderer));
	renderer->func = func;
	renderer->users=LIST_create(CWM_window);
	return renderer;
}

void CWM_renderer_data_set(CWM_renderer renderer, void *data)
{
	renderer->data=data;
}

void CWM_renderer_free(CWM_renderer r)
{
	LIST_LOOP(CWM_window, r->users, w){
		(*w)->renderer=NULL;
	}
	LIST_free(CWM_window)(r->users);
	free(r);
}

void CWM_renderer_register(CWM_renderer r, CWM_window w)
{
	List_push(r->users, &w);
}

void CWM_renderer_unregister(CWM_renderer r, CWM_window w)
{
	List_rme(r->users, &w);
}
