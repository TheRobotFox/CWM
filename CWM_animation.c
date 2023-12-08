#include "CWM_animation.h"

typedef struct {
	uint32_t interval;
	uint32_t last_call;
	void (*act)(void);
} CWM_animation_interval;

typedef enum {
	EVENT,
	INTERVAL,
} CWM_animation_type;

struct CWM_animation_struct{
	CWM_animation_type type;
	union{
		CWM_animation_event event;
		CWM_animation_interval interval;
	};
};

static uint32_t tick=0;
static List registred=0;
static List internal_group=0;

void CWM_animation_tick_advance(int32_t n)
{
	extern uint32_t tick;
	tick+=n;
}

static void CWM_animation_interval_update(CWM_animation_interval *interval)
{
	for(; interval->last_call<tick; interval->last_call+=interval->interval)
		interval->act();
}

static void CWM_animation_event_update(CWM_animation_event *event)
{
	float t = (float)(tick-event->time_start)/event->time_length;
	if(t<0 || t>1)
		return;
	t=event->interpolation(t);
	event->act(t, event->start, event->end, event->ptr);
}


void CWM_animation_group_update(List group)
{
	List_forward(group, CWM_animation_struct_update);
}

void CWM_animation_unregister_struct(CWM_animation event)
{
	if(internal_group){
		List_rme(internal_group, event);
	}
}

void CWM_animation_struct_update(CWM_animation anim)
{
	switch(anim->type)
	{
	case INTERVAL:
		CWM_animation_interval_update(&anim->interval);
		break;
	case EVENT:
		CWM_animation_event_update(&anim->event);
		break;
	}
}

void CWM_animation_register_struct(CWM_animation event)
{
	if(!internal_group){
		internal_group = CWM_animation_group_create();
		CWM_animation_register_group(internal_group);
	}
	List_push(internal_group, event);
}

void CWM_animation_update()
{
	if(registred)
		List_forward(registred, CWM_animation_group_update);
}

void CWM_animation_register_group(List group)
{
	if(!group)
		return;

	if(!registred){
		registred = List_create(sizeof(List));
	}
	List_push(registred, group);
}

void CWM_animation_unregister_group(List group)
{
	if(registred && group)
		List_rme(registred, group);
}

void CWM_animation_free()
{
	List_forward(registred, CWM_animation_group_free);
	List_free(registred);
	registred=0;
}

List CWM_animation_group_create()
{
	return List_create(sizeof(struct CWM_animation_struct));
}

void CWM_animation_group_register(CWM_animation_group group, CWM_animation anim)
{
	List_push(group, anim);
}
void CWM_animation_group_unregister(CWM_animation_group group, CWM_animation anim)
{
	List_rme(group, anim);
}

void CWM_animation_group_free(CWM_animation_group group)
{
	List_free(group);
}

static CWM_animation CWM_animation_struct_create()
{
	return malloc(sizeof(struct CWM_animation_struct));
}

CWM_animation CWM_animation_interval_create(uint32_t interval, void (*act)(void))
{
	CWM_animation s = CWM_animation_struct_create();
	s->type=INTERVAL;
	s->interval.interval=interval;
	s->interval.act=act;
	s->interval.last_call=tick;
	return s;
}

void CWM_animation_struct_free(CWM_animation anim)
{
	free(anim);
}

float CWM_animation_interpolation_linear(float t)
{
	return t;
}

float CWM_animation_interpolation_smoothstep(float t)
{
	return t * t * (3 - 2 * t);
}

float CWM_animation_interpolation_smootherstep(float t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}
