#include "memory_setup.h"
#include "utils.h"

/* assembly functions */
extern void SC_0x09_SETIBAT0(uint32_t upper, uint32_t lower);
extern void SC_0x36_SETBATS(void);

static void SCSetupIBAT4DBAT5() {
    asm volatile("eieio; isync");

    // Give our and the kernel full execution rights.
    // 00800000-01000000 => 30800000-31000000 (read/write, user/supervisor)
    unsigned int ibat4u = 0x008000FF;
    unsigned int ibat4l = 0x30800012;
    asm volatile("mtspr 560, %0" : : "r" (ibat4u));
    asm volatile("mtspr 561, %0" : : "r" (ibat4l));

    // Give our and the kernel full data access rights.
    // 00800000-01000000 => 30800000-31000000 (read/write, user/supervisor)
    unsigned int dbat5u = ibat4u;
    unsigned int dbat5l = ibat4l;
    asm volatile("mtspr 570, %0" : : "r" (dbat5u));
    asm volatile("mtspr 571, %0" : : "r" (dbat5l));

    asm volatile("eieio; isync");
}

static void thread_callback(int argc, void *argv) {
    // We need to give the kernel access to our memory.
    // VA: 01000000..01800000 PA: 32000000..32800000  with r/w for user and supervisor
    SC_0x09_SETIBAT0(0x010000FF,0x32000012);

    // Setting the IBAT and DBAT to give us full access (rwx) to 0x00800000 - 0x010000000
    SC_0x36_SETBATS();
}

void setup_memory() {
    /*
    The syscall is already (expected to be) set up by the loader at this point.

    uint32_t setIBAT0 = 0xFFF02344;
    kern_write((void*)(KERN_SYSCALL_TBL_1 + (0x25 * 4)), (uint32_t) setIBAT0);
    kern_write((void*)(KERN_SYSCALL_TBL_2 + (0x25 * 4)), (uint32_t) setIBAT0);
    kern_write((void*)(KERN_SYSCALL_TBL_3 + (0x25 * 4)), (uint32_t) setIBAT0);
    kern_write((void*)(KERN_SYSCALL_TBL_4 + (0x25 * 4)), (uint32_t) setIBAT0);
    kern_write((void*)(KERN_SYSCALL_TBL_5 + (0x25 * 4)), (uint32_t) setIBAT0); */

    // Setup the IBAT DBAT syscall
    kern_write((void*)(KERN_SYSCALL_TBL_1 + (0x36 * 4)), (uint32_t) SCSetupIBAT4DBAT5);
    kern_write((void*)(KERN_SYSCALL_TBL_2 + (0x36 * 4)), (uint32_t) SCSetupIBAT4DBAT5);
    kern_write((void*)(KERN_SYSCALL_TBL_3 + (0x36 * 4)), (uint32_t) SCSetupIBAT4DBAT5);
    kern_write((void*)(KERN_SYSCALL_TBL_4 + (0x36 * 4)), (uint32_t) SCSetupIBAT4DBAT5);
    kern_write((void*)(KERN_SYSCALL_TBL_5 + (0x36 * 4)), (uint32_t) SCSetupIBAT4DBAT5);

    /* Get coreinit handle and keep it in memory */
    unsigned int coreinit_handle;
    OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle);

    /* Get our memory functions */
    unsigned int* functionPointer;
    void* (*p_memset)(void * dest, unsigned int value, unsigned int bytes);

    OSDynLoad_FindExport(coreinit_handle, 0, "memset", &p_memset);

    void* (*MEMAllocFromDefaultHeapEx)(unsigned int size, unsigned int align);
    void  (*MEMFreeToDefaultHeap)(void *ptr);

    OSDynLoad_FindExport(coreinit_handle, 1, "MEMAllocFromDefaultHeapEx", &functionPointer);
    MEMAllocFromDefaultHeapEx = (void*(*)(unsigned int, unsigned int))*functionPointer;
    OSDynLoad_FindExport(coreinit_handle, 1, "MEMFreeToDefaultHeap", &functionPointer);
    MEMFreeToDefaultHeap = (void (*)(void *))*functionPointer;

    void (*OSExitThread)(int);
    void (*OSYieldThread)(void);
    int (*OSCreateThread)(void *thread, void *entry, int argc, void *args, unsigned int stack, unsigned int stack_size, int priority, unsigned short attr);
    int (*OSResumeThread)(void *thread);
    int (*OSIsThreadTerminated)(void *thread);
    int (*OSJoinThread)(void *thread,int * returnVal);

    OSDynLoad_FindExport(coreinit_handle, 0, "OSCreateThread", &OSCreateThread);
    OSDynLoad_FindExport(coreinit_handle, 0, "OSResumeThread", &OSResumeThread);
    OSDynLoad_FindExport(coreinit_handle, 0, "OSIsThreadTerminated", &OSIsThreadTerminated);
    OSDynLoad_FindExport(coreinit_handle, 0, "OSExitThread", &OSExitThread);
    OSDynLoad_FindExport(coreinit_handle, 0, "OSJoinThread", &OSJoinThread);
    OSDynLoad_FindExport(coreinit_handle, 0, "OSYieldThread", &OSYieldThread);

    //Allocate a stack for the thread
    void *stack = MEMAllocFromDefaultHeapEx(0x4000, 0x20);
    //
    // Create the thread variable
    void *thread = MEMAllocFromDefaultHeapEx(0x1000, 8);
    if(!thread || !stack) {
        OSFatal("Thread memory allocation failed. Exit and re-enter browser.");
    }

    // Create one thread for each core
    for(int i = 0; i<3; i++) {
        p_memset(stack,0,0x4000);
        p_memset(thread,0,0x1000);

        int ret = OSCreateThread(thread, thread_callback, 0, (void*)0, (unsigned int)stack+0x4000, 0x4000, 0, 0x8 + (1 << i));
        if (ret == 0) {
            OSFatal("Failed to create thread. Exit and re-enter browser.");
        }

        // Schedule it for execution
        OSResumeThread(thread);

        // wait until terminated
        while(OSIsThreadTerminated(thread) == 0) {
            OSYieldThread();
        }
    }

    MEMFreeToDefaultHeap(stack);
    MEMFreeToDefaultHeap(thread);
}
