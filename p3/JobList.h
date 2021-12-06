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
	char *state;
	time_t time;
	int *retrn;
}jobItem;

typedef struct tJNode * tJobPos;
struct tJNode
{
    jobItem data;
    tJobPos next;
};

typedef tJobPos tJobList;


/*function prototypes*/

void createEmptyJobList(tJobList * l);
bool isEmptyJobList(tJobList l);
tJobPos jobFirst(tJobList l);
tJobPos jobLast(tJobList l);
tJobPos jobNext(tJobPos p, tJobList l);
tJobPos jobPrevious(tJobPos p, tJobList l);
bool insertJobItem(jobItem d, tJobList *l);
void deleteAtJobPosition(tJobPos p, tJobList *l);
jobItem getJobItem(tJobPos p, tJobList l);
void updateJobItem(jobItem d, tJobPos p, tJobList *l);
tJobPos findItemPid(pid_t pid, tJobList l);
tJobPos findItemState(char *state, tJobList l);


#endif 
