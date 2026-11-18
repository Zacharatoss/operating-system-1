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

void init(Channel *cha){
	int ans = sem_init(&cha->empty, 1, 1);
	if (ans == -1){
		perror("Σφαλμα στην sem_init");
        exit(EXIT_FAILURE);
	}
	int ans1 = sem_init(&cha->full, 1, 0);
	if (ans1 == -1){
		perror("Σφαλμα στην sem_init");
        exit(EXIT_FAILURE);
	}
	return;
}

void destroy(Channel *cha){
	int ans = sem_destroy(&cha->empty);
	if (ans == -1){
		perror("Σφαλμα στην sem_destroy");
        exit(EXIT_FAILURE);
	}
	int ans1 = sem_destroy(&cha->full);
	if (ans1 == -1){
		perror("Σφαλμα στην sem_destroy");
        exit(EXIT_FAILURE);
	}
	return;	
}

pid_t spawn(char *args[]){
	pid_t pid; 
	pid = fork (); 
	if (pid == -1){
	    perror("Σφάλμα στη fork");
        exit(EXIT_FAILURE);	
	}
	if (pid == 0) { // I am the child 
		execvp (args[0], args); 
        perror("Σφάλμα στη execvp");
        exit(EXIT_FAILURE);
	}
	return pid; 	
}


int main (int argc, char **argv) { 
	int opt;
	char *p_str = NULL;
	char *n_str = NULL;

	while ((opt = getopt(argc,argv, "p:n:")) != -1){
		switch (opt) {
            case 'p':
                p_str = optarg; // Το optarg δείχνει στην τιμή μετά το -p
                break;
            case 'n':
                n_str = optarg; // Το optarg δείχνει στην τιμή μετά το -n
                break;
            case '?':
                // Η getopt βρήκε άγνωστο flag ή flag χωρίς τιμή
                fprintf(stderr, "Χρήση: %s -p <πιθανότητα> -n <προσπάθειες>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
	}

	if (p_str == NULL || n_str == NULL) {
        fprintf(stderr, "Σφάλμα: Λείπουν παράμετροι.\n");
        fprintf(stderr, "Χρήση: %s -p <πιθανότητα> -n <προσπάθειες>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
	char *point = NULL;
	double p = strtod(p_str, &point);
	if ( point == p_str || *point != '\0'){
		fprintf(stderr, "Λάθος τιμές στην παράμετρο p. Δεκτές τίμες από 0.0-1.0\n");
		exit(EXIT_FAILURE);
	}
	if ( !(p >= 0.0 && p <= 1.0)){
		fprintf(stderr, "Λάθος τιμές στην παράμετρο p. Δεκτές τίμες από 0.0-1.0\n");
		exit(EXIT_FAILURE);
	}

	char *point1 = NULL;
	int n = strtol(n_str, &point1, 10);
	if ( point1 == n_str || *point1 != '\0'){
		fprintf(stderr, "Λάθος τιμές στην παράμετρο n. Δεκτές τίμες μεγαλυτερες απο 0\n");
		exit(EXIT_FAILURE);
	}
	if ( n <= 0){
		fprintf(stderr, "Λάθος τιμές στην παράμετρο n. Δεκτές τίμες μεγαλυτερες απο 0\n");
		exit(EXIT_FAILURE);
	}





	shm_unlink(MYSMH);
	int fd = shm_open(MYSMH, O_CREAT | O_RDWR | O_EXCL, 0600);
    if (fd == -1){
        perror("Σφαλμα στην shm_open");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(fd, sizeof(SharedMemory)) == -1){
        perror("Σφαλμα στην ftruncate");
        if (shm_unlink(MYSMH) == -1){
            perror("Σφαλμα στην unlink");
            exit(EXIT_FAILURE);         
        }
        exit(EXIT_FAILURE);        
    }
    SharedMemory *shared_msg = (SharedMemory *)mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_msg == MAP_FAILED) {
        perror("mmap failed");
        shm_unlink(MYSMH);
        exit(EXIT_FAILURE);
    }
    close(fd);

	init(&shared_msg->P1_to_ENC1);
	init(&shared_msg->P2_to_ENC2);
	init(&shared_msg->ENC1_to_CHAN);
	init(&shared_msg->ENC2_to_CHAN);
	init(&shared_msg->CHAN_to_ENC2);
	init(&shared_msg->CHAN_to_ENC1);
	init(&shared_msg->ENC1_to_P1);
	init(&shared_msg->ENC2_to_P2);
 



	// Ορίσματα για κάθε παιδί
	char *enc1_args[] = {"./enc", "1", n_str, NULL};
	char *enc2_args[] = {"./enc", "2", n_str, NULL};
	char *chan_args[] = {"./chan", p_str, NULL};

	// Δημιουργία παιδιών
	pid_t pid_enc1 = spawn(enc1_args);
	pid_t pid_enc2 = spawn(enc2_args);
	pid_t pid_chan = spawn(chan_args);

	printf("start: ξεκίνησαν ENC1, ENC2, CHAN\n");

	// Αναμονή και των τριών
	waitpid(pid_enc1, NULL, 0);
	waitpid(pid_enc2, NULL, 0);
	waitpid(pid_chan, NULL, 0);

	

	destroy(&shared_msg->P1_to_ENC1);
	destroy(&shared_msg->P2_to_ENC2);
	destroy(&shared_msg->ENC1_to_CHAN);
	destroy(&shared_msg->ENC2_to_CHAN);
	destroy(&shared_msg->CHAN_to_ENC2);
	destroy(&shared_msg->CHAN_to_ENC1);
	destroy(&shared_msg->ENC1_to_P1);
	destroy(&shared_msg->ENC2_to_P2);



	if (munmap(shared_msg, sizeof(SharedMemory)) == -1){
        perror("Σφαλμα στην munmap");
        exit(EXIT_FAILURE);         
    }
    if (shm_unlink(MYSMH) == -1){
        perror("Σφαλμα στην unlink");
        exit(EXIT_FAILURE);         
    }
}