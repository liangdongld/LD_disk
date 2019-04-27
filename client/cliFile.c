#include "func.h"
void Recv_Show_File(int fd,myFile file){
    do{
        recv_n(fd,(char*)&file,sizeof(file));
        printf("- %-20s\t%-10s\t%-10s\n",file.fileName,file.type,file.fileSize);
    }while(strlen(file.fileName)!=0);
}
char * get_file_size(const char *path)
{
    unsigned long filesize = -1;	
    struct stat statbuff;
    char * size = (char*)calloc(16,1);
    if(stat(path, &statbuff) < 0){
        return NULL;
    }else{
        filesize = statbuff.st_size;
    }
    sprintf(size,"%ld",filesize);
    return size;
}
char * get_file_type(char * filename){
    int len = strlen(filename)-1;
    char * type;
    while(len>0){
        if(filename[len]=='.'){
            type = &filename[len+1];
            filename[len]='\0';
            return type;
        }else{
            len--;
        }
    }
    return NULL;
}
void Up_Load_File(int fd,char * filename){
    int ffd = open(filename,O_RDONLY);
    if(ffd==-1){
        perror("open"); 
        return;
    }
    close(ffd);
    myFile file;
    bzero(&file,sizeof(file));
    char * size = get_file_size(filename);
    strcpy(file.fileSize,size);
    free(size);
    Compute_file_md5(filename,file.md5);
    strcpy(file.fileName,filename);
    char * type = get_file_type(file.fileName);
    if(type!=NULL){
        strcpy(file.type,type);
    }
    send_n(fd,(char*)&file,sizeof(file));
    char mes;
    recv_n(fd,&mes,1);
    if(mes=='1'){//发送文件
        tranFile(fd,filename);
    }else if(mes=='0'){
        printf("100%%\n\n");
    }
    printf("Send File Success!\n");
}
int Down_Load_File(int fd){
    char mes;
    recv(fd,&mes,1,0);
    if(mes=='t'){
        recvFile(fd);
        return 0;
    }else if(mes=='f'){
        printf("error\n");
        return -1;
    }
    return -1;
}
int MAKE_DIR(int fd,char * dirname){
    myFile file;
    strcpy(file.fileName,dirname);
    strcpy(file.type,"dir");
    send_n(fd,(char*)&file,sizeof(file));
    char mes;
    recv_n(fd,&mes,1);
    if(mes=='0')return 1;
    return -1;
}
int RM_FILE(int fd){
    char mes;
    recv_n(fd,&mes,1);
    if(mes=='f'){
        printf("Can't search this file\n");
        return -1;
    }
    recv_n(fd,&mes,1);
    if(mes=='f'){
        printf("error\n");
        return -1;
    }
    recv_n(fd,&mes,1);
    if(mes=='f'){
        printf("error\n");
        return -1;
    }else{
        printf("Remove success!\n");
        return 0;
    }
}
int RM_DIR(int fd){
    char mes;
    do{
        recv(fd,&mes,1,0);
        if(mes=='s')return 0;
    }while(mes=='t');
    return -1;
}
