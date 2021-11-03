#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef CMDLIST_H

#define CMDLIST_H

#define MAXLIST 4096

struct NODE {
	char *cmd;
};

typedef struct NODE TNODE;

typedef TNODE * TCMDLIST[MAXLIST];

void PrintCmdList(TCMDLIST l);
void ClearCmdList(TCMDLIST l);
void PrintCmdN(TCMDLIST l, int n);
char * GetCmdN(TCMDLIST l, int n);
void CreateCmdList(TCMDLIST l);
int InsertCmd(TCMDLIST l, char *cmd);

#endif

