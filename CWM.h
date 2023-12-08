#pragma once
#include "Conscreen/Conscreen.h"
#include <stdbool.h>
#include "CWM_animation.h"


enum CWM_window_point_type{
	CWM_ABSOLUTE,
	CWM_RELATIVE,
	// Conscreen_AUTOMATIC TODO
};

typedef struct CWM_window* CWM_window;

typedef struct{
	float x,y;
} f32vec2;

typedef enum {
	NONE,
	INFO,
	WARNING,
	ERROR,
	FATAL
} CWM_error_level;

typedef Conscreen_point i16vec2;

int CWM_init();

// Obtain rootwindow
CWM_window CWM_window_root_get();

void CWM_render();

void CWM_error(CWM_window screen, CWM_error_level level, const Conscreen_char*const format, ...);

// get pointer to new child window for give window
CWM_window CWM_window_push(CWM_window parent);

// free window and do cleanup
void CWM_window_remove(CWM_window w);

void CWM_window_title_set(CWM_window w, const Conscreen_char* title, size_t length);
void CWM_window_text_set(CWM_window w, const Conscreen_char* text, size_t length);
void CWM_window_depth_set(CWM_window w, int16_t depth);
void CWM_window_pos_set_absolute(CWM_window w, uint16_t x, uint16_t y);
void CWM_window_pos_set_relative(CWM_window w, float x, float y);

void CWM_window_size_set_absolute(CWM_window w, uint16_t x, uint16_t y);
void CWM_window_size_set_relative(CWM_window w, float x, float y);

void CWM_window_opactity_set(CWM_window w, float opacity);
void CWM_window_border_show(CWM_window w, bool show);

i16vec2 CWM_window_size_get(CWM_window w);
i16vec2 CWM_window_size_get_content(CWM_window w);
i16vec2 CWM_window_pos_get(CWM_window w);
i16vec2 CWM_window_pos_get_content(CWM_window w);


void CWM_window_renderer_text(CWM_window w, Conscreen_event event, Conscreen_pixel *content, void *data);

void CWM_deinit();
