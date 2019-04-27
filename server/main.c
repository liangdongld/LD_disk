#include "factory.h"
chatRoom room;
int exitFds[2];//创建管道
void cleanup(void *p){
    pthread_mutex_unlock(&((pQue_t)p)->queMutex);
}
void *func(void *p){
    char path[128];
    //getcwd(path,128);
    strcpy(path,"/");
    pFactory pf=(pFactory)p;
    pQue_t pq=&pf->que;
    pthread_cleanup_push(cleanup,pq);//清理函数，传值
    pNode_t pcur;
    pMysql_fun my=pf->my;
    int ret;
    while(1){
        pthread_mutex_lock(&pq->queMutex);
        if(!pq->queSize){
            pthread_cond_wait(&pf->cond,&pq->queMutex);
        }
        ret=queGet(pq,&pcur);
        pthread_mutex_unlock(&pq->queMutex);
        if(0==ret){//0代表取出成功  线程执行任务
            train mes;
            //验证登陆
            int datalen;
            char *id=(char*)calloc(USERID_LEN,1);
            char *passwd=(char*)calloc(PASSWD_LEN,1);
start:
            bzero(&mes,sizeof(train));
            bzero(id,USERID_LEN);
            bzero(passwd,PASSWD_LEN);
            recv_n(pcur->ndSocketfd,(char*)&datalen,4);//接收对方用户名
            recv_n(pcur->ndSocketfd,id,datalen);
            ret=sendsalt(my,pcur->ndSocketfd,id);
            if(ret==0)goto start;
            recv_n(pcur->ndSocketfd,(char*)&datalen,4);//接收对方密文
            recv_n(pcur->ndSocketfd,passwd,datalen);
            ret=signin(my,id,passwd);
            printf("client ip:%s,prot:%d\n",inet_ntoa(pcur->client.sin_addr),ntohs(pcur->client.sin_port));
            if(0==ret){
                printf("登陆失败\n");
                strcpy(mes.buf,"-1");
            }else{
                strcpy(pcur->username,id);
                pcur->userID=getUserID(my,id);
                printf("登陆成功\nUserID : %d\n",pcur->userID);
                char s[]="sigin";
                MYSQL_LOG_INSERT(my,pcur->userID,path,s,NULL,inet_ntoa(pcur->client.sin_addr),ntohs(pcur->client.sin_port));
                strcpy(mes.buf,"0");
            }
            mes.dataLen=1;
            send_n(pcur->ndSocketfd,(char*)&mes,mes.dataLen+4);
            if(ret==0)goto start;
            free(id);
            free(passwd);
            //进入登录界面
            printf("UserName : %d ,current work dir:%s\n",pcur->userID,path);//debug
            char *cmd_all,*cmd,*arg;
            cmd_all=(char*)calloc(128,1);
            MYSQL_SHOW_FILE(pcur->ndSocketfd,my,path);//发送ls -l
            while(1){
                if(1){
                    bzero(cmd_all,128);
                    ret=recv(pcur->ndSocketfd,cmd_all,127,0);
                    if(!strcmp(cmd_all,"\0"))goto end;
                    cmd=getcmd(cmd_all);
                    arg=getarg(cmd_all);
                    MYSQL_LOG_INSERT(my,pcur->userID,path,cmd,arg,inet_ntoa(pcur->client.sin_addr),ntohs(pcur->client.sin_port));
                    printf("cmd:%s  arg:%s\n",cmd,arg);
                    printf("userID:%d current work dir:%s\n",pcur->userID,path);
                    if(!strcmp(cmd,"ls")){//ls命令
                        MYSQL_SHOW_FILE(pcur->ndSocketfd,my,path);//发送ls -l
                    }else if(!(strcmp(cmd,"cd"))){
                        MyChdir(pcur->ndSocketfd,my,path,arg); 
                    }else if(!(strcmp(cmd,"pwd"))){
                    }else if(!strcmp(cmd,"rm")){
                        REMOVE_FILE(my,pcur->ndSocketfd,path,arg);
                    }else if(!strcmp(cmd,"rmdir")){
                        REMOVE_DIR(my,path,pcur->ndSocketfd,arg);
                        send(pcur->ndSocketfd,"s",1,0);
                    }else if(!strcmp(cmd,"gets")){
                        Send_File_ToClient(my,pcur->ndSocketfd,arg,path);
                    }else if(!strcmp(cmd,"puts")){
                        Recv_File_FromClient(my,pcur->ndSocketfd,path);
                    }else if(!strcmp(cmd,"mkdir")){
                        MAKE_DIR(my,pcur->ndSocketfd,path);
                    }else if(!strcmp(cmd,"exit")){
                        goto end;
                    }else if(!strcmp(cmd,"chat")){
                        task(pcur->ndSocketfd,&room,pcur->username);
                    }else if(!strcmp(cmd,"useradd")){
                        if(!strcmp(pcur->username,"root")){
                            User_Add(pcur->ndSocketfd,my);
                        }else{
                            send(pcur->ndSocketfd,"0",1,0);
                        }
                    } else{
                        char *tmp=(char*)calloc(1,1);
                        sprintf(tmp,"0");
                        mes.dataLen=strlen(tmp);
                        strcpy(mes.buf,tmp);
                        send_n(pcur->ndSocketfd,(char*)&mes,mes.dataLen+4);
                        free(tmp);
                        continue;
                    }
                    mes.dataLen=strlen(path);
                    strcpy(mes.buf,path);
                    send_n(pcur->ndSocketfd,(char*)&mes,4+mes.dataLen);
                }else{
                    continue;
                }
            }
        }
end:
        close(pcur->ndSocketfd); 
        free(pcur);
        pcur=NULL;
    }
    pthread_cleanup_pop(1);
}
void sigExitFunc(int signum){
    write(exitFds[1],&signum,sizeof(int));
}
int main(){
    char buf[4][20];
    getconfig(buf);
    fflush(stdout);
    pipe(exitFds);
    if(fork()){
        close(exitFds[0]);
        signal(SIGUSR1,sigExitFunc);
        wait(NULL);
        exit(0);
    }
    close(exitFds[1]);
    Factory f;
    int i;
    int threadNum=atoi(buf[2]);
    int factoryCapacity=atoi(buf[3]);
    pMysql_fun my=(pMysql_fun)calloc(1,sizeof(Mysql_fun));
    bzero(&room,sizeof(room));
    initRoom(&room);
    MYSQL_init(my);
    factoryInit(&f,threadNum,factoryCapacity,func,my);
    factoryStart(&f);
    int socketfd;
    tcpInit(&socketfd,buf[0],buf[1]);
    int newfd;
    struct sockaddr_in client;
    socklen_t addrlen=sizeof(struct sockaddr_in);
    pQue_t pq=&f.que;
    pNode_t pnew;
    //用epoll设置监控事件
    int epfd=epoll_create(1);
    struct epoll_event event,evs[2];
    event.events=EPOLLIN;
    event.data.fd=socketfd;
    epoll_ctl(epfd,EPOLL_CTL_ADD,socketfd,&event);
    event.data.fd=exitFds[0]; 
    epoll_ctl(epfd,EPOLL_CTL_ADD,exitFds[0],&event);
    int fdReadyNum;
    while(1){
        fdReadyNum=epoll_wait(epfd,evs,2,-1);//分别监控客户端连接和结束信号
        for(i=0;i<fdReadyNum;i++){
            if(socketfd==evs[i].data.fd){//当有请求连接时
                bzero(&client,sizeof(struct sockaddr_in));
                newfd=accept(socketfd,(struct sockaddr*)&client,&addrlen);//有请求连接时执行
                pnew=(pNode_t)calloc(1,sizeof(Node_t));
                pnew->ndSocketfd=newfd;
                pnew->client=client;
                pnew->username=(char*)calloc(128,1);
                printf("newfd=%d client ip=%s,port=%d\n",newfd,inet_ntoa(client.sin_addr),ntohs(client.sin_port));
                pthread_mutex_lock(&pq->queMutex);
                queInster(pq,pnew);
                pthread_mutex_unlock(&pq->queMutex);
                pthread_cond_signal(&f.cond);
            }
            if(exitFds[0]==evs[i].data.fd){
                close(socketfd);
                for(i=0;i<f.threadnum;i++){
                    pthread_cancel(f.pthid[i]);
                }
                for(i=0;i<f.threadnum;i++){
                    pthread_join(f.pthid[i],NULL);
                }
                printf("thread_pool exit success\n");
                exit(0);
            }
        }
    }
    return 0;
}
