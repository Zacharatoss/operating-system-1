#ifndef MY_HEADER_H   // 1. Ελέγχει αν ΔΕΝ έχει οριστεί το "MY_HEADER_H"
#define MY_HEADER_H

#include <semaphore.h>
#include <unistd.h>
#define MYSMH "/shm_sdi2400048"
typedef enum { MSG_DATA, MSG_ACK, MSG_NACK, MSG_TERM } MsgType;

typedef struct{
    MsgType type;   // τι ειδους μηνυμα ειναι 
    char text[256];   // το πραγματικο κειμενο
    unsigned int checksum;  // αποτελεσμα της συναρτησης hash
} Message;


typedef struct {
    Message msg;
    sem_t empty;
    sem_t full;   
} Channel;

typedef struct {
    Channel P1_to_ENC1;
    Channel P2_to_ENC2;   
    Channel ENC1_to_CHAN;
    Channel ENC2_to_CHAN;
    Channel CHAN_to_ENC2;
    Channel CHAN_to_ENC1;
    Channel ENC1_to_P1;
    Channel ENC2_to_P2;
} SharedMemory;



void chan_receive(Channel *ch, Message *m);
void chan_send(Channel *ch, Message *m);

#endif