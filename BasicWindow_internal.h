#pragma once
#include "Conscreen/List/List.h"
#include "BasicWindow.h"

typedef struct _BasicWindow {
	List renderers;
	bool chain_dirty;
	RR_context render_ctx;

} _BasicWindow;

void BW_internal_free(BasicWindow w);

void BW_internal_gen_chain(BasicWindow w);
RR_context BW_get_context(BasicWindow w);
