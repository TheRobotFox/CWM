#pragma once

#include "CWM.h"
#include "Conscreen/Conscreen_ANSI.h"
#include "RR.h"
#include <stddef.h>

enum INFO_side {
	INFO_TOP,
	INFO_BOTTOM,
	INFO_LEFT,
	INFO_RIGHT,
};

enum INFO_align {
    INFO_START,
    INFO_CENTER,
    INFO_END
};

RR_renderer R_info();
void R_info_free(RR_renderer r);
void R_info_set_text(RR_renderer r, const char* name, size_t length);
void R_info_set_style(RR_renderer r, Conscreen_ansi style);
void R_info_set_clamp(RR_renderer r, const char* clamp);
void R_info_set_side(RR_renderer r, enum INFO_side side);
void R_info_set_align(RR_renderer r, enum INFO_align align);

void R_info_set_level(RR_renderer r, CWM_error_level level);

void R_info_vprintf(RR_renderer r, const Conscreen_char* format, va_list arg);
void R_info_printf(RR_renderer r, const Conscreen_char* format, ...);
