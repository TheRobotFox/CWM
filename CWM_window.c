#include "CWM_internal.h"
#include <math.h>


static int CWM_internal_window_text_line_next(const Conscreen_char *text, int length, int width, int index)
{

	int current=0;
	for(int i=0; i<length; i++){

		current++;
		if(text[i-1]=='\n' || current>=width){

			if(i>index)
				return i;
			current=0;
		}
	}
	return -1;
}

static int CWM_internal_window_text_lines_count(const Conscreen_char *text, int length, int width, int index)
{

	int count=0;
	do{
		index=CWM_internal_window_text_line_next(text,length,width,index);
		count++;
	}while(index!=-1);
	return count;
}

static int CWM_internal_window_text_crop(const Conscreen_char *str, uint32_t length, i16vec2 size)
{
	int start=0;
	while(CWM_internal_window_text_lines_count(str, length,size.x,start)>size.y){
		start=CWM_internal_window_text_line_next(str,length,size.x,start);
	}
	return start;
}

void CWM_window_renderer_text(CWM_window w, Conscreen_event event, Conscreen_pixel *content, void *data)
{
	i16vec2 pos, size;

	size=CWM_window_size_get_content(w); // use window body size

	// draw opacity
	if(size.x && size.y){

		CWM_string *string = data;
		Conscreen_pixel p = {.style=string->style};
		const Conscreen_char* text = string->str;
		pos=CWM_window_pos_get_content(w);

		// Crop text to fit screen
		int start=CWM_internal_window_text_crop(text,string->len,size);
		for(int i=0; i<string->len; i++)
		{
			p.character = text[i];
			*content++ =p;
		}
	}

}

uint32_t window_title_wrap=0;

static void CWM_window_forward_handler(void (*func)(void*), void *arg)
{
	func(*(CWM_window*)arg);
}

CWM_window CWM_internal_window_create()
{
	CWM_window w = malloc(sizeof(struct CWM_window));

	w->child_windows=List_create(sizeof(CWM_window));

	w->parent=0;

	CWM_string tmp_string = {.str=0, .len=0, .style={.nec=1}};
	w->info.error=tmp_string;
	w->info.title=tmp_string;

	w->window.Z_depth=0;
	w->window.show=1;
	w->window.pos.type=CWM_RELATIVE;
	w->window.size.type=CWM_RELATIVE;
	w->window.border_show=1;
	w->window.opacity=0.5;

	w->window.pos.relative.x=.25;
	w->window.pos.relative.y=.25;
	w->window.size.relative.x=.75;
	w->window.size.relative.y=.75;

	w->window.border_style=CONSCREEN_ANSI_DEFAULT(255,0,255);
	w->window.border_style.background=CONSCREEN_RGB(0,255,255);

	w->info.title.style=w->window.border_style;
	w->info.error.style=CONSCREEN_ANSI_NORMAL;
	w->info.error_level=NONE;

	w->renderer=NULL;

	//w->window.border_style=CONSCREEN_ANSI_DEFAULT(255,255,0);


	return w;
}


void CWM_internal_window_free(CWM_window w)
{
	// recursively free child_windows
	List_forward(w->child_windows, CWM_internal_window_free);

	// free elements
	List_free(w->child_windows);
	free(w);

}


void CWM_window_remove(CWM_window w)
{
	CWM_internal_init_check();

	// remove from parent
	if(w->parent){
		List_rme(w->parent->child_windows, w);
	}

	CWM_internal_window_free(w);
}


CWM_window CWM_window_root_get()
{
	CWM_internal_init_check();
	return root_window;
}


CWM_window CWM_window_push(CWM_window parent)
{
	CWM_internal_init_check();
	CWM_window w = CWM_internal_window_create();
	w->parent=parent;
	List_push(parent->child_windows, &w);
	return w;
}

void CWM_window_title_set(CWM_window w, const Conscreen_char* title, size_t length)
{
	CWM_internal_string_set(&w->content.title,title,length);
}

struct Error_display
{
	Conscreen_char *name;
	Conscreen_ansi style;
};
static struct Error_display errors[] = { [INFO] = {STR("INFO"), CONSCREEN_ANSI_DEFAULT(0,128,255)}, [WARNING] = {STR("WARNING"), CONSCREEN_ANSI_DEFAULT(255,180,0)}, [ERROR] = {STR("ERROR"), CONSCREEN_ANSI_DEFAULT(255,50,50)}, [FATAL] = {STR("FATAL"), CONSCREEN_ANSI_DEFAULT(255,0,0)}};

void CWM_error(CWM_window w, CWM_error_level level, const Conscreen_char*const format, ...)
{
	va_list arg;
	va_start(arg, format);
	Conscreen_string string = Conscreen_string_create();
	Conscreen_string_vsprintf(string, format, arg);
	va_end(arg);
	CWM_internal_string_set(&w->content.error,Conscreen_string_start(string),Conscreen_string_length(string));	w->content.level = level;
}

void CWM_window_depth_set(CWM_window w, int16_t depth)
{
	w->window.Z_depth=depth;
}

void CWM_window_pos_set_absolute(CWM_window w, uint16_t x, uint16_t y)
{
	w->window.pos.type=CWM_ABSOLUTE;
	w->window.pos.absolute.x=x;
	w->window.pos.absolute.y=y;
}

void CWM_window_pos_set_relative(CWM_window w, float x, float y)
{
	w->window.pos.type=CWM_RELATIVE;
	w->window.pos.relative.x=x;
	w->window.pos.relative.y=y;
}
void CWM_window_size_set_absolute(CWM_window w, uint16_t x, uint16_t y)
{
	w->window.size.type=CWM_ABSOLUTE;
	w->window.size.absolute.x=x;
	w->window.size.absolute.y=y;
}

void CWM_window_size_set_relative(CWM_window w, float x, float y)
{
	w->window.size.type=CWM_RELATIVE;
	w->window.size.relative.x=x;
	w->window.size.relative.y=y;
}

void CWM_window_opactity_set(CWM_window w, float opacity)
{
	if(opacity<0)
		opacity=0;
	else if(opacity>1)
		opacity=1;
	w->window.opacity=opacity;
}

void CWM_window_border_show(CWM_window w, bool show){
	w->window.border_show=show;
}

i16vec2 CWM_window_size_get(CWM_window w)
{
	i16vec2 size;

	if(w){
		i16vec2 parrent_size = CWM_window_size_get_content(w->parent), parrent_pos=CWM_window_pos_get_content(w->parent);
		i16vec2 pos=CWM_window_pos_get(w);
		switch (w->window.size.type)
		{
			case CWM_ABSOLUTE:
			size.x = w->window.size.absolute.x;
			size.y = w->window.size.absolute.y;
			break;
			case CWM_RELATIVE:
			size.x = w->window.size.relative.x*parrent_size.x+.4;
			size.y = w->window.size.relative.y*parrent_size.y+.4;
			break;
		}

		size.x = mini16(size.x, parrent_size.x+parrent_pos.x-pos.x);
		size.y = mini16(size.y, parrent_size.y+parrent_pos.y-pos.y);
	}else{
		size=Conscreen_screen_size();
	}
	return size;
}

i16vec2 CWM_window_size_get_content(CWM_window w)
{
	i16vec2 size=CWM_window_size_get(w);
	if(w && (size.x || size.y) && w->window.border_show){
		size.x-=2;
		size.y-=2;
	}
	return size;
}

i16vec2 CWM_window_pos_get(CWM_window w)
{
	i16vec2 pos;

	if(w){
		i16vec2 parrent_pos = CWM_window_pos_get_content(w->parent);

		switch (w->window.pos.type)
		{

			case CWM_ABSOLUTE:
			pos.x = maxi16(w->window.pos.absolute.x, parrent_pos.x);
			pos.y = maxi16(w->window.pos.absolute.y, parrent_pos.y);
			break;

			case CWM_RELATIVE:{

			i16vec2 parrent_size = CWM_window_size_get_content(w->parent);
			pos.x = parrent_pos.x + (parrent_size.x * w->window.pos.relative.x);
			pos.y = parrent_pos.y + (parrent_size.y * w->window.pos.relative.y);
			break;
			}

		}
	}else{
		//TODO: Check for zero calls
		pos.x=0;
		pos.y=0;
	}

	return pos;
}

i16vec2 CWM_window_pos_get_content(CWM_window w)
{
	i16vec2 pos=CWM_window_pos_get(w);
	if(w){

		if(w->window.border_show){
			pos.x++;
			pos.y++;
		}
	}
	return pos;
}

static Conscreen_color CWM_internal_color_blend(Conscreen_color *a, Conscreen_color *b, float opacity)
{

	Conscreen_color c;
	c.r=a->r*(1-opacity)+b->r*opacity;
	c.g=a->g*(1-opacity)+b->g*opacity;
	c.b=a->b*(1-opacity)+b->b*opacity;
	return c;
}


enum CWM_side{
	CWM_TOP,
	CWM_BOTTOM,
	CWM_LEFT,
	CWM_RIGHT,
};

const Conscreen_char *clamps[4][2] = {
	{STR(">|"), STR("|<")},
	{STR("<["), STR("]>")},
	{STR("∨—"), STR("—∧")},
	{STR("∨—"), STR("—∧")}
};

struct pi16vec2{
	int16_t *x, *y;
};

// TODO: draw string
static int16_t CWM_internal_window_draw_clamp(CWM_window w, enum CWM_side side, int16_t length)
{
	const Conscreen_char **clamp;
	i16vec2 _pos = CWM_window_pos_get(w),
			_size = CWM_window_size_get(w);
	struct pi16vec2 pos,
			 size;

	Conscreen_pixel p = {.style=w->window.border_style};

	// swap axies
	if(side<2){
		pos.x=&_pos.x;
		pos.y=&_pos.y;
		size.x=&_size.x;
		size.y=&_size.y;
	}else{
		pos.y=&_pos.x;
		pos.x=&_pos.y;
		size.y=&_size.x;
		size.x=&_size.y;
	}

	if(side%2)
		*pos.y+=*size.y-1;



	clamp = clamps[side];
	int16_t clamp_length = STRLEN(clamp[0]);

	length = mini16(length, *size.x-clamp_length-2);

	*pos.x += (*size.x-length)/2-clamp_length;

	for(int j=0; j<2; j++){
		for(int i=0; i<clamp_length; i++){
			p.character=clamp[j][i];
			Conscreen_screen_set(*pos.x, *pos.y, p);
			(*pos.x)++;
		}
		(*pos.x)+=length;
	}
	return length;
}

static void CWM_internal_window_draw_title(CWM_window w)
{
	int16_t length = w->content.title.len;
	length = CWM_internal_window_draw_clamp(w, CWM_TOP, length);

	int16_t cutoff = w->content.title.len-length,
			offset=0;
	if(cutoff)
		offset=window_title_wrap%cutoff;

	Conscreen_pixel p={.style=w->content.title.style};
	const Conscreen_char *str = w->content.title.str;

	i16vec2 pos = CWM_window_pos_get(w),
			size = CWM_window_size_get(w);

	int16_t start = pos.x+(size.x-length)/2;

	for(int i=0; i<length; i++)
	{
		p.character=str[i+offset];
		Conscreen_screen_set(start++,pos.y, p);
	}

}

static void CWM_internal_window_draw_error(CWM_window w)
{
	Conscreen_char *error_name = errors[w->content.level].name;
	int16_t error_length = STRLEN(error_name);
	int16_t length = w->content.error.len+error_length+3;

	length = CWM_internal_window_draw_clamp(w, CWM_BOTTOM, length);

	int16_t cutoff = w->content.error.len-length,
			offset=0;
	if(cutoff)
		offset=window_title_wrap%cutoff;

	Conscreen_pixel p={.style=w->content.error.style};
	const Conscreen_char *str = w->content.error.str;

	i16vec2 pos = CWM_window_pos_get(w),
			size = CWM_window_size_get(w);
	pos.y+=size.y-1;

	int16_t start = pos.x+(size.x-length)/2;

	int i=0;
	p.character = CHR('[');
	if(++i<length)
		Conscreen_screen_set(start++,pos.y, p);

	p.style = errors[w->content.level].style;
	for(int j=0; j<error_length && i<length; j++,i++)
	{
		p.character=error_name[j];
		Conscreen_screen_set(start++,pos.y, p);
	}
	p.style = w->content.error.style;

	p.character = CHR(']');

	if(++i<length)
		Conscreen_screen_set(start++,pos.y, p);
	p.character = CHR(' ');

	if(++i<length)
		Conscreen_screen_set(start++,pos.y, p);

	for(int j=0; i<length; j++, i++)
	{
		p.character=str[j+offset];
		Conscreen_screen_set(start++,pos.y, p);
	}
}

static void CWM_internal_window_draw_border(CWM_window w)
{
	i16vec2 pos  = CWM_window_pos_get(w),
		size = CWM_window_size_get(w);

	if(size.x && size.y){

		Conscreen_ansi attr=w->window.border_style;
		Conscreen_pixel corners = {STR('+'), attr}, vertical = {STR('|'), attr}, horizontal = {STR('-'), attr};

		Conscreen_screen_set(pos.x,pos.y, corners);
		Conscreen_screen_set(pos.x+size.x-1, pos.y, corners);
		Conscreen_screen_set(pos.x, pos.y+size.y-1, corners);
		Conscreen_screen_set(pos.x+size.x-1, pos.y+size.y-1, corners);

		for(int i=1; i<size.x-1; i++){
			Conscreen_screen_set(pos.x+i,pos.y, horizontal);
			Conscreen_screen_set(pos.x+i,pos.y+size.y-1, horizontal);
		}

		for(int i=1; i<size.y-1; i++){
			Conscreen_screen_set(pos.x,pos.y+i, vertical);
			Conscreen_screen_set(pos.x+size.x-1,pos.y+i, vertical);
		}
		CWM_internal_window_draw_title(w);
		CWM_internal_window_draw_error(w);
	}
}

Conscreen_pixel *CWM_window_draw(CWM_window w, i16vec2 size)
{
	free(w->content.buffer);
	size_t pixels = size.x*size.y;
	w->content.buffer = malloc(pixels*sizeof(Conscreen_pixel));
	for(int i=0; i<pixels; i++)
		w->content.buffer[i]=(Conscreen_pixel){0, CONSCREEN_ANSI_NORMAL};
}
static void CWM_internal_window_draw(CWM_window w)
{
	// render border
	if(w->window.border_show)
		CWM_internal_window_draw_border(w);
	if(w->renderer)
	{
		i16vec2 size =CWM_window_size_get(w);
		Conscreen_pixel *buffer = malloc(size.x*size.y*sizeof(Conscreen_pixel));
		w->renderer->func(w, buffer, w->renderer.data);
	}


}

static bool CWM_internal_window_depth_cmp(void* a, void* b)
{
	CWM_window _a=*(CWM_window*)a, _b=*(CWM_window*)b;
	return _a->window.Z_depth < _b->window.Z_depth;
}

void CWM_internal_window_render(CWM_window w)
{
	CWM_internal_window_draw(w);
	List_sort(w->child_windows, CWM_internal_window_depth_cmp);
	List_forward(w->child_windows, CWM_internal_window_render);
}

void CWM_debug(){
	static int i = 0;
	i16vec2 size = Conscreen_screen_size();
	if(i>=size.x)
		i=0;
	Conscreen_ansi a=CONSCREEN_ANSI_DEFAULT(255, 0, 0);
	Conscreen_pixel p= {STR('2'),a};
	Conscreen_screen_set(i,size.y/2, p);
	i++;
}

void CWM_render(){
	Conscreen_screen_begin(); // update Screenbuffer
	// CWM_debug();
	CWM_internal_window_render(root_window); // draw windows

	Conscreen_screen_flush(); // display frame
}
