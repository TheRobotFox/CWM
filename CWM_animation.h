#include <stdint.h>
#include "Conscreen/List/List.h"

typedef float (*Interpolation)(float);

typedef union {
	void* ptr;
	int8_t b;
	int16_t s;
	int32_t i;
	int64_t l;
	float f;
	double d;
} CWM_generic;

typedef struct {
	uint32_t time_start,
			 time_length;
	Interpolation interpolation;
	CWM_generic start;
	CWM_generic end;
	CWM_generic *ptr;
	void (*act)(float t, CWM_generic, CWM_generic, CWM_generic*);
} CWM_animation_event;

typedef struct CWM_animation_struct* CWM_animation;
typedef List CWM_animation_group;

float CWM_animation_interpolation_smoothstep(float t);
float CWM_animation_interpolation_linear(float t);
float CWM_animation_interpolation_smootherstep(float t);

void CWM_animation_update();
void CWM_animation_free();
void CWM_animation_tick_advance(int32_t n);

void CWM_animation_register_group(CWM_animation_group group);
void CWM_animation_unregister_group(CWM_animation_group group);
void CWM_animation_register_struct(CWM_animation anim);
void CWM_animation_unregister_struct(CWM_animation anim);

CWM_animation_group CWM_animation_group_create();
void CWM_animation_group_register(CWM_animation_group group, CWM_animation anim);
void CWM_animation_group_unregister(CWM_animation_group group, CWM_animation anim);
void CWM_animation_group_update(CWM_animation_group group);
void CWM_animation_group_free(CWM_animation_group group);

void CWM_animation_struct_update(CWM_animation anim);
void CWM_animation_struct_free(CWM_animation anim);

CWM_animation CWM_animation_event_create(CWM_animation_event *event);
CWM_animation CWM_animation_interval_create(uint32_t interval, void (*act)(void));

