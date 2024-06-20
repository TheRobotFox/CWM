#pragma once
#include "CWM.h"
#include "RR.h"

RR_renderer R_border();
void R_border_free(RR_renderer r);

void R_border_set_style(RR_renderer r, Conscreen_ansi style);
