#include "chat.h"
#include "factory.h"
int initRoom(pchatRoom room){
    bzero(room,sizeof(chatRoom));
    room->onLinenum=0;
    room->Max=-1;
    pthread_mutex_init(&room->door,NULL);
    return 1;
}
//加入聊天室
int joinRoom(pchatRoom room,char * username,int fd){
    cliChat me;
    strcpy(me.username,username);
    me.cliFd=fd;
    if(room->onLinenum>=254){
        return -1;
    }
    cliMes mes;
    int i;
    pthread_mutex_lock(&room->door);
    room->onLinenum++;
    for(i=0;i<254;i++){
        if(room->cli[i].flag==0){
            strcpy(room->cli[i].username,me.username);
            room->cli[i].cliFd=me.cliFd;
            room->cli[i].flag=1;
            strcpy(mes.username,"root");
            sprintf(mes.mes,"---%s join this room---",username);
            sendMsgAll(room,mes,me.cliFd);
            if(i>room->Max)room->Max=i;
            break;
        }
    }
    pthread_mutex_unlock(&room->door);
    return 1;
}
//离开聊天室
int exitRoom(pchatRoom room,cliChat me){
    if(room->onLinenum==0){
        printf("--------Error!\n");
        return -1;
    }
    pthread_mutex_lock(&room->door);
    room->onLinenum--;
    pthread_mutex_unlock(&room->door);
    printf("online num:%d\n",room->onLinenum);
    if(room->onLinenum==0){
        room->Max=-1;
        printf("onbody in room!\n");
        return 1;
    }
    int i;
    for(i=0;i<room->Max;i++){
        if(room->cli[i].cliFd==me.cliFd){
            room->cli[i].flag=0;
            if(i==room->Max){
                int q;
                for(q=0;q<room->Max;q++){
                    if(room->cli[q].flag==1)
                        room->Max=q;
                }
            }
            break;
        }
    }
    return 1;
}
//发送消息
int sendMsgAll(pchatRoom room,cliMes mes,int fd){
    int i;
    int flag=0;
    if(!strcmp(mes.mes,"/exit")){
        flag=1;
        sprintf(mes.mes,"---%s exit this room---",mes.username);
        strcpy(mes.username,"root");
    }
    for(i=0;i<=room->Max;i++){
        if(room->cli[i].flag==1&&room->cli[i].cliFd!=fd&&room->cli[i].cliFd>2&&room->onLinenum!=1){
            //printf("%d      %d send to %d\n",i,fd,room->cli[i].cliFd);
            send_n(room->cli[i].cliFd,(char*)&mes,sizeof(cliMes));
        }
        if(flag==1){
            cliChat me;
            me.cliFd=fd;
            strcpy(me.username,mes.username);
            exitRoom(room,me);
            return 0;
        }
    }
    return 1;
}
void task(int fd,pchatRoom room,char * username){
    cliMes cli;
    char buf[1024];
    int datalen;
    joinRoom(room,username,fd);
    printf("online num: %d\n",room->onLinenum);
    while(1){
        bzero(&cli,sizeof(cliMes));
        bzero(buf,1024);
        recv_n(fd,(char*)&datalen,4);
        recv_n(fd,buf,datalen);
        strcpy(cli.username,username);
        strcpy(cli.mes,buf);
        int ret = sendMsgAll(room,cli,fd);
        if(ret == 0)break;
    }
}
