#pragma  once
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "CWM.h"
#include "Conscreen/Conscreen_ANSI.h"
#include "Conscreen/Conscreen_screen.h"
#include "Conscreen/Conscreen_string.h"
#include "BasicWindow_internal.h"
#include "R_dwm.h"

extern uint32_t tick;


typedef struct {
	Conscreen_char *str;
	size_t len;
	Conscreen_ansi style;
} CWM_string;

// TODO: User vars hashmap
typedef struct _CWM_window{
	_BasicWindow window;
	struct _CWM_window *parent;
	LIST(CWM_window) children;
	DWM_window frame;

} _CWM_window;

extern BasicWindow root_window;

void CWM_internal_init_check();

void CWM_internal_string_set(CWM_string *string,const Conscreen_char *text, size_t length);

CWM_window CWM_internal_create();
void CWM_internal_push(CWM_window parent, CWM_window w);
void CWM_internal_remove(CWM_window w);

int16_t maxi16(int16_t a, int16_t b);
int16_t mini16(int16_t a, int16_t b);

// internal function for creating a template window stuct instance pointer
void CWM_internal_console_write(char* string, int size);

// free window struct
// Conscreen_internal_free_cleanup should always be called after
void CWM_internal_free(BasicWindow w);

// remove zero pointers from freed child_windows
void CWM_internal_free_cleanup(BasicWindow parent);


void CWM_internal_render(BasicWindow w);

int CWM_char_is_empty(Conscreen_char c);

void CWM_gen_chains(BasicWindow w);
