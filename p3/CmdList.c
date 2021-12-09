#include "CmdList.h"


void CreateCmdList(TCMDLIST l)
{
	l[0]=NULL;
}

int InsertNode (TCMDLIST l, TNODE *p)
{
	int i;
	
	for(i=0;i<MAXLIST;i++)
		if(l[i]==NULL)
			break;
	if(i==MAXLIST-1)
		return -1;
	l[i]=p;
	l[i+1]=NULL;
	return 0;
}

void FreeNode (TNODE *p)
{
	free(p->cmd);
	free(p);
}

void ClearCmdList (TCMDLIST l)
{
	int i;
	
	for(i=0;l[i]!=NULL;i++){
		FreeNode(l[i]);
		l[i]=NULL;
	}
}

TNODE * GetNodeN (TCMDLIST l, int n)
{
	return l[n];
}

int GetListSize (TCMDLIST l)
{
	int i;
	
	for(i=0;l[i]!=NULL;i++);
	return i;
}

void PrintNode (TNODE *p)
{
	printf("%s",p->cmd);
}

void PrintCmdN(TCMDLIST l, int n)
{
	if (GetListSize(l)>n)
		PrintNode(GetNodeN(l,n));		
}

void PrintCmdList (TCMDLIST l)
{
	int i;
	
	for(i=0;l[i]!=NULL;i++){
		printf("%d->",i);
		PrintNode(l[i]);
	}
}

char * GetCmdN(TCMDLIST l, int n)
{
	TNODE * p;
	if (GetListSize(l)<=n)
		return NULL;
	p=GetNodeN(l,n);
	return p->cmd;
}
	

TNODE * CreateNode (char * cmd)
{
	TNODE *p = (TNODE *) malloc (sizeof (TNODE *));
	
	if (p==NULL)
		return NULL;
	if ((p->cmd=strdup(cmd))==NULL){
		free(p);
		return (NULL);
	}
	return p;
}

int InsertCmd (TCMDLIST l, char * cmd)
{
	TNODE *p=CreateNode(cmd);
	
	if(p==NULL)
		return -1;
	if (InsertNode(l,p)==-1){
		FreeNode(p);
		return -1;
	}
	return 0;
}
	








	
