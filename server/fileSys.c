#include "factory.h"
int MYSQL_FILE_INSERT(pMysql_fun my,myFile file,char * path){//添加文件
    MYSQL_bzero(my);
    int flag = 0;
    char * ret;
    ret = MYSQL_SELECT(my,"sysFile","fileID","md5",file.md5);
    if(ret == NULL){//服务器无文件
        sprintf(my->query,"insert into sysFile(fileSize,md5) values('%s','%s')",file.fileSize,file.md5);
        unsigned int t = mysql_query(my->conn,my->query);
        if(t){
            printf("ERROR %s \n",mysql_error(my->conn));
            return -1;
        }
        mysql_free_result(my->res);
        ret = MYSQL_SELECT(my,"sysFile","fileID","md5",file.md5);
        flag = atoi(ret); 
    }
    //服务器已有文件
    sprintf(my->query,"insert into cliFile(fileID,fileName,type,path) values('%d','%s','%s','%s')",atoi(ret),file.fileName,file.type,path);
    free(ret);
    unsigned int t = mysql_query(my->conn,my->query);
    if(t){
        printf("ERROR %s \n",mysql_error(my->conn));
        return -1;
    }
    return flag;
}
int MYSQL_SHOW_FILE(int fd,pMysql_fun my,char * path){//ls -l
    MYSQL_bzero(my);
    myFile file;
    sprintf(my->query,"select cliFile.filename,cliFile.type,sysFile.fileSize,sysFile.md5 from cliFile,sysFile where sysFile.fileID=cliFile.fileID and path = '%s'",path);
    unsigned int t = mysql_query(my->conn,my->query);
    if(t){
        printf("ERROR %s \n",mysql_error(my->conn));
        return -1;
    }else{
        my->res=mysql_use_result(my->conn);
        if(my->res){
            while((my->row=mysql_fetch_row(my->res))!=NULL){
                memset(&file,0,sizeof(file));
                for(t=0;t<mysql_num_fields(my->res);t++){
                    if(t==0)strcpy(file.fileName,my->row[t]);
                    if(t==1)strcpy(file.type,my->row[t]);
                    if(t==2)strcpy(file.fileSize,my->row[t]);
                    if(t==3)strcpy(file.md5,my->row[t]);
                }
                send_n(fd,(char*)&file,sizeof(file));
            }
        }
    }
    memset(&file,0,sizeof(file));
    mysql_free_result(my->res);
    sprintf(my->query,"select filename,type from cliFile where path = '%s' and type = 'dir'",path);
    t = mysql_query(my->conn,my->query);
    if(t){
        printf("ERROR %s \n",mysql_error(my->conn));
        return -1;
    }else{
        my->res=mysql_use_result(my->conn);
        if(my->res){
            while((my->row=mysql_fetch_row(my->res))!=NULL){
                memset(&file,0,sizeof(file));
                for(t=0;t<mysql_num_fields(my->res);t++){
                    if(t==0)strcpy(file.fileName,my->row[t]);
                    if(t==1)strcpy(file.type,my->row[t]);
                }
                send_n(fd,(char*)&file,sizeof(file));
            }
        }
    }
    memset(&file,0,sizeof(file));
    mysql_free_result(my->res);
    send_n(fd,(char*)&file,sizeof(file));//结束符
    return 1;
}
int MYSQL_FIND_DIR(pMysql_fun my,char * path,char * arg){//检查cd后边的文件夹是否存在
    MYSQL_bzero(my);
    sprintf(my->query,"select type from cliFile where path = '%s' and filename = '%s' ",path,arg);
    unsigned int t = mysql_query(my->conn,my->query);
    int flag=0;
    if(t){
        printf("ERROR %s \n",mysql_error(my->conn));
        return -1;
    }else{
        my->res=mysql_use_result(my->conn);
        if(my->res){
            my->row=mysql_fetch_row(my->res);
            if(my->row==NULL)flag = -1;
            else if(!strcmp(my->row[0],"dir"))flag = 1;
        }
    }
    mysql_free_result(my->res);
    return flag;
}
char * MyChdir(int fd,pMysql_fun my,char * path,char * arg){//cd命令
    if(!strcmp(arg,"..")){//上级目录
        if(strlen(path)==1){
            send(fd,"f",strlen("f"),0);
            return NULL;
        }else{
            int len = strlen(path) - 1;
            while(path[len]!='/'){
                path[len]='\0';
                len--;
            }
            if(len!=0)path[len]='\0';
        }
    }else if(!strcmp(arg,".")){}
    else{
        if(1==MYSQL_FIND_DIR(my,path,arg)){
            if(strlen(path)==1)strcat(path,arg);
            else{
                strcat(path,"/");
                strcat(path,arg);
            }
        }else{
            send(fd,"f",strlen("f"),0);
            return NULL;
        }
    }
    send(fd,"t",strlen("t"),0);
    return path;
}
int Recv_File_FromClient(pMysql_fun my,int fd,char * path){
    myFile file;
    char mes;
    recv_n(fd,(char*)&file,sizeof(file));
    int fileID = MYSQL_FILE_INSERT(my,file,path);
    if(fileID>0){//需要接受文件
        mes = '1';
        send_n(fd,&mes,1);
        int ret = recvFile(fd,fileID);
        if(ret==-1)return -1;
    }else{
        mes = '0';
        send_n(fd,&mes,1);
    }
    return 1;
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
int Send_File_ToClient(pMysql_fun my,int fd,char * filename,char * path){ 
    char name[128];
    strcpy(name,filename);
    char * type = get_file_type(name);
    char * fileID = MYSQL_SELECT_FILEID(my,path,name,type);
    if(fileID==NULL){
        send(fd,"f",1,0);//未找到文件
        return -1;
    }
    send(fd,"t",1,0);
    tranFile(fd,fileID,filename);
    free(fileID);
    return 1;
}
int MAKE_DIR(pMysql_fun my,int fd,char * path){
    myFile file;
    char mes;
    MYSQL_bzero(my);
    recv_n(fd,(char*)&file,sizeof(file));
    sprintf(my->query,"insert into cliFile(path,filename,type) values('%s','%s','dir')",path,file.fileName);
    unsigned int t = mysql_query(my->conn,my->query);
    if(t){
        mes = '1';
        printf("ERROR %s \n",mysql_error(my->conn));
        return -1;
    }
    mes = '0';
    send_n(fd,&mes,1);
    return 0;
}
int REMOVE_FILE(pMysql_fun my,int fd,char * path,char * filename){//此filename带有type
    char * type = get_file_type(filename); 
    char * ret = MYSQL_SELECT_FILEID(my,path,filename,type);
    char mes='t';
    if(ret==NULL){
        mes = 'f';
        send_n(fd,&mes,1);
        return -1;
    };
    send_n(fd,&mes,1);
    MYSQL_bzero(my);
    int fileID = atoi(ret);
    sprintf(my->query,"delete from cliFile where filename = '%s' and type = '%s' and path ='%s'",filename,type,path);
    unsigned int t = mysql_query(my->conn,my->query);
    if(t){
        printf("ERROR %s \n",mysql_error(my->conn));
        mes = 'f';
        send_n(fd,&mes,1);
        return -1;
    }
    mes = 't';
    send_n(fd,&mes,1);
    MYSQL_bzero(my);
    int flag=0;
    sprintf(my->query,"select id from cliFile where fileID = %d",fileID);
    t=mysql_query(my->conn,my->query);
    if(t){
        printf("WRONG:%s",mysql_error(my->conn));
        mes = 'f';
        send_n(fd,&mes,1);
        return -1;
    }else{
        my->res=mysql_use_result(my->conn);
        if(my->res){
            my->row=mysql_fetch_row(my->res);
            if(my->row==NULL){
                flag = 1;
            }else{
                flag = 0;
            }
        }
    }
    mysql_free_result(my->res);
    mes = 't';
    send_n(fd,&mes,1);
    if(flag == 1){//需要删除服务器端文件
        unlink(ret);
    sprintf(my->query,"delete from sysFile where fileID = %d ",fileID);
    unsigned int t = mysql_query(my->conn,my->query);
    if(t){
        printf("ERROR %s \n",mysql_error(my->conn));
        return -1;
        }
    }
    return 1;
}
int REMOVE_DIR(pMysql_fun my,char * path,int fd,char * dirname){
    unsigned int t;
    char * count;
    if(1!=MYSQL_FIND_DIR(my,path,dirname)){
        return -1;
    }
    MYSQL_bzero(my);
    char dir[128];
    if(strlen(path)==1)
        sprintf(dir,"%s%s",path,dirname);
    else
        sprintf(dir,"%s/%s",path,dirname);
    sprintf(my->query,"select count(*) from cliFile where path = '%s'",dir);
    t=mysql_query(my->conn,my->query);
    if(t){
        printf("WRONG:%s",mysql_error(my->conn));
        return -1;   
    }else{
        my->res=mysql_use_result(my->conn);
        if(my->res){
            my->row=mysql_fetch_row(my->res);
            count=my->row[0];
        }
    }
    int filecount = atoi(count);
    mysql_free_result(my->res);
    if(filecount==0){//只需要删除路径
        sprintf(my->query,"delete from cliFile where path = '%s' and type = 'dir' and filename = '%s'",path,dirname);
        t = mysql_query(my->conn,my->query);
        if(t){
            printf("ERROR %s \n",mysql_error(my->conn));
            return -1;
        }
    }else{
        pmyFile rmfile = (pmyFile)calloc(filecount,sizeof(myFile));
        sprintf(my->query,"select filename,type from cliFile where path = '%s'",dir);
        unsigned int t = mysql_query(my->conn,my->query);
        if(t){
            printf("ERROR %s \n",mysql_error(my->conn));
            return -1;
        }else{
            my->res=mysql_use_result(my->conn);
            if(my->res){
                int i = 0;
                while((my->row=mysql_fetch_row(my->res))!=NULL){
                    for(t=0;t<mysql_num_fields(my->res);t++){
                        if(t==0)strcpy(rmfile[i].fileName,my->row[t]);
                        if(t==1)strcpy(rmfile[i].type,my->row[t]);
                    }
                    i++;
                }
            }
            mysql_free_result(my->res);
        }
        char filename[128];
        for(int t = 0;t<filecount;t++){
            if(!strcmp(rmfile[t].type,"dir")){
                REMOVE_DIR(my,dir,fd,rmfile[t].fileName);
                continue;
            }
            sprintf(filename,"%s.%s",rmfile[t].fileName,rmfile[t].type);
            REMOVE_FILE(my,fd,dir,filename);
        }
        free(rmfile);
        REMOVE_DIR(my,path,fd,dirname);
    }
    return 0;
}
