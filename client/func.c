#include "func.h"
int tranFile(int fd,char *file) {
    train t;
    strcpy(t.buf, file);
    t.dataLen = strlen(file);
    send_n(fd, (char*)&t, t.dataLen+4);
    int newfd = open(file, O_RDONLY);//打开需要传输的文件
    if(-1==newfd){
        perror("open");
        return -1;
    }
    struct stat statbuf;
    fstat(newfd, &statbuf);
    memcpy(t.buf,&statbuf.st_size, sizeof(statbuf.st_size));//赋值长度
    t.dataLen = sizeof(statbuf.st_size);
    send_n(fd, (char*)&t, t.dataLen + 4);
    time_t start=time(NULL),now;
    now=start; 
    int fileLoadSize=0;
    while((t.dataLen = read(newfd, t.buf, sizeof(t.buf))))
    {
        if(t.dataLen>0)
        {
            send_n(fd,(char*)&t,t.dataLen+4);
            fileLoadSize+=t.dataLen;
            now=time(NULL);
            if(now-start>0)
            {
                printf("%5.2f%s\r",(double)fileLoadSize/statbuf.st_size*100,"%");
                fflush(stdout);
                start=now;
            }
        }else{
            printf("100.00%s\n","%");
            close(fd);
            break;
        }
    }
    send_n(fd, (char*)&t, t.dataLen + 4);
    return 0;
}
int recvFile(int socketfd){  
    printf("Start recvFile!\n"); 
    int ret; 
    char *path=(char*)calloc(128,1);
    getcwd(path,127);
    char *tmp=(char*)calloc(128,1);
    sprintf(tmp,"%s/dowmFile",path);
    chdir(tmp);
    int dataLen;
    char *buf=(char*)calloc(4096,1);
    recv_n(socketfd,(char*)&dataLen,sizeof(int));
    recv_n(socketfd,buf,dataLen);
    off_t fileTotalSize,fileLoadSize=0;
    //接文件大小
    recv_n(socketfd,(char*)&dataLen,sizeof(int));
    recv_n(socketfd,(char*)&fileTotalSize,dataLen);
    int fd;
    fd=open(buf,O_WRONLY|O_CREAT,0666);
    if(-1==fd)
    {
        perror("open");
        return -1;
    }
    time_t start=time(NULL),now;
    now=start; 
    while(1)
    {
        ret=recv_n(socketfd,(char*)&dataLen,sizeof(int));
        if(-1==ret)
        {
            printf("server crash\n");
            return -1;
        }
        if(dataLen>0)
        {
            ret=recv_n(socketfd,buf,dataLen);
            if(-1==ret)
            {
                printf("server crash\n");
                return -1;
            }
            write(fd,buf,dataLen);
            fileLoadSize+=dataLen;
            now=time(NULL);
            if(now-start>0)
            {
                printf("%5.2f%s\r",(double)fileLoadSize/fileTotalSize*100,"%");
                fflush(stdout);
                start=now;
            }
        }else{
            printf("100.00%s\n","%");
            close(fd);
            break;
        }
    }
    free(buf);
    chdir(path);
    free(path);
    free(tmp);
    return 0;
}
void getconfig(char buf[][20]) {
    char ch;
    int i = 0;
    bzero(buf, 30);
    int config = open("config", O_RDONLY);
    while (i < 2) {
        read(config,&ch,1);
        if(ch!='\n'){
            buf[i][strlen(buf[i])]=ch;
        }else{
            ch=0;
            i++;
            memset(buf[i],0,20);
        }
    }
}
char *getcmd(char *cmd_all){//gets file   exit
    char *ret=(char*)calloc(10,sizeof(char));
    int i=0;
    while(1){
        if(cmd_all[i]==' ')break;
        if(cmd_all[i]==0)break;
        i++;
    }
    memcpy(ret,cmd_all,i);
    return ret;
}
char *getarg(char *cmd_all){
    unsigned long i=0;
    int count=0;
    while(1){
        if(cmd_all[i]==' ')break;
        if(cmd_all[i]=='\0')return NULL;
        i++;
    }
    char *ret=(char*)calloc(10,sizeof(char));
    i++;
    for(;i<strlen(cmd_all);i++){
        ret[count]=cmd_all[i];
        count++;
    }
    return ret;
}
int userADD(int fd){
    char flag;
    recv(fd,&flag,1,0);
    if(flag=='0'){
        printf("You're not root!\n");
        return -1;
    }
    char id[20];
    char salt[9];
    char * p;
    printf("please input new username:");
    fflush(stdout);
    scanf("%s",id);
    fflush(stdin);
    send_n(fd,id,strlen(id));
    recv(fd,salt,8,0);
    p = getpass("please input new user passwd:");
    p=crypt(p,salt);
    send_n(fd,p,strlen(p));
    recv_n(fd,&flag,1);
    if(flag=='1'){
        printf("UserAdd success!\n");
        return 1;
    }else {
        printf("fail!\n");
        return -1;
    }
}
