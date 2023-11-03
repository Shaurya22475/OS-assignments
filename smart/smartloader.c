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

void sigsegv_handler(int signo, siginfo_t *info, void *context){
    if (signo == SIGSEGV){
        Elf32_Addr faulting_address = (Elf32_Addr)info->si_addr;
        for (int i = 0; i < ehdr.e_phnum; i++) {
        if (read(fd, &phdr, sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)) {
            perror("read");
            printf("hey\n");
            close(fd);
            exit(1);
        }
        printf("Counter:: %d %d\n",i,phdr.p_type);
        if (faulting_address >= phdr.p_vaddr && faulting_address <= phdr.p_vaddr + phdr.p_memsz){
            printf("Faulting address: %u, segment address: %u %u\n",faulting_address,phdr.p_vaddr,phdr.p_vaddr+phdr.p_memsz);

            size_t required_size = phdr.p_memsz;
            size_t page_size = 4096;
            size_t allocation_size = ((required_size + page_size - 1) / page_size) * page_size;
            void* segment_address = mmap((void * )phdr.p_vaddr, allocation_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, phdr.p_offset);
            printf("Segment address allocated  : %u",(int)segment_address);
             if (segment_address == MAP_FAILED) {
                    perror("mmap");
                    printf("hey\n");
                    close(fd);
                    exit(1);
                }
            printf("Alloc done;;\n");
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
    unsigned int curr= lseek(fd,0,SEEK_CUR);

    Elf32_Addr entry_point = ehdr.e_entry;
    printf("Number of program headers: %d",ehdr.e_phnum);
    int (*_start)()=(int (*)())(entry_point);
    printf("seg..%u",phdr.p_vaddr);
    int result = _start();
    // void* segment_address = mmap(NULL, phdr.p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, phdr.p_offset);
    // if (segment_address == MAP_FAILED) {
    //     perror("mmap");
    //     close(fd);
    //     exit(1);  
    // }

    printf("User _start return value = %d\n", result);

        // if (munmap(segment_address, phdr.p_memsz) == -1) {
        //     perror("munmap");
        //}

    close(fd);
    
}
int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <ELF Executable> \n", argv[0]);
        exit(1);
    }
    printf("%d",getpid());
    load_and_run_elf(argv);
    loader_cleanup();
    return 0;
}
