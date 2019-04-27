#include "work_que.h"
void queInster(pQue_t pq,pNode_t inset){
    if(NULL==pq->queHead){
        pq->queHead=inset;
        pq->queTail=inset;
    }else{
        pq->queTail->ndNext=inset;
        pq->queTail->ndNext=pq->queTail;
    }
    pq->queSize++;
}
int queGet(pQue_t pq,pNode_t *pget){
    if(!pq->queSize){
        return -1;
    }
    *pget=pq->queHead;
    pq->queHead=pq->queHead->ndNext;
    pq->queSize--;
    return 0;
}
