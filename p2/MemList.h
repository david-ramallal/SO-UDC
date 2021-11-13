#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#ifndef MEMLIST_H

#define MEMLIST_H

typedef struct memItem{
	char *address;
	size_t memSize;
	struct tm memTime;
	char *memType;
	char *otherInfo;
	int df;
}memItem;

typedef struct tNode * tMemPos;
struct tNode
{
    memItem data;
    tMemPos next;
};

typedef tMemPos tMemList;


/*function prototypes*/

void createEmptyMemList(tMemList * l);
bool isEmptyMemList(tMemList l);
tMemPos first(tMemList l);
tMemPos last(tMemList l);
tMemPos next(tMemPos p,tMemList l);
tMemPos previous(tMemPos p,tMemList l);
bool insertItem(memItem d, tMemList *l);
void deleteAtPosition(tMemPos p, tMemList *l);
memItem getItem(tMemPos p, tMemList l);
void updateItem(memItem d, tMemPos p, tMemList *l);
tMemPos findItem(char *address, tMemList l);
tMemPos findItemSize(size_t size, tMemList l);
tMemPos findItemOtherInfo(char *otherInfo, tMemList l);


#endif 
