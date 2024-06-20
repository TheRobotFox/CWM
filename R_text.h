#pragma once
#include "Conscreen/Conscreen_ANSI.h"
#include "Conscreen/Conscreen_string.h"
#include "RR.h"
#include <stddef.h>

RR_renderer R_text();
void R_text_free(RR_renderer r);

void R_text_set_text(RR_renderer r, const Conscreen_char *str, size_t len);
void R_text_set_style(RR_renderer r, Conscreen_ansi style);
