#pragma  once
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Conscreen/Conscreen_ANSI.h"
#include "CWM_render.h"

extern uint32_t tick;

typedef struct{
	enum CWM_window_point_type type;
	union{
		i16vec2 absolute;
		f32vec2 relative;
	};
} CWM_point;

typedef struct CWM_string{
	Conscreen_char *str;
	size_t len;
	Conscreen_ansi style;
} CWM_string;

// TODO: User vars hashmap
typedef struct CWM_window{
	struct CWM_window *parent;
	List child_windows;

	struct{
		uint8_t show : 1;              	// show window or not
		uint8_t border_show: 1;       	// show window border

		CWM_point pos; 	// position transformation
						// ONLY USE CWM_window_pos_get() or CWM_window_pos_get_content() to obtain position

		CWM_point size;	// size transformation
						// ONLY USE CWM_window_size_get() or CWM_window_size_get_content() to obtain size

		Conscreen_ansi border_style; // Border color / style

		int16_t Z_depth;
		float opacity;
	} window;

	struct{
		CWM_string title;                // window title

		//TODO: refactor error
		CWM_string error;                // window error
		CWM_error_level error_level;


	} info;
	CWM_renderer renderer;

} *CWM_window;

extern CWM_window root_window;

void CWM_internal_init_check();


int16_t maxi16(int16_t a, int16_t b);
int16_t mini16(int16_t a, int16_t b);

void CWM_internal_string_set(CWM_string *string, const Conscreen_char *text, size_t length);

CWM_window CWM_internal_window_create();

// internal function for creating a template window stuct instance pointer
void CWM_internal_console_write(char* string, int size);

// free window struct
// Conscreen_internal_window_free_cleanup should always be called after
void CWM_internal_window_free(CWM_window w);

// remove zero pointers from freed child_windows
void CWM_internal_window_free_cleanup(CWM_window parent);


void CWM_internal_window_render(CWM_window w);
