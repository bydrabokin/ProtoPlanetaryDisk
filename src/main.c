#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#define PI 3.141592653589793
#define HydrogenDensity 0.0899 //kg  
#define DT 10
#define SCREN_HALF_X 900
#define SCREN_HALF_Y 450

double G, SCALE;

typedef struct {
    double x, y;
    double vx, vy;
    int r; 
    double m;
    bool active;
    
} Ball;


void drawCircle(SDL_Renderer *renderer, int x, int y, int r) {

    int dx;
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

                particles[i].m = ((4.0/3.0) * PI * (particles[i].r / 100.0) * (particles[i].r / 100.0) * (particles[i].r / 100.0)) * HydrogenDensity;
                particles[j].m = ((4.0/3.0) * PI * (particles[j].r / 100.0) * (particles[j].r / 100.0) * (particles[j].r / 100.0)) * HydrogenDensity;

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

    for (int i = 0; i < length; i++) {
        if (!particles[i].active) continue;
        for (int j = i+1; j < length; j++) {
            if (!particles[j].active) continue;
            if (&particles[i] != &particles[j]) {
                
                dx = (particles[i].x - particles[j].x);
                dy = (particles[i].y - particles[j].y);
                    
                r = sqrt(dx*dx + dy*dy);

                if (r - 10< particles[i].r + particles[j].r) {
                    printf("Collision\n");
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

int main() {

    bool running = true;

    SDL_Event e;

    // window
    SDL_Window *win = SDL_CreateWindow("Gas clouds", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1800, 900, 0); // Init window
    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); // render

    Ball particles[512];
    int particlesLength = 0;

    int mousex, mousey;

    double radius = 6;
    double zoomFactor = 1;
    int drawx, drawy;

    SCALE = 1e12;

    while (running) {

        G = 6.6743e-11 * SCALE;


        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }  
            
            
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    SDL_GetMouseState(&mousex, &mousey); 
                    double mass = 1.0;
                    drawx = SCREN_HALF_X + (mousex - SCREN_HALF_X) * zoomFactor;
                    drawy = SCREN_HALF_Y + (mousey - SCREN_HALF_Y) * zoomFactor;
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

            } if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_MINUS) {
                    SCALE /= 5.0;

                } else if (e.key.keysym.sym = SDLK_PLUS) {
                    SCALE *= 5.0;
                }
            }
        }


        applyGravity(particles, particlesLength);
        collisions(particles, particlesLength);


        // First clear
        SDL_RenderClear(renderer);

        // DRAW
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        for (int i = 0; i <= particlesLength; i++) {
            if (!particles[i].active) continue;
            drawx = SCREN_HALF_X + (particles[i].x - SCREN_HALF_X) / zoomFactor;
            drawy = SCREN_HALF_Y + (particles[i].y - SCREN_HALF_Y) / zoomFactor;
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