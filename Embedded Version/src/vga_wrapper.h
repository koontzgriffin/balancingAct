#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include "address_map_arm_brl4.h"

volatile unsigned int *vga_pixel_ptr = NULL;
void *vga_pixel_virtual_base;
volatile unsigned int *vga_char_ptr = NULL;
void *vga_char_virtual_base;

//void *vga_cntl_virtual_base;
//volitile unsigned int *vga_cntl_ptr = NULL; 

void *h2p_lw_sw_addr = NULL;
void *h2p_lw_key_addr = NULL;
void *h2p_lw_virtual_base;



void write_text(int x, int y, char *text_ptr){
	int offset;
	volatile char *character_buffer = (char *)vga_char_ptr; // VGA character buffer
    /* assume that the text string fits on one line */
    offset = (y << 7) + x;
    while (*(text_ptr))
    {
        // write to the character buffer
        *(character_buffer + offset) = *(text_ptr);
        ++text_ptr;
        ++offset;
    }
}

void clear_text()
{
    volatile char *character_buffer = (char *)vga_char_ptr; // VGA character buffer
    int offset, x, y;
    for (x = 0; x < 79; x++)
    {
        for (y = 0; y < 59; y++)
        {
            /* assume that the text string fits on one line */
            offset = (y << 7) + x;
            // write to the character buffer
            *(character_buffer + offset) = ' ';
        }
    }
}

void put_pixel(int x, int y, short pixel_color){
	if((x < 0 || y < 0) || (x >= 320 || y >= 240)){
		return;
	}
	char * pixel_ptr = (char *)vga_pixel_ptr + (y<<10) + (x<<1);
	*(char *)pixel_ptr = pixel_color;
}

void clear_screen(short pixel_color){
	char *pixel_ptr;
	int row, col;
	for (row = 0; row < 240; row++){
		for (col = 0; col < 320; col++){
			pixel_ptr = (char *)vga_pixel_ptr + (row << 10) + (col << 1);
			// set pixel color
            *(short *)pixel_ptr = pixel_color;
		}
	}
}

void draw_line(int x1, int y1, int x2, int y2, short color){
	//printf("drawing line...\n");
	int dx = x2-x1;
	int dy = y2-y1;
	float slope = (float)dy / (float)dx;
	if(slope == 0){
		int y, ymax;
		if(y1 < y2){
			y = y1;
			ymax = y2;
		} else {
			y = y2;
			ymax = y1;
		}
		for(y; y <= ymax; y++){ 
			put_pixel(x1, y, color);
		}
	}
	//printf("slope = %.2f\n", slope);
	int b = (float)y1 - slope*(float)x1;
	int x;
	for(x = x1; x <= x2; x++){
		int y = slope*(float)x + (float)b;
		//printf("putting pixel %d, %d", x, y);
		if((y < 240 && y >= 0) && (x < 320 && x >= 0)){ 
			put_pixel(x, y, color);
		}
	}
}

void draw_circle(int x, int y, int r, short color){
	int x_center = x;
	int y_center = y;
	int x_current, y_current;
	int p;

	x_current = 0;
	y_current = r;
	p = 1 - r;

	if(((x + r >= 320) || (x - r < 0)) || ((y + r >= 240) || (y - r < 0))){
		return;
	}

	while(x_current <= y_current){
		put_pixel(x_center + x_current, y_center - y_current, color);
		put_pixel(x_center - x_current, y_center - y_current, color);
		put_pixel(x_center + x_current, y_center + y_current, color);
		put_pixel(x_center - x_current, y_center + y_current, color);
		put_pixel(x_center + y_current, y_center + x_current, color);
		put_pixel(x_center - y_current, y_center + x_current, color);
		put_pixel(x_center + y_current, y_center - x_current, color);
		put_pixel(x_center - y_current, y_center - x_current, color);

		x_current++;

		if(p < 0) {
			p = p + 2 * x_current + 1;
		} else {
			y_current--;
			p = p + 2 * (x_current - y_current) + 1;
		}
	}
}

int VGA_init(int fd) {

    vga_pixel_virtual_base = mmap(NULL, FPGA_ONCHIP_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, FPGA_ONCHIP_BASE);
    if (vga_pixel_virtual_base == MAP_FAILED)
    {
        printf("ERROR: mmap3() failed...\n");
        close(fd);
        return 1;
    }
    vga_pixel_ptr = (unsigned int *)(vga_pixel_virtual_base);

    vga_char_virtual_base = mmap(NULL, FPGA_CHAR_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, FPGA_CHAR_BASE);
    if (vga_char_virtual_base == MAP_FAILED)
    {
        printf("ERROR: mmap2() failed...\n");
        close(fd);
        return 1;
    }
    vga_char_ptr = (unsigned int *)(vga_char_virtual_base);

    // vga ctrl set up
    //vga_cntl_virtual_base = mmap(NULL, FPGA_CHAR_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, PIXEL_BUF_CTRL_BASE);
    //if (vga_ctnl_virtual_base == MAP_FAILED)
    //{
    //    printf("ERROR: vga_cntl mmap2() failed...\n");
    //    close(fd);
    //    return 1;
    //}
    //vga_cntl_ptr = (unsigned int *)(vga_cntl_virtual_base);



    return 0;
}
