/* reader.h — scripture reader module interface (banked to bank 15). */

#ifndef READER_H
#define READER_H

#include <gb/gb.h>

void reader_init(void) BANKED;
void reader_force_redraw(void) BANKED;
void reader_update(UBYTE pressed, UBYTE held) BANKED;
void reader_render(void) BANKED;

#endif
