#ifndef __FACTORY_H__
#define __FACTORY_H__
#include "head.h"
#include "work_que.h"
#include "md5.h"
#include "chat.h"
#define DE_DIR "~/ftp/service"
#define LOCALHOST "localhost"
#define SQL_PWD "123456"
#define DB "ftp"
#define SQL_USER "root"
#define STR_LEN 8
#define USERID_LEN 20
#define PASSWD_LEN 30
typedef void* (*threadfunc_t)(void*);//void*类型的函数指针，传递参数为void*类型
typedef struct{
    int dataLen;
    char buf[4096];
}train;
typedef struct{
    MYSQL *conn;
    MYSQL_ROW row;
    MYSQL_RES *res;
    char *user;
    char *sever;
    char *passwd;
    char *database;
    char *query;
}Mysql_fun,*pMysql_fun;
typedef struct{
    pthread_t *pthid;
    int threadnum;
    Que_t que;
    pthread_cond_t cond;//条件变量
    threadfunc_t  downFileFunc;//threadfunc_t 为自己定义的函数指针
    short startFlag;//是否启动
    pthread_mutex_t chd;
    pMysql_fun my;
}Factory,*pFactory;
typedef struct file{
    char  fileName[128];
    char  type[10];
    char  fileSize[16];
    char  md5[33];
}myFile,*pmyFile;
int tranFile(int newfd,char* rfile,char * sfile);
int recvFile(int socketfd,int);
int send_n(int sfd,char*ptran,int len);
int recv_n(int sfd,char*ptran,int len);
void factoryInit(pFactory pf,int threadnum,int capacity,threadfunc_t thradfunc,pMysql_fun my);
void factoryStart(pFactory);
int tcpInit(int *sfd,char *ip,char *port);
char *ls_l(char*,char*);
void cd(char*,char*,pFactory);
void pwd(char *d);
void getconfig(char buf[][20]);
char *getcmd(char *);
char *getarg(char *);
int finddir(char *dirname,char *filename);
//以下用到了MySQL
int MYSQL_init(pMysql_fun my);
char *MYSQL_SELECT(pMysql_fun my,const char * table,const char *r_type,const char *find_type,char *key);
int MYSQL_ID_INSERT(pMysql_fun my,char*,char*,char*);
int MYSQL_bzero(pMysql_fun my);
int signin(pMysql_fun,char*,char*);
int sendsalt(pMysql_fun,int,char*);
int getUserID(pMysql_fun my,char* username);
char *GenerateSalt();
int MYSQL_LOG_INSERT(pMysql_fun,int,char*,char*,char*,char*,int);
int User_Add(int fd,pMysql_fun my);
//文件系统
int MYSQL_FILE_INSERT(pMysql_fun,myFile,char*);//返回值为2，则需要接收文件 为1则不需要接收文件
int MYSQL_SHOW_FILE(int,pMysql_fun,char*);
int MYSQL_FIND_DIR(pMysql_fun,char*,char*);
char * MyChdir(int,pMysql_fun,char*,char*);
int Recv_File_FromClient(pMysql_fun my,int fd,char * path);
char *MYSQL_SELECT_FILEID(pMysql_fun my,char * path,char * filename,char * type);
int Send_File_ToClient(pMysql_fun,int,char *,char*);
int MAKE_DIR(pMysql_fun ,int,char * path);
int REMOVE_FILE(pMysql_fun my,int fd,char * path,char * filename);//此filename带有type
int REMOVE_DIR(pMysql_fun my,char * path,int fd,char * dirname);
#endif
