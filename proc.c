#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h> 
#include "file.h"
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>



// Συνάρτηση που γράφει στα τερματικά των χρηστών που ήρθαν
void *receiver(void *arg){
    Channel *in = arg;     // το arg είναι κατευθείαν ο Channel*
    Message m;
    // Βρόχος λήψης: το νήμα μπλοκάρει στο chan_receive (sem_wait) μέχρι να έρθει μήνυμα, χωρίς busy-waiting
    while (1){
        // Λαμβάνει το μήνυμα από την enc
        chan_receive(in, &m);
        // Έλεγχος για τερματισμό
        if (m.type == MSG_TERM){
            printf("Ο συνομιλητής έφυγε\n");
            exit(EXIT_SUCCESS);
        } else {
            printf("[εισερχόμενο]: %s\n", m.text);
        }
    }
    return NULL;
}

 
int main (int argc, char *argv[]){
    setbuf(stdout, NULL);   // χωρίς buffering: κάθε printf εμφανίζεται αμέσως

    // Έλεγχος των ορισμάτων
    if (argc != 2){
        fprintf(stderr, "Χρήση: ./proc <1|2>\n");
        exit(EXIT_FAILURE);
    }
    // Μετατροπή string σε int 
    int num = atoi(argv[1]);

    if (!(num == 1 || num == 2)){
        fprintf(stderr, "Χρήση: ./proc <1|2>\n");
        exit(EXIT_FAILURE);
    }

    // Άνοιγμα της υπάρχουσας shm (τη δημιούργησε το start) για ανάγνωση και εγγραφή
	int fd = shm_open(MYSMH, O_RDWR, 0600);
    if (fd == -1){
        perror("Σφαλμα στην shm_open, εκκινήστε πρώτα την ./start");
        exit(EXIT_FAILURE);
    }

	// Αντιστοίχιση της shm στον χώρο διευθύνσεων της διεργασίας
	SharedMemory *shared_msg = (SharedMemory *)mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_msg == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    } 

	close(fd);

    Channel *out;
    Channel *in;

    // Επιλογή καναλιών του P1 και P2 αντίστοιχα
    if (num == 1){
        out = &shared_msg->P1_to_ENC1;
        in = &shared_msg->ENC1_to_P1;
    } else {
        out = &shared_msg->P2_to_ENC2;
        in = &shared_msg->ENC2_to_P2;
    }

    // Δημιουργεία id για το νήμα
    pthread_t rid;

    // Δημηουργεια του νηματος ώστε αποστολή και λήψη να γίνονται παράλληλα
    if (pthread_create(&rid, NULL, receiver, in) != 0){
        fprintf(stderr, "Σφάλμα στην pthread_create\n");
        exit(EXIT_FAILURE);
    }

    printf("Γράψε μηνύματα. Για τερματισμό γράψε: Αντίο\n");
    char line[256];
    // Διάβασμα μηνυμάτων από το πληκτρολόγιο (stdin), γραμμή-γραμμή
    while (fgets(line, sizeof(line), stdin) != NULL){
        //  Η fgets κρατάει το '\n' στο τέλος, οπότε το αντικαθιστούμε με '\0'
        line[strcspn(line, "\n")] = '\0';
        // Δημιουργεία της δομης του μηνύματος 
        Message m;
        m.type = MSG_DATA;
        // Εισαγωγή στο πεδίο text το μήνυμα του χρήστη
        strncpy(m.text, line, sizeof(m.text));
        // Προσθέτουμε '\0'
        m.text[sizeof(m.text) - 1] = '\0';
        // Σε περίπτωση που είναι το "Αντίο" στέλνουμε πρώτα TERM ώστε να ενημερωθούν οι υπόλοιποι, και μετά τερματίζουμε
        if (strcmp(m.text, "Αντίο") == 0){
            m.type = MSG_TERM;
            chan_send(out, &m);
            exit(EXIT_SUCCESS);
        }
        // Στέλνει στην enc το μήνυμα
        chan_send(out, &m);
    }
    // Τερματισμός σε περίπτωση EOF
    Message t;
    t.type = MSG_TERM;
    t.text[0] = '\0';
    chan_send(out, &t);
    exit(EXIT_SUCCESS);
}