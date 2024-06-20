#include "RR_basic.h"

typedef struct _RR_window {
	List renderers;

	bool chain_dirty;

	RR_context render_ctx;

} _CWM_window;

typedef enum {
    CR_OPACITY,
    CR_TITLE,
    CR_ERROR,
    CR_BORDER,
	CR_TEXT,
	CR_CHILDREN,
    _CR_END
} CWM_renderers;

typedef enum{
    CR_PRE,
    CR_POST
} CWM_render_kind;
