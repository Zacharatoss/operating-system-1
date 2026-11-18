#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h> 
#include "common.h"
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

int encNum;   // 1 ή 2, το βλέπουν όλα τα threads


void *forward(void *arg){
    TwoChannels *tc = (TwoChannels *) arg;
    Message m;
    int i = 0;
    while (i == 0){
        chan_receive(tc->in, &m);
        printf("ENC%d: προωθώ '%s'\n", encNum, m.text);
        chan_send(tc->out, &m);
    }
    return NULL;
}

 
int main (int argc, char *argv[]){
    setbuf(stdout, NULL);   // χωρίς buffering: κάθε printf εμφανίζεται αμέσως

    if (argc != 3){
        fprintf(stderr, "Χρήση: ./enc <1|2> <N>\n");
        exit(EXIT_FAILURE);
    }
    encNum = atoi(argv[1]);
    int N = atoi(argv[2]);

    if (!(encNum == 1 || encNum == 2)){
        fprintf(stderr, "Χρήση: ./enc <1|2> <N>\n");
        exit(EXIT_FAILURE);
    }

    


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

    TwoChannels towardsP2;
    TwoChannels towardsP1;


    if (encNum == 1){
        towardsP2.in = &shared_msg->P1_to_ENC1;
        towardsP2.out = &shared_msg->ENC1_to_CHAN;
        towardsP1.in = &shared_msg->CHAN_to_ENC1;
        towardsP1.out = &shared_msg->ENC1_to_P1;
    } else {
        towardsP2.in = &shared_msg->CHAN_to_ENC2;
        towardsP2.out = &shared_msg->ENC2_to_P2;
        towardsP1.in = &shared_msg->P2_to_ENC2;
        towardsP1.out = &shared_msg->ENC2_to_CHAN;        
    }

    pthread_t id1;
    pthread_t id2;
    pthread_create(&id1, NULL, forward, &towardsP2);
    pthread_create(&id2, NULL, forward, &towardsP1);

    pthread_join(id1, NULL);
    pthread_join(id2, NULL);


	if (munmap(shared_msg, sizeof(SharedMemory)) == -1){
        perror("Σφαλμα στην munmap");
        exit(EXIT_FAILURE);         
    }
	return 0;
}