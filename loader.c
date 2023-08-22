#include "loader.h"

Elf32_Ehdr ehdr;
Elf32_Phdr phdr;
int fd;

/*
 * release memory and other cleanups
 */
void loader_cleanup() {
    // Add any necessary cleanup code here
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** exe) {
    fd = open(exe[1], O_RDONLY);

    if (fd == -1) {
        perror("open");
        exit(1); 
    }

    if (read(fd, &ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("read");
        close(fd);
        exit(1);
    }

    unsigned int program_header_offset = ehdr.e_phoff;
    unsigned int curr= lseek(fd,0,SEEK_CUR);
    printf("File pointer is at this addess after reading header: %u \n",curr);
    printf("This is the position of the program header:%u \n",program_header_offset);

    Elf32_Addr entry_point = ehdr.e_entry;

    for (int i = 0; i < ehdr.e_phnum; i++) {
        if (read(fd, &phdr, sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)) {
            perror("read");
            close(fd);
            exit(1);
        }

        if (phdr.p_type == PT_LOAD) {
            if (entry_point >= phdr.p_vaddr && entry_point <= phdr.p_vaddr + phdr.p_memsz) {
                void* segment_address = mmap(NULL, phdr.p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, phdr.p_offset);
                printf("the address of the memory location where we are loading the segment is: %ld \n",(long)segment_address);
                if (segment_address == MAP_FAILED) {
                    perror("mmap");
                    close(fd);
                    exit(1);  
                }

                int (*_start)() = (int (*)())(segment_address + (entry_point - phdr.p_vaddr));
                int result = _start();
                printf("User _start return value = %d\n", result);

                if (munmap(segment_address, phdr.p_memsz) == -1) {
                    perror("munmap");
                }

                break;
            }
        }
    }

    close(fd);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <ELF Executable> \n", argv[0]);
        exit(1);
    }
    load_and_run_elf(argv);
    loader_cleanup();
    return 0;
}
