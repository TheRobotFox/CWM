#include "CWM_internal.h"

struct SelectInfo
{
    size_t index;
    LIST(CWM_string) lines;
};

void CWM_renderers_select(Conscreen_pixel *content, i16vec2 size, void *select_info);
void CWM_renderers_text(Conscreen_pixel *content, i16vec2 size, void *cwm_string);
