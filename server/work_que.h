#ifndef __WORKQUE_H___
#define __WORKQUE_H___
#include "head.h"
typedef struct tag_node{//队列节点
    int ndSocketfd;
    char *username;
    int userID;
    struct sockaddr_in client;
    struct tag_node *ndNext;
}Node_t,*pNode_t;
typedef struct{
    pNode_t queHead,queTail;
    int queCapacity;
    int queSize;
    pthread_mutex_t queMutex;
}Que_t,*pQue_t;
void queInit(pQue_t,int Capacity);
void queInster(pQue_t,pNode_t inset);
int queGet(pQue_t,pNode_t*);
#endif
