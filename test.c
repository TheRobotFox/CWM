#include "BasicWindow.h"
#include "CWM.h"
#include "CWM_internal.h"
#include "Conscreen/Conscreen.h"
#include "Conscreen/Conscreen_console.h"
#include "Conscreen/Conscreen_screen.h"
#include "Conscreen/Conscreen_string.h"
#include "Conscreen/List/List.h"
#include "RR.h"
#include "RR_context.h"
#include "R_border.h"
#include "R_i3.h"
#include "R_info.h"
#include "R_text.h"
#include <stdint.h>


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

void render(RR_context ctx){
	Conscreen_screen_begin();
	Conscreen_screen_clear();
	Conscreen_point p = Conscreen_screen_size();
	RR_render(ctx, (RR_point){p.x, p.y}, get, set, NULL);
	Conscreen_screen_flush();
}

int main(void)
{
	Conscreen_init();
	RR_renderer i3 = R_i3();
	BasicWindow w1 = BW_create(NULL);
	BW_title_set(w1, "Fenster 1", 9);
	BasicWindow w2 = BW_create(NULL);
	BW_title_set(w2, "Fenster 2", 9);
	RR_context ctx = RR_context_create();
	RR_context_make_chain(ctx, 1, i3);
	BW_gen_chain(w1);
	BW_gen_chain(w2);
	R_i3_set(i3, w1);
	render(ctx);
	/* Conscreen_console_get_key(); */
	R_i3_split(i3, I3_UP);
	R_i3_split(i3, I3_LEFT);
	R_i3_set(i3, w2);
	R_i3_select(i3, I3_DOWN);
	R_i3_split(i3, I3_LEFT);
	render(ctx);
	Conscreen_console_get_key();
	R_i3_set(i3, w2);
	render(ctx);
	Conscreen_console_get_key();
	BW_error(w2, INFO, "%d", R_i3_select_parent(i3));
	R_i3_set(i3, w2);
	render(ctx);
	Conscreen_console_get_key();
	R_i3_split(i3, I3_LEFT);
	R_i3_split(i3, I3_DOWN);
	R_i3_split(i3, I3_RIGHT);
	R_i3_set(i3, w1);
	render(ctx);
	Conscreen_console_get_key();
	BW_free(w1);
	BW_free(w2);
	R_i3_free(i3);
	RR_context_free(ctx);
	Conscreen_deinit();
}
