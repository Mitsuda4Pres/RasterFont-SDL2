//rftest.c
//Showcase/sandbox for rasterfont.h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "include/rasterfont.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 600
#define RED_OPAQUE 0xFF0000FF
#define GREEN_OPAQUE 0xFF00FF00
#define BLUE_OPAQUE 0xFFFF0000
#define YELLOW_OPAQUE 0xFF00FFFF
#define MAGENTA_OPAQUE 0xFFFF00FF
#define CYAN_OPAQUE 0xFFFFFF00
#define WHITE_OPAQUE 0xFFFFFFFF

//mouse flags
#define NONE 0x00
#define MOUSE_LEFT_BTN_DOWN 0x01
#define MOUSE_RIGHT_BTN_DOWN 0x02
#define MOUSE_WHEEL_CLICK_DOWN 0x04

typedef struct{
	/*Contains state data about entire program*/
	//
	//Structural
	SDL_Window *window;
	SDL_Renderer *renderer;
	struct RF_Font font;
	int app_is_running;
	int debug_text;    //toggle for debug text display
	//
	//Timing
	Uint64 prev_time;
	Uint64 current_time;
	double dt;
	double dt_accumulator;
	int phy_cycles_per_frame;
	//
	//Mouse context
	int mouse_x;
	int mouse_y;
	char button_down_state; //flag with LEFT_CLICK_DOWN, RIGHT_CLICK_DOWN, other state reserved for more buttons/wheel
	char btn_released_state; //same
} context;

//Function declarations
int initialize_window(context *ctx);
int processInput(context *ctx);
int update(context *ctx);
int draw(context *ctx);
void destroyWindow(context *ctx);

//Emscripten mainloop for non-blocking web-based callback system
static void mainloop(void *arg){
	context *ctx = (context *)(arg);
	if(ctx->app_is_running == 0){
		destroyWindow(ctx);
		#ifdef __EMSCRIPTEN__ 
		emscripten_cancel_main_loop();
		#else
		exit(0);
		#endif
	}	
	draw(ctx);
}

//Actual entry point of program, initializes, sets up objects, calls mainloop()_
int main(int argc, char *argv[]){
	context ctx;
	ctx.app_is_running = initialize_window(&ctx);
	//setup(&ctx->obj);
	#if __EMSCRIPTEN__
	emscripten_set_main_loop_arg(mainloop, &ctx, 0, 1);
	#else
	while(1) {mainloop();}
	#endif
	return 0;
}

//SDL Initializer funtion
int initialize_window(context *ctx) {
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0){
		fprintf(stderr, "Error initializing SDL. %s\n", SDL_GetError());
		return 1;
	}
	if(TTF_Init() != 0){
		fprintf(stderr, "Error initializing SDL_TTF. %s\n", SDL_GetError());
		return 1;
	}
	ctx->window = SDL_CreateWindow(
		"RasterFont Test",    //title
		SDL_WINDOWPOS_CENTERED,	//SDL macro to find centered position for x-coord
		SDL_WINDOWPOS_CENTERED,	//y-coord
		SCREEN_WIDTH,
		SCREEN_HEIGHT,			//defined in include.m4pcm.h
		SDL_WINDOW_BORDERLESS
	);
	//
	if(!ctx->window){
		fprintf(stderr, "Error creating SDL window.\n");
		return 1;
	}
	//
	ctx->renderer = SDL_CreateRenderer(ctx->window, -1, 0); //render to window, index to default display driver, no flags
	if(!ctx->renderer){
		fprintf(stderr, "Error creating SDL renderer.\n");
		return 1;
	}
	//
	//
	//Initialize debug state
	ctx->debug_text = 0;
	ctx->button_down_state = 0;
	//
	//load raster font
	ctx->font = RF_loadFontFromFile("resources/classic.rff");
	
	if(strcmp(ctx->font.error_msg, "clear") != 0)
		printf("RF_loadctx->fontFromFile() produced error: %s\n", ctx->font.error_msg);
	//Test display ctx->font struct elements
	printf("ctx->font name: %s\n", ctx->font.name);
	printf("ctx->font width: %d\n", ctx->font.base_width);
	printf("ctx->font height: %d\n", ctx->font.base_height);
	printf("1st character: %c, lines: %d, offset: %d\n", ctx->font.characters[0].name, ctx->font.characters[0].total_segments, ctx->font.characters[0].offset);
	printf("First line: %f, %f, %f, %f\n", ctx->font.characters[0].segments[0].a.x, ctx->font.characters[0].segments[0].a.y,ctx->font.characters[0].segments[0].b.x,ctx->font.characters[0].segments[0].b.y);
	printf("Fifth line: %f, %f, %f, %f\n", ctx->font.characters[0].segments[4].a.x, ctx->font.characters[0].segments[4].a.y,ctx->font.characters[0].segments[4].b.x,ctx->font.characters[0].segments[4].b.y);
	printf("2nd character: %c, lines: %d, offset: %d\n", ctx->font.characters[1].name, ctx->font.characters[1].total_segments, ctx->font.characters[1].offset);
	printf("First line: %f, %f, %f, %f\n", ctx->font.characters[1].segments[0].a.x, ctx->font.characters[1].segments[0].a.y,ctx->font.characters[1].segments[0].b.x,ctx->font.characters[1].segments[0].b.y);
	printf("Fifth line: %f, %f, %f, %f\n", ctx->font.characters[1].segments[4].a.x, ctx->font.characters[1].segments[4].a.y,ctx->font.characters[1].segments[4].b.x,ctx->font.characters[1].segments[4].b.y);
	printf("3rd character: %c, lines: %d, offset: %d\n", ctx->font.characters[2].name, ctx->font.characters[2].total_segments, ctx->font.characters[2].offset);
	printf("First line: %f, %f, %f, %f\n", ctx->font.characters[2].segments[0].a.x, ctx->font.characters[2].segments[0].a.y,ctx->font.characters[2].segments[0].b.x,ctx->font.characters[2].segments[0].b.y);
	printf("Fifth line: %f, %f, %f, %f\n", ctx->font.characters[2].segments[4].a.x, ctx->font.characters[2].segments[4].a.y,ctx->font.characters[2].segments[4].b.x,ctx->font.characters[2].segments[4].b.y);

	return 1; //1 = app is running, 0 = app is not running/exit
}

//Handle all input events
int processInput(context *ctx){
	SDL_Event event;
	while(SDL_PollEvent(&event) != 0){
		switch(event.type){
			case SDL_QUIT: {
				ctx->app_is_running = 0;	
				break;
			}
			case SDL_KEYDOWN: {
				switch(event.key.keysym.sym){
					case SDLK_ESCAPE:{
					//normally I would kill the app with
					//ctx->app_is_running = 0;
					//But this isn't needed in browser environment	
						break;
					}
					case SDLK_F1:{
						ctx->debug_text = !ctx->debug_text;
						if(ctx->debug_text)
							printf("Debug text on.\n");
						if(!ctx->debug_text)
							printf("Debug text off.\n");
						break;	
					}
				//Other keyboard actions here
				}
				break;
			}
			case SDL_MOUSEMOTION:{
				ctx->mouse_x = event.motion.x;
				ctx->mouse_y = event.motion.y;
				break;
			}
			case SDL_MOUSEBUTTONDOWN:{
				switch(event.button.button){
					case SDL_BUTTON_LEFT:{
						ctx->button_down_state |= MOUSE_LEFT_BTN_DOWN; 
						//randomVelocityAll(&ctx->obj);
						//randomImpulseAll(&ctx->obj);
						ctx->mouse_x = event.motion.x;
						ctx->mouse_y = event.motion.y;
		
						break;
					}
				}
				break;
			}
			case SDL_MOUSEBUTTONUP:{
				switch(event.button.button){
					case SDL_BUTTON_LEFT:{
						ctx->button_down_state &= ~MOUSE_LEFT_BTN_DOWN; //Operation to clear bit flag (AND equals NOT flag)
						break;
					}
				}
				break;
			}
		}
	}
	return 0;
}

//Update objects per cycle. Add delta time functionality.
int update(context *ctx){

		return 0;
}

//Draw objects based on data-oriented architecture.
int draw(context *ctx){
	SDL_SetRenderDrawColor(ctx->renderer,0,0,0,255);
	SDL_RenderClear(ctx->renderer);
	SDL_SetRenderDrawColor(ctx->renderer, 255,255,255,255);
	//FC_DrawAlign(ctx->chrono_font, ctx->renderer, (SCREEN_WIDTH/2), 8, FC_ALIGN_CENTER, "%d Bubbles!", ctx->obj.bubble_count);
	RF_printString(ctx->renderer, 10, 10, 12, "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG", &ctx->font, 0);
	RF_printString(ctx->renderer, 20,100, 48, "THE QUICK BROWN FOX", &ctx->font, 0);
	RF_printString(ctx->renderer, 40,250, 96, "JUMPS OVER", &ctx->font, 0);
	RF_printString(ctx->renderer, 600,50, 16, "THE LAZY DOG", &ctx->font, 0);
//	RF_printString(ctx->renderer, 10, 10, 12, "D D D D D D D D", &ctx->font, 0);
//  	RF_printString(ctx->renderer, 20,100, 48, "D D D D D", &ctx->font, 0);                          //debug text display
//  	RF_printString(ctx->renderer, 40,250, 96, "D D D D D D D", &ctx->font, 0);                              
//  	RF_printString(ctx->renderer, 600,50, 16, "D D D D ", &ctx->font, 0);
	
	//
	int mx, my;
		
	SDL_RenderPresent(ctx->renderer);
	return 0;	
}

//Destroy SDL objects and free memory.
void destroyWindow(context *ctx){
	//Free structural elements
    SDL_DestroyRenderer(ctx->renderer);
    ctx->renderer = NULL;
	SDL_DestroyWindow(ctx->window);
	ctx->window = NULL;
	RF_destroyFont(&ctx->font);
	TTF_Quit();
	SDL_Quit();
}
//End of life-cycle functions

/*
 *
 *
 *			Gameplay functions		
 *
 *
 */

