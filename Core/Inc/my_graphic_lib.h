/*
 * my_graphic_lib.h
 *
 *  Created on: Sep 27, 2020
 *      Author: Aishwarya
 */

#ifndef INC_MY_GRAPHIC_LIB_H_
#define INC_MY_GRAPHIC_LIB_H_

#include <stdint.h>

typedef enum {
	OBJECT_CIRCLE,
	OBJECT_LINE,
	OBJECT_SQUARE,
	OBJECT_IMAGE
} object_item;

typedef struct {
	object_item object_type;

	struct  {
		int x1;
		int x2;
		int y1;
		int y2;
		uint8_t thick;
	} line;

	struct  {
		int x1;
		int x2;
		int y1;
		int y2;
		uint8_t fill;
	} square;

	struct  {
		int radius;
		int x1;
		int y1;
		uint8_t fill;
	} circle;

	struct  {
		int x1;
		int y1;
		int x2;
		int y2;
	} image;

}object_t;



extern uint8_t letter[][5];
extern uint8_t SSD1306_Buffer[];	//size of this is 128 * 8 bytes

void set_pixel(int x, int y, int col);
void drawline(int x1, int  y1,int  x2,int  y2, int col);
void draw_rect(int x1, int y1, int x2, int y2, int col);
void print_string( char *str, int x1, int y1, int on );

void object_init(object_t *obj);
void object_move(object_t *obj, int x, int y);


#endif /* INC_MY_GRAPHIC_LIB_H_ */
