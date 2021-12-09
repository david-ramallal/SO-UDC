#include "MemList.h"

#include <string.h>
#include <stdlib.h>

void createEmptyMemList(tMemList * l)
{
    *l = NULL;
}

bool createMemNode (tMemPos * p)
{
    *p = malloc(sizeof(struct tNode));
    return *p != NULL;
}

tMemPos findPosition (tMemList l, memItem d)
{
    tMemPos p;

    p=l;
    while ((p->next != NULL ) && (strcmp(p->next->data.address, d.address)) < 0)
        p = p->next;
    return p;
}

bool isEmptyMemList(tMemList l)
{
    return (l==NULL);
}

tMemPos first(tMemList l)
{
    return l;
}

tMemPos last(tMemList l)
{
    tMemPos p;

    for(p=l; p->next != NULL ;p=p->next);
    return p;
}

tMemPos next(tMemPos p,tMemList l)
{
    return p->next;
}

tMemPos previous(tMemPos p,tMemList l)
{
    tMemPos q;
    if(p==l)
        return NULL;
    else
    {
        for(q=l;q->next != p; q=q->next);
        return q;
    }

}

bool insertItem(memItem d, tMemList *l)
{
    tMemPos q, p;

    if ( !createMemNode(&q))
        return false;
    else
    {
        q->data = d;
        q->next = NULL;
        if(*l == NULL) // empty list
            *l = q;
        else if (d.address != (*l)->data.address){ //inserting at the beggining of a non empty list
            q->next = *l;
            *l = q;
        }else {  //insertion at the middle/end of a non empty list
            p = findPosition (*l, d);
            q -> next = p -> next;
            p -> next = q;
        }
        return true;
    }
}

void deleteAtPosition(tMemPos p, tMemList *l)
{
    tMemPos  q;
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

memItem getItem(tMemPos p, tMemList l)
{
    return p->data;
}

void updateItem(memItem d, tMemPos p, tMemList *l)
{
    p->data = d;
}

tMemPos findItem(char *address, tMemList l)
{
    tMemPos p;
    char *addr;
    addr = malloc(sizeof(char*));
    
    for(p=l; (p!=NULL) ; p = p->next){
		sprintf(addr, "%p", p->data.address);
		if(strcmp(addr, address) == 0)
			break;
		
	} 
	
    if ((p!=NULL) && (strcmp(addr, address) == 0)){
		free(addr);
        return p;
    }else{
		free(addr);
        return NULL;
	}
}

tMemPos findItemSize(size_t size, tMemList l)
{
    tMemPos p;

    for (p=l;(p!=NULL) && (p->data.memSize != size); p=p->next);
    if ((p!=NULL) && (p->data.memSize == size))
        return p;
    else
        return NULL;
}

tMemPos findItemOtherInfo(char *otherInfo, tMemList l)
{
    tMemPos p;

    for (p=l;(p!=NULL) && (strcmp(p->data.otherInfo,otherInfo) < 0); p=p->next);
    if ((p!=NULL) && (strcmp(p->data.otherInfo,otherInfo) == 0))
        return p;
    else
        return NULL;
}

tMemPos findItemKey(int key, tMemList l)
{
    tMemPos p;

    for (p=l;(p!=NULL) && (p->data.df != key); p=p->next);
    if ((p!=NULL) && (p->data.df == key))
        return p;
    else
        return NULL;
}
