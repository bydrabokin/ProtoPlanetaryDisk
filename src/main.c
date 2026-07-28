#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>

#define PI 3.141592653589793
#define HydrogenDensity 0.0899 //kg  
#define DT 10
#define SCREEN_HALF_X 900
#define SCREEN_HALF_Y 450

double G, SCALE, zoomFactor, radius, ratio;

typedef struct {
    double x, y;
    double vx, vy;
    double r; 
    double m;
    bool active;
    
} Ball;

Ball star;
Ball particles[8196];

SDL_Color getRGB(double r) {
    
    double min = radius;
    double max = 4*radius;
    double value = r;
    SDL_Color rgb;

    if (value > max) value = max;
    else if (value < min) value = min;

    value = (value - min) / (max - min) * 400.0;

    if (value <= 200.0)  rgb = (SDL_Color){55, 55, value - 145, 255};
    else if (value > 200.0)  rgb = (SDL_Color){(int)value - 145, 55, 55, 255};

    return rgb;
    
}

void drawCircle(SDL_Renderer *renderer, int x, int y, double r) {

    int dx;

    
    SDL_Color color = getRGB((r*zoomFactor));

    
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    if (r*zoomFactor >= 50) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    }

    for (int i = -r; i <= r; i++) {
        dx = (int)sqrt(r*r - i*i);
        SDL_RenderDrawLine(renderer, x-dx, y+i, x+dx, y+i);
    }
} 

void applyGravity(Ball *particles, int length) {

    double dx, dy, r;
    double force;
    double a, ax, ay;

    

    for (int i = 0; i < length; i++) {
        ax = ay = 0;
        if (!particles[i].active) continue;
        for (int j = 0; j < length; j++) {
            if (!particles[j].active) continue;
            if (&particles[i] != &particles[j]) {

                dx = (particles[i].x - particles[j].x) / 100.0; //cm 
                dy = (particles[i].y - particles[j].y) / 100.0; //cm
                
                r = sqrt(dx*dx + dy*dy);

                
                if (j == 0 && r*100 < star.r) {
                    particles[i].active = false;
                    continue;
                }
                

                particles[i].m = ((4.0/3.0) * PI * (particles[i].r / 100.0) * (particles[i].r / 100.0) * (particles[i].r / 100.0)) * HydrogenDensity;
                particles[j].m = ((4.0/3.0) * PI * (particles[j].r / 100.0) * (particles[j].r / 100.0) * (particles[j].r / 100.0)) * HydrogenDensity;

                if (j == 0) {
                    particles[j].m = ((4.0/3.0) * PI * (particles[j].r / 100.0) * (particles[j].r / 100.0) * (particles[j].r / 100.0)) * HydrogenDensity / ratio;
                }

                force = G * (particles[i].m*particles[j].m) / (r*r);
                
                a = force / particles[i].m;
                
                ax += -a * (dx/r);
                ay += -a * (dy/r);
                
            }
        }

        particles[i].vx += ax * DT;
        particles[i].vy += ay * DT;

    }
    for (int i = 0; i < length; i++) {
        particles[i].x += particles[i].vx * DT;
        particles[i].y += particles[i].vy * DT;
    }
}

void collisions(Ball *particles, int length) {

    double dx, dy, r, totalMass;

    for (int i = 1; i < length; i++) {
        if (!particles[i].active) continue;
        for (int j = i+1; j < length; j++) {
            if (!particles[j].active) continue;
            if (&particles[i] != &particles[j]) {
                
                dx = (particles[i].x - particles[j].x);
                dy = (particles[i].y - particles[j].y);
                    
                r = sqrt(dx*dx + dy*dy);

                if (r < particles[i].r + particles[j].r) {
                    totalMass = particles[i].m + particles[j].m;
                    particles[i].r = pow(particles[i].r * particles[i].r * particles[i].r + particles[j].r * particles[j].r * particles[j].r, 1.0/3.0);
                    particles[i].vx = (particles[i].m * particles[i].vx + particles[j].m * particles[j].vx) / ( totalMass);
                    particles[i].vy = (particles[i].m * particles[i].vy + particles[j].m * particles[j].vy) / ( totalMass);
                    particles[i].m = totalMass;
                    particles[j].active = false;
                    

                    
                }
            }
        }
    }
}

void start(int particlesLength, int num, int steps, int altInit, int separation) {

    int length = particlesLength; 


    G = 6.6743e-11 * SCALE;

    for (int i = 0; i < num; i++) {
        
        int alt = altInit + separation * i;
        double speed = 10*sqrt((G * star.m) / (alt/100.0));
        double mass = ((4.0/3.0) * PI * (radius / 100.0) * (radius / 100.0) * (radius / 100.0)) * HydrogenDensity;


        for (int j = 0; j < steps; j++) {
            
            double degrees = j * (360.0/steps);
            double radians = degrees * (PI / 180);
            double x = cos(radians) * alt + SCREEN_HALF_X;
            double y = sin(radians) * alt + SCREEN_HALF_Y;
            
            double speedy = speed * cos(radians);
            double speedx = speed * -sin(radians);

            double randomNumber1 = (rand()%2000 - 1000) / 100.0;
            double randomNumber2 = (rand()%2000 - 1000) / 100.0;

            particles[length] = (Ball){x + randomNumber1, y+ randomNumber2, speedx, speedy, radius, mass, true};
            length++;
        }
    
    }
}

int main() {

    bool running = true;

    SDL_Event e;

    // window
    SDL_Window *win = SDL_CreateWindow("Proto Planetary Disk", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1800, 900, 0); // Init window
    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); // render

    
    int particlesLength = 0;

    int mousex, mousey;

    radius = 1.0;
    zoomFactor = 1;
    int drawx, drawy;

    SCALE = 5e9;
    srand(time(0));
    
    ratio = radius * 5.0;

    star.x = SCREEN_HALF_X;
    star.y = SCREEN_HALF_Y;
    star.vx = 0;
    star.vy = 0;
    star.r = 50;
    star.m = ((4.0/3.0) * PI * (star.r / 100.0) * (star.r / 100.0) * (star.r / 100.0)) * HydrogenDensity/ratio;
    star.active = true; 

    particles[particlesLength] = (Ball)star;
    particlesLength++;

    double mass = ((4.0/3.0) * PI * (radius / 100.0) * (radius / 100.0) * (radius / 100.0)) * HydrogenDensity ;
    int heights = 30;
    int ballsPerdisk = 30;
    int intialAlt = 200;
    int separation = 5;
    start(particlesLength, heights, ballsPerdisk, intialAlt, separation);
    particlesLength += ballsPerdisk*heights;



    while (running) {

        G = 6.6743e-11 * SCALE;


        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }  
            
            
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    SDL_GetMouseState(&mousex, &mousey); 
                    mass = ((4.0/3.0) * PI * (radius / 100.0) * (radius / 100.0) * (radius / 100.0)) * HydrogenDensity;
                    drawx = SCREEN_HALF_X + (mousex - SCREEN_HALF_X) * zoomFactor;
                    drawy = SCREEN_HALF_Y + (mousey - SCREEN_HALF_Y) * zoomFactor;
                    particles[particlesLength] = (Ball){drawx, drawy, 0, 0, radius, mass, true};
                    particlesLength++;
                }
            } if (e.type == SDL_MOUSEWHEEL) {
                if (e.wheel.direction == SDL_MOUSEWHEEL_NORMAL) {
                    if (e.wheel.y > 0 && zoomFactor < 2000) {
                        zoomFactor *= 1.2; // Scrolled up
                    } else if (e.wheel.y < 0 && zoomFactor > 0.4) {
                        zoomFactor *= 0.8; // Scrolled down
                    }
                }

            } 
        }


        applyGravity(particles, particlesLength);
        collisions(particles, particlesLength);


        // First clear
        SDL_RenderClear(renderer);

        // DRAW
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        for (int i = 0; i < particlesLength; i++) {
            if (!particles[i].active) continue;
            drawx = SCREEN_HALF_X + (particles[i].x - SCREEN_HALF_X) / zoomFactor;
            drawy = SCREEN_HALF_Y + (particles[i].y - SCREEN_HALF_Y) / zoomFactor;
            drawCircle(renderer, drawx, drawy, particles[i].r / zoomFactor);
        }

        // Finally update
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderPresent(renderer);

    }

    
    // exit
    SDL_DestroyRenderer(renderer);  // cleanup after loop exits
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}