/*
 * 
 *David Garcia Ramallal - david.ramallal@udc.es
 *Alfredo Javier Freire Bouzas - javier.freire.bouzas@udc.es 
 * 
 */
 
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/utsname.h>
#include "CmdList.h"

#define MAXLINEA 1024

TCMDLIST L;

struct CMD{
    char * name;
    void (*func) (char **);
};

void cmd_autores (char *tr[]);
void cmd_ayuda (char *tr[]);
void cmd_carpeta (char *tr[]);
void cmd_fecha (char *tr[]);
void cmd_fin (char *tr[]);
void cmd_hist (char *tr[]);
void cmd_comando (char *tr[]);
void cmd_pid (char *tr[]);
void cmd_infosis (char *tr[]);
int trocearCadena ( char *cadena, char *trozos[]);
void procesarEntrada (char *tr[]);

struct CMD C[]={
        {"fin",cmd_fin},
        {"bye",cmd_fin},
        {"salir",cmd_fin},
        {"carpeta",cmd_carpeta},
        {"autores",cmd_autores},
        {"pid",cmd_pid},
        {"fecha", cmd_fecha},
        {"infosis", cmd_infosis},
        {"ayuda", cmd_ayuda},
        {"hist", cmd_hist},
        {"comando", cmd_comando},
        {NULL,NULL}
};

void cmd_autores (char *tr[])
{
    printf("The authors are:\n");
    
    if (tr[0]!=NULL && !strcmp(tr[0], "-n"))
        printf("David Garcia Ramallal\nAlfredo Javier Freire Bouzas\n");
    else if (tr[0]!=NULL && !strcmp(tr[0], "-l"))
        printf("david.ramallal@udc.es\njavier.freire.bouzas@udc.es (\njavier.freire.bouzas@udc.es)\n");
    else
        printf("David Garcia Ramallal - david.ramallal@udc.es\nAlfredo Javier Freire Bouzas - javier.freire.bouzas@udc.es\n");

}

void cmd_pid(char *tr[])
{

    if (tr[0]!=NULL && !strcmp(tr[0], "-p"))
        printf("Shells' parent process pid: %d\n",getppid());
    else
        printf("Shells' process pid: %d\n",getpid());
}

void cmd_carpeta (char *tr[])
{
    char dir[MAXLINEA];
    
    if(tr[0]==NULL)
		printf("%s\n",getcwd(dir, MAXLINEA));
	else
		if(chdir(tr[0])==-1)
			perror("Cannot change directory");			
}

void cmd_fecha (char *tr[])
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    if (tr[0]!=NULL && !strcmp(tr[0], "-d"))
        printf("%02d/%02d/%04d\n", tm.tm_mday, (tm.tm_mon + 1), (tm.tm_year + 1900));
    else if (tr[0]!=NULL && !strcmp(tr[0], "-h"))
        printf("%02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
    else
        printf("%02d/%02d/%04d\n%02d:%02d:%02d\n", tm.tm_mday, (tm.tm_mon + 1), (tm.tm_year + 1900), tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void cmd_infosis (char *tr[])
{	
    struct utsname unameData;
    uname(&unameData);
    printf("%s\n%s\n%s\n%s\n%s\n", unameData.sysname, unameData.version, unameData.release , unameData.machine , unameData.nodename );
}

void cmd_ayuda (char *tr[])
{	
	if (tr[0]==NULL)
		printf("autores [-l|-n]\npid [-p]\ncarpeta [direct]\nfecha [-d|-h]\nhist [-c|-N]\ncomando N\ninfosis\nayuda [cmd]\nfin\nsalir\nbye\n");
	else{
		if (tr[0]!=NULL && !strcmp(tr[0], "autores"))
			printf("autores [-l|-n]\nPrints names and logins of authors\nauthors -l prints only logins\nauthors -n prints only names\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "pid"))
			printf("pid [-p]\nPrints pid of the process executing the shell\npid -p prints pid of shell's parent process\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "carpeta"))
			printf("carpeta [direct]\nPrints current working directory\nWith argument, changes the current working directory of the shell to direct\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "fecha"))
			printf("fecha [-d|-h]\nPrints current time and date\nfecha -d prints date DD/MM/YYYY\nfecha -h prints time hh:mm:ss\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "hist"))
			printf("hist [-c|-N]\nPrints in order all commands that have been input\nhist -c empties list of historic commands\nhist -N prints the first N commands\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "comando"))
			printf("comando N\nRepeats command number N from historic list\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "infosis"))
			printf("infosis\nPrints information on the machine running the shell\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "ayuda"))
			printf("ayuda [cmd]\nDisplays list of available commands\nayuda cmd gives brief help on the usage of command cmd\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "fin"))
			printf("fin\nEnds the shell\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "salir"))
			printf("salir\nEnds the shell\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "bye"))
			printf("bye\nEnds the shell\n");		
		}
}

void cmd_fin(char *tr[])
{
    exit(0);
}

void cmd_hist(char *tr[])
{
	int i;
	
	if(tr[0]==NULL){
		PrintCmdList(L);
		return;
	}
	
	if(!strcmp(tr[0],"-c"))
		ClearCmdList(L);
	else
		for(i=0;i<atoi(tr[0])+1;i++){
			printf("%d->",i);
			PrintCmdN(L,i);
		}
		
}

void cmd_comando(char *tr[])
{
	void procesarEntrada (char **);
	int trocearCadena(char *,char **);
	
	int n;
	char *p, aux[MAXLINEA], *tro[MAXLINEA/2];
	
	if(tr[0]==NULL){
		PrintCmdList(L);
		return;
	}
	n=atoi(tr[0]);
	p=GetCmdN(L,n);
	if (p!=NULL){
		printf("%s", p);
		strcpy (aux,p);
		trocearCadena(aux,tro);
		procesarEntrada(tro);
	}
}

int trocearCadena ( char *cadena, char *trozos[])
{
    int i=1;

    if ((trozos[0] = strtok(cadena," \n\t")) == NULL)
        return 0;
    while ((trozos[i] = strtok(NULL, " \n\t")) != NULL)
        i++;
    return i;
}

void procesarEntrada (char *tr[])
{
    int i;

    if(tr[0]==NULL)
        return;
    for(i=0; C[i].name!=NULL;i++)
        if (!strcmp(tr[0], C[i].name)){
            (*C[i].func)(tr+1);
            return;
        }

    printf("%s command not found\n",tr[0]);
}

int main()
{
    char linea[MAXLINEA];
    char *tr[MAXLINEA/2];
    char aux[MAXLINEA];
    CreateCmdList(L);

    while (1){
        printf("*) ");
        fgets(linea, MAXLINEA, stdin);
        strcpy (aux, linea);
        if (trocearCadena(linea, tr)>0)
			InsertCmd(L,aux);        
        procesarEntrada(tr);
    }

    return 0;
}
