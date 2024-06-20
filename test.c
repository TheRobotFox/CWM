#include "CWM.h"
#include "CWM_internal.h"
#include "Conscreen/Conscreen.h"
#include "Conscreen/Conscreen_console.h"
#include "Conscreen/Conscreen_screen.h"
#include "RR.h"
#include "RR_context.h"
#include "R_border.h"
#include "R_text.h"
#include <stdint.h>


int main(void)
{
	CWM_init();
	CWM_window root = CWM_window_root_get();
	CWM_window new_window = CWM_window_push(root);
	CWM_window left = CWM_window_push(root);
	CWM_window_pos_set_absolute(left, 0, 0);
	CWM_window_size_set_relative(left, 0.75, 0.75);
	CWM_window_pos_set_relative(new_window, 0.25, 0.25);
	CWM_window current = new_window;
	CWM_window_title_set(new_window, STR("new window"), 10);
	CWM_window_title_set(left, STR("left window"), 11);
	CWM_window_depth_set(left, 0);
	CWM_window_depth_set(new_window, 1);
	CWM_window_renderer_toggle(new_window, "TEXT", true);
	CWM_window_renderer_toggle(left, "TEXT", true);

	bool insert=0;
	Conscreen_string buffer = Conscreen_string_create();
	CWM_error(current, INFO, STR("NORMAL MODE"));
	while (true){
		CWM_render();
		char c = Conscreen_console_get_key();
		if(insert){
			if(c==27){
				CWM_error(current, INFO, STR("NORMAL MODE"));
				insert=false;
				continue;
			}
			Conscreen_string_push(buffer,c);
			CWM_window_text_set(current, Conscreen_string_start(buffer), Conscreen_string_length(buffer));
		}else if(c=='i'){
			insert=true;
			CWM_error(current, INFO, STR("INSERT MODE"));
		} else if(c==23){
			CWM_error(current, INFO, STR("WINDOW SELECT"));
			CWM_render();
			while (true) {
				char c = getchar();
				if(c=='h' || c=='j'){
					current= left;
					CWM_window_depth_set(left, 1);
					CWM_window_depth_set(new_window, 0);
					break;
				}
				if(c=='l' || c=='k'){
					current= new_window;
					CWM_window_depth_set(left, 0);
					CWM_window_depth_set(new_window, 1);
					break;
				}
			}
		}else
			CWM_error(current, ERROR, STR("Unknown Command %c (%d)"), c, (int)c);
	}
}


/* int main(void) */
/* { */
/* 	RR_renderer text = R_text(); */
/* 	R_text_set_text(text, "YOOOOOO", 7); */
/* 	RR_renderer border = R_border(); */
/* 	R_border_set_title(border, "App", 3); */
/* 	R_border_set_error(border, INFO, "Heyy, that's pretty cool! %d", 5); */
/* 	RR_context chain = RR_make_render_chain(2, border, text); */
/* 	Conscreen_init(); */
/* 	while(true){ */
/* 		Conscreen_screen_begin(); */
/* 		Conscreen_point p = Conscreen_screen_size(); */
/* 		RR_render(chain, (RR_point){p.x, p.y}, get, set, NULL); */
/* 		Conscreen_screen_flush(); */
/* 	} */
/* } */
