#include "cpu.h"
#include "gb.h"
#include "logger.h"
#include <stdlib.h>

cpu_t *cpu_init(void) {
    cpu_t *cpu = calloc(1, sizeof(cpu_t));
    if (cpu == NULL) {
        log_error("Failed to allocate cpu_t (cpu struct)\n");
        return NULL;
    }

    cpu->pc = 0x100;

    return cpu;
}

void cpu_free(cpu_t *cpu) { free(cpu); }
