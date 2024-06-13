#include "CWM/CWM.h"

struct _CWMM_window;
typedef struct _CWMM_window* CWMM_window;

struct _CWMM_manager;
typedef struct _CWMM_manager* CWMM_manager;

CWMM_manager CWMM_manager_create();

CWMM_window CWMM_window_register(CWMM_manager m);
void CWMM_window_unregister(CWMM_manager m, CWMM_window w);

typedef void (*CWMM_callback)(char key, void *data);

void CWMM_window_callback_set(CWMM_window w, CWMM_callback callback);
void* CWMM_window_data_get(CWMM_window w);
void CWMM_window_data_set(CWMM_window w, void *data);
