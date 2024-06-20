#include "CWM.h"

typedef CWM_window CWMM_window;
enum CWMM_split {
    HORIZONTAL,
    VERTICAL
};

CWMM_window CWMM_manager_create(CWM_window parent);
void CWMM_manager_free(CWMM_window m);


typedef void (*CWMM_callback)(char key, void *data);

CWMM_window CWMM_window_push(CWMM_window manager, CWMM_callback callback);

void* CWMM_window_data_get(CWMM_window w);
void CWMM_window_data_set(CWMM_window w, void *data);
