#include "func.h"
void * task(void *p){
    int fd = *(int*)p;
    cliMes mes;
    while(1){
        bzero(&mes,sizeof(cliMes));
        recv_n(fd,(char*)&mes,sizeof(cliMes));
        printf("\n|-------%s------------------\n",mes.username);
        printf("|%s\n",mes.mes);
        printf("|-----------------------------\n");
    }
}
int sendALL(int fd){
    train mes;
    while(1){
        read(STDIN_FILENO,mes.buf,1024);
        mes.buf[strlen(mes.buf)-1]='\0';
        mes.dataLen=strlen(mes.buf);
        send_n(fd,(char*)&mes,mes.dataLen+4);
        if(!strcmp(mes.buf,"/exit"))break;
        fflush(stdin);
        bzero(&mes,sizeof(mes));
    }
    return 0;
}
