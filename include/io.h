#ifndef GBEMULATE_IO_H
#define GBEMULATE_IO_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * io_freadb - read a binary file into a buffer.
 * @fname:  Name of the file to read.
 * @nbytes: Pointer to a variable where the number of bytes read will be stored.
 *
 * Return: Pointer to the allocated buffer on success, or NULL on failure.
 *         The caller is responsible for freeing the returned buffer.
 */
char *io_freadb(const char *fname, size_t *nbytes);

#endif
