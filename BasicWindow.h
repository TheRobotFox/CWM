#pragma once
#include "RR.h"
#include "Conscreen/Conscreen.h"

//Basic Window infrastructure

typedef enum {
	NONE,
	INFO,
	WARNING,
	ERROR,
	FATAL
} BW_error_level;

typedef enum{
    CR_PRE,
    CR_POST
} BW_render_kind;

struct _BasicWindow;
typedef struct _BasicWindow* BasicWindow;


BasicWindow BW_create(BasicWindow buffer);

void BW_error(BasicWindow w, BW_error_level level, const Conscreen_char*const format, ...);

// free window and do cleanup
void BW_free(BasicWindow w);

// Set Attributes
void BW_opacity_set(BasicWindow w, float opacity);
void BW_title_set(BasicWindow w, const Conscreen_char* title, size_t length);

void BW_text_set(BasicWindow w, const Conscreen_char* text, size_t length);
void BW_text_set_style(BasicWindow w, Conscreen_ansi style);

// set renderers
RR_renderer BW_get_renderer(BasicWindow w, const char* name);
int BW_renderer_toggle(BasicWindow w, const char* name, bool active);
int BW_add(BasicWindow w, const char* name, BW_render_kind kind, RR_renderer r);
int BW_rm(BasicWindow w, const char* name);
void BW_gen_chain(BasicWindow w);
