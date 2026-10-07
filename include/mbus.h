#ifndef GBEMULATE_MBUS_H
#define GBEMULATE_MBUS_H

#include "gb.h"
#include "types.h"

u8 mbus_read(gb_t *gb, u16 addr);
void mbus_write(gb_t *gb, u16 addr, u8 data);

#endif
