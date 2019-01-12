#ifndef ELF_LOADIUNG_H
#define	COMMON_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t LoadAndCopyFile(const char *filepath);

#ifdef __cplusplus
}
#endif

#endif	/* ELF_LOADIUNG_H */

