// child.c: the child program

#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <string.h> 
#include <sys/wait.h>
#include "common.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

 
int main () { 
	int fd = shm_open(MYSMH, O_RDWR, 0600);
    if (fd == -1){
        perror("Σφαλμα στην shm_open");
        exit(EXIT_FAILURE);
    }
	SharedMemory *shared_msg = (SharedMemory *)mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_msg == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    } 
	close(fd);
	Message m;
	chan_receive(&shared_msg->P1_to_ENC1, &m);
	printf("the message is %s\n", m.text);

	if (munmap(shared_msg, sizeof(SharedMemory)) == -1){
        perror("Σφαλμα στην munmap");
        exit(EXIT_FAILURE);         
    }
	return 0;
}