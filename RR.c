#include "CWM.h"
#include "Conscreen/List/List.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include "RR.h"

struct _RR_context
{
    LIST(RR_renderer) chain;

    i16vec2 size;
    RR_set_function setScreen;
    RR_get_function getScreen;
    RR_context last_context;
    void *data;

};

struct _RR_renderer
{
	RR_render_function func;
	LIST(RR_context) users;
	void* data;
};

void RR_set(RR_context context, int16_t x, int16_t y, void *data) {
    context->setScreen(context->last_context, x, y, data);
}
void* RR_get(RR_context context, int16_t x, int16_t y) {
    return context->getScreen(context->last_context, x, y);
}

static inline RR_context RR_context_next(RR_context last, RR_get_function getScreen, RR_set_function setScreen) {

    RR_context context = malloc(sizeof(struct _RR_context));

    context->chain = last->chain;
    context->data = NULL;
    context->getScreen = getScreen;
    context->setScreen = setScreen;
    context->last_context = last;
    return context;
}

RR_context RR_make_render_chain(int count, ...) {

    RR_context context = malloc(sizeof(struct _RR_context));
    context->chain = LIST_create(RR_renderer);

    va_list renderers;
    va_start(renderers, count);
    while (count--)
        List_push(context->chain, va_arg(renderers, RR_renderer));
    va_end(renderers);

    List_reverse(context->chain);
    return context;
}
int RR_render(RR_context context, i16vec2 new_size, RR_get_function getScreen,
               RR_set_function setScreen) {

    RR_renderer next = *(RR_renderer*)List_pop(context->chain);
    if(!next) return -1;

    context->size = new_size;

    // Execute new Renderer
    next->func(RR_context_next(context, getScreen, setScreen), next->data);
    return 0;
}

void RR_data_set(RR_context context, void *data)
{
    context->data=data;
}
void* RR_data_get(RR_context context) {
    return context->data;
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
