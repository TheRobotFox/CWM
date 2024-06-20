#include "CWM.h"
#include "CWM_internal.h"
#include "Conscreen/Conscreen_console.h"
#include "Conscreen/Conscreen_screen.h"
#include "Conscreen/Conscreen_string.h"
#include "RR_context.h"
#include "R_dwm.h"
#include <stdlib.h>
#include <string.h>

static struct {
	int initilized;
	void (*deinit_next)(int);
} status={0};

CWM_window root_window;

#if defined(unix) || defined(__unix__) || defined(__unix)

static void CWM_internal_signal_deinit(int dummy){
	CWM_deinit();
	if(status.deinit_next){
		status.deinit_next(dummy);
	}
	exit(0);
}

#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
static BOOL WINAPI CWM_internal_signal_deinit(DWORD dummy){
	CWM_deinit();
	return 0;
}
#endif

static void CWM_internal_atexit_deinit(){
	CWM_deinit();
}

void CWM_internal_init_check(){
	if(!status.initilized){
		PRINTF(STR("CWM not initilized!"));
		exit(1);
	}
}

int16_t mini16(int16_t a, int16_t b){
	if(a<b)
		return a;
	return b;
}


int16_t maxi16(int16_t a, int16_t b){
	if(a>b)
		return a;
	return b;
}

void CWM_internal_string_set(CWM_string *string,const Conscreen_char *text, size_t length)
{
	if(string->str) free(string->str);
	string->str = malloc(length*sizeof(Conscreen_char));
	memcpy(string->str, text, length*sizeof(Conscreen_char));
	string->len=length;
}

RR_renderer cwm_root_dwm;
RR_context cwm_render_context;

void CWM_deinit(){
	CWM_internal_init_check();
	CWM_window_remove(root_window);
	R_dwm_free(cwm_root_dwm);
	RR_context_free(cwm_render_context);
	/* CWM_animation_free(); */
	Conscreen_deinit();
	status.initilized=0;
}

/* static void CWM_internal_animate_window_title_wrap() */
/* { */
/* 	extern uint32_t window_title_wrap; */
/* 	 window_title_wrap+=1; */
/* } */

// setup important CWM
int CWM_init(){
	if(!status.initilized){
		status.initilized=1;

		Conscreen_init();

		cwm_root_dwm = R_dwm();
		cwm_render_context = RR_context_create();
		RR_context_make_chain(cwm_render_context,
							  1, cwm_root_dwm);
		// setup root window

		root_window = CWM_internal_window_create();
		DWM_register(cwm_root_dwm, root_window->frame, 0);

		CWM_window_renderer_toggle(root_window, "BORDER", false);
		CWM_window_renderer_toggle(root_window, "TITLE", false);
		CWM_window_renderer_toggle(root_window, "ERROR", false);
		CWM_window_renderer_toggle(root_window, "OPACITY", false);
		CWM_window_renderer_toggle(root_window, "TEXT", false);
		CWM_window_gen_chain(root_window);

		CWM_window_pos_set_absolute(root_window,0,0);
		CWM_window_size_set_relative(root_window,1,1);
		CWM_window_title_set(root_window, STR("App"), 3);

		// register animation

		/* CWM_animation anim = CWM_animation_interval_create(10, CWM_internal_animate_window_title_wrap); */
		/* CWM_animation_register_struct(anim); */
		/* CWM_animation_struct_free(anim); */

		#if defined(unix) || defined(__unix__) || defined(__unix)
		status.deinit_next=signal(SIGINT,CWM_internal_signal_deinit);
		#elif defined(_WIN32) || defined(_WIN64)
		SetConsoleCtrlHandler(CWM_internal_signal_deinit,TRUE);
		#endif
		atexit(CWM_internal_atexit_deinit);
		return 0;
	}
	// Conscreen_error(root_window, Conscreen_WARNING,"ConScreen already initilized!");
	return 1;
}

void CWM_internal_conscreen_set(RR_context _ctx, int16_t x, int16_t y, RR_pixel p)
{
	Conscreen_screen_set(x, y, p);
}
RR_pixel CWM_internal_conscreen_get(RR_context _ctx, int16_t x, int16_t y)
{
	return Conscreen_screen_get(x, y);
}

void CWM_render()
{
	Conscreen_screen_begin();
	Conscreen_point size = Conscreen_screen_size();

	CWM_window w = CWM_window_root_get();

	CWM_window_prepare(w);

	RR_render(cwm_render_context,
			  (RR_point){size.x, size.y},
			  CWM_internal_conscreen_get,
			  CWM_internal_conscreen_set,
			  NULL);
	Conscreen_screen_flush();
}
