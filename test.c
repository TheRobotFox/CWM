#include "AM.h"
#include "BasicWindow.h"
#include "BasicWindow_internal.h"
#include "CWM.h"
#include "CWM_internal.h"
#include "Conscreen/Conscreen.h"
#include "Conscreen/Conscreen_console.h"
#include "Conscreen/Conscreen_screen.h"
#include "Conscreen/Conscreen_string.h"
#include "Conscreen/List/List.h"
#include "RR.h"
#include "RR_context.h"
#include "RR_renderer.h"
#include "R_border.h"
#include "R_dwm.h"
#include "WM_i3.h"
#include "R_info.h"
#include "R_text.h"
#include <stdint.h>
#include <stdlib.h>


/* int main(void) */
/* { */
/* 	CWM_init(); */
/* 	BasicWindow root = CWM_root_get(); */
/* 	BasicWindow new_window = CWM_push(root); */
/* 	BasicWindow left = CWM_push(root); */
/* 	CWM_pos_set_absolute(left, 0, 0); */
/* 	CWM_size_set_relative(left, 0.75, 0.75); */
/* 	CWM_pos_set_relative(new_window, 0.25, 0.25); */
/* 	BasicWindow current = new_window; */
/* 	BW_title_set(new_window, STR("new window"), 10); */
/* 	BW_title_set(left, STR("left window"), 11); */
/* 	BW_renderer_toggle(new_window, "TEYT", true); */
/* 	BW_renderer_toggle(left, "TEYT", true); */
/* 	CWM_depth_set(left, 0); */
/* 	CWM_depth_set(new_window, 1); */
/* 	BW_opacity_set(new_window, 0.75); */
/* 	BW_opacity_set(left, 0.75); */

/* 	bool insert=0; */
/* 	Conscreen_string buffer = Conscreen_string_create(); */
/* 	BW_error(current, INFO, STR("NORMAL MODE")); */
/* 	while (true){ */
/* 		CWM_render(); */
/* 		char c = Conscreen_console_get_key(); */
/* 		if(insert){ */
/* 			if(c==27){ */
/* 				BW_error(current, INFO, STR("NORMAL MODE")); */
/* 				insert=false; */
/* 				continue; */
/* 			} */
/* 			if(c==127){ */
/* 				List_pop(buffer); */
/* 			} else { */
/* 				Conscreen_string_push(buffer,c); */
/* 			} */
/* 			BW_teyt_set(current, Conscreen_string_start(buffer), Conscreen_string_length(buffer)); */
/* 		}else if(c=='i'){ */
/* 			insert=true; */
/* 			BW_error(current, INFO, STR("INSERT MODE")); */
/* 		} else if(c==23){ */
/* 			BW_error(current, INFO, STR("WINDOW SELECT")); */
/* 			CWM_render(); */
/* 			while (true) { */
/* 				char c = getchar(); */
/* 				if(c=='h' || c=='j'){ */
/* 					current= left; */
/* 					CWM_depth_set(left, 1); */
/* 					CWM_depth_set(new_window, 0); */
/* 					break; */
/* 				} */
/* 				if(c=='l' || c=='k'){ */
/* 					current= new_window; */
/* 					CWM_depth_set(left, 0); */
/* 					CWM_depth_set(new_window, 1); */
/* 					break; */
/* 				} */
/* 			} */
/* 		}else */
/* 			BW_error(current, ERROR, STR("Unknown Command %c (%d)"), c, (int)c); */
/* 	} */
/* } */

void set(RR_context ctx, int16_t x, int16_t y, RR_pixel pix)
{
	Conscreen_screen_set(x, y, pix);
}
RR_pixel get(RR_context ctx, int16_t x, int16_t y)
{
	return Conscreen_screen_get(x, y);
}

/* int main(void) */
/* { */
/* 	RR_renderer text = R_text(); */
/* 	R_text_set_text(text, "YOOOOOO", 7); */
/* 	RR_renderer border = R_border(); */
/* 	RR_renderer title = R_info(); */
/* 	RR_renderer error = R_info(); */
/* 	R_info_set_text(title, "App", 3); */
/* 	R_info_set_side(title, INFO_LEFT); */
/* 	R_info_set_align(title, INFO_END); */
/* 	R_info_set_clamp(error, "v%^"); */
/* 	R_info_set_level(error, INFO); */
/* 	R_info_printf(error, "Heyy, that's pretty cool! %d", 5); */
/* 	RR_context chain = RR_context_create(); */
/* 	RR_context_make_chain(chain, 4, title, error, border, text); */
/* 	Conscreen_init(); */
/* 	while(true){ */
/* 		Conscreen_screen_begin(); */
/* 		Conscreen_point p = Conscreen_screen_size(); */
/* 		RR_render(chain, (RR_point){p.x, p.y}, get, set, NULL); */
/* 		Conscreen_screen_flush(); */
/* 	} */
/* } */

/* void render(RR_context ctx, I3_context i3){ */
/* 	WM_i3_prepare(i3); */
/* 	Conscreen_screen_begin(); */
/* 	Conscreen_screen_clear(); */
/* 	Conscreen_point p = Conscreen_screen_size(); */
/* 	RR_render(ctx, (RR_point){p.x, p.y}, get, set, NULL); */
/* 	Conscreen_screen_flush(); */
/* } */

/* int main(void) */
/* { */
/* 	Conscreen_init(); */
/* 	RR_renderer dwm = R_dwm(); */
/* 	I3_context i3 = WM_i3(dwm); */
/* 	BasicWindow w1 = BW_create(NULL); */
/* 	BW_title_set(w1, "Fenster 1", 9); */
/* 	BasicWindow w2 = BW_create(NULL); */
/* 	BW_title_set(w2, "Fenster 2", 9); */
/* 	RR_context ctx = RR_context_create(); */
/* 	RR_context_make_chain(ctx, 1, dwm); */
/* 	BW_gen_chain(w1); */
/* 	BW_gen_chain(w2); */
/* 	WM_i3_set(i3, w1); */
/* 	render(ctx, i3); */
/* 	/\* Conscreen_console_get_key(); *\/ */
/* 	WM_i3_split(i3, I3_UP); */
/* 	WM_i3_split(i3, I3_LEFT); */
/* 	WM_i3_set(i3, w2); */
/* 	WM_i3_select(i3, I3_DOWN); */
/* 	WM_i3_split(i3, I3_LEFT); */
/* 	render(ctx, i3); */
/* 	Conscreen_console_get_key(); */
/* 	WM_i3_set(i3, w2); */
/* 	render(ctx, i3); */
/* 	Conscreen_console_get_key(); */
/* 	BW_error(w2, INFO, "%d", WM_i3_select_parent(i3)); */
/* 	WM_i3_set(i3, w2); */
/* 	render(ctx, i3); */
/* 	Conscreen_console_get_key(); */
/* 	WM_i3_split(i3, I3_LEFT); */
/* 	WM_i3_split(i3, I3_DOWN); */
/* 	WM_i3_split(i3, I3_RIGHT); */
/* 	WM_i3_set(i3, w1); */
/* 	render(ctx, i3); */
/* 	Conscreen_console_get_key(); */
/* 	BW_free(w1); */
/* 	BW_free(w2); */
/* 	WM_i3_free(i3); */
/* 	R_dwm_free(dwm); */
/* 	RR_context_free(ctx); */
/* 	Conscreen_deinit(); */
/* } */

typedef struct {
	BasicWindow disp;
	char *buff;
	int max;
	int index;
} Writer_info;

void writer_key(App app, char key, void *data)
{
	Writer_info *info = data;
	if(key==127){
		if(info->index>0)
			info->index--;
	} else{
		if(info->index==info->max){
			BW_error(app, WARNING, "Buffer length exceeded! %d %d", key, info->max);
			return;
		}
		info->buff[info->index++]=key;
	}
	info->buff[info->index]=0;
	BW_text_set(app, info->buff, info->index);
}

void render(RR_context ctx, AM i3){
	AM_prepare(i3);
	Conscreen_screen_begin();
	Conscreen_screen_clear();
	Conscreen_point p = Conscreen_screen_size();
	RR_render(ctx, (RR_point){p.x, p.y}, get, set, NULL);
	Conscreen_screen_flush();
}


App writer_create(const char* name, int size)
{
	App writer = AM_app_create(name, writer_key);
	Writer_info *info = malloc(sizeof(Writer_info));
	info->buff = malloc(size+1);
	info->max=size;
	info->index=0;
	info->disp= (BasicWindow)writer;

	AM_app_data_set(writer, info);
	return writer;
}

#define WRITER_COUNT 10
const char* writer_names[WRITER_COUNT] = {
"Writer 1",
"Writer 2",
"Writer 3",
"Writer 4",
"Writer 5",
"Writer 6",
"Writer 7",
"Writer 8",
"Writer 9",
"Writer 10"
};
AM app_man;
int main(void)
{
	app_man = AM_create();
	App writers[WRITER_COUNT];
	for(int i=0; i<WRITER_COUNT; i++){
		writers[i]=writer_create(writer_names[i], i*2);
		AM_register(app_man, writers[i]);
	}

	Conscreen_init();

	while(true){
		render(BW_get_context(app_man), app_man);
		AM_key_event(app_man, Conscreen_console_get_key());
	}

	Conscreen_deinit();
}
