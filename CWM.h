#pragma once
#include "Conscreen/Conscreen.h"
#include <stdbool.h>
#include "CWM_animation.h"
#include "BasicWindow.h"


struct _CWM_window;
typedef struct _CWM_window* CWM_window;

int CWM_init();

// Obtain rootwindow
BasicWindow CWM_root_get();

void CWM_render();

void CWM_error(BasicWindow w, BW_error_level level, const Conscreen_char*const format, ...);

// get pointer to new child window for give window
BasicWindow CWM_push(BasicWindow parent);

void CWM_move(BasicWindow w, BasicWindow target);

// free window and do cleanup
void CWM_remove(BasicWindow w);


void CWM_depth_set(BasicWindow w, int depth);

void CWM_pos_set_absolute(BasicWindow w, uint16_t x, uint16_t y);
void CWM_pos_set_relative(BasicWindow w, float x, float y);

void CWM_size_set_absolute(BasicWindow w, uint16_t x, uint16_t y);
void CWM_size_set_relative(BasicWindow w, float x, float y);

void CWM_opactity_set(BasicWindow w, float opacity);

void CWM_deinit();
