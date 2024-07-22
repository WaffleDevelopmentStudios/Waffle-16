#include <stdio.h>
#include <stdbool.h>
#include <iostream>
#include <cstring>
#include <SDL2/SDL.h>

typedef float     f32;
typedef double    f64;
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint16_t  word;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;
typedef size_t    usize;
typedef ssize_t   isize;

// Emulator Configs
#define screenWIDTH     1280
#define screenHEIGHT    720

// Memory Widths (in Bytes)
#define ramWidth        2147483648 
#define vramWidth       268435456
#define ssdSectorWidth  256

struct {
    // Emulator Stuff
    SDL_Window* window;
    SDL_Texture* texture;
    SDL_Renderer* renderer;
    u32 pixels[screenWIDTH * screenHEIGHT];
    bool quit;

    // Profiler
	float deltaTime;
	float framesPerSecond;
} state;

struct {

    

} emulator_data;

int main(int argc, char *argv[]) {

	if (SDL_Init(SDL_INIT_EVERYTHING)  != 0) {
		std::cout << "[Main]  SDL failed to initialize. Error: " << SDL_GetError() << std::endl;
		return -1;
	}

	 state.window =
        SDL_CreateWindow(
            "Waffle-16 Emulator",
            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            screenWIDTH,
            screenHEIGHT,
            SDL_WINDOW_ALLOW_HIGHDPI);

	if (state.window == NULL) {
		printf("[Main]  SDL Window failed to initialize.\n");
		return -1;
	}

	state.renderer = SDL_CreateRenderer(state.window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (state.renderer == NULL) {
		printf("[Main]  SDL Renderer failed to initialize.\n");
		return -1;
	}
	state.texture =
        SDL_CreateTexture(
            state.renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STREAMING,
            screenWIDTH,
            screenHEIGHT);

	float lastFrame;

	while (!state.quit) {
		float currentFrame = static_cast<float>(SDL_GetTicks()) / 1000;
		state.deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		state.framesPerSecond = 1 / state.deltaTime;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    state.quit = true;
                    break;
            }
        }

		memset(state.pixels, 0, sizeof(state.pixels));
        execute();

		SDL_UpdateTexture(state.texture, NULL, state.pixels, screenWIDTH * sizeof(u32));
        SDL_RenderCopyEx(state.renderer, state.texture, NULL, NULL, 0.0, NULL, SDL_FLIP_VERTICAL);
        SDL_RenderPresent(state.renderer);
	}

	SDL_DestroyTexture(state.texture);
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
	return 0;
}

void execute() {

}