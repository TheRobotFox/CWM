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

typedef enum {
VBRANCH=0, HBRANCH=1, LEAF = 2
} I3_type;

typedef struct _Leaf {
    BasicWindow win;
    DWM_window frame;
} _Leaf;

typedef struct _I3_node {
    I3_type type;
    uint16_t fracs; // how many fractions the window is big
    struct _I3_node *parent;
    union {
        _Leaf leaf;
        LIST(I3_node) nodes;
    };
} _I3_node;
typedef _I3_node* I3_node;
typedef struct _I3_context {
    I3_node layout;
    I3_node selected;
    float cursor_pos[2];
    bool dirty;
    RR_renderer dwm;
} _I3_context;


void set(RR_context ctx, int16_t x, int16_t y, RR_pixel pix)
{
	Conscreen_screen_set(x, y, pix);
}
RR_pixel get(RR_context ctx, int16_t x, int16_t y)
{
	return Conscreen_screen_get(x, y);
}

void app_callback(App app, char key, void *data)
{
    I3_context ctx = data;
    char buff[2048];
    int len = snprintf(buff, 2048, "Cursor: (%.2f, %.2f)\nselect: %p", ctx->cursor_pos[0], ctx->cursor_pos[1], ctx->selected);
    BW_text_set(app, buff, len);
    BW_error(app, INFO, "Pressed %c", key);
}

void render(RR_context ctx, AM i3){
	AM_prepare(i3);
	Conscreen_screen_begin();
	Conscreen_screen_clear();
	Conscreen_point p = Conscreen_screen_size();
	RR_render(ctx, (RR_point){p.x, p.y}, get, set, NULL);
	Conscreen_screen_flush();
}

enum AM_state
{
    INSERT, NAVIGATE
};

typedef struct {
    _BasicWindow win;
    LIST(App) apps;
    I3_context i3;
    enum AM_state state;
} _AM_window;

int main(void)
{
	AM app_man = AM_create();
	Conscreen_init();
    App debug = AM_app_create("Debug App", app_callback);

    AM_app_data_set(debug, ((_AM_window*)app_man)->i3);

	AM_register(app_man, debug);

	while(true){
		render(BW_get_context(app_man), app_man);
		AM_key_event(app_man, Conscreen_console_get_key());
	}

	Conscreen_deinit();
}
