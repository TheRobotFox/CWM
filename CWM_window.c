#include "BasicWindow.h"
#include "CWM.h"
#include "CWM_internal.h"
#include "Conscreen/List/List.h"
#include "RR.h"
#include "BasicWindow_internal.h"
#include "R_dwm.h"
#include "R_info.h"

CWM_window CWM_internal_create()
{
	CWM_window w = malloc(sizeof(_CWM_window));
	BW_create(&w->window);

	w->parent=NULL;
	w->children=LIST_create(CWM_window);
	w->frame=DWM_window_create();
	DWM_window_renderer_set(w->frame, BW_get_context(&w->window));

	return w;
}

BasicWindow CWM_root_get()
{
	
	CWM_internal_init_check();
	return root_window;
}
void CWM_internal_push(CWM_window parent, CWM_window w)
{
	CWM_internal_init_check();
	w->parent=parent;

	DWM_register(BW_get_renderer(&parent->window, "DWM"), w->frame, 0);
	List_push(parent->children, &w);
}

BasicWindow CWM_push(BasicWindow parent)
{
	CWM_window w = CWM_internal_create();
	CWM_internal_push((CWM_window)parent, w);
	return (BasicWindow)w;
}

void CWM_move(BasicWindow w, BasicWindow target)
{
	CWM_internal_remove((CWM_window)w);
	CWM_internal_push((CWM_window)target, (CWM_window)w);
}

void CWM_internal_remove(CWM_window w)
{
	// remove from parent
	if(!w->parent) return;
	DWM_unregister(BW_get_renderer(&w->parent->window, "DWM"), w->frame);
	List_rme(w->parent->children, w);

}
void CWM_remove(BasicWindow _w)
{
	CWM_window w = (CWM_window)_w;

	CWM_internal_remove(w);

	// recursively free child_windows
	LIST_FORWARD(BasicWindow, w->children, CWM_remove);

	BW_internal_free(&w->window);
	free(w);
}


void CWM_pos_set_absolute(BasicWindow w, uint16_t x, uint16_t y)
{
	DWM_window_pos_set_abs(((CWM_window)w)->frame, x, y);
}
void CWM_pos_set_relative(BasicWindow w, float x, float y)
{
	DWM_window_pos_set_rel (((CWM_window)w)->frame, x, y);
}

void CWM_size_set_absolute(BasicWindow w, uint16_t x, uint16_t y)
{
	DWM_window_size_set_abs (((CWM_window)w)->frame, x, y);
}
void CWM_size_set_relative(BasicWindow w, float x, float y)
{
	DWM_window_size_set_rel (((CWM_window)w)->frame, x, y);
}
void CWM_depth_set(BasicWindow _w, int depth)
{
	CWM_window w = (CWM_window)_w;
	if(!w->parent) return;
	DWM_unregister(BW_get_renderer(&w->parent->window, "DWM"), ((CWM_window)w)->frame);
	DWM_register(BW_get_renderer(&w->parent->window, "DWM"), ((CWM_window)w)->frame, depth);
}

void CWM_gen_chains(BasicWindow _w)
{
	CWM_window w = (CWM_window)_w;
	BW_gen_chain(_w);
	LIST_FORWARD(BasicWindow, w->children, CWM_gen_chains);
}
