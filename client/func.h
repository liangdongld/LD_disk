#ifndef __FUNC_H__
#define __FUNC_H__
#include "head.h"
#include "md5.h"
#define args_check(argc,num) {if(argc!=num) {printf("error args\n");return -1;}}
typedef struct{
    int dataLen;
    char buf[4096];
}train;
typedef struct file{
    char  fileName[128];
    char  type[10];
    char  fileSize[16];
    char  md5[33];
}myFile,*pmyFile;
typedef struct{
    char username[20];
    char mes[1024];
}cliMes;
int send_n(int sfd,char* ptran,int len);
int recv_n(int sfd,char* ptran,int len);
void getconfig(char buf[][20]);
char *getcmd(char *cmd_all);
char *getarg(char *cmd_all);
int recvFile(int socketfd);
int tranFile(int fd,char *file);
void Recv_Show_File(int fd,myFile file);
char * get_file_size(const char *path);
char * get_file_type(char * filename);
void Up_Load_File(int fd,char * filename);
int Down_Load_File(int fd);
int MAKE_DIR(int fd,char * dirname);
int RM_FILE(int fd);
int RM_DIR(int fd);
void *task(void *p);
int sendALL(int fd);
int userADD(int fd);
#endif
