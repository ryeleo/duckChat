#ifndef CHANNEL_H
#define CHANNEL_H

#include "duchchat.h"

typedef struct channel{
    char name[CHANNEL_MAX];
    channel *next;
};

/*
 * get whatever channel is at
 */
int nextChannel();

int peekNextChannel();

int rmChannel();

int insChannel(); 

#endif
