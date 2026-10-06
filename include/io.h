#ifndef GBEMULATE_IO_H
#define GBEMULATE_IO_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

char *io_freadb(const char *fname, size_t *nbytes);

#endif
