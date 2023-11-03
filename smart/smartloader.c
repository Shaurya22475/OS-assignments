#include "loader.h"

Elf32_Ehdr ehdr;
Elf32_Phdr phdr;
int fd;
int page_faults=0;
int total_internal_segmentation;
/*
 * release memory and other cleanups
 */
void loader_cleanup() {
    // Add any necessary cleanup code here
}

/*
 * Load and run the ELF executable file
 */

void sigsegv_handler(int signo, siginfo_t *info, void *context){

    if (signo == SIGSEGV){
        Elf32_Addr faulting_address = (Elf32_Addr)info->si_addr;
        page_faults++;
        for (int i = 0; i < ehdr.e_phnum; i++) {
        if (read(fd, &phdr, sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)) {
            perror("read");
            printf("hey\n");
            close(fd);
            exit(1);
        }
        int internal_segmentation=0;
        if (faulting_address >= phdr.p_vaddr && faulting_address <= phdr.p_vaddr + phdr.p_memsz){
            printf("Faulting address: %u, segment address: %u %u\n",faulting_address,phdr.p_vaddr,phdr.p_vaddr+phdr.p_memsz);
            size_t page_size = 4096;
            void* segment_address = mmap((void * )faulting_address, page_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE|MAP_FIXED, fd, phdr.p_offset);
            if (phdr.p_vaddr+phdr.p_memsz>= faulting_address+page_size){
                internal_segmentation=0;
            }
            else{
                internal_segmentation=(int)(faulting_address+page_size)-((int)(phdr.p_vaddr+phdr.p_memsz));
            }
             if (segment_address == MAP_FAILED) {
                    perror("mmap");
                    printf("hey\n");
                    close(fd);
                    exit(1);
                }
            total_internal_segmentation+=internal_segmentation;
            break;
        }
      }
      lseek(fd,ehdr.e_phoff,SEEK_SET);
      printf("EXITINIG OUT AFTER ALLOCATING:\n");
   } 
}

void load_and_run_elf(char** exe) {
     
    struct sigaction sa;
    sa.sa_sigaction = sigsegv_handler;
    sa.sa_flags = SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);
    fd = open(exe[1], O_RDONLY);

    if (fd == -1) {
        perror("open");
        exit(1); 
    }

    read(fd, &ehdr, sizeof(Elf32_Ehdr));

    unsigned int program_header_offset = ehdr.e_phoff;

    Elf32_Addr entry_point = ehdr.e_entry;
    printf("Number of program headers: %d",ehdr.e_phnum);
    int (*_start)()=(int (*)())(entry_point);
    printf("seg..%u",phdr.p_vaddr);
    int result = _start();
    printf("User _start return value = %d\n", result);
    close(fd);
}
int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <ELF Executable> \n", argv[0]);
        exit(1);
    }
    printf("%d",getpid());
    load_and_run_elf(argv);
    printf("Total number of page faults: %d\n",page_faults);
    printf("Total number of page allocations carried out : %d\n",page_faults);
    printf("Total internal segmentation: %f KB\n",total_internal_segmentation/1024.0);
    loader_cleanup();
    return 0;
}
