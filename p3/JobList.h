#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#ifndef JOBLIST_H

#define JOBLIST_H

typedef struct jobItem{
	pid_t pid;
	int priority;
	char *user;
	char *comm;
	time_t time;
	char *state;
	int retrn;
}jobItem;

typedef struct tNode * tJobPos;
struct tNode
{
    jobItem data;
    tJobPos next;
};

typedef tJobPos tJobList;


/*function prototypes*/

void createEmptyJobList(tJobList * l);
bool isEmptyJobList(tJobList l);
tJobPos first(tJobList l);
tJobPos last(tJobList l);
tJobPos next(tJobPos p, tJobList l);
tJobPos previous(tJobPos p, tJobList l);
bool insertItem(jobItem d, tJobList *l);
void deleteAtPosition(tJobPos p, tJobList *l);
jobItem getItem(tJobPos p, tJobList l);
void updateItem(jobItem d, tJobPos p, tJobList *l);
tJobPos findItemPid(pid_t pid, tJobList l);
tJobPos findItemState(char *state, tJobList l);


#endif 
