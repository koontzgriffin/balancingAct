#include <iostream>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <vector>
#include <cmath>

using namespace std;
// Math & Physics constants
const float pi = 3.14;
const float g = 68.0;
const float pixel_size = 5;
//Screen dimension constants
const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 720;
// game time constants
int FPS = 30;
const int TARGET_FRAME_TIME = 1000/FPS;
int last_frame_time = 0;
// game state
bool game_is_running = false;
bool lost = false;
SDL_Point mouse_position;
// window 
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
// score 
int num_bounces = 0;
int ball_starting_x = (SCREEN_WIDTH/2);
int ball_starting_y = 50;

float component_in_direction(float x1, float y1, float x2, float y2){
    // returns the magnitude of component of 2 in the direction of 1 
    return (x1*x2 + y1*y2);//  / sqrt(x2*x2 + y2*y2);
}

float ceil_or_floor(float x){
    if(x < 0){
        return floor(x);
    } else {
        return ceil(x);
    }
}

class Beam {
    public:
        int Centerx;
        int Centery;
        int width;
        float angle;
        float prev_angle;

        Beam(int x, int y, int w, float a) {
            Centerx = x;
            Centery = y;
            width = w;
            angle = a; 
            prev_angle = a;
        }

        void draw() {
            float y = (width/2)*sin(angle);
            float x = (width/2)*cos(angle);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderDrawLine(renderer, Centerx-x, Centery-y, Centerx+x, Centery+y);
        }

        void update() {
            prev_angle = angle;
            angle = (pi)*((float)mouse_position.y/(float)SCREEN_HEIGHT) - (pi/2);
            //cout<<angle<<endl;
            //cout<<mouse_position.x<<", "<<mouse_position.y<<endl;
        }

        int min_x(){
            return (-((float)width/2)*cos(angle) + Centerx);
        }

        int max_x(){
            return (((float)width/2)*cos(angle) + Centerx);
        }

        void get_normal(float arr[]){
            float theta = (pi/2) + angle; 
            arr[0] = -cos(theta);
            arr[1] = -sin(theta);
        }

        void show_normal(){
            float norm[2];
            get_normal(norm);
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_RenderDrawLine(renderer, Centerx, Centery, Centerx + norm[0]*100, Centery + norm[1]*100);

        }

        void get_slope_vector(float arr[]){
            float x = abs(sin(angle + pi/2));
            float y = abs(cos(angle + pi/2));

            if (angle < 0){
                x = -x;
            }

            arr[0] = x;
            arr[1] = y;
        }
};

class Ball {
    public:
        int Centerx;
        int Centery;
        int radius;
        int color[3] = {255, 0, 0};
        float velocity[2] = {0, 0}; // {x, y}

        Ball(int x, int y, int r){
            Centerx = x;
            Centery = y;
            radius = r;
        }

        void draw(){
            const int diameter = (radius * 2);

            int x = (radius - 1);
            int y = 0;
            int tx = 1;
            int ty = 1;
            int error = (tx - diameter);

            SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], 255);

            while (x >= y)
            {
                //  Each of the following renders an octant of the circle
                SDL_RenderDrawPoint(renderer, Centerx + x, Centery - y);
                SDL_RenderDrawPoint(renderer, Centerx + x, Centery + y);
                SDL_RenderDrawPoint(renderer, Centerx - x, Centery - y);
                SDL_RenderDrawPoint(renderer, Centerx - x, Centery + y);
                SDL_RenderDrawPoint(renderer, Centerx + y, Centery - x);
                SDL_RenderDrawPoint(renderer, Centerx + y, Centery + x);
                SDL_RenderDrawPoint(renderer, Centerx - y, Centery - x);
                SDL_RenderDrawPoint(renderer, Centerx - y, Centery + x);

                if (error <= 0)
                {
                    ++y;
                    error += ty;
                    ty += 2;
                }

                if (error > 0)
                {
                    --x;
                    tx += 2;
                    error += (tx - diameter);
                }
            }
        }

        void change_color(){
            if(color[0] == 255){
                color[0] = 0;
                color[1] = 255;
            } else if(color[1] == 255){
                color[1] = 0;
                color[2] = 255;
            }else if(color[2] == 255){
                color[2] = 0;
                color[0] = 255;
            }

        }

        void update(float dt, Beam beam){
            //cout<<"beam angle = "<<beam.angle<<endl;
            // calculate acceleration
            float a[2] = {0,g};
            
            if(check_collision(beam)){
                fix_overlap(beam);
                reflect(beam);
                change_color();
                cout<<"bounce!"<<endl;
                num_bounces += 1;
            }
            else{
                a[0] = 0;
                a[1] = g;
            }

            // initial velocity
            float v_iy = velocity[1];
            float v_ix = velocity[0];
            // velocity due to acceleration
            velocity[1] += ((a[1]*dt)*pixel_size);
            velocity[0] += ((a[0]*dt)*pixel_size);
            // update position
            int delta_y = ceil_or_floor(0.5*(v_iy + velocity[1])*(((float)dt)));
            int delta_x = ceil_or_floor(0.5*(v_ix + velocity[0])*(((float)dt)));
            Centerx += delta_x;
            Centery += delta_y;

        }

        void reflect(Beam beam){
            float norm[2];
            beam.get_normal(norm);
            float mag = norm[0]*velocity[0] + norm[1]*velocity[1];
            float component[2];
            component[0] = velocity[0] - 2*mag*norm[0];
            component[1] = velocity[1] - 2*mag*norm[1];

            velocity[0] = component[0]*0.999;
            velocity[1] = component[1]*0.999;
        }
        
        void get_closest_point(Beam beam, int arr[]){
            int beam_x_max = beam.max_x();
            int beam_x_min = beam.min_x();
            int min_x;
            int min_y;
            int min_distance;
            for(int x = beam_x_min; x < beam_x_max; x++){
                int beam_point_y = float(beam.Centery) - float(x - beam.Centerx)*tan(-beam.angle);
                int dx = Centerx - x;
                int dy = beam_point_y - Centery;
                int distance = sqrt(dx*dx + dy*dy);
                if ((x == beam_x_min || distance < min_distance) && (x <= beam.max_x()) && (x >= beam.min_x())){
                    min_x = x;
                    min_y = beam_point_y;
                    min_distance = distance;
                }
            }
            arr[0] = min_x;
            arr[1] = min_y;
        }

        bool check_collision(Beam beam){
            int beam_point[2];
            get_closest_point(beam, beam_point);
            int dx = Centerx - beam_point[0];
            int dy = beam_point[1] - Centery;
            int distance = sqrt(dx*dx + dy*dy);
            //cout<<"beam point = "<<beam_point[0]<<", "<<beam_point[1]<<endl;
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderDrawLine(renderer, beam_point[0], beam_point[1], Centerx, Centery);
            if(distance <= radius){
                return true;
            }
            return false;
        }

        void fix_overlap(Beam beam){
            int beam_point[2];
            get_closest_point(beam, beam_point);
            int dx = Centerx - beam_point[0];
            int dy = beam_point[1] - Centery;
            int distance = sqrt(dx*dx + dy*dy);
            int overlap = radius - distance; 
            // move the ball to fix overlap
            Centerx -= overlap*sin(beam.angle);
            Centery -= overlap*cos(beam.angle);
        }
};

// game objects
Beam beam = Beam(SCREEN_WIDTH/2, (SCREEN_HEIGHT*3)/4, 600, 0);
Ball ball = Ball((SCREEN_WIDTH/2), 50, 10);

void reset(){
    ball.velocity[0] = 0;
    ball.velocity[1] = 0;
    ball.Centerx = ball_starting_x;
    ball.Centery = ball_starting_y;
    lost = false;
}

void check_for_loss(){
    if(ball.Centerx < 0 || ball.Centerx > SCREEN_WIDTH || ball.Centery > SCREEN_HEIGHT){
        lost = true;
    }
}

void show_slope_vector(Ball ball, Beam beam){
        float x = abs(sin(beam.angle + pi/2));
        float y = abs(cos(beam.angle + pi/2));

        if (beam.angle < 0){
            x = -x;
        }

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDrawLine(renderer, ball.Centerx, ball.Centery, ball.Centerx + x*100, ball.Centery + y*100);
}

bool initialize_window(){
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        return false;
    }
    //Create window
    window = SDL_CreateWindow( "Balancing Act", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    if( window == NULL )
    {
        printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if(!renderer){
        printf( "Renderer could not be created! SDL_Error: %s\n", SDL_GetError() );
        return false;
    }
    return true;
}

void process_input(){
    SDL_Event event;
    // handel inputs
    while(SDL_PollEvent(&event)){
        //handle each event
        if(event.type == SDL_QUIT){
            game_is_running = false;
            break;
        }
        if(event.type == SDL_KEYDOWN){
            reset();
            break;
        }
    }
}

void update(){
    SDL_GetMouseState(                    //    Sets mouse_position to...
        &mouse_position.x,                // ...mouse arrow coords on window
        &mouse_position.y
    );

    beam.update();
    ball.update((float)1/30, beam);
    
    // lock frame rate
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), last_frame_time + TARGET_FRAME_TIME));
    // Get delta_time factor converted to seconds to be used to update objects
    //float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0;
    // Store the milliseconds of the current frame to be used in the next one
    last_frame_time = SDL_GetTicks();
}

void render(){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // draw beam
    beam.draw();
    //beam.show_normal();
    ball.draw();
    //show_slope_vector(ball, beam);

    SDL_RenderPresent(renderer);
}

void destroy_window() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main() {
    game_is_running = initialize_window();

    while(game_is_running){
        process_input();
        update();
        render();
        check_for_loss();
    }

    destroy_window();

    return 0;
}