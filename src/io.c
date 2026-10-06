#include "io.h"
#include "logger.h"

char *io_freadb(const char *fname, size_t *nbytes)
{
    FILE *f = fopen(fname, "rb");
    if (f == NULL)
    {
        log_error("Failed to open file '%s'\n", fname);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    *nbytes = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buffer = malloc(*nbytes);
    if (buffer == NULL)
    {
        log_error("Failed to allocate buffer for file '%s'\n", fname);
        fclose(f);
        return NULL;
    }

    fread(buffer, *nbytes, 1, f);
    fclose(f);
    return buffer;
}
