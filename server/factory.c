#include "factory.h"
void getconfig(char buf[][20]) {
    char ch;
    int i = 0;
    bzero(buf, 64);
    int config = open("config", O_RDONLY);
    while (i < 4) {
        read(config,&ch,1);
        if(ch!='\n'){
            buf[i][strlen(buf[i])]=ch;
        }else{
            ch=0;
            i++;
            memset(buf[i],0,20);
        }
    }
    close(config);
}
void factoryInit(pFactory pf,int threadnum,int capacity,threadfunc_t threadfunc,pMysql_fun my){
    memset(pf,0,sizeof(Factory));
    pf->pthid=(pthread_t*)calloc(threadnum,sizeof(pthread_t));
    pf->que.queCapacity=capacity;
    pf->downFileFunc=threadfunc;
    pf->threadnum=threadnum;
    pf->my=my;
    pthread_mutex_init(&pf->chd,NULL);//目录锁
    pthread_mutex_init(&pf->que.queMutex,NULL);
    pthread_cond_init(&pf->cond,NULL);
    return;
}
void factoryStart(pFactory pf){
    int i;
    if(0==pf->startFlag){
        for(i=0;i<pf->threadnum;i++){
            pthread_create(&pf->pthid[i],NULL,pf->downFileFunc,pf);
        }
        pf->startFlag=1;
    }
}
char *getcmd(char *cmd_all){
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
       if(cmd_all[i]==0)break;
       i++;
    }
    if(cmd_all[i]==0)return NULL;
    if(cmd_all[i]==' ')i++;
    if(cmd_all[i]==' ')return NULL;
    char *ret=(char*)calloc(10,sizeof(char));
    for(;i<strlen(cmd_all);i++){
        ret[count]=cmd_all[i];
        count++;
    }
    return ret;
}
