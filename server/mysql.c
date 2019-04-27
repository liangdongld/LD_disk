#include "factory.h"
char *GenerateSalt(){
    char *str=(char*)calloc(STR_LEN+1,1);//开辟长度为预设值的空间，用来存储随机生成的字符串
    int i,flag;
    srand(time(NULL));//通过时间函数设置随机数种子，使得每次运行结果随机。
    for(i = 0; i < STR_LEN; i ++){
        flag = rand()%3;
        switch(flag){
        case 0:
            str[i] = rand()%26 + 'a'; 
            break;
        case 1:
            str[i] = rand()%26 + 'A'; 
            break;
        case 2:
            str[i] = rand()%10 + '0'; 
            break;
        }
    }
    return str;
}
int MYSQL_init(pMysql_fun my){
    my->conn=mysql_init(NULL);
    my->sever=(char*)calloc(1,sizeof(LOCALHOST)+1);
    strcpy(my->sever,LOCALHOST);
    my->passwd=(char*)calloc(1,sizeof(SQL_PWD)+1);
    strcpy(my->passwd,SQL_PWD);
    my->database=(char*)calloc(1,sizeof(DB)+1);
    strcpy(my->database,DB);
    my->user=(char*)calloc(1,sizeof(SQL_USER)+1);
    strcpy(my->user,SQL_USER);
    my->query=(char*)calloc(1,128);

    my->conn=mysql_init(NULL);
    if(!mysql_real_connect(my->conn,my->sever,my->user,my->passwd,my->database,0,NULL,0)){
        return -1;
    }
    return 1;
}
int MYSQL_bzero(pMysql_fun my){
    if(my->query==NULL)return -1;
    bzero(my->query,128);
    return 1;
}
char *MYSQL_SELECT(pMysql_fun my,const char * table,const char *r_type,const char *find_type,char *key){
    unsigned int t;
    sprintf(my->query,"select %s from %s where %s = '%s'",r_type,table,find_type,key);
    t=mysql_query(my->conn,my->query);
    char *ret=NULL;
    if(t){
        printf("WRONG:%s",mysql_error(my->conn));
        return NULL;
    }else{
        my->res=mysql_use_result(my->conn);
        if(my->res){
            my->row=mysql_fetch_row(my->res);
            if(my->row==NULL)
            {
                return NULL;
            }
            ret=(char*)malloc(strlen(my->row[0])+1);
            strcpy(ret,my->row[0]);
        }
    }
    mysql_free_result(my->res);
    return ret;
}
int MYSQL_ID_INSERT(pMysql_fun my,char *username,char* salt,char* passwd){//存储的密码为密文
    MYSQL_bzero(my); 
    sprintf(my->query,"insert into shadow(username,salt,passwd) values('%s','%s','%s')",username,salt,passwd);
    unsigned int t = mysql_query(my->conn,my->query);
    if(t){
        printf("ERROR %s \n",mysql_error(my->conn));
        return -1;
    }
    return 1;
}
int signin(pMysql_fun my,char *username,char *passwd){
    char * pwd = MYSQL_SELECT(my,"shadow","passwd","username",username);
    if(!strcmp(pwd,passwd)){
        free(pwd);
        return 1;
    }else{
        free(pwd);
        return 0;
    }
}
int sendsalt(pMysql_fun my,int fd,char* username){
    char * salt = MYSQL_SELECT(my,"shadow","salt","username",username);
    train mes;
    if(salt==NULL){
        mes.dataLen=1;
        strcpy(mes.buf,"1");
        send_n(fd,(char*)&mes,mes.dataLen+4);
        return 0;
    }
    mes.dataLen=strlen(salt);
    strcpy(mes.buf,salt);
    send_n(fd,(char*)&mes,mes.dataLen+4);
    free(salt);
    return 1;
}
int getUserID(pMysql_fun my,char* username){
    char* ret = MYSQL_SELECT(my,"shadow","userID","username",username);
    int id=atoi(ret);
    return id;
}
int MYSQL_LOG_INSERT(pMysql_fun my,int userID,char * path,char * cmd,char * argument,char * ip ,int port){
    MYSQL_bzero(my);
    sprintf(my->query,"insert into log(time,userID,path,cmd,argument,ip,prot) values(now(),'%d','%s','%s','%s','%s','%d')",userID,path,cmd,argument,ip,port);
    unsigned int t = mysql_query(my->conn,my->query);
    if(t){
        printf("ERROR %s \n",mysql_error(my->conn));
        return -1;
    }
    return 1;
}
char *MYSQL_SELECT_FILEID(pMysql_fun my,char * path,char * filename,char * type){
    unsigned int t;
    sprintf(my->query,"select fileID from cliFile where path ='%s' and filename='%s' and type ='%s'",path,filename,type);
    t=mysql_query(my->conn,my->query);
    char *ret=NULL;
    if(t){
        printf("WRONG:%s",mysql_error(my->conn));
        return NULL;
    }else{
        my->res=mysql_use_result(my->conn);
        if(my->res){
            my->row=mysql_fetch_row(my->res);
            if(my->row==NULL)
            {
                return NULL;
            }
            ret=(char*)malloc(strlen(my->row[0])+1);
            strcpy(ret,my->row[0]);
        }
    }
    mysql_free_result(my->res);
    return ret;
}
int User_Add(int fd,pMysql_fun my){
    send(fd,"1",1,0);
    char id[20];
    bzero(id,20);
    recv(fd,id,20,0);
    char* salt = GenerateSalt();
    send(fd,salt,strlen(salt),0);
    char passwd[15];
    bzero(passwd,15);
    recv(fd,passwd,15,0);
    int ret = MYSQL_ID_INSERT(my,id,salt,passwd);
    if(ret == 1){
        send(fd,"1",1,0);
    }else{
        send(fd,"0",1,0);
    }
    return ret;
}
