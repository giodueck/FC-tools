#include <SDL2/SDL.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_hints.h>
#include <stdlib.h>
#include <string.h>

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

#define FCGUI_ORANGE (SDL_Color) { 0xE4, 0x6B, 0x00, 0xFF }

// Running modes
enum fcgui_running_mode {
    FCGUI_START = -1,
    FCGUI_BREAK,
    FCGUI_STEP,
    FCGUI_CONTINUE,
    FCGUI_RUN,
    FCGUI_RESET,
};

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
const char *fcgui_title = "Factorio Computer Emulator";

/* Global state variables */
static int fcgui_ups = 0;
static int fcgui_ff = 0;    // fast forward mode
static int fcgui_mode = FCGUI_START;

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

    TTF_SetFontStyle(fcgui_font, font_options.style);
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
    TTF_SetFontStyle(fcgui_font, TTF_STYLE_NORMAL);
}

// Draw registers with their name, hex value and decimal equivalent. Also draw flag state
void fcgui_draw_registers(horizon_vm_t *vm, int xoffset, int yoffset)
{
    char buf[BUFSIZ] = { 0 };
    fcgui_font_options_t font_options = { .fg = FCGUI_LIGHT_GREY };

    // Header
    sprintf(buf, "Registers");
    font_options.fg = FCGUI_ORANGE;
    font_options.style = TTF_STYLE_BOLD;
    fcgui_draw_text(buf, xoffset, yoffset, font_options);
    yoffset += fcgui_ptsize * 1.5;
    font_options.fg = FCGUI_LIGHT_GREY;
    font_options.style = 0;

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

    // Flags
    yoffset += 17 * fcgui_ptsize;
    sprintf(buf, "Flags");
    font_options.fg = FCGUI_ORANGE;
    font_options.style = TTF_STYLE_BOLD;
    fcgui_draw_text(buf, xoffset, yoffset, font_options);
    font_options.fg = FCGUI_LIGHT_GREY;
    font_options.style = 0;

    xoffset += 5 * fcgui_ptsize;
    sprintf(buf, "Z");
    if (vm->z)
    {
        font_options.fg = FCGUI_ORANGE;
        font_options.style = TTF_STYLE_BOLD;
    }
    else
    {
        font_options.fg = FCGUI_GREY;
        font_options.style = TTF_STYLE_NORMAL;
    }
    fcgui_draw_text(buf, xoffset, yoffset, font_options);
    xoffset += 3 * fcgui_ptsize;

    sprintf(buf, "N");
    if (vm->n)
    {
        font_options.fg = FCGUI_ORANGE;
        font_options.style = TTF_STYLE_BOLD;
    }
    else
    {
        font_options.fg = FCGUI_GREY;
        font_options.style = TTF_STYLE_NORMAL;
    }
    fcgui_draw_text(buf, xoffset, yoffset, font_options);
    xoffset += 3 * fcgui_ptsize;

    sprintf(buf, "V");
    if (vm->v)
    {
        font_options.fg = FCGUI_ORANGE;
        font_options.style = TTF_STYLE_BOLD;
    }
    else
    {
        font_options.fg = FCGUI_GREY;
        font_options.style = TTF_STYLE_NORMAL;
    }
    fcgui_draw_text(buf, xoffset, yoffset, font_options);
    xoffset += 3 * fcgui_ptsize;
}

// Draw a section of RAM in hexadecimal. The section to draw is decided with
// the address register (AR)
void fcgui_draw_ram(horizon_vm_t *vm, int xoffset, int yoffset)
{
    char buf[BUFSIZ] = { 0 };
    char catbuf[BUFSIZ] = { 0 };
    fcgui_font_options_t font_options = { .fg = FCGUI_LIGHT_GREY };

    // Get memory cell start of address AR points to
    int cell = vm->registers[HO_AR] / HOVM_RAM_CELL_SIZE;

    // Header
    sprintf(buf, "RAM");
    font_options.fg = FCGUI_ORANGE;
    font_options.style = TTF_STYLE_BOLD;
    fcgui_draw_text(buf, xoffset, yoffset, font_options);
    yoffset += fcgui_ptsize * 1.5;
    font_options.fg = FCGUI_LIGHT_GREY;
    font_options.style = 0;

    // Memory contents
    for (int i = 0; i < 8; i++)
    {
        sprintf(buf, "%4x", cell * HOVM_RAM_CELL_SIZE + i * 8);
        font_options.fg = FCGUI_GREY;
        fcgui_draw_text(buf, xoffset, yoffset + i * (1.5 * fcgui_ptsize), font_options);
        font_options.fg = FCGUI_LIGHT_GREY;

        buf[0] = 0;
        for (int j = 0; j < 8; j++)
        {
            sprintf(catbuf, "%08x  ", vm->ram[cell * HOVM_RAM_CELL_SIZE + i * 8 + j]);
            strcat(buf, catbuf);
        }

        fcgui_draw_text(buf, xoffset + 50, yoffset + i * (1.5 * fcgui_ptsize), font_options);
    }
}

// Draw the top of the stack, up to a maximum depth
void fcgui_draw_stack(horizon_vm_t *vm, int xoffset, int yoffset, int depth)
{
    char buf[BUFSIZ] = { 0 };
    fcgui_font_options_t font_options = { .fg = FCGUI_LIGHT_GREY };

    // Get SP
    int top = vm->registers[HO_SP] - 1;

    // Header
    sprintf(buf, "Stack");
    font_options.fg = FCGUI_ORANGE;
    font_options.style = TTF_STYLE_BOLD;
    fcgui_draw_text(buf, xoffset, yoffset, font_options);
    yoffset += fcgui_ptsize * 1.5;
    font_options.fg = FCGUI_LIGHT_GREY;
    font_options.style = 0;

    // Draw top of stack
    for (int i = 0; i < depth; i++)
    {
        if (top - i < 0 || top - i > HOVM_STACK_SIZE)
        {
            sprintf(buf, "--------");
            fcgui_draw_text(buf, xoffset, yoffset + i * 1.5 * fcgui_ptsize, font_options);
            continue;
        }

        sprintf(buf, "%08x  %d", vm->stack[top - i], vm->stack[top - i]);
        fcgui_draw_text(buf, xoffset, yoffset + i * 1.5 * fcgui_ptsize, font_options);
    }
}

// Draw some program instructions: the one PC points to in the center,
// then some instructiions preceding and some following it
void fcgui_draw_program(horizon_vm_t *vm, int xoffset, int yoffset)
{
    char buf[BUFSIZ] = { 0 };
    fcgui_font_options_t font_options = { .fg = FCGUI_LIGHT_GREY };
    int instr_count = 8;

    // Get PC
    int pc = vm->registers[HO_PC];

    // Header
    sprintf(buf, "Program");
    font_options.fg = FCGUI_ORANGE;
    font_options.style = TTF_STYLE_BOLD;
    fcgui_draw_text(buf, xoffset, yoffset, font_options);
    font_options.style = 0;

    // Cycle counter
    sprintf(buf, "Cycles: %u", vm->cycles);
    font_options.fg = FCGUI_GREY;
    fcgui_draw_text(buf, xoffset + 8 * fcgui_ptsize, yoffset, font_options);
    font_options.fg = FCGUI_LIGHT_GREY;
    yoffset += fcgui_ptsize * 1.5;

    // preceding instructions
    char addrbuf[BUFSIZ] = { 0 };
    for (int i = instr_count; i > 0; i--)
    {
        if (pc - i >= 0)
        {
            sprintf(addrbuf, "%4x ", pc - i);
            font_options.fg = FCGUI_GREY;
            fcgui_draw_text(addrbuf, xoffset, yoffset, font_options);
            font_options.fg = FCGUI_LIGHT_GREY;
        }

        hovm_disassemble(buf, vm, pc - i);
        fcgui_draw_text(buf, xoffset + 4 * fcgui_ptsize, yoffset, font_options);
        yoffset += fcgui_ptsize * 1.5;
    }

    sprintf(addrbuf, "%4x ", pc);
    font_options.fg = FCGUI_GREY;
    fcgui_draw_text(addrbuf, xoffset, yoffset, font_options);
    font_options.fg = FCGUI_LIGHT_GREY;

    hovm_disassemble(buf, vm, pc);
    font_options.fg = FCGUI_ORANGE;
    fcgui_draw_text(buf, xoffset + 4 * fcgui_ptsize, yoffset, font_options);
    yoffset += fcgui_ptsize * 1.5;
    font_options.fg = FCGUI_LIGHT_GREY;

    // following instructions
    for (int i = 1; i < instr_count; i++)
    {
        sprintf(addrbuf, "%4x ", pc + i);
        font_options.fg = FCGUI_GREY;
        fcgui_draw_text(addrbuf, xoffset, yoffset, font_options);
        font_options.fg = FCGUI_LIGHT_GREY;

        hovm_disassemble(buf, vm, pc + i);
        fcgui_draw_text(buf, xoffset + 4 * fcgui_ptsize, yoffset, font_options);
        yoffset += fcgui_ptsize * 1.5;
    }
}

// Draw control hints
void fcgui_draw_controls(int xoffset, int yoffset)
{
    char buf[BUFSIZ] = { 0 };
    fcgui_font_options_t font_options = { .fg = FCGUI_LIGHT_GREY };

    // Header
    sprintf(buf, "Controls");
    font_options.fg = FCGUI_ORANGE;
    font_options.style = TTF_STYLE_BOLD;
    fcgui_draw_text(buf, xoffset, yoffset, font_options);
    yoffset += fcgui_ptsize * 1.5;
    font_options.fg = FCGUI_GREY;
    font_options.style = 0;

    // Step
    sprintf(buf, "Step        <S>/<Space>");
    fcgui_draw_text(buf, xoffset, yoffset, font_options);
    // Continue
    sprintf(buf, "Continue    <C>");
    fcgui_draw_text(buf, xoffset + 20 * fcgui_ptsize, yoffset, font_options);
    // Reset
    sprintf(buf, "Reset       <C-R>");
    fcgui_draw_text(buf, xoffset, yoffset + 1.5 * fcgui_ptsize, font_options);
    // Run
    sprintf(buf, "Run to halt <R>");
    fcgui_draw_text(buf, xoffset + 20 * fcgui_ptsize, yoffset + 1.5 * fcgui_ptsize, font_options);
}

void fcgui_start(int arch, uint32_t *program, size_t program_size)
{
    if (arch != ARCH_HORIZON)
        return;

    horizon_vm_t vm = { 0 };

    hovm_load_rom(&vm, program, program_size);

    fcgui_init();

    // Update loop
    uint8_t quit = 0;
    uint64_t last_time = SDL_GetTicks64();
    uint64_t ups_timer = 1000, ups_counter = 0;
    while (!quit)
    {
        /* Time keeping */
        // UPS counting
        uint64_t new_time = SDL_GetTicks64();
        uint64_t elapsed_time = new_time - last_time;
        last_time = new_time;
        ups_counter++;
        ups_timer += elapsed_time;

        if (ups_timer >= 1000)
        {
            fcgui_ups = ups_counter;

            char title_str[256];
            SDL_snprintf(title_str, sizeof(title_str), "[%d UPS]\t%s", fcgui_ups, fcgui_title);
            SDL_SetWindowTitle(fcgui_window, title_str);

            ups_timer -= 1000;
            ups_counter = 0;
        }

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
                    case SDLK_s:
                    case SDLK_SPACE:
                        fcgui_mode = FCGUI_STEP;
                        fcgui_ff = 0;
                        break;
                    case SDLK_c:
                        fcgui_mode = FCGUI_CONTINUE;
                        if (SDL_GetModState() & KMOD_SHIFT)
                            fcgui_ff = 1;
                        else
                            fcgui_ff = 0;
                        break;
                    case SDLK_r:
                        if (SDL_GetModState() & KMOD_CTRL)
                        {
                            fcgui_mode = FCGUI_RESET;
                            fcgui_ff = 0;
                            break;
                        }
                        fcgui_mode = FCGUI_RUN;
                        if (SDL_GetModState() & KMOD_SHIFT)
                            fcgui_ff = 1;
                        else
                            fcgui_ff = 0;
                        break;
                    default:
                        break;
                }
            }
        }

        /* Update */
        switch (fcgui_mode)
        {
            case FCGUI_START:
                fcgui_mode = FCGUI_BREAK;
                break;
            case FCGUI_STEP:
                hovm_step(&vm);
                fcgui_mode = FCGUI_BREAK;
                break;
            case FCGUI_CONTINUE:
                hovm_step(&vm);
                break;
            case FCGUI_RUN:
                hovm_step(&vm);
                break;
            case FCGUI_RESET:
                hovm_reset(&vm);
                hovm_load_rom(&vm, program, program_size);
                break;
            case FCGUI_BREAK:
            default:
                continue;
        }

        if (vm.ram[vm.registers[HO_PC]] == HOVM_HALT || vm.breakpoint_map[vm.registers[HO_PC]])
        {
            fcgui_mode = FCGUI_BREAK;
            fcgui_ff = 0;
        }

        /* Drawing */
        // Fast forward mode skips redrawing
        if (fcgui_ff)
            continue;
        // Clear
        SDL_SetRenderDrawColor(fcgui_renderer, fcgui_bgcolor.r, fcgui_bgcolor.g, fcgui_bgcolor.b, fcgui_bgcolor.a);
        SDL_RenderClear(fcgui_renderer);

        // RAM
        int xoffset = 20, yoffset = 10;
        fcgui_draw_ram(&vm, xoffset, yoffset);

        // Program
        yoffset = 250;
        fcgui_draw_program(&vm, xoffset, yoffset);

        // Constrols
        xoffset = 20, yoffset = 700;
        fcgui_draw_controls(xoffset, yoffset);

        // Registers + flags
        xoffset = fcgui_width - 300, yoffset = 10;
        fcgui_draw_registers(&vm, xoffset, yoffset);

        // Stack
        yoffset = 340;
        fcgui_draw_stack(&vm, xoffset, yoffset, 16);

        /* End drawing */
        SDL_RenderPresent(fcgui_renderer);
    }

    fcgui_quit();
}
