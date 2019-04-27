#include "factory.h"
int tranFile(int fd,char *file,char * filename) {
	train t;
	strcpy(t.buf, filename);
	t.dataLen = strlen(filename);
	send_n(fd, (char*)&t, t.dataLen+4);
	int newfd = open(file, O_RDONLY);//打开需要传输的文件
	struct stat statbuf;
	fstat(newfd, &statbuf);
	memcpy(t.buf,&statbuf.st_size, sizeof(statbuf.st_size));//赋值长度
	t.dataLen = sizeof(statbuf.st_size);
	send_n(fd, (char*)&t, t.dataLen + 4);
	while ((t.dataLen = read(newfd, t.buf, sizeof(t.buf)))) {
		send_n(fd, (char*)&t, t.dataLen + 4);
	}
	send_n(fd, (char*)&t, t.dataLen + 4);
    close(newfd);
	return 0;
}    
int recvFile(int socketfd,int fileID){  
    int ret; 
    int dataLen;
    char *buf=(char*)calloc(4096,1);
    recv_n(socketfd,(char*)&dataLen,sizeof(int));
    recv_n(socketfd,buf,dataLen);
    off_t fileTotalSize;
    //接文件大小
    recv_n(socketfd,(char*)&dataLen,sizeof(int));
    recv_n(socketfd,(char*)&fileTotalSize,dataLen);
    int fd;
    bzero(buf,4096);
    sprintf(buf,"%d",fileID);
    fd=open(buf,O_WRONLY|O_CREAT,0666);
    if(-1==fd)
    {
        perror("open");
        return -1;
    }
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
        }else{
            close(fd);
            break;
        }
    }
    free(buf);
    return 0;
}
