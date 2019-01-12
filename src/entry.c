#include "elf_abi.h"
#include "fs_defs.h"
#include "common.h"
#include "utils.h"
#include "structs.h"
#include "elf_loading.h"
#include "memory_setup.h"

int _start(int argc, char **argv) {
    setup_memory();

    uint32_t newEntry = LoadAndCopyFile(CAFE_OS_SD_PATH WIIU_PATH "/payload.elf");

    return ((int (*)(int, char **))newEntry)(argc, argv);
}
