#include "common.h"



void chan_send(Channel *ch, Message *m){
    sem_wait(&ch->empty);
    ch->msg = *m;
    sem_post(&ch->full);
}

void chan_receive(Channel *ch, Message *m){
    sem_wait(&ch->full);
    *m = ch->msg;
	sem_post(&ch->empty);
}
