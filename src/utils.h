#ifndef UTILS_H
#define	UTILS_H

#include <stdint.h>
#include "structs.h"

#ifdef __cplusplus
extern "C" {
#endif

void revertMainHook();

void loadFunctionPointers(private_data_t * private_data);

void PrepareScreen(private_data_t *private_data);

void ExitFailure(private_data_t *private_data, const char *failure);

/* Read a 32-bit word with kernel permissions */
uint32_t __attribute__ ((noinline)) kern_read(const void *addr);

/* Write a 32-bit word with kernel permissions */
void __attribute__ ((noinline)) kern_write(void *addr, uint32_t value);


#ifdef __cplusplus
}
#endif

#endif	/* UTILS_H */

