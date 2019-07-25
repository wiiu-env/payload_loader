#include "elf_abi.h"
#include "fs_defs.h"
#include "common.h"
#include "utils.h"
#include "structs.h"
#include "elf_loading.h"
#include "memory_setup.h"

int _start(int argc, char **argv) {
    setup_memory();

    private_data_t private_data;
    loadFunctionPointers(&private_data);

    uint32_t newEntry = LoadAndCopyFile(CAFE_OS_SD_PATH WIIU_PATH "/payload.elf", &private_data);
    if(newEntry == 0) {
        // When loading the payload failed, wait 1 second to display the error and return back to System Menu.
        revertMainHook(&private_data);
        private_data.OSSleepTicks(SECS_TO_TICKS(1));
        private_data._SYSLaunchTitleWithStdArgsInNoSplash(private_data.sysmenuTitleID, 0);
        return ( (int (*)(int, char **))(*(unsigned int*)ADDRESS_OSTitle_main_entry_ptr) )(argc, argv);
    }

    return ((int (*)(int, char **))newEntry)(argc, argv);
}
