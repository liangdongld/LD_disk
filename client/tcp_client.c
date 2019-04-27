#include "func.h"
int main(){
    char con[3][20];
    getconfig(con); 
    fflush(stdout); 
    //初始化
    int socketfd=socket(AF_INET,SOCK_STREAM,0);
    if(-1==socketfd){
        perror("socket");
        return -1;
    }
    struct sockaddr_in ser;
    memset(&ser,0,sizeof(ser));
    ser.sin_family=AF_INET;
    ser.sin_port=htons(atoi(con[1]));
    ser.sin_addr.s_addr=inet_addr(con[0]);
    int ret;
    ret=connect(socketfd,(struct sockaddr*)&ser,sizeof(struct sockaddr));
    if(-1==ret){
        perror("connect");
        return -1;
    }
    char *buf=(char*)calloc(4096,sizeof(char));
    int dataLen;  
    char *cmd_all=(char*)calloc(128,sizeof(char));
    char *cmd=(char*)calloc(10,sizeof(char)); 
    char *arg=(char*)calloc(121,sizeof(char));
    char *id=(char*)calloc(512,1);
    char *passwd;
    train mes; 
    bzero(id,512);
start:
    write(STDOUT_FILENO,"Please input userid:",strlen("Please input userid:"));
    read(STDIN_FILENO,id,512);
    id[strlen(id)-1]=0;
    strcpy(mes.buf,id);
    mes.dataLen=strlen(id);
    send_n(socketfd,(char*)&mes,mes.dataLen+4);
    bzero(&mes,sizeof(mes)); 
    recv_n(socketfd,(char*)&dataLen,sizeof(int));//接收盐值
    recv_n(socketfd,buf,dataLen);
    if(!strcmp(buf,"1")){
        printf("User name is not found!\n");
        goto start;
    }
    passwd=getpass("Please input password:");
    char * pwd = crypt(passwd,buf);
    strcpy(mes.buf,pwd);
    mes.dataLen=strlen(mes.buf);
    send_n(socketfd,(char*)&mes,mes.dataLen+4);
    recv_n(socketfd,(char*)&dataLen,sizeof(int));
    recv_n(socketfd,buf,dataLen);
    if(!strcmp(buf,"1")){
        goto start;
        printf("FALSE\n");
    }
    //接收ls-l      
    system("clear");
    myFile file;
    Recv_Show_File(socketfd,file);
    while(1){
        bzero(buf,4096); 
        bzero(cmd_all,128);
        printf("please input cmd:");
        fflush(stdout);
        read(STDIN_FILENO,cmd_all,128);
        cmd_all[strlen(cmd_all)-1]=0;
        cmd=getcmd(cmd_all);
        send(socketfd,cmd_all,127,0);
        system("clear");
        if(!strcmp(cmd,"gets")){
            int i = Down_Load_File(socketfd);
            if(0==i)printf("Gets File Success!\n");
        }else if(!strcmp(cmd,"puts")){
            arg=getarg(cmd_all);
            Up_Load_File(socketfd,arg);
        }else if(!strcmp(cmd,"ls")){
            Recv_Show_File(socketfd,file);
        }else if(!strcmp(cmd,"cd")){
            arg=getarg(cmd_all);
            recv(socketfd,buf,1,0);
            if(!strcmp(buf,"t")){
                printf("chdir ok!\n");
            }else if(!strcmp(buf,"f")){
                printf("error\n");
            }
        }else if(!strcmp(cmd,"mkdir")){
            arg=getarg(cmd_all);
            int i =  MAKE_DIR(socketfd,arg);
            if(i==1)printf("Operate success!\n");
        }else if(!strcmp(cmd,"rm")){
            RM_FILE(socketfd);
        }else if(!strcmp(cmd,"rmdir")){
            int i = RM_DIR(socketfd);
            if(i==0)printf("Remove Dir OK\n");
        }else if(!strcmp(cmd,"exit")){
            goto end;
        }else if(!strcmp(cmd,"chat")){
           pthread_t th;
           pthread_create(&th,NULL,task,(void*)&socketfd);
           sendALL(socketfd);
           pthread_cancel(th);
        }else if(!strcmp(cmd,"useradd")){
            userADD(socketfd);
        }
        recv_n(socketfd,(char*)&dataLen,sizeof(int));
        recv_n(socketfd,buf,dataLen);
        if(!strcmp(buf,"0")){
            printf("Please input ture cmd!\n");
            continue;
        }else{
            printf("Current work dir : %s\n",buf);
        }
    }
end:
    printf("bye bye\n");
    free(buf);
    free(cmd_all);
    free(arg);
    close(socketfd);
    return 0;
}
