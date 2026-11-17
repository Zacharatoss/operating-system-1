#include "common.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

int main(){
    int fd = shm_open(MYSMH, O_CREAT | O_RDWR | O_EXCL, 0666);
    if (fd == -1){
        perror("Σφαλμα στην shm_open");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(fd, sizeof(Message)) == -1){
        perror("Σφαλμα στην ftruncate");
        if (shm_unlink(MYSMH) == -1){
            perror("Σφαλμα στην unlink");
            exit(EXIT_FAILURE);         
        }
        exit(EXIT_FAILURE);        
    }
    Message *shared_msg = (Message *)mmap(NULL, sizeof(Message), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_msg == MAP_FAILED) {
        perror("mmap failed");
        shm_unlink(MYSMH);
        exit(EXIT_FAILURE);
    }
    close(fd);
    strncpy(shared_msg->text, "test", sizeof(shared_msg->text));
    shared_msg->text[sizeof(shared_msg->text) - 1] = '\0';

    printf("γραψε %s\n", shared_msg->text);

    if (munmap(shared_msg, sizeof(Message)) == -1){
        perror("Σφαλμα στην munmap");
        exit(EXIT_FAILURE);         
    }
    if (shm_unlink(MYSMH) == -1){
        perror("Σφαλμα στην unlink");
        exit(EXIT_FAILURE);         
    }
    return 0;
}