#ifndef STRUCTS_H
#define	STRUCTS_H

#include <stdint.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _private_data_t {
    EXPORT_DECL(void *, MEMAllocFromDefaultHeapEx,int size, int align);
    EXPORT_DECL(void, MEMFreeToDefaultHeap,void *ptr);

    EXPORT_DECL(void*, memcpy, void *p1, const void *p2, unsigned int s);
    EXPORT_DECL(void*, memset, void *p1, int val, unsigned int s);

    EXPORT_DECL(unsigned int, OSEffectiveToPhysical, const void*);
    EXPORT_DECL(void, exit, int);
    EXPORT_DECL(void, DCInvalidateRange, const void *addr, unsigned int length);
    EXPORT_DECL(void, DCFlushRange, const void *addr, unsigned int length);
    EXPORT_DECL(void, ICInvalidateRange, const void *addr, unsigned int length);

    EXPORT_DECL(int, FSInit, void);
    EXPORT_DECL(int, FSAddClientEx, void *pClient, int unk_zero_param, int errHandling);
    EXPORT_DECL(int, FSDelClient, void *pClient);
    EXPORT_DECL(void, FSInitCmdBlock, void *pCmd);
    EXPORT_DECL(int, FSGetMountSource, void *pClient, void *pCmd, int type, void *source, int errHandling);
    EXPORT_DECL(int, FSMount, void *pClient, void *pCmd, void *source, const char *target, uint32_t bytes, int errHandling);
    EXPORT_DECL(int, FSUnmount, void *pClient, void *pCmd, const char *target, int errHandling);
    EXPORT_DECL(int, FSOpenFile, void *pClient, void *pCmd, const char *path, const char *mode, int *fd, int errHandling);
    EXPORT_DECL(int, FSGetStatFile, void *pClient, void *pCmd, int fd, void *buffer, int error);
    EXPORT_DECL(int, FSReadFile, void *pClient, void *pCmd, void *buffer, int size, int count, int fd, int flag, int errHandling);
    EXPORT_DECL(int, FSCloseFile, void *pClient, void *pCmd, int fd, int errHandling);

    EXPORT_DECL(int, SYSRelaunchTitle, int argc, char** argv);
} private_data_t;

#ifdef __cplusplus
}
#endif

#endif	/* STRUCTS_H */

