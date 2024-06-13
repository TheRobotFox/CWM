
#include <stdint.h>
struct _RR_context;
typedef struct _RR_context* RR_context;

typedef void (*RR_render_function)(RR_context context, void *data);


struct _RR_renderer
typedef struct _RR_renderer* RR_renderer;

CWM_renderer CWM_renderer_create(RR_render_function func);
void CWM_renderer_data_set(CWM_renderer renderer, void *data);

typedef void (*RR_set_function)(RR_context context, int16_t x, int16_t y, void *data);
typedef void* (*RR_get_function)(RR_context context, int16_t x, int16_t y);
void RR_set(RR_context context, int16_t x, int16_t y, void *data);
void* RR_get(RR_context context, int16_t x, int16_t y);
