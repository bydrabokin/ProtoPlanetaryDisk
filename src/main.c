#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#define SCALE 1e12
#define G 6.6743e-11 * SCALE 
#define PI 3.141592653589793
#define HydrogenDensity 0.0899 //kg  
#define DT 10

typedef struct {
    double x, y;
    double vx, vy;
    int r;
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
    double m1, m2;
    double force;
    double a, ax, ay;

    for (int i = 0; i < length; i++) {
        ax = ay = 0;
        for (int j = 0; j < length; j++) {
            if (&particles[i] != &particles[j]) {

                dx = (particles[i].x - particles[j].x) / 100.0; //cm 
                dy = (particles[i].y - particles[j].y) / 100.0; //cm
                
                r = sqrt(dx*dx + dy*dy);

                m1 = ((4.0/3.0) * PI * (particles[i].r / 100.0) * (particles[i].r / 100.0) * (particles[i].r / 100.0)) * HydrogenDensity;
                m2 = ((4.0/3.0) * PI * (particles[j].r / 100.0) * (particles[j].r / 100.0) * (particles[j].r / 100.0)) * HydrogenDensity;

                force = G * (m1*m2) / (r*r);
                
                a = force / m1;
                
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

    double dx, dy, r;

    for (int i = 0; i < length; i++) {
        for (int j = i+1; j < length; j++) {

            if (&particles[i] != &particles[j]) {
                
                dx = (particles[i].x - particles[j].x);
                dy = (particles[i].y - particles[j].y);
                    
                r = sqrt(dx*dx + dy*dy);

                if (r < particles[i].r + particles[j].r - 5) {
                    printf("Collision\n");
                    particles[i].vx *= -0.1;
                    particles[i].vy *= -0.1;

                    continue;
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

    while (running) {

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }  
            
            
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    SDL_GetMouseState(&mousex, &mousey); 
                    particles[particlesLength] = (Ball){mousex, mousey, 0, 0, radius};
                    particlesLength++;
                }
            } if (e.type == SDL_MOUSEWHEEL) {
                if (e.wheel.direction == SDL_MOUSEWHEEL_NORMAL) {
                    if (e.wheel.y > 0 && radius < 200) {

                        // Scrolled up
                        radius *= 1.1;

                    } else if (e.wheel.y < 0 && radius > 1) {
                        
                        // Scrolled down
                        radius *= 0.9;
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

        for (int i = 0; i <= particlesLength; i++) {
            drawCircle(renderer, particles[i].x, particles[i].y, particles[i].r);
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