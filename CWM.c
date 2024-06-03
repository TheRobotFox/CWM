#include "CWM_internal.h"
#include "Conscreen/Conscreen_string.h"

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


void CWM_internal_string_set(CWM_string *string,const Conscreen_char *text, size_t length){
	if(string->str)
		free(string->str);

	string->str=(Conscreen_char*)malloc(length*sizeof(Conscreen_char));
	string->len=length;
	memcpy(string->str, text, length*sizeof(Conscreen_char));
}

void CWM_deinit(){
	CWM_internal_init_check();
	CWM_window_remove(root_window);
	CWM_animation_free();
	Conscreen_deinit();
	status.initilized=0;
}

static void CWM_internal_animate_window_title_wrap()
{
	extern uint32_t window_title_wrap;
	 window_title_wrap+=1;
}

// setup important CWM
int CWM_init(){
	if(!status.initilized){
		status.initilized=1;

		Conscreen_init();
		root_window = CWM_internal_window_create();
		CWM_window_border_show(root_window,0);
		CWM_window_pos_set_absolute(root_window,0,0);
		CWM_window_size_set_relative(root_window,1,1);
		CWM_window_title_set(root_window, STR("App"), 3);

		// register animation

		CWM_animation anim = CWM_animation_interval_create(10, CWM_internal_animate_window_title_wrap);
		CWM_animation_register_struct(anim);
		CWM_animation_struct_free(anim);

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

int CWM_char_is_empty(Conscreen_char c){
	switch(c){
	case ' ':
	case '\n':
		return 1;
	default:
		return 0;

	}
}
