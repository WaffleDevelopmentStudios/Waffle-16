#include <stdio.h>
#include <stdbool.h>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <SDL2/SDL.h>

typedef float     f32;
typedef double    f64;
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;
typedef size_t    usize;
typedef ssize_t   isize;

// Emulator Configs
#define screenWIDTH     640
#define screenHEIGHT    320

// Memory Widths (in Bytes)
#define ramWidth        2000000000
#define vramWidth       256000000
#define ssdSectorWidth  512

using namespace std;

struct {
	// Emulator Stuff
	SDL_Window* window;
	SDL_Texture* texture;
	SDL_Renderer* renderer;
	u32 pixels[screenWIDTH * screenHEIGHT];
	bool quit = false;
	bool stepThrough = true;
	bool stepThroughCooldown = false;
} state;

class Profiler {

public:
	u32 currentFrame;
	float lastFrame;
	float deltaTime;
	u32 FPS;

	void updateFrame() {
		currentFrame = static_cast<float>(SDL_GetTicks()) / 1000;
		deltaTime = currentFrame - lastFrame;
		lastFrame = static_cast<float>(SDL_GetTicks()) / 1000;
		FPS = 1 / deltaTime;
	}
};

u16 ram[ramWidth / 2];
u16 vram[vramWidth / 2];
u16 ssdCache[ssdSectorWidth * 2];

#include "sections/cpu.hpp"
#include "sections/gpu.hpp"

/*
 - 00 - General Purpose
 - 01 - General Purpose
 - 02 - General Purpose
 - 03 - General Purpose
 - 04 - General Purpose
 - 05 - General Purpose
 - 06 - General Purpose
 - 07 - General Purpose
 - 08 - General Purpose
 - 09 - General Purpose
 - 0A - General Purpose
 - 0B - General Purpose
 - 0C - General Purpose
 - 0D - General Purpose
 - 0E - General Purpose
 - 0F - General Purpose
 - 10 - 
 - 11 - 
 - 12 - 
 - 13 - 
 - 14 - 
 - 15 - 
 - 16 - 
 - 17 - 
 - 18 - 
 - 19 - 
 - 1A - 
 - 1B - SSD Ready
 - 1C - Bookkeeping Stack Pointer
 - 1D - Stack Pointer
 - 1E - Program Counter (Upper Bit)
 - 1F - Program Counter (Lower Bit)
*/

/*
Instruction Set:
 - 00 - No Operation                    00()
 - 01 - Move Val To Reg                 01(w-reg, r-val)
 - 02 - Move Reg To Reg                 02(w-reg, r-reg)
 - 03 - Jump to Val                     03(addr-val x2)
 - 04 - Jump to Reg                     04(addr-reg x2)
 - 05 - Branch if Reg Equals Val        05(addr-reg x2, c-reg, c-val)
 - 06 - Branch if Reg !Equals Val       06(addr-reg x2, c-reg, c-val)
 - 07 - Store Reg to RAM @ Val          07(r-reg, addr-val x2)
 - 08 - Load RAM to Reg @ Val           08(w-reg, addr-val x2)
 - 09 - Store Reg to RAM @ Reg          09(r-reg, addr-reg x2)
 - 0A - Load RAM to Reg @ Reg           0A(w-reg, addr-reg x2)
 - 0B - Store Reg to Video RAM @ Val    0B(r-reg, addr-val x2)
 - 0C - Load Video RAM to Reg @ Val     0C(w-reg, addr-val x2)
 - 0D - Store Reg to Video RAM @ Reg    0D(r-reg, addr-reg x2)
 - 0E - Load Video RAM to Reg @ Reg     0E(w-reg, addr-reg x2)
 - 0F - Return                          0F()
 - 10 - 
 - FF - Halt!
*/

CPU_Core cpu_core;
GPU_Core gpu_core;

u16 instructions[255] = {
	
};

void runDataGrabber();

int main(int argc, char *argv[]) {
	memset(cpu_core.registers, 0, sizeof(cpu_core.registers      ));
	memset(cpu_core.registers, 0, sizeof(cpu_core.stack          ));
	memset(cpu_core.registers, 0, sizeof(cpu_core.bookkeepingStack));
	memset(ram            , 0, sizeof(ram                  ));
	memset(vram           , 0, sizeof(vram                 ));
	/*ifstream romFile("rom.bin", ios::in | ios::binary);
	if (romFile.is_open()) {
		for (int i = 0; i < 255; i++) {
			romFile.read(reinterpret_cast<char*>(&instructions[i]), sizeof(u16));
		}
	}
	romFile.close();*/
	for (int i = 0; i < (sizeof(instructions) / 2); i++) ram[i] = instructions[i];

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
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_STREAMING,
			screenWIDTH,
			screenHEIGHT);

	Profiler profiler;
	while (!state.quit) {
		profiler.updateFrame();
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				std::cout << "Window Closed. Stopping." << std::endl;
				state.quit = true;
				break;
			} else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        state.quit = true;
						std::cout << "Escape Pressed. Stopping." << std::endl;
                        break;
                    case SDLK_o:
						runDataGrabber();
                        break;
					case SDLK_p:
						if (!state.stepThroughCooldown && state.stepThrough) cpu_core.execute();
						state.stepThroughCooldown = true;
                        break;
					case SDLK_UP:
						state.stepThrough = true;
                        break;
					case SDLK_DOWN:
						state.stepThrough = false;
                        break;
                }
				if (event.key.keysym.sym != SDLK_p) {
					state.stepThroughCooldown = false;
				}
            }
		}
		if (!state.stepThrough) {
			for (int executionStep = 0; executionStep < 65536; executionStep++) {
				cpu_core.execute();
				if (state.quit) break;
			}
		}

		memset(state.pixels, 0, sizeof(state.pixels));
		for (int i = 0; i < screenWIDTH * screenHEIGHT * 2; i += 2) {
			state.pixels[i / 2] = (vram[i] << 16) + vram[i + 1];
		}
		SDL_UpdateTexture(state.texture, NULL, state.pixels, screenWIDTH * sizeof(u32));
		SDL_RenderCopyEx(state.renderer, state.texture, NULL, NULL, 0.0, NULL, SDL_FLIP_VERTICAL);
		SDL_RenderPresent(state.renderer);
	}
	SDL_Delay(1000);
	SDL_DestroyTexture(state.texture);
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	return 0;
}

/*
 - 00 - No Operation
 - 01 - Move Val To Reg
 - 02 - Move Reg To Reg
 - 03 - Jump to Val
 - 04 - Jump to Reg
 - 05 - Store Reg to RAM @ Val
 - 06 - Load RAM to Reg @ Val
 - 07 - Store Reg to RAM @ Reg
 - 08 - Load RAM to Reg @ Reg
 - 09 - Store Reg to Video RAM @ Val
 - 0A - Load Video RAM to Reg @ Val
 - 0B - Store Reg to Video RAM @ Reg
 - 0C - Load Video RAM to Reg @ Reg
 - 0D - Increment Reg
 - 0E - Branch if Reg Equals Val
 - 0F - Branch if Reg !Equals Val
 - FF - Halt!
*/



void runDataGrabber() {
	std::cout << std::endl;
	std::cout << "Debug Data: {" << std::endl;
	std::cout << " - Registers ----" << std::endl;
	std::cout << " - " << std::hex << cpu_core.registers[0x00] << " - " << cpu_core.registers[0x01] << " - " << cpu_core.registers[0x02] << " - " << cpu_core.registers[0x03] << std::endl;
	std::cout << " - " << std::hex << cpu_core.registers[0x04] << " - " << cpu_core.registers[0x05] << " - " << cpu_core.registers[0x06] << " - " << cpu_core.registers[0x07] << std::endl;
	std::cout << " - " << std::hex << cpu_core.registers[0x08] << " - " << cpu_core.registers[0x09] << " - " << cpu_core.registers[0x0A] << " - " << cpu_core.registers[0x0B] << std::endl;
	std::cout << " - " << std::hex << cpu_core.registers[0x0C] << " - " << cpu_core.registers[0x0D] << " - " << cpu_core.registers[0x0E] << " - " << cpu_core.registers[0x0F] << std::endl;
	std::cout << " - " << std::hex << cpu_core.registers[0x10] << " - " << cpu_core.registers[0x11] << " - " << cpu_core.registers[0x12] << " - " << cpu_core.registers[0x13] << std::endl;
	std::cout << " - " << std::hex << cpu_core.registers[0x14] << " - " << cpu_core.registers[0x15] << " - " << cpu_core.registers[0x16] << " - " << cpu_core.registers[0x17] << std::endl;
	std::cout << " - " << std::hex << cpu_core.registers[0x18] << " - " << cpu_core.registers[0x19] << " - " << cpu_core.registers[0x1A] << " - " << cpu_core.registers[0x1B] << std::endl;
	std::cout << " - " << std::hex << cpu_core.registers[0x1C] << " - " << cpu_core.registers[0x1D] << " - " << cpu_core.registers[0x1E] << " - " << cpu_core.registers[0x1F] << std::endl;
	std::cout << std::endl;
	std::cout << " - RAM --------" << std::endl;
	int effectiveAddress = cpu_core.registers[0x1F]; //std::floor(registers[0x1F] / 4) * 4;
	for (int i = 0; i < 8; i++) {
		std::cout << " - " << std::hex << effectiveAddress + (i * 4) << " | " << ram[effectiveAddress + (i * 4)] << " - " << ram[effectiveAddress + 1 + (i * 4)] << " - " << ram[effectiveAddress + 2 + (i * 4)] << " - " << ram[effectiveAddress + 3 + (i * 4)] << std::endl;
	}
	std::cout << std::endl;
	std::cout << " - Video RAM ----" << std::endl;
	effectiveAddress = 0;
	for (int i = 0; i < 8; i++) {
		std::cout << " - " << std::hex << effectiveAddress + (i * 4) << " | " << vram[effectiveAddress + (i * 4)] << " - " << vram[effectiveAddress + 1 + (i * 4)] << " - " << vram[effectiveAddress + 2 + (i * 4)] << " - " << vram[effectiveAddress + 3 + (i * 4)] << std::endl;
	}
	std::cout << "}" << std::endl;
}
