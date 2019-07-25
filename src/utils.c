#include "utils.h"

extern void SCKernelCopyData(unsigned int addr, unsigned int src, unsigned int len);
extern void SC_0x25_KernelCopyData(unsigned int addr, unsigned int src, unsigned int len);

static void KernelWriteU32(uint32_t addr, uint32_t value, private_data_t * pdata) {
    pdata->ICInvalidateRange(&value, 4);
    pdata->DCFlushRange(&value, 4);

    uint32_t dst = (uint32_t) pdata->OSEffectiveToPhysical((void *)addr);
    uint32_t src = (uint32_t) pdata->OSEffectiveToPhysical((void *)&value);

    SC_0x25_KernelCopyData(dst, src, 4);

    pdata->DCFlushRange((void *)addr, 4);
    pdata->ICInvalidateRange((void *)addr, 4);
}

void revertMainHook(private_data_t * private_data) {
    kern_write((void*)(KERN_SYSCALL_TBL_1 + (0x25 * 4)), (unsigned int)SCKernelCopyData);
    kern_write((void*)(KERN_SYSCALL_TBL_2 + (0x25 * 4)), (unsigned int)SCKernelCopyData);
    kern_write((void*)(KERN_SYSCALL_TBL_3 + (0x25 * 4)), (unsigned int)SCKernelCopyData);
    kern_write((void*)(KERN_SYSCALL_TBL_4 + (0x25 * 4)), (unsigned int)SCKernelCopyData);
    kern_write((void*)(KERN_SYSCALL_TBL_5 + (0x25 * 4)), (unsigned int)SCKernelCopyData);

    unsigned int repl_addr = ADDRESS_main_entry_hook;
    unsigned int bufferU32 = 0x4E800421;
    KernelWriteU32(repl_addr,bufferU32,private_data);
}

void PrepareScreen(private_data_t *private_data) {
    // Prepare screen
    int screen_buf0_size = 0;
    int screen_buf1_size = 0;
    unsigned int screen_color = 0; // (r << 24) | (g << 16) | (b << 8) | a;

    // Init screen and screen buffers
    private_data->OSScreenInit();
    screen_buf0_size = private_data->OSScreenGetBufferSizeEx(0);
    screen_buf1_size = private_data->OSScreenGetBufferSizeEx(1);
    private_data->OSScreenSetBufferEx(0, (void *)0xF4000000);
    private_data->OSScreenSetBufferEx(1, (void *)0xF4000000 + screen_buf0_size);

    // Clear screens
    private_data->OSScreenClearBufferEx(0, screen_color);
    private_data->OSScreenClearBufferEx(1, screen_color);

    // Flush the cache
    private_data->DCFlushRange((void *)0xF4000000, screen_buf0_size);
    private_data->DCFlushRange((void *)0xF4000000 + screen_buf0_size, screen_buf1_size);

    // Flip buffers
    private_data->OSScreenFlipBuffersEx(0);
    private_data->OSScreenFlipBuffersEx(1);
}

void ExitFailure(private_data_t *private_data, const char *failure) {
    PrepareScreen(private_data);
    private_data->OSScreenClearBufferEx(1,0);
    private_data->OSScreenPutFontEx(1, 0, 0, failure);
    private_data->OSScreenFlipBuffersEx(1);
    private_data->OSScreenClearBufferEx(1,0);
    private_data->OSScreenPutFontEx(1, 0, 0, failure);
    private_data->OSScreenFlipBuffersEx(1);
}

void loadFunctionPointers(private_data_t * private_data) {
    unsigned int coreinit_handle;

    OSDynLoad_Acquire("coreinit", &coreinit_handle);

    unsigned int *functionPtr = 0;

    OSDynLoad_FindExport(coreinit_handle, 1, "MEMAllocFromDefaultHeapEx", &functionPtr);
    private_data->MEMAllocFromDefaultHeapEx = (void * (*)(int, int))*functionPtr;
    OSDynLoad_FindExport(coreinit_handle, 1, "MEMFreeToDefaultHeap", &functionPtr);
    private_data->MEMFreeToDefaultHeap = (void (*)(void *))*functionPtr;

    OS_FIND_EXPORT(coreinit_handle, "memcpy", private_data->memcpy);
    OS_FIND_EXPORT(coreinit_handle, "memset", private_data->memset);
    OS_FIND_EXPORT(coreinit_handle, "DCFlushRange", private_data->DCFlushRange);
    OS_FIND_EXPORT(coreinit_handle, "DCInvalidateRange", private_data->DCInvalidateRange);
    OS_FIND_EXPORT(coreinit_handle, "ICInvalidateRange", private_data->ICInvalidateRange);
    OS_FIND_EXPORT(coreinit_handle, "OSEffectiveToPhysical", private_data->OSEffectiveToPhysical);
    OS_FIND_EXPORT(coreinit_handle, "exit", private_data->exit);

    OS_FIND_EXPORT(coreinit_handle, "FSInit", private_data->FSInit);
    OS_FIND_EXPORT(coreinit_handle, "FSAddClientEx", private_data->FSAddClientEx);
    OS_FIND_EXPORT(coreinit_handle, "FSDelClient", private_data->FSDelClient);
    OS_FIND_EXPORT(coreinit_handle, "FSInitCmdBlock", private_data->FSInitCmdBlock);
    OS_FIND_EXPORT(coreinit_handle, "FSGetMountSource", private_data->FSGetMountSource);
    OS_FIND_EXPORT(coreinit_handle, "FSMount", private_data->FSMount);
    OS_FIND_EXPORT(coreinit_handle, "FSUnmount", private_data->FSUnmount);
    OS_FIND_EXPORT(coreinit_handle, "FSOpenFile", private_data->FSOpenFile);
    OS_FIND_EXPORT(coreinit_handle, "FSGetStatFile", private_data->FSGetStatFile);
    OS_FIND_EXPORT(coreinit_handle, "FSReadFile", private_data->FSReadFile);
    OS_FIND_EXPORT(coreinit_handle, "FSCloseFile", private_data->FSCloseFile);

    OS_FIND_EXPORT(coreinit_handle, "OSSleepTicks", private_data->OSSleepTicks);
    OS_FIND_EXPORT(coreinit_handle, "OSScreenPutFontEx", private_data->OSScreenPutFontEx);

    OS_FIND_EXPORT(coreinit_handle, "OSScreenInit", private_data->OSScreenInit);
    OS_FIND_EXPORT(coreinit_handle, "OSScreenGetBufferSizeEx", private_data->OSScreenGetBufferSizeEx);
    OS_FIND_EXPORT(coreinit_handle, "OSScreenSetBufferEx", private_data->OSScreenSetBufferEx);
    OS_FIND_EXPORT(coreinit_handle, "OSScreenClearBufferEx", private_data->OSScreenClearBufferEx);
    OS_FIND_EXPORT(coreinit_handle, "OSScreenFlipBuffersEx", private_data->OSScreenFlipBuffersEx);
    OS_FIND_EXPORT(coreinit_handle, "OSForceFullRelaunch", private_data->OSForceFullRelaunch);


    unsigned int sysapp_handle;
    OSDynLoad_Acquire("sysapp.rpl", &sysapp_handle);
    OS_FIND_EXPORT(sysapp_handle, "SYSRelaunchTitle", private_data->SYSRelaunchTitle);
    OS_FIND_EXPORT(sysapp_handle, "SYSLaunchMenu", private_data->SYSLaunchMenu);
    OS_FIND_EXPORT(sysapp_handle, "_SYSLaunchMiiStudio", private_data->_SYSLaunchMiiStudio);
    OS_FIND_EXPORT(sysapp_handle, "_SYSGetSystemApplicationTitleId", private_data->_SYSGetSystemApplicationTitleId);
    OS_FIND_EXPORT(sysapp_handle, "_SYSLaunchTitleWithStdArgsInNoSplash", private_data->_SYSLaunchTitleWithStdArgsInNoSplash);

    private_data->sysmenuTitleID = private_data->_SYSGetSystemApplicationTitleId(0);
}

/* Read a 32-bit word with kernel permissions */
uint32_t __attribute__ ((noinline)) kern_read(const void *addr) {
    uint32_t result;
    asm volatile (
        "li 3,1\n"
        "li 4,0\n"
        "li 5,0\n"
        "li 6,0\n"
        "li 7,0\n"
        "lis 8,1\n"
        "mr 9,%1\n"
        "li 0,0x3400\n"
        "mr %0,1\n"
        "sc\n"
        "nop\n"
        "mr 1,%0\n"
        "mr %0,3\n"
        :	"=r"(result)
        :	"b"(addr)
        :	"memory", "ctr", "lr", "0", "3", "4", "5", "6", "7", "8", "9", "10",
        "11", "12"
    );

    return result;
}

/* Write a 32-bit word with kernel permissions */
void __attribute__ ((noinline)) kern_write(void *addr, uint32_t value) {
    asm volatile (
        "li 3,1\n"
        "li 4,0\n"
        "mr 5,%1\n"
        "li 6,0\n"
        "li 7,0\n"
        "lis 8,1\n"
        "mr 9,%0\n"
        "mr %1,1\n"
        "li 0,0x3500\n"
        "sc\n"
        "nop\n"
        "mr 1,%1\n"
        :
        :	"r"(addr), "r"(value)
        :	"memory", "ctr", "lr", "0", "3", "4", "5", "6", "7", "8", "9", "10",
        "11", "12"
    );
}
