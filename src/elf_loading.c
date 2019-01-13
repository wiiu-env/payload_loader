#include "elf_abi.h"
#include "fs_defs.h"
#include "common.h"
#include "utils.h"
#include "structs.h"
#include "elf_loading.h"

static int32_t LoadFileToMem(private_data_t *private_data, const char *filepath, uint8_t **fileOut, uint32_t * sizeOut) {
    int32_t iFd = -1;
    void *pClient = private_data->MEMAllocFromDefaultHeapEx(FS_CLIENT_SIZE, 4);
    if(!pClient) {
        return 0;
    }

    void *pCmd = private_data->MEMAllocFromDefaultHeapEx(FS_CMD_BLOCK_SIZE, 4);
    if(!pCmd) {
        private_data->MEMFreeToDefaultHeap(pClient);
        return 0;
    }

    int32_t success = 0;
    private_data->FSInit();
    private_data->FSInitCmdBlock(pCmd);
    private_data->FSAddClientEx(pClient, 0, -1);

    do {
        char tempPath[FS_MOUNT_SOURCE_SIZE];
        char mountPath[FS_MAX_MOUNTPATH_SIZE];

        int32_t status = private_data->FSGetMountSource(pClient, pCmd, 0, tempPath, -1);
        if (status != 0) {
            OSFatal("FSGetMountSource failed. Please insert a FAT32 formatted sd card.");
        }
        status = private_data->FSMount(pClient, pCmd, tempPath, mountPath, FS_MAX_MOUNTPATH_SIZE, -1);
        if(status != 0) {
            OSFatal("SD mount failed. Please insert a FAT32 formatted sd card.");
        }

        status = private_data->FSOpenFile(pClient, pCmd, filepath, "r", &iFd, -1);
        if(status != 0) {
            char buf[0x255];
            __os_snprintf(buf,0x254,"FSOpenFile failed. File missing %s",filepath);            
            OSFatal(buf);
        }

        FSStat stat;
        stat.size = 0;

        void *pBuffer = NULL;

        private_data->FSGetStatFile(pClient, pCmd, iFd, &stat, -1);

        if(stat.size > 0)
            pBuffer = private_data->MEMAllocFromDefaultHeapEx((stat.size + 0x3F) & ~0x3F, 0x40);
        else
            OSFatal("ELF file empty.");

        uint32_t done = 0;

        while(done < stat.size) {
            int32_t readBytes = private_data->FSReadFile(pClient, pCmd, pBuffer + done, 1, stat.size - done, iFd, 0, -1);
            if(readBytes <= 0) {
                break;
            }
            done += readBytes;
        }

        if(done != stat.size) {
            private_data->MEMFreeToDefaultHeap(pBuffer);
        } else {
            *fileOut = (uint8_t*)pBuffer;
            *sizeOut = stat.size;
            success = 1;
        }

        private_data->FSCloseFile(pClient, pCmd, iFd, -1);
        private_data->FSUnmount(pClient, pCmd, mountPath, -1);
    } while(0);

    private_data->FSDelClient(pClient);
    private_data->MEMFreeToDefaultHeap(pClient);
    private_data->MEMFreeToDefaultHeap(pCmd);
    return success;
}

static uint32_t load_elf_image_to_mem (private_data_t *private_data, uint8_t *elfstart) {
    Elf32_Ehdr *ehdr;
    Elf32_Phdr *phdrs;
    uint8_t *image;
    int32_t i;

    ehdr = (Elf32_Ehdr *) elfstart;

    if(ehdr->e_phoff == 0 || ehdr->e_phnum == 0) {
        return 0;
    }

    if(ehdr->e_phentsize != sizeof(Elf32_Phdr)) {
        return 0;
    }

    phdrs = (Elf32_Phdr*)(elfstart + ehdr->e_phoff);

    for(i = 0; i < ehdr->e_phnum; i++) {
        if(phdrs[i].p_type != PT_LOAD) {
            continue;
        }

        if(phdrs[i].p_filesz > phdrs[i].p_memsz) {
            continue;
        }

        if(!phdrs[i].p_filesz) {
            continue;
        }

        uint32_t p_paddr = phdrs[i].p_paddr;
        image = (uint8_t *) (elfstart + phdrs[i].p_offset);

        private_data->memcpy ((void *) p_paddr, image, phdrs[i].p_filesz);
        private_data->DCFlushRange((void*)p_paddr, phdrs[i].p_filesz);

        if(phdrs[i].p_flags & PF_X) {
            private_data->ICInvalidateRange ((void *) p_paddr, phdrs[i].p_memsz);
        }
    }

    //! clear BSS
    Elf32_Shdr *shdr = (Elf32_Shdr *) (elfstart + ehdr->e_shoff);
    for(i = 0; i < ehdr->e_shnum; i++) {
        const char *section_name = ((const char*)elfstart) + shdr[ehdr->e_shstrndx].sh_offset + shdr[i].sh_name;
        if(section_name[0] == '.' && section_name[1] == 'b' && section_name[2] == 's' && section_name[3] == 's') {
            private_data->memset((void*)shdr[i].sh_addr, 0, shdr[i].sh_size);
            private_data->DCFlushRange((void*)shdr[i].sh_addr, shdr[i].sh_size);
        } else if(section_name[0] == '.' && section_name[1] == 's' && section_name[2] == 'b' && section_name[3] == 's' && section_name[4] == 's') {
            private_data->memset((void*)shdr[i].sh_addr, 0, shdr[i].sh_size);
            private_data->DCFlushRange((void*)shdr[i].sh_addr, shdr[i].sh_size);
        }
    }

    return ehdr->e_entry;
}

uint32_t LoadAndCopyFile(const char *filepath) {
    private_data_t private_data;

    loadFunctionPointers(&private_data);

    unsigned char *pElfBuffer = NULL;
    unsigned int uiElfSize = 0;

    LoadFileToMem(&private_data, filepath, &pElfBuffer, &uiElfSize);

    if(!pElfBuffer) {
        OSFatal("Failed to load homebrew_launcher.elf");
    }
    unsigned int newEntry = load_elf_image_to_mem(&private_data, pElfBuffer);
    if(newEntry == 0) {
        OSFatal("failed to load .elf");
    }

    private_data.MEMFreeToDefaultHeap(pElfBuffer);
    return newEntry;
}
