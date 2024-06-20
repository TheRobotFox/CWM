#include "CWM.h"
#include "CWM_internal.h"
#include "Conscreen/Conscreen_ANSI.h"
#include "Conscreen/Conscreen_screen.h"
#include "Conscreen/Conscreen_string.h"
#include "Conscreen/List/List.h"
#include "RR.h"
#include "RR_context.h"
#include "RR_renderer.h"
#include "R_border.h"
#include "R_dwm.h"
#include "R_info.h"
#include "R_opacity.h"
#include "R_text.h"
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const struct{
	f_CWM_renderer_create create;
	f_CWM_renderer_free destroy;
	const char*const name;
	CWM_render_kind kind;
} renderers_builtIn[_CR_END] = {
    [CR_OPACITY] = {R_opacity, R_opacity_free, "OPACITY", CR_PRE},
    [CR_TITLE]  = {R_info, R_info_free, "TITLE", CR_PRE},
    [CR_ERROR]  = {R_info, R_info_free, "ERROR", CR_PRE},
    [CR_BORDER]  = {R_border, R_border_free, "BORDER", CR_PRE},
	[CR_TEXT]    = {R_text, R_text_free, "TEXT", CR_POST},
	[CR_CHILDREN]= {R_dwm, R_dwm_free, "DWM", CR_POST}
};

typedef struct
{
	RR_renderer r;
	CWM_render_kind kind;
	bool active;
	const char* const name;
} _CWM_renderer;
typedef _CWM_renderer *CWM_renderer ;

bool CWM_renderer_find(const void *av, const void *name)
{
	const CWM_renderer a=av;
	return strcmp(a->name, name)==0;
}

void CWM_window_gen_chain(CWM_window w)
{
	List renderers = LIST_create(RR_renderer);
	LIST_LOOP(_CWM_renderer, w->renderers, cr){
		if(cr->kind==CR_PRE && cr->active)
			List_push(renderers, &cr->r);
	}
	LIST_LOOP(_CWM_renderer, w->renderers, cr){
		if(cr->kind==CR_POST && cr->active)
			List_push(renderers, &cr->r);
	}

	RR_context_chain_set(w->render_ctx, renderers);
	w->chain_dirty = false;
}

CWM_window CWM_internal_window_create()
{
	CWM_window w = (CWM_window)malloc(sizeof(struct _CWM_window));

	w->parent=NULL;
	w->renderers=LIST_create(_CWM_renderer);
	w->chain_dirty = true;
	w->frame=DWM_window_create();
	w->render_ctx=RR_context_create();
	w->children = LIST_create(CWM_window);

	for(size_t i=0; i<_CR_END; i++)
		CWM_window_add(w,
								renderers_builtIn[i].name,
								renderers_builtIn[i].kind,
								renderers_builtIn[i].create());

	RR_renderer error = CWM_window_get_renderer(w, "ERROR");
	if(error){
		R_info_set_side(error, INFO_BOTTOM);
		/* R_info_set_style(error, CONSCREEN_ANSI_DEFAULT(0, 255, 255)); */
	}

	CWM_window_gen_chain(w);
	DWM_window_renderer_set(w->frame, w->render_ctx);

	return w;
}

void CWM_window_prepare(CWM_window w)
{
	if(w->chain_dirty) CWM_window_gen_chain(w);
	LIST_FORWARD(CWM_window, w->children, CWM_window_prepare);
}

void CWM_internal_window_remove(CWM_window w)
{
	CWM_internal_init_check();

	// remove from parent
	if(!w->parent) return;
	DWM_unregister(CWM_window_get_renderer(w->parent, "DWM"), w->frame);
	List_rme(w->parent->children, w);


}

void CWM_window_remove(CWM_window w)
{

	CWM_internal_window_remove(w);

	// recursively free child_windows
	LIST_FORWARD(CWM_window, w->children, CWM_window_remove);

	// free Renderers
	for(size_t i=0; i<_CR_END; i++)
		renderers_builtIn[i].destroy(
			CWM_window_get_renderer(w, renderers_builtIn[i].name));

	List_free(w->children);
	List_free(w->renderers);
	RR_context_free(w->render_ctx);
	free(w);
}

CWM_window CWM_window_root_get()
{
	CWM_internal_init_check();
	return root_window;
}

void CWM_internal_window_push(CWM_window parent, CWM_window w)
{
	CWM_internal_init_check();
	w->parent=parent;

	DWM_register(CWM_window_get_renderer(parent, "DWM"), w->frame, 0);
	List_push(parent->children, &w);
}

CWM_window CWM_window_push(CWM_window parent)
{
	CWM_window w = CWM_internal_window_create();
	CWM_internal_window_push(parent, w);
	return w;
}

void CWM_window_move(CWM_window w, CWM_window target)
{
	CWM_internal_window_remove(w);
	CWM_internal_window_push(target, w);
}
void CWM_window_opactity_set(CWM_window w, float opacity)
{
	if(opacity<0)
		opacity=0;
	else if(opacity>1)
		opacity=1;
	R_opacity_set(CWM_window_get_renderer(w, "OPACITY"), opacity);
}

RR_renderer CWM_window_get_renderer(CWM_window w, const char*const name)
{
	CWM_renderer res = List_finde(w->renderers, CWM_renderer_find, name);
	if(!res){
		 printf("ARRRGG!!\n");
		 exit(-1);
	}
	return res->r;
}

bool CWM_window_renderer_toggle(CWM_window w, const char*const name, bool active)
{
	CWM_renderer cr = List_finde(w->renderers, CWM_renderer_find, name);

	if(!cr) return true;
	if(cr->active==active) return false;
	cr->active = active;
	w->chain_dirty = true;
	return false;
}

bool CWM_window_add(CWM_window w, const char*const name, CWM_render_kind kind, RR_renderer r)
{
	// No duplicates
	int index = List_findi(w->renderers, CWM_renderer_find, name);
	if(index!=-1) return true;

	_CWM_renderer cr = {
		.active = true,
		.kind = kind,
		.name = name,
		.r=r
	};
	List_push(w->renderers, &cr);
	w->chain_dirty=true;
	return false;
}

bool CWM_window_rm(CWM_window w, const char*const name)
{
	// Don't remove builtIn
	for(int i=0; i<_CR_END; i++)
		if(!strcmp(name, renderers_builtIn[i].name)) return true;

	int index = List_findi(w->renderers, CWM_renderer_find, name);
	if(index<0) return true;
	List_rmi(w->renderers, index);
	w->chain_dirty=true;
	return false;
}

void CWM_window_text_set_style(CWM_window w, Conscreen_ansi style)
{
	R_text_set_style(CWM_window_get_renderer(w, "TEXT"), style);
}

void CWM_window_text_set(CWM_window w, const Conscreen_char* text, size_t length)
{
	R_text_set_text(CWM_window_get_renderer(w, "TEXT"), text, length);
}

void CWM_error(CWM_window w, CWM_error_level level, const Conscreen_char*const format, ...)
{
	RR_renderer r = CWM_window_get_renderer(w, "ERROR");
	va_list arg;
	va_start(arg, format);
	R_info_vprintf(r, format, arg);
	va_end(arg);
	R_info_set_level(r,  level);
}

void CWM_window_title_set(CWM_window w, const Conscreen_char *title, size_t length)
{
	R_info_set_text(CWM_window_get_renderer(w, "TITLE"), title, length);
}

void CWM_window_pos_set_absolute(CWM_window w, uint16_t x, uint16_t y)
{
	DWM_window_pos_set_abs(w->frame, x, y);
}
void CWM_window_pos_set_relative(CWM_window w, float x, float y)
{
	DWM_window_pos_set_rel (w->frame, x, y);
}

void CWM_window_size_set_absolute(CWM_window w, uint16_t x, uint16_t y)
{
	DWM_window_size_set_abs (w->frame, x, y);
}
void CWM_window_size_set_relative(CWM_window w, float x, float y)
{
	DWM_window_size_set_rel (w->frame, x, y);
}
void CWM_window_depth_set(CWM_window w, int depth)
{
	if(!w->parent) return;
	DWM_unregister(CWM_window_get_renderer(w->parent, "DWM"), w->frame);
	DWM_register(CWM_window_get_renderer(w->parent, "DWM"), w->frame, depth);
}
