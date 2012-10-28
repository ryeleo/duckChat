#include "duckchat.h"
#include "channel.h"

int nextChannel(channel *cur, channel *next){
    if (cur->next != NULL){
        next = &(cur->next);
        return 0;
    }
    return -1;
}

int rmChannel(channel *cur, channel *removed){
    channel
        *iter;

    nextChannel(cur, iter);

    while (iter->next != cur){
        nextChannel(iter, iter);
    }

    iter->next = cur->next;
    free(cur);
    cur = NULL;
    
}

int insChannel(channel *ins){
    channel ins_m = malloc(sizeof(channel));
    strncpy(ins_m->name, ins->, CHANNEL_MAX);

}
