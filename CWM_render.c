#include "CWM_render.h"

struct CWM_renderer
{
	CWM_render_function func;
};

CWM_renderer CWM_renderer_create(CWM_render_function func)
{
	CWM_renderer renderer = malloc(sizeof(struct CWM_renderer));
	renderer->func = func;
	return renderer;
}
