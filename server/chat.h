#ifndef __CHAT_H__
#define __CHAT_H__
#include "head.h"
typedef struct {
    char username[20];
    int cliFd;
    int flag;
}cliChat;
typedef struct{
    char username[20];
    char mes[1024];
}cliMes;
typedef struct{
    cliChat cli[254];
    int onLinenum;
    int Max;
    pthread_mutex_t door;
}chatRoom,*pchatRoom;
int initRoom(pchatRoom room);
int joinRoom(pchatRoom room,char * username,int fd);
int exitRoom(pchatRoom room,cliChat me);
int sendMsgAll(pchatRoom room,cliMes mes,int fd);
void task(int fd,pchatRoom room,char*);
#endif
