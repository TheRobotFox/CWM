#pragma once
#include "Conscreen/Conscreen.h"
#include <stdbool.h>
#include "CWM_animation.h"
#include "RR.h"

typedef RR_renderer (*f_CWM_renderer_create)(void);
typedef void (*f_CWM_renderer_free)(RR_renderer);

struct _CWM_window;
typedef struct _CWM_window* CWM_window;

typedef enum {
	NONE,
	INFO,
	WARNING,
	ERROR,
	FATAL
} CWM_error_level;

typedef enum{
    CR_PRE,
    CR_POST,
    CR_DATA
} CWM_render_kind;

int CWM_init();

// Obtain rootwindow
CWM_window CWM_window_root_get();

void CWM_render();

void CWM_error(CWM_window w, CWM_error_level level, const Conscreen_char*const format, ...);

// get pointer to new child window for give window
CWM_window CWM_window_push(CWM_window parent);

void CWM_window_move(CWM_window w, CWM_window target);

// free window and do cleanup
void CWM_window_remove(CWM_window w);

// Set Attributes
void CWM_window_title_set(CWM_window w, const Conscreen_char* title, size_t length);

void CWM_window_text_set(CWM_window w, const Conscreen_char* text, size_t length);
void CWM_window_text_set_style(CWM_window w, Conscreen_ansi style);


void CWM_window_depth_set(CWM_window w, int depth);

void CWM_window_pos_set_absolute(CWM_window w, uint16_t x, uint16_t y);
void CWM_window_pos_set_relative(CWM_window w, float x, float y);

void CWM_window_size_set_absolute(CWM_window w, uint16_t x, uint16_t y);
void CWM_window_size_set_relative(CWM_window w, float x, float y);

void CWM_window_opactity_set(CWM_window w, float opacity);

RR_renderer CWM_window_get_renderer(CWM_window w, const char* name);
bool CWM_window_renderer_toggle(CWM_window w, const char* name, bool active);
bool CWM_window_add(CWM_window w, const char* name, CWM_render_kind kind, RR_renderer r);
bool CWM_window_rm(CWM_window w, const char* name);
void CWM_window_gen_chain(CWM_window w);


void CWM_deinit();
