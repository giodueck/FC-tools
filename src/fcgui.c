#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>

#include "fcgui.h"
#include "horizon/horizon_vm.h"
#include "program.h"

/* Global SDL variables */
static SDL_Window *fcgui_window = NULL;
static SDL_Renderer *fcgui_renderer = NULL;
static TTF_Font *fcgui_font = NULL;

/* Settings */
const int fcgui_width = 640;
const int fcgui_height = 480;
const int fcgui_pixel_width = 1;
const int fcgui_pixel_height = 1;
const int fcgui_fps_limit = 60;
const int fcgui_ptsize = 10;

/* Global state variables */


void fcgui_init()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(fcgui_width * fcgui_pixel_width, fcgui_height * fcgui_pixel_height, 0, &fcgui_window, &fcgui_renderer);
    SDL_RenderSetScale(fcgui_renderer, fcgui_pixel_width, fcgui_pixel_height);

    TTF_Init();
    fcgui_font = TTF_OpenFont("/usr/share/fonts/TTF/Hack-Regular.ttf", fcgui_ptsize);
    TTF_SetFontStyle(fcgui_font, TTF_STYLE_NORMAL);
    TTF_SetFontOutline(fcgui_font, 0);
    TTF_SetFontKerning(fcgui_font, 1);
    TTF_SetFontHinting(fcgui_font, TTF_HINTING_MONO | TTF_HINTING_LIGHT_SUBPIXEL);
}

void fcgui_quit()
{
    TTF_Quit();
    fcgui_font = NULL;

    SDL_DestroyWindow(fcgui_window);
    SDL_DestroyRenderer(fcgui_renderer);
    SDL_Quit();
    fcgui_window = NULL;
    fcgui_renderer = NULL;
}

void fcgui_start(int arch, uint32_t *program, size_t program_size)
{
    if (arch != ARCH_HORIZON)
        return;

    horizon_vm_t vm = { 0 };

    hovm_load_rom(&vm, program, program_size);

    fcgui_init();
    SDL_Delay(1000);
    fcgui_quit();
}
