#include "JobList.h"

#include <string.h>
#include <stdlib.h>

void createEmptyJobList(tJobList * l)
{
    *l = NULL;
}

bool createJobNode (tJobPos * p)
{
    *p = malloc(sizeof(struct tJNode));
    return *p != NULL;
}

tJobPos findJobPosition (tJobList l, jobItem d)
{
    tJobPos p;
    p=l;
    while ((p->next != NULL ) && (p->next->data.pid != d.pid))
        p = p->next;
    return p;
}

bool isEmptyJobList(tJobList l)
{
    return (l==NULL);
}

tJobPos jobFirst(tJobList l)
{
    return l;
}

tJobPos jobLast(tJobList l)
{
    tJobPos p;

    for(p=l; p->next != NULL ;p=p->next);
    return p;
}

tJobPos jobNext(tJobPos p,tJobList l)
{
    return p->next;
}

tJobPos jobPrevious(tJobPos p,tJobList l)
{
    tJobPos q;
    if(p==l)
        return NULL;
    else
    {
        for(q=l;q->next != p; q=q->next);
        return q;
    }

}

bool insertJobItem(jobItem d, tJobList *l)
{
    tJobPos q, p;

    if ( !createJobNode(&q))
        return false;
    else
    {
        q->data = d;
        q->next = NULL;
        if(*l == NULL) // empty list
            *l = q;
        else if (d.pid != (*l)->data.pid){ //inserting at the beggining of a non empty list
            q->next = *l;
            *l = q;
        }else {  //insertion at the middle/end of a non empty list
            p = findJobPosition (*l, d);
            q -> next = p -> next;
            p -> next = q;
        }
        return true;
    }
}

void deleteAtJobPosition(tJobPos p, tJobList *l)
{
    tJobPos  q;
    if (p == *l) //delete first element
        *l = p->next;
    else if (p->next == NULL) //delete last element
    {
        for(q=*l ; q->next !=p ; q=q->next);
        //q is the previous node to p
        q->next = NULL;
    }
    else //delete an intermediate position
    {
        q = p->next;
        p->data = q->data;
        p->next = q->next;
        p = q;
    }

    free(p);
}

jobItem getJobItem(tJobPos p, tJobList l)
{
    return p->data;
}

void updateJobItem(jobItem d, tJobPos p, tJobList *l)
{
    p->data = d;
}

tJobPos findItemState(char* state, tJobList l)
{
    tJobPos p;
    char *st;
    st = malloc(sizeof(char*));
    
    for(p=l; (p!=NULL) ; p = p->next){
		*st = *(p->data.state);
		if(strcmp(st, state) == 0)
			break;
		
	} 
	
    if ((p!=NULL) && (strcmp(st,state) == 0)){
		free(st);
        return p;
    }else{
		free(st);
        return NULL;
	}
}

tJobPos findItemPid(pid_t pid, tJobList l)
{
    tJobPos p;

    for (p=l;(p!=NULL) && (p->data.pid != pid); p=p->next);
    if ((p!=NULL) && (p->data.pid == pid))
        return p;
    else
        return NULL;
}
