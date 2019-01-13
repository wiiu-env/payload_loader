#ifndef COMMON_H
#define	COMMON_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OSDynLoad_Acquire ((void (*)(char* rpl, unsigned int *handle))0x0102A3B4)
#define OSDynLoad_FindExport ((void (*)(unsigned int handle, int isdata, char *symbol, void *address))0x0102B828)
#define OSFatal ((void (*)(char* msg))0x01031618)
#define __os_snprintf ((int(*)(char* s, int n, const char * format, ... ))0x0102F160)

#define ADDRESS_OSTitle_main_entry_ptr              0x1005E040
#define ADDRESS_main_entry_hook                     0x0101c56c

#define KERN_SYSCALL_TBL_1                          0xFFE84C70 // unknown
#define KERN_SYSCALL_TBL_2                          0xFFE85070 // works with games
#define KERN_SYSCALL_TBL_3                          0xFFE85470 // works with loader
#define KERN_SYSCALL_TBL_4                          0xFFEAAA60 // works with home menu
#define KERN_SYSCALL_TBL_5                          0xFFEAAE60 // works with browser (previously KERN_SYSCALL_TBL)

#define CAFE_OS_SD_PATH                             "/vol/external01"
#define SD_PATH                                     "sd:"
#define WIIU_PATH "/wiiu"

#define EXPORT_DECL(res, func, ...) res (* func)(__VA_ARGS__);
#define OS_FIND_EXPORT(handle, funcName, func) OSDynLoad_FindExport(handle, 0, funcName, &func)

#ifdef __cplusplus
}
#endif

#endif	/* COMMON_H */

