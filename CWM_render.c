#include "CWM_render.h"
#include "CWM.h"
#include "CWM_internal.h"
#include "Conscreen/List/List.h"


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
