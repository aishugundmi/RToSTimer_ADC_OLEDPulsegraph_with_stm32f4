#include "my_graphic_lib.h"
#include "string.h"
#include "ssd1306.h"


void set_pixel(int x, int y, int col)
{
	if(x >= 128 || y >= 64) return;
	if(x < 0 || y < 0) return;

	int page = (y/8);
	int buffer_index = ( page * 128 ) + x;
	int bit_position = y % 8;

	if(col) {
		SSD1306_Buffer[ buffer_index ] |= ( 1 << bit_position );
	} else {
		SSD1306_Buffer[ buffer_index ] &= ~( 1 << bit_position );
	}
}

void drawline(int x1, int  y1,int  x2,int  y2, int col)
{
    unsigned char l,b,q;

   // if(x1>127)return;//x1=127;
  //  if(y1>63)return;//y1=63;
  //  if(x2>127)return;//x2=127;
  //  if(y2>63)return;//y2=63;
    if(x1>x2)
    {
    	q=x1;
    	x1=x2;
    	x2=q;

    	q=y1;
    	y1=y2;
    	y2=q;
    }

    if(y2>y1)
    {
        l=x2-x1;
        b=y2-y1;
        if(l>=b)
        for(q=0;q<=l;q++)
        	set_pixel(x1 + q, (y1 + ((q*b)/l)), col);
        else
        for(q=0;q<=b;q++)
        	set_pixel((x1 + ((q*l)/b)), y1 + q, col);
    }
    else{
        l=x2-x1;
        b=y1-y2;
        if(l>=b)
        for(q=0;q<=l;q++)
        set_pixel(x1 + q, (y1 - ((q*b)/l)), col);
        else
        for(q=0;q<=b;q++)
        set_pixel((x1 + ((q*l)/b)), y1 - q, col);
    }
}


void draw_rect(int x1, int y1, int x2, int y2, int col)
{
	drawline(x1, y1, x1, y2, col);
	drawline(x1, y1, x2, y1, col);
	drawline(x2, y1, x2, y2, col);
	drawline(x1, y2, x2, y2, col);
}


void print_string( char *str, int x1, int y1, int on )
{
	int x = x1;
	int y = y1;

	int mask = 0;

	if(on) mask = 1;

	while(*str)
	{
		uint8_t let = *str++;
		for(int i = 0; i < 6; i++)
		{
			y = y1;
			uint8_t row_data = letter[let - ' '][i];
			for(int j = 0; j < 8; j++)
			{
				int col;

				if( i == 5 )
					col = 0;
				else
				{
					if( row_data & 1<<j )
						col = 1;
					else
						col = 0;
				}
				set_pixel(x, y + j, col & mask);
				//this will refresh the display instantly
			}
			x++;
		}

	}
}

 void object_init(object_t *obj)
{
	 if(obj->object_type == OBJECT_SQUARE) {
		 draw_rect(obj->square.x1, obj->square.y1, obj->square.x2,obj->square.y2, 1);
	 }
	 if(obj->object_type == OBJECT_CIRCLE) {
		 ssd1306_DrawCircle(obj->circle.x1, obj->circle.y1, obj->circle.radius, 1);
	 }
	 ssd1306_UpdateScreen();
}


void object_move(object_t *obj, int x, int y)
{
	if(obj->object_type == OBJECT_SQUARE) {

		draw_rect(obj->square.x1,obj->square.y1, obj->square.x2,obj->square.y2, 0);

		obj->square.x1 +=x;
		obj->square.y1 +=y;
		obj->square.x2 +=x;
		obj->square.y2 +=y;

		draw_rect(obj->square.x1,obj->square.y1, obj->square.x2,obj->square.y2, 1);

	}

	if(obj->object_type == OBJECT_CIRCLE) {

			ssd1306_DrawCircle(obj->circle.x1,obj->circle.y1, obj->circle.radius, 0);

			obj->circle.x1 +=x;
			obj->circle.y1 +=y;

			ssd1306_DrawCircle(obj->circle.x1,obj->circle.y1, obj->circle.radius, 1);

		}


	ssd1306_UpdateScreen();
}


