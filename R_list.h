#pragma once
#include "Conscreen/List/List.h"
#include "RR.h"
#include <stddef.h>

// use '\t' for split
typedef char* (*R_list_format)(void *element);

RR_renderer R_list();
void R_list_down(RR_renderer r);
void R_list_up(RR_renderer r);
size_t R_list_get(RR_renderer r);
void R_list_set(RR_renderer r, List l, R_list_format format);
void R_list_free(RR_renderer r);
