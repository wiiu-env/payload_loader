#ifndef ELF_LOADIUNG_H
#define	COMMON_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "structs.h"

uint32_t LoadAndCopyFile(const char *filepath, private_data_t * private_data);

#ifdef __cplusplus
}
#endif

#endif	/* ELF_LOADIUNG_H */

