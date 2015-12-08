#include <GL/glew.h>
#include <glm/matrix.hpp>
#include <iostream>
#include <cassert>
#include <SDL2\SDL.h>
#include <ft2build.h>
#include FT_FREETYPE_H

SDL_Window* window = nullptr;
SDL_GLContext glContext;
bool quit = false;
FT_Library library;

void init() {
	FT_Init_FreeType(&library);
	SDL_Init(SDL_INIT_EVERYTHING);

	// Set OpenGL attributes.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	// Create the window. Use data from the config.
	window = SDL_CreateWindow(
		"unnamed",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		1280,
		720, 
		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);


	glContext = SDL_GL_CreateContext(window); // Create context.

	// Initialise glew and TTF.
	glewExperimental = GL_TRUE;
	glewInit();

	glClearColor(0.2f, 0.4f, 0.6f, 1.0f); // Set clear color.

	// Enable GL stuff.
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void update();
void draw();
int main(int argc, char** argv) {
	init();

	while (!quit) {
		SDL_Event evnt;
		while (SDL_PollEvent(&evnt)) {
			switch (evnt.type) {
			case SDL_KEYDOWN:
				switch (evnt.key.keysym.sym) {
				case SDLK_ESCAPE:
					quit = true;
					break;
				}
				break;
			}
		}
		update();
		draw();
		SDL_Delay(16);
	}

	SDL_Quit();
	FT_Done_FreeType(library);
	return 0;
}

void update() {


}

void draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	SDL_GL_SwapWindow(window);
}