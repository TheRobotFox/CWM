#include "BasicWindow.h"
#include "BasicWindow_internal.h"
#include "RR_context.h"
#include "R_border.h"
#include "R_dwm.h"
#include "R_info.h"
#include "R_opacity.h"
#include "R_text.h"
#include <stddef.h>


// Default renderers to use include in basic Window
typedef RR_renderer (*f_BW_renderer_create)(void);
typedef void (*f_BW_renderer_free)(RR_renderer);

static const struct{
	f_BW_renderer_create create;
	f_BW_renderer_free destroy;
	const char*const name;
	BW_render_kind kind;
} renderers_builtIn[] = {
    {R_opacity, R_opacity_free, "OPACITY", CR_PRE},
    {R_info, R_info_free, "TITLE", CR_PRE},
    {R_info, R_info_free, "ERROR", CR_PRE},
    {R_border, R_border_free, "BORDER", CR_PRE},
    {R_text, R_text_free, "TEXT", CR_POST},
    {R_dwm, R_dwm_free, "DWM", CR_POST}
};
const size_t renderers_builtIn_count = sizeof(renderers_builtIn)/sizeof(*renderers_builtIn);

typedef struct
{
	RR_renderer r;
	BW_render_kind kind;
	bool active;
	const char* const name;
} _BW_renderer;
typedef _BW_renderer *BW_renderer ;


bool BW_renderer_find(const void *av, const void *name)
{
	const _BW_renderer * a=av;
	return strcmp(a->name, name)==0;
}

void BW_internal_gen_chain(BasicWindow w)
{
	List renderers = LIST_create(RR_renderer);
	LIST_LOOP(_BW_renderer, w->renderers, cr){
		if(cr->kind==CR_PRE && cr->active)
			List_push(renderers, &cr->r);
	}
	LIST_LOOP(_BW_renderer, w->renderers, cr){
		if(cr->kind==CR_POST && cr->active)
			List_push(renderers, &cr->r);
	}

	RR_context_chain_set(w->render_ctx, renderers);
	w->chain_dirty = false;
}

BasicWindow BW_create(BasicWindow buffer)
{
	if(!buffer) buffer = malloc(sizeof(_BasicWindow));
	BasicWindow w = buffer;

	w->renderers=LIST_create(_BW_renderer);
	w->chain_dirty = true;
	w->render_ctx=RR_context_create();

	for(size_t i=0; i< renderers_builtIn_count; i++)
		BW_add(w,
				renderers_builtIn[i].name,
				renderers_builtIn[i].kind,
				renderers_builtIn[i].create());

	RR_renderer error = BW_get_renderer(w, "ERROR");
	if(error){
		R_info_set_side(error, INFO_BOTTOM);
		/* R_info_set_style(error, CONSCREEN_ANSI_DEFAULT(0, 255, 255)); */
	}
	return w;
}

void BW_gen_chain(BasicWindow w)
{
	if(w->chain_dirty) BW_internal_gen_chain(w);
}

void BW_internal_free(BasicWindow w)
{
	// free Renderers
	for(size_t i=0; i<renderers_builtIn_count; i++)
		renderers_builtIn[i].destroy(
			BW_get_renderer(w, renderers_builtIn[i].name));

	List_free(w->renderers);
	RR_context_free(w->render_ctx);
}
void BW_free(BasicWindow w)
{
	BW_internal_free(w);
	free(w);
}

void BW_opacity_set(BasicWindow w, float opacity)
{
	if(opacity<0)
		opacity=0;
	else if(opacity>1)
		opacity=1;
	R_opacity_set(BW_get_renderer(w, "OPACITY"), opacity);
}

RR_renderer BW_get_renderer(BasicWindow w, const char*const name)
{
	BW_renderer res = List_finde(w->renderers, BW_renderer_find, name);
	if(!res) return NULL;
	return res->r;
}

int BW_renderer_toggle(BasicWindow w, const char*const name, bool active)
{
	BW_renderer cr = List_finde(w->renderers, BW_renderer_find, name);

	if(!cr) return -1;
	if(cr->active==active) return 0;
	cr->active = active;
	w->chain_dirty = true;
	return 0;
}

int BW_add(BasicWindow w, const char*const name, BW_render_kind kind, RR_renderer r)
{
	// No duplicates
	int index = List_findi(w->renderers, BW_renderer_find, name);
	if(index!=-1) return 1;

	_BW_renderer cr = {
		.active = true,
		.kind = kind,
		.name = name,
		.r=r
	};
	List_push(w->renderers, &cr);
	w->chain_dirty=true;
	return 0;
}

int BW_rm(BasicWindow w, const char*const name)
{
	// Don't remove builtIn
	for(size_t i=0; i<renderers_builtIn_count; i++)
		if(!strcmp(name, renderers_builtIn[i].name)) return 1;

	int index = List_findi(w->renderers, BW_renderer_find, name);
	if(index<0) return -1;
	List_rmi(w->renderers, index);
	w->chain_dirty=true;
	return 0;
}

void BW_text_set_style(BasicWindow w, Conscreen_ansi style)
{
	R_text_set_style(BW_get_renderer(w, "TEXT"), style);
}

void BW_text_set(BasicWindow w, const Conscreen_char* text, size_t length)
{
	R_text_set_text(BW_get_renderer(w, "TEXT"), text, length);
}

void BW_error(BasicWindow w, BW_error_level level, const Conscreen_char*const format, ...)
{
	RR_renderer r = BW_get_renderer(w, "ERROR");
	va_list arg;
	va_start(arg, format);
	R_info_vprintf(r, format, arg);
	va_end(arg);
	R_info_set_level(r,  level);
}

void BW_title_set(BasicWindow w, const Conscreen_char *title, size_t length)
{
	R_info_set_text(BW_get_renderer(w, "TITLE"), title, length);
}

RR_context BW_get_context(BasicWindow w)
{
	return w->render_ctx;
}
