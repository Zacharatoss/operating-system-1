#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h> 
#include "common.h"
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>


void *receiver(void *arg){
    Channel *in = arg;          // το arg είναι κατευθείαν ο Channel*
    Message m;
    while (1){
        chan_receive(in, &m);
        printf("[εισερχόμενο]: %s\n", m.text);
    }
    return NULL;
}

 
int main (int argc, char *argv[]){
    setbuf(stdout, NULL);   // χωρίς buffering: κάθε printf εμφανίζεται αμέσως

    if (argc != 2){
        fprintf(stderr, "Χρήση: ./proc <1|2>\n");
        exit(EXIT_FAILURE);
    }
    int num = atoi(argv[1]);

    if (!(num == 1 || num == 2)){
        fprintf(stderr, "Χρήση: ./proc <1|2>\n");
        exit(EXIT_FAILURE);
    }


	int fd = shm_open(MYSMH, O_RDWR, 0600);
    if (fd == -1){
        perror("Σφαλμα στην shm_open, εκκινήστε πρώτα την ./start");
        exit(EXIT_FAILURE);
    }
	SharedMemory *shared_msg = (SharedMemory *)mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_msg == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    } 
	close(fd);

    Channel *out;
    Channel *in;


    if (num == 1){
        out = &shared_msg->P1_to_ENC1;
        in = &shared_msg->ENC1_to_P1;
    } else {
        out = &shared_msg->P2_to_ENC2;
        in = &shared_msg->ENC2_to_P2;
    }

    pthread_t rid;

    pthread_create(&rid, NULL, receiver, in);

    char line[256];
    while (fgets(line, sizeof(line), stdin) != NULL){
        line[strcspn(line, "\n")] = '\0';
        Message m;
        m.type = MSG_DATA;
        strncpy(m.text, line, sizeof(m.text));
        m.text[sizeof(m.text) - 1] = '\0';
        chan_send(out, &m);
    }

	if (munmap(shared_msg, sizeof(SharedMemory)) == -1){
        perror("Σφαλμα στην munmap");
        exit(EXIT_FAILURE);         
    }
	return 0;
}