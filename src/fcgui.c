#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_hints.h>
#include <stdlib.h>

#include "fcgui.h"
#include "horizon/horizon_vm.h"
#include "program.h"

/* Custom definitions */
typedef struct {
    int style;
    int outline;
    int kerning;
    int hinting;
    int alignment;
    SDL_Color fg, bg;
} fcgui_font_options_t;

enum fcgui_text_elignment {
    FCGUI_ALIGN_LEFT,
    FCGUI_ALIGN_RIGHT,
    FCGUI_ALIGN_CENTER
};

#define FCGUI_BLACK      (SDL_Color) { 0x00, 0x00, 0x00, 0xFF }
#define FCGUI_WHITE      (SDL_Color) { 0xFF, 0xFF, 0xFF, 0xFF }
#define FCGUI_GREY       (SDL_Color) { 0x80, 0x80, 0x80, 0xFF }
#define FCGUI_DARK_GREY  (SDL_Color) { 0x40, 0x40, 0x40, 0xFF }
#define FCGUI_LIGHT_GREY (SDL_Color) { 0xD0, 0xD0, 0xD0, 0xFF }

#define FCGUI_RED     (SDL_Color) { 0xFF, 0x00, 0x00, 0xFF }
#define FCGUI_GREEN   (SDL_Color) { 0x00, 0xFF, 0x00, 0xFF }
#define FCGUI_BLUE    (SDL_Color) { 0x00, 0x00, 0xFF, 0xFF }
#define FCGUI_YELLOW  (SDL_Color) { 0xFF, 0xFF, 0x00, 0xFF }
#define FCGUI_CYAN    (SDL_Color) { 0x00, 0xFF, 0xFF, 0xFF }
#define FCGUI_MAGENTA (SDL_Color) { 0xFF, 0x00, 0xFF, 0xFF }

#define FCGUI_DARK_RED     (SDL_Color) { 0x80, 0x00, 0x00, 0xFF }
#define FCGUI_DARK_GREEN   (SDL_Color) { 0x00, 0x80, 0x00, 0xFF }
#define FCGUI_DARK_BLUE    (SDL_Color) { 0x00, 0x00, 0x80, 0xFF }
#define FCGUI_DARK_YELLOW  (SDL_Color) { 0x80, 0x80, 0x00, 0xFF }
#define FCGUI_DARK_CYAN    (SDL_Color) { 0x00, 0x80, 0x80, 0xFF }
#define FCGUI_DARK_MAGENTA (SDL_Color) { 0x80, 0x00, 0x80, 0xFF }

/* Global SDL variables */
static SDL_Window *fcgui_window = NULL;
static SDL_Renderer *fcgui_renderer = NULL;
static TTF_Font *fcgui_font = NULL;

/* Settings */
const int fcgui_width = 1200;
const int fcgui_height = 800;
const int fcgui_pixel_width = 1;
const int fcgui_pixel_height = 1;
const int fcgui_fps_limit = 60;
const int fcgui_ptsize = 16;
const SDL_Color fcgui_bgcolor = FCGUI_BLACK;

/* Global state variables */


void fcgui_init()
{
    // SDL_SetHint(SDL_HINT_VIDEODRIVER, "wayland,x11");
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

void fcgui_perror(const char *message)
{
    printf("[%lu] Error: %s\n", SDL_GetTicks64(), message);
}

void fcgui_draw_text(const char *string, int x, int y, fcgui_font_options_t font_options)
{
    if (fcgui_font == NULL)
    {
        fcgui_perror("font not initialized");
        return;
    }

    SDL_Surface *surface = NULL;
    SDL_Texture *texture = NULL;

    surface = TTF_RenderText_Solid(fcgui_font, string, font_options.fg);
    texture = SDL_CreateTextureFromSurface(fcgui_renderer, surface);

    int textw = 0, texth = 0;
    SDL_QueryTexture(texture, NULL, NULL, &textw, &texth);

    if (font_options.alignment == FCGUI_ALIGN_LEFT)
        SDL_RenderCopy(fcgui_renderer, texture, NULL, &((SDL_Rect) {.x = x, .y = y, .w = textw, .h = texth}));
    else if (font_options.alignment == FCGUI_ALIGN_RIGHT)
        SDL_RenderCopy(fcgui_renderer, texture, NULL, &((SDL_Rect) {.x = x - textw, .y = y, .w = textw, .h = texth}));
    else if (font_options.alignment == FCGUI_ALIGN_CENTER)
        SDL_RenderCopy(fcgui_renderer, texture, NULL, &((SDL_Rect) {.x = x - textw / 2, .y = y, .w = textw, .h = texth}));

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void fcgui_draw_registers(horizon_vm_t *vm, int xoffset, int yoffset)
{
    char buf[BUFSIZ] = { 0 };
    fcgui_font_options_t font_options = { .fg = FCGUI_LIGHT_GREY };

    sprintf(buf, "R0  = %08x = %d", vm->registers[HO_R0], vm->registers[HO_R0]);
    fcgui_draw_text(buf, xoffset, yoffset, font_options);
    sprintf(buf, "R1  = %08x = %d", vm->registers[HO_R1], vm->registers[HO_R1]);
    fcgui_draw_text(buf, xoffset, yoffset + 1 * fcgui_ptsize, font_options);
    sprintf(buf, "R2  = %08x = %d", vm->registers[HO_R2], vm->registers[HO_R2]);
    fcgui_draw_text(buf, xoffset, yoffset + 2 * fcgui_ptsize, font_options);
    sprintf(buf, "R3  = %08x = %d", vm->registers[HO_R3], vm->registers[HO_R3]);
    fcgui_draw_text(buf, xoffset, yoffset + 3 * fcgui_ptsize, font_options);
    sprintf(buf, "R4  = %08x = %d", vm->registers[HO_R4], vm->registers[HO_R4]);
    fcgui_draw_text(buf, xoffset, yoffset + 4 * fcgui_ptsize, font_options);
    sprintf(buf, "R5  = %08x = %d", vm->registers[HO_R5], vm->registers[HO_R5]);
    fcgui_draw_text(buf, xoffset, yoffset + 5 * fcgui_ptsize, font_options);
    sprintf(buf, "R6  = %08x = %d", vm->registers[HO_R6], vm->registers[HO_R6]);
    fcgui_draw_text(buf, xoffset, yoffset + 6 * fcgui_ptsize, font_options);
    sprintf(buf, "R7  = %08x = %d", vm->registers[HO_R7], vm->registers[HO_R7]);
    fcgui_draw_text(buf, xoffset, yoffset + 7 * fcgui_ptsize, font_options);
    sprintf(buf, "R8  = %08x = %d", vm->registers[HO_R8], vm->registers[HO_R8]);
    fcgui_draw_text(buf, xoffset, yoffset + 8 * fcgui_ptsize, font_options);
    sprintf(buf, "R9  = %08x = %d", vm->registers[HO_R9], vm->registers[HO_R9]);
    fcgui_draw_text(buf, xoffset, yoffset + 9 * fcgui_ptsize, font_options);
    sprintf(buf, "R10 = %08x = %d", vm->registers[HO_R10], vm->registers[HO_R10]);
    fcgui_draw_text(buf, xoffset, yoffset + 10 * fcgui_ptsize, font_options);
    sprintf(buf, "R11 = %08x = %d", vm->registers[HO_R11], vm->registers[HO_R11]);
    fcgui_draw_text(buf, xoffset, yoffset + 11 * fcgui_ptsize, font_options);
    sprintf(buf, "AR  = %08x = %d", vm->registers[HO_AR], vm->registers[HO_AR]);
    fcgui_draw_text(buf, xoffset, yoffset + 12 * fcgui_ptsize, font_options);
    sprintf(buf, "SP  = %08x = %d", vm->registers[HO_SP], vm->registers[HO_SP]);
    fcgui_draw_text(buf, xoffset, yoffset + 13 * fcgui_ptsize, font_options);
    sprintf(buf, "LR  = %08x = %d", vm->registers[HO_LR], vm->registers[HO_LR]);
    fcgui_draw_text(buf, xoffset, yoffset + 14 * fcgui_ptsize, font_options);
    sprintf(buf, "PC  = %08x = %d", vm->registers[HO_PC], vm->registers[HO_PC]);
    fcgui_draw_text(buf, xoffset, yoffset + 15 * fcgui_ptsize, font_options);

}

void fcgui_draw_ram(horizon_vm_t *vm, int xoffset, int yoffset)
{
    char buf[BUFSIZ] = { 0 };
    fcgui_font_options_t font_options = { .fg = FCGUI_LIGHT_GREY };

}

void fcgui_start(int arch, uint32_t *program, size_t program_size)
{
    if (arch != ARCH_HORIZON)
        return;

    horizon_vm_t vm = { 0 };
    vm.registers[0] = 0x80000000;

    hovm_load_rom(&vm, program, program_size);

    fcgui_init();

    // Update loop
    uint8_t quit = 0;
    while (!quit)
    {
        /* Input handling */
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = 1;
            }
            else if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                    case SDLK_q:
                        quit = 1;
                        break;
                    default:
                        break;
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                switch (e.button.button)
                {
                    case SDL_BUTTON_LEFT:
                        printf("x = %d, y = %d\n", e.button.x, e.button.y);
                        break;
                    default:
                        break;
                }
            }
        }

        /* Update */
        hovm_step(&vm);

        /* Time keeping */
        /* Drawing */
        // Clear
        SDL_SetRenderDrawColor(fcgui_renderer, fcgui_bgcolor.r, fcgui_bgcolor.g, fcgui_bgcolor.b, fcgui_bgcolor.a);
        SDL_RenderClear(fcgui_renderer);

        // Registers
        int xoffset = fcgui_width - 300, yoffset = 10;
        fcgui_draw_registers(&vm, xoffset, yoffset);

        // RAM
        xoffset = 10, yoffset = 10;
        fcgui_draw_ram(&vm, xoffset, yoffset);

        /* End drawing */
        SDL_RenderPresent(fcgui_renderer);
    }

    fcgui_quit();
}
