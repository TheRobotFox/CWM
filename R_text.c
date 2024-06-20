#include "R_text.h"
#include "CWM_internal.h"
#include "RR.h"
#include "RR_context.h"
#include "RR_renderer.h"
#include <stdlib.h>

#define TAB_LENGTH 4

static int crop_text(const Conscreen_char *str, uint32_t length, RR_point size)
{
	int i=0,
	  x_length=0,
	  y_length=0;

	for(i=length-1; i>0; i--){
		if(str[i]=='\t')
			x_length+=TAB_LENGTH- (x_length%TAB_LENGTH);
		else
			length++;

		if(str[i]=='\n'){
			y_length++;
			x_length=0;
		} else if(x_length>=size.x){
			y_length++;
			x_length-=size.x;
		}
		if(y_length==size.y) break;
	}
	return i;
}

#define SET_PIXEL(ctx, offset, size, pixel) RR_set(ctx, offset%size.x, offset/size.x, p)

static void render_text(RR_context ctx, void *data)
{

	RR_point size = RR_size_get(ctx);
	// draw opacity
	if(size.x && size.y){

		CWM_string *string = (CWM_string *)data;
		Conscreen_pixel p = {.style=string->style};
		const Conscreen_char* text = string->str;

		// Crop text to fit screen
		int start=crop_text(text,string->len, size),
			offset=0;

		for(size_t i=start; i<string->len; i++)
		{
			switch (text[i]) {
				case '\n':
					p.character=' ';
					for(; offset%size.x; offset++)
						SET_PIXEL(ctx, offset, size, p);
					break;
				case '\t':
					p.character=' ';
					for(; (offset%size.x)%TAB_LENGTH; offset++)
						SET_PIXEL(ctx, offset, size, p);
					break;
				default:
					p.character = text[i];
					SET_PIXEL(ctx, offset, size, p);
					offset++;
			}
		}
		p.character=' ';
		while(offset<size.x*size.y){
			SET_PIXEL(ctx, offset, size, p);
			offset++;
		}
		RR_render(ctx, size, RR_get_default, RR_set_default, NULL);
	}

}

RR_renderer R_text()
{
	RR_renderer r = RR_renderer_create(render_text);
	CWM_string *data = malloc(sizeof(CWM_string));
	data->len=0;
	data->style = CONSCREEN_ANSI_NORMAL;
	data->str=NULL;
	RR_renderer_data_set(r, data);
	return r;
}

void R_text_free(RR_renderer r)
{
	free(RR_renderer_data_get(r));
	RR_renderer_free(r);
}

void R_text_set_text(RR_renderer r, const Conscreen_char *str, size_t len)
{
	CWM_string *data = RR_renderer_data_get(r);
	CWM_internal_string_set(data, str, len);
}
void R_text_set_style(RR_renderer r, Conscreen_ansi style)
{
	CWM_string *data = RR_renderer_data_get(r);
	data->style=style;
}
