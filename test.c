#include "CWM_internal.h"

static void CWM_debug_print_window_List(CWM_window w)
{
	//PRINTF(STR("debug printing " _STR "\n"), w->info.title.str);
}

int main(void)
{
	CWM_init();
	CWM_window root = CWM_window_root_get();
	CWM_window new_window = CWM_window_push(root);
	CWM_window left = CWM_window_push(root);
	CWM_window_pos_set_absolute(left, 0, 0);
	CWM_window current = new_window;
	CWM_window_title_set(new_window, STR("new window"), 10);
	CWM_window_title_set(left, STR("left window"), 11);
	CWM_debug_print_window_List(root);
	CWM_debug_print_window_List(new_window);
	CWM_window_depth_set(left, 0);
	CWM_window_depth_set(new_window, 1);

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
				if(c=='h'){
					current= left;
					CWM_window_depth_set(left, 1);
					CWM_window_depth_set(new_window, 0);
					break;
				}
				if(c=='l'){
					current= new_window;
					CWM_window_depth_set(left, 0);
					CWM_window_depth_set(new_window, 1);
					break;
				}
			}
		}else
			CWM_error(current, ERROR, STR("Unknown Command %c (%d)"), c, c);
	}
}
