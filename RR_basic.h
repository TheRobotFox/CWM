#pragma once
#include "RR.h"

typedef enum {
    CR_OPACITY,
    CR_TITLE,
    CR_ERROR,
    CR_BORDER,
	CR_TEXT,
	CR_CHILDREN,
    _CR_END
} CWM_renderers;


typedef enum {
	NONE,
	INFO,
	WARNING,
	ERROR,
	FATAL
} CWM_error_level;
