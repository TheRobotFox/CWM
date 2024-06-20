#include "Conscreen/List/List.h"
#include <stdint.h>
#include <stdlib.h>
#include "RR_renderer.h"
// Just Layers of renderers


struct _RR_renderer
{
	RR_render_function func;
	void* data;
};

RR_renderer RR_renderer_create(RR_render_function func)
{
	RR_renderer renderer = (RR_renderer)malloc(sizeof(struct _RR_renderer));
	renderer->func = func;
	return renderer;
}

void RR_renderer_data_set(RR_renderer renderer, void *data)
{
	renderer->data=data;
}
void* RR_renderer_data_get(RR_renderer renderer)
{
	return renderer->data;
}

void RR_renderer_render(RR_renderer r, RR_context context)
{
	r->func(context, r->data);
}

void RR_renderer_free(RR_renderer r)
{
	free(r);
}
