#include <stdio.h>
#include "vga_wrapper.h"
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include "gsensor_wrapper.h"

// math and physics constants
float pi = 3.14;
float g = 80.0;
float pixel_size = 1;

//colors
short red = 0b1111111111111111;
short blue = 0b11110000;
short green = 0b0000011111000000;

// screen dimensions constants
int SCREEN_WIDTH = 320;
int SCREEN_HEIGHT = 240;

// time constants
int FPS = 30;
float dt = .03;

// game state
int lost = 0;
// score 
int num_bounces = 0;


// platform
int platform_x = 159;
int platform_y = 140;
int platform_width = 250;
float angle;
short platform_color;

// ball
int ball_starting_x = 160;
int ball_starting_y = 50;
int radius = 10;
int ballx = 160;
int bally = 50;
float ball_velocity[2];
short ball_color = 1000;

//////////////////////
// helper functions //
//////////////////////

float ceil_or_floor(float x){
    if(x < 0){
        return floor(x);
    } else {
        return ceil(x);
    }
}

///////////////////////////
// platform functions /////
///////////////////////////

int platform_min_x(){
    return (-((float)platform_width/2)*cos(angle) + platform_x);
}

int platform_max_x(){
    return (((float)platform_width/2)*cos(angle) + platform_x);
}

void get_normal(float arr[]){
    float theta = (pi/2) + angle; 
    arr[0] = -cos(theta);
    arr[1] = -sin(theta);
}

void draw_platform(float angle, int width){
	float y = (((float)width)/2)*sin(angle);
	float x = (((float)width)/2)*cos(angle);
	//printf("x , y = %d, %d\n", (int)x, (int)y);
	draw_line(platform_x - (int)x, platform_y - (int)y, platform_x + (int)x, platform_y + (int)y, platform_color);
	draw_line(platform_x - (int)x, platform_y - (int)y+1, platform_x + (int)x, platform_y + (int)y+1, platform_color);
	draw_line(platform_x - (int)x, platform_y - (int)y+2, platform_x + (int)x, platform_y + (int)y+2, platform_color);
}

////////////////////
// Ball functions //
////////////////////

void get_closest_point(int arr[]){
    int beam_x_max = platform_max_x();
    int beam_x_min = platform_min_x();
    int min_x;
    int min_y;
    int min_distance;
    int x;
    for(x = beam_x_min; x < beam_x_max; x++){
        int beam_point_y = (float)platform_y - (float)(x - platform_x)*tan(-angle);
        int dx = ballx - x;
        int dy = beam_point_y - bally;
        int distance = sqrt(dx*dx + dy*dy);
        if ((x == beam_x_min || distance < min_distance) && (x <= beam_x_max) && (x >= beam_x_min)){
            min_x = x;
            min_y = beam_point_y;
            min_distance = distance;
        }
    }
    arr[0] = min_x;
    arr[1] = min_y;
}

bool check_collision(){
    int beam_point[2];
    get_closest_point(beam_point);
    int dx = ballx - beam_point[0];
    int dy = beam_point[1] - bally;
    int distance = sqrt(dx*dx + dy*dy);

    if(distance <= radius){
        return 1;
    }
    return 0;
}

void reflect(){
    float norm[2];
    get_normal(norm);
    float mag = norm[0]*ball_velocity[0] + norm[1]*ball_velocity[1];
    float component[2];
    component[0] = ball_velocity[0] - 2*mag*norm[0];
    component[1] = ball_velocity[1] - 2*mag*norm[1];

    ball_velocity[0] = component[0]*0.999;
    ball_velocity[1] = component[1]*0.999;
}

void update_ball_pos(float dt){
    // calculate acceleration
    float a[2];
    a[0] = 0;
    a[1] = g;

    if(check_collision()){
        //fix_overlap(beam);
        reflect();
        //change_color();
        printf("bounce!\n");
        num_bounces += 1;
    }
    // initial velocity
    float v_iy = ball_velocity[1];
    float v_ix = ball_velocity[0];
    // velocity due to acceleration
    ball_velocity[1] += ((a[1]*dt)*(float)pixel_size);
    ball_velocity[0] += ((a[0]*dt)*(float)pixel_size);
    // update position
    int delta_y = ceil_or_floor(0.5*(v_iy + ball_velocity[1])*(((float)dt)));
    int delta_x = ceil_or_floor(0.5*(v_ix + ball_velocity[0])*(((float)dt)));
    ballx += delta_x;
    bally += delta_y;
}

//////////////////////////
// game state functions //
//////////////////////////

int check_loss(){
	if(ballx < 0 || ballx >= SCREEN_WIDTH || bally >= SCREEN_HEIGHT){
        lost = 1;
    }
    else return 0;
}

void reset(){
    ball_velocity[0] = 0;
    ball_velocity[1] = 0;
    ballx = ball_starting_x;
    bally = ball_starting_y;
    lost = 0;
    platform_color = red;
    ball_color = red;
}

int main(){
	// open /dev/mem
	int fd;
	if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1){
		printf("ERROR: could not open \"/dev/mem\"...\n");
		return 1;
	}

	VGA_init(fd);
	gsensor_init(fd);
	sleep(1);

	reset();

	while (1){
		//clock_t start_time, current_time;
		//start_time = clock();
		// sets the desired frame time for 1/30 secs
		//clock_t frame_time = CLOCKS_PER_SEC / 30;

		int pixel_color = 0;

		/////////////////////////////
		// main game and display code
		/////////////////////////////
		if(check_loss()){
		   reset();
		   printf("YOU LOST!\n");
		   printf("Congrats on %d bounces!\n", num_bounces);
		   num_bounces = 0;
		}

		angle = get_board_angle();
		update_ball_pos(dt);

		clear_screen(pixel_color);
		draw_circle(ballx, bally, radius, ball_color);
		draw_circle(ballx, bally, radius-1, ball_color);
		draw_circle(ballx, bally, radius-2, ball_color);

		draw_platform(angle, platform_width);
		/////////////////////////////////
		// end main game and display code
		/////////////////////////////////

		//current_time = clock();
		// kill the rest of time
		//do {} while((current_time - start_time) < frame_time);
		//sleep(.8);
	}
	return 0;
}
