#ifndef FC_GUI_H
#define FC_GUI_H

#include <stdint.h>
#include <sys/types.h>

// Start the main gui for execution
void fcgui_start(int arch, uint32_t *program, size_t program_size);

#endif // FC_GUI_H
