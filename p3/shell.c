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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include "CmdList.h"
#include "MemList.h"
#include "JobList.h"
#include <errno.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <ctype.h>
#include <signal.h>

#define MAXLINEA 1024
#define MAXVAR 1024
#define LEERCOMPLETO ((ssize_t)-1)

TCMDLIST L;
tMemList *memList;
char *entorno_main[MAXVAR];
char *currentFile;
tJobList *jobLst;

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
void cmd_crear(char *tr[]);
void cmd_borrar(char *tr[]);
void cmd_borrarrec(char *tr[]);
void cmd_listfich(char *tr[]);
void cmd_listdir(char *tr[]);
void cmd_malloc(char *tr[]);
void cmd_mmap(char *tr[]);
void cmd_shared(char *tr[]);
void cmd_dealloc(char *tr[]);
void cmd_memoria(char *tr[]);
void cmd_volcarmem(char *tr[]);
void cmd_llenarmem(char *tr[]);
void cmd_recursiva(char *tr[]);
void cmd_es(char *tr[]);
void cmd_priority(char *tr[]);
void cmd_rederr(char *tr[]);
void cmd_entorno(char *tr[]);
void cmd_mostrarvar(char *tr[]);
void cmd_cambiarvar(char *tr[]);
void cmd_uid(char *tr[]);
void cmd_fork(char *tr[]);
void cmd_ejec(char *tr[]);
void cmd_ejecpri(char *tr[]);
void cmd_fg(char *tr[]);
void cmd_fgpri(char *tr[]);
void cmd_back(char *tr[]);
void cmd_backpri(char *tr[]);
void cmd_ejecas(char *tr[]);
void cmd_fgas(char *tr[]);
void cmd_bgas(char *tr[]);
void cmd_listjobs(char *tr[]);
void cmd_job(char *tr[]);
void cmd_borrarjobs(char *tr[]);
int trocearCadena ( char *cadena, char *trozos[]);
void procesarEntrada (char *tr[]);

struct SEN{
	char *nombre;
	int senal;
};

static struct SEN sigstrnum[]={
	{"HUP", SIGHUP},
	{"INT", SIGINT},
	{"QUIT", SIGQUIT},
	{"ILL", SIGILL},
	{"TRAP", SIGTRAP},
	{"ABRT", SIGABRT},
	{"IOT", SIGIOT},
	{"BUS", SIGBUS},
	{"FPE", SIGFPE},
	{"KILL", SIGKILL},
	{"USR1", SIGUSR1},
	{"SEGV", SIGSEGV},
	{"USR2", SIGUSR2},
	{"PIPE", SIGPIPE},
	{"ALRM", SIGALRM},
	{"TERM", SIGTERM},
	{"CHLD", SIGCHLD},
	{"CONT", SIGCONT},
	{"STOP", SIGSTOP},
	{"TSTP", SIGTSTP},
	{"TTIN", SIGTTIN},
	{"TTOU", SIGTTOU},
	{"URG", SIGURG},
	{"XCPU", SIGXCPU},
	{"XFSZ", SIGXFSZ},
	{"VTALRM", SIGVTALRM},
	{"PROF", SIGPROF},
	{"WINCH", SIGWINCH},
	{"IO", SIGIO},
	{"SYS", SIGSYS},
/*senhales que no hay en todas partes*/
	#ifdef SIGPOLL
		{"POLL", SIGPOLL},
	#endif
	#ifdef SIGPWR
		{"PWR", SIGPWR},
	#endif
	#ifdef SIGEMT
		{"EMT", SIGEMT},
	#endif
	#ifdef SIGINFO
		{"INFO", SIGINFO},
	#endif
	#ifdef SIGSTKFLT
		{"STKFLT", SIGSTKFLT},
	#endif
	#ifdef SIGCLD
		{"CLD", SIGCLD},
	#endif
	#ifdef SIGLOST
		{"LOST", SIGLOST},
	#endif
	#ifdef SIGCANCEL
		{"CANCEL", SIGCANCEL},
	#endif
	#ifdef SIGTHAW
		{"THAW", SIGTHAW},
	#endif
	#ifdef SIGFREEZE
		{"FREEZE", SIGFREEZE},
	#endif
	#ifdef SIGLWP
		{"LWP", SIGLWP},
	#endif
	#ifdef SIGWAITING
		{"WAITING", SIGWAITING},
	#endif
	{NULL,-1}
};
/*fin array sigstrnum */

int Senal(char * sen){
/*devuel el numero de senial a partir del nombre*/
	int i;
	for (i=0; sigstrnum[i].nombre!=NULL; i++)
		if (!strcmp(sen, sigstrnum[i].nombre))
			return sigstrnum[i].senal;
	return -1;
}

char *NombreSenal(int sen) /*devuelve el nombre senal a partir de la senal*/{
/* para sitios donde no hay sig2str*/
	int i;
	for (i=0; sigstrnum[i].nombre!=NULL; i++)
		if (sen==sigstrnum[i].senal)
			return sigstrnum[i].nombre;
	return ("SIGUNKNOWN");
}

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
        {"crear", cmd_crear},
        {"borrar", cmd_borrar},
        {"borrarrec", cmd_borrarrec},
        {"listfich",cmd_listfich},
        {"listdir", cmd_listdir},
        {"malloc", cmd_malloc},
        {"mmap", cmd_mmap},
        {"shared", cmd_shared},
        {"dealloc", cmd_dealloc},
        {"memoria", cmd_memoria},
        {"volcarmem", cmd_volcarmem},
        {"llenarmem", cmd_llenarmem},
        {"recursiva", cmd_recursiva},
        {"e-s", cmd_es},
        {"priority", cmd_priority},
        {"rederr", cmd_rederr},
        {"entorno", cmd_entorno},
        {"mostrarvar", cmd_mostrarvar},
        {"cambiarvar", cmd_cambiarvar},
        {"uid", cmd_uid},
        {"fork", cmd_fork},
        {"ejec", cmd_ejec},
        {"ejecpri", cmd_ejecpri},
        {"fg", cmd_fg},
        {"fgpri", cmd_fgpri},
        {"back", cmd_back},
        {"backpri", cmd_backpri},
        {"ejecas", cmd_ejecas},
        {"fgas", cmd_fgas},
        {"bgas", cmd_bgas},
        {"listjobs", cmd_listjobs},
        {"job", cmd_job},
        {"borrarjobs", cmd_borrarjobs},
        {NULL,NULL}
};

char LetraTF (mode_t m)
{
	switch (m&S_IFMT) { /*and bit a bit con los bits de formato,0170000 */
		case S_IFSOCK: return 's'; /*socket */
		case S_IFLNK:  return 'l'; /*symbolic link*/
		case S_IFREG:  return '-'; /* fichero normal*/
		case S_IFBLK:  return 'b'; /*block device*/
		case S_IFDIR:  return 'd'; /*directorio */
		case S_IFCHR:  return 'c'; /*char device*/
		case S_IFIFO:  return 'p'; /*pipe*/
		default: return '?';       /*desconocido, no deberia aparecer*/
	}
}

char * ConvierteModo (mode_t m, char *permisos)
{
	strcpy (permisos,"---------- ");
	
	permisos[0]=LetraTF(m);
	if (m&S_IRUSR) permisos[1]='r';
	if (m&S_IWUSR) permisos[2]='w';
	if (m&S_IXUSR) permisos[3]='x';
	if (m&S_IRGRP) permisos[4]='r';
	if (m&S_IWGRP) permisos[5]='w';
	if (m&S_IXGRP) permisos[6]='x';
	if (m&S_IROTH) permisos[7]='r';
	if (m&S_IWOTH) permisos[8]='w';
	if (m&S_IXOTH) permisos[9]='x';
	if (m&S_ISUID) permisos[3]='s';
	if (m&S_ISGID) permisos[6]='s';
	if (m&S_ISVTX) permisos[9]='t';
	return permisos;
}

char * owner_print(struct stat buffer)
{
  int owner;
  owner = buffer.st_uid;
  if (getpwuid(owner) == NULL)
    return "???????";
  else
    return getpwuid(owner)->pw_name;  
}

char * group_print(struct stat buffer)
{
  int group;
  group = buffer.st_uid;
  if (getgrgid(group) == NULL)
    return "???????";
  else
    return getgrgid(group)->gr_name;  
}	

int size_print(struct stat buffer)
{
	return buffer.st_size;	
}	

int inode_print(struct stat buffer)
{
	return buffer.st_ino;	
}

int nlinks_print(struct stat buffer)
{
	return buffer.st_nlink;	
}	

void longTime_print(struct stat buffer)
{
	time_t mt;
	struct tm tm;
	mt = buffer.st_mtime;
	localtime_r(&mt, &tm);
	printf("%04d/%02d/%02d-%02d:%02d ", (tm.tm_year + 1900), (tm.tm_mon + 1), tm.tm_mday, tm.tm_hour, tm.tm_min);				
}

void accTime_print(struct stat buffer)
{
	time_t at;
	struct tm tm;
	at = buffer.st_atime;
	localtime_r(&at, &tm);
	printf("%04d/%02d/%02d-%02d:%02d ", (tm.tm_year + 1900), (tm.tm_mon + 1), tm.tm_mday, tm.tm_hour, tm.tm_min);	
}

void printFILE(char *fName, bool linK, bool lonG, bool acC, bool hiD){
	if(!(!hiD && fName[0] == '.')){
		char toLink[MAXLINEA] = "", * perm;
		struct stat buffer;
		int mode;
	
		if(lstat(fName, &buffer) == -1)
			printf("It is not possible to access %s: %s\n", fName, strerror(errno));
		else{
			if(!lonG && !acC)
				printf("%d %s\n", size_print(buffer), fName);
			else{			
				if(acC)
					accTime_print(buffer);
				else
					longTime_print(buffer);
				perm =(char *) malloc (12);
				mode = buffer.st_mode;
				ConvierteModo(mode, perm);
				printf(" %2d (%8d) %8s %8s %14s %6d %s", nlinks_print(buffer), inode_print(buffer), owner_print(buffer), group_print(buffer), perm, size_print(buffer), fName);
				free(perm);
				if(linK){
					readlink(fName, toLink, MAXLINEA);
					if(S_ISLNK(buffer.st_mode))
						printf("->%s\n", toLink);
					else printf("\n");
				}else
					printf("\n");
			}
		}
	}
}

void printLISTDIR(char *fName, bool lonG, bool linK, bool acC, bool hiD){
	char dir[MAXLINEA];
	getcwd(dir, MAXLINEA);
	struct stat buffer;
	struct stat buffer2;
	DIR * dirc;
	struct dirent *ent;
	
	if(lstat(fName, &buffer) == -1)
		printf("It is not possible to access %s: %s\n", fName, strerror(errno));
	else if (!S_ISREG(buffer.st_mode) && !S_ISLNK(buffer.st_mode)){
		if ((dirc = opendir(fName)) == NULL)
			printf("It is not possible to access %s: %s\n", fName, strerror(errno));
		else{
			printf("************%s\n", fName);
			chdir(fName);
			while ((ent = readdir (dirc)) != NULL){
				if (!lonG && !acC){
					lstat(ent->d_name, &buffer2);
					if(hiD)
						printf("%d %s\n", size_print(buffer2), ent->d_name);
					else if(ent->d_name[0] != '.')
						printf("%d %s\n", size_print(buffer2), ent->d_name);
				}else if(lonG || acC)
					printFILE(ent->d_name, linK, lonG, acC, hiD);
			}
			closedir(dirc);
		}
		chdir(dir);
	}	
}

void printREC(char *fName, bool lonG, bool linK, bool acC, bool hiD, bool recA, bool recB){
	
	if(!hiD && fName[0] == '.')
		return;
	
	char dir[MAXLINEA];
	getcwd(dir, MAXLINEA);
	struct stat buffer;
	DIR * dirc;
	struct dirent *ent;
	
	if(lstat(fName, &buffer) == -1)
		printf("It is not possible to access %s: %s\n", fName, strerror(errno));
	else{
		if(S_ISDIR(buffer.st_mode)){
			if ((dirc = opendir(fName)) != NULL){
				chdir(fName);
				while ((ent = readdir (dirc)) != NULL){
					if(strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")){
						if(recB){
							printREC(ent->d_name, lonG, linK, acC, hiD, recA, recB);
							printLISTDIR(ent->d_name, lonG, linK, acC, hiD);
						}
						else{
							printLISTDIR(ent->d_name, lonG, linK, acC, hiD);
							printREC(ent->d_name, lonG, linK, acC, hiD, recA, recB);
							
						}
					}
				}
				closedir(dirc);				
			}
			chdir(dir);
		}
	}
}

void delete(char *fileName)
{
      if(rmdir(fileName) == -1)
		if(unlink(fileName) == -1)
			printf("It is not possible to delete %s: %s\n", fileName, strerror(errno));
}

void deleteRec(char *fileName)
{
    char dir[MAXLINEA];
    getcwd(dir, MAXLINEA);
    struct stat buffer; 
    DIR * dirc;
    struct dirent *ent;  
   
    if(lstat(fileName, &buffer) == -1)
		printf("It is not possible to delete %s: %s\n", fileName, strerror(errno));
    else{
        if (S_ISDIR(buffer.st_mode)){
			if((dirc = opendir(fileName)) == NULL)
				printf("It is not possible to delete %s: %s\n", fileName, strerror(errno));
			else{
				chdir(fileName);
				while ((ent = readdir (dirc)) != NULL){
					if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
						continue;
					else  
						deleteRec(ent->d_name);
				}        
				closedir(dirc);            
          }
          chdir(dir);
          delete(fileName);
       }else
          delete(fileName);
  }  
}

void printWeekDay(int numDay, char **weekDay){
	switch (numDay){
		case 0: *weekDay = "Mon";
			break;
		case 1: *weekDay = "Tue";
			break;
		case 2: *weekDay = "Wed";
			break;
		case 3: *weekDay = "Thu";
			break;
		case 4: *weekDay = "Fri";
			break;
		case 5: *weekDay = "Sat";
			break;
		case 6: *weekDay = "Sun";
			break;
		default: 
			break;
		};
}

void printMonth(int numMonth, char **month){
	switch (numMonth){
		case 0: *month = "Jan";
			break;
		case 1: *month = "Feb";
			break;
		case 2: *month = "Mar";
			break;
		case 3: *month = "Apr";
			break;
		case 4: *month = "May";
			break;
		case 5: *month = "Jun";
			break;
		case 6: *month = "Jul";
			break;
		case 7: *month = "Aug";
			break;
		case 8: *month = "Sep";
			break;
		case 9: *month = "Oct";
			break;
		case 10: *month = "Nov";
			break;
		case 11: *month = "Dec";
			break;
		default: 
			break;
		};	
}

void printMemList(char *memType, tMemList l){
	tMemPos p;
	char *weekDay = "";
	char *month = "";
	
	if(!strcmp(memType, "malloc")){
		printf("******List of malloc blocks assigned for the process %d\n", getpid());
		for(p=first(l); p != NULL ; p = next(p, l)){
			memItem item = getItem(p, l);
			printWeekDay(item.memTime.tm_wday, &weekDay);
			printMonth(item.memTime.tm_mon, &month);
			if(!strcmp(item.memType, "malloc"))
				printf("%p: size:%zd. malloc %s %s %d %02d:%02d:%02d %d\n", item.address, item.memSize, weekDay, month, item.memTime.tm_mday, item.memTime.tm_hour, item.memTime.tm_min, item.memTime.tm_sec, (item.memTime.tm_year + 1900));
		}
	}else 
	if(!strcmp(memType, "mmap")){
		printf("******List of mmap blocks assigned for the process %d\n", getpid());
		for(p=first(l); p != NULL ; p = next(p, l)){
			memItem item = getItem(p, l);
			printWeekDay(item.memTime.tm_wday, &weekDay);
			printMonth(item.memTime.tm_mon, &month);
			if(!strcmp(item.memType, "mmap"))
				printf("%p: size:%zd. mmap %s (fd:%d) %s %s %d %02d:%02d:%02d %d\n", item.address, item.memSize, item.otherInfo ,item.df , weekDay, month, item.memTime.tm_mday, item.memTime.tm_hour, item.memTime.tm_min, item.memTime.tm_sec, (item.memTime.tm_year + 1900));
		}
	}else 
	if(!strcmp(memType, "shared")){
		printf("******List of shared blocks assigned for the process %d\n", getpid());
		for(p=first(l); p != NULL ; p = next(p, l)){
			memItem item = getItem(p, l);
			printWeekDay(item.memTime.tm_wday, &weekDay);
			printMonth(item.memTime.tm_mon, &month);
			if(!strcmp(item.memType, "shared"))
				printf("%p: size:%zd. shared memory (key: %d) %s %s %d %02d:%02d:%02d %d\n", item.address, item.memSize, item.df , weekDay, month, item.memTime.tm_mday, item.memTime.tm_hour, item.memTime.tm_min, item.memTime.tm_sec, (item.memTime.tm_year + 1900));
		}
	}else 
	if(!strcmp(memType, "all")){
		printf("******List of blocks assigned for the process %d\n", getpid());
		for(p=first(l); p != NULL ; p = next(p, l)){
			memItem item = getItem(p, l);
			printWeekDay(item.memTime.tm_wday, &weekDay);
			printMonth(item.memTime.tm_mon, &month);
			if(!strcmp(item.memType, "shared"))
				printf("%p: size:%zd. shared memory (key: %d) %s %s %d %02d:%02d:%02d %d\n", item.address, item.memSize, item.df , weekDay, month, item.memTime.tm_mday, item.memTime.tm_hour, item.memTime.tm_min, item.memTime.tm_sec, (item.memTime.tm_year + 1900));
			if(!strcmp(item.memType, "mmap"))
				printf("%p: size:%zd. mmap %s (fd:%d) %s %s %d %02d:%02d:%02d %d\n", item.address, item.memSize, item.otherInfo ,item.df , weekDay, month, item.memTime.tm_mday, item.memTime.tm_hour, item.memTime.tm_min, item.memTime.tm_sec, (item.memTime.tm_year + 1900));
		    if(!strcmp(item.memType, "malloc"))
				printf("%p: size:%zd. malloc %s %s %d %02d:%02d:%02d %d\n", item.address, item.memSize, weekDay, month, item.memTime.tm_mday, item.memTime.tm_hour, item.memTime.tm_min, item.memTime.tm_sec, (item.memTime.tm_year + 1900));	
		}
	}
}

void cmd_autores (char *tr[])
{
    printf("The authors are:\n");

    if (tr[0]!=NULL && !strcmp(tr[0], "-n"))
        printf("David Garcia Ramallal\nAlfredo Javier Freire Bouzas\n");
    else if (tr[0]!=NULL && !strcmp(tr[0], "-l"))
        printf("david.ramallal@udc.es\njavier.freire.bouzas@udc.es\n");
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
    else if(chdir(tr[0])==-1)
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
        printf("autores [-l|-n]\npid [-p]\ncarpeta [direct]\nfecha [-d|-h]\nhist [-c|-N]\ncomando N\ninfosis\nayuda [cmd]\nfin\nsalir\nbye\ncrear [-f ] name\nborrar name1 name2 ...\nborrarrec name1 name2 ...\nlistfich [-long] [-link] [-acc] name1 name2 name3 ...\nlistdir [-reca] [-recb] [-hid] [-long] [-link] [-acc] name1 name2 ...\nmalloc [-free] [tam]\nmmap [-free] fich [perm]\nshared [-free|-create|-delkey] cl [tam]\ndealloc [-malloc|-shared|-mmap] ...\nmemoria [-blocks] [-vars] [-funcs] [-all] [-pmap]\nvolcarmem addr [cont]\nllenarmem addr [cont] [byte]\nrecursiva n\ne-s read fich addr cont\ne-s write [-o] fich addr cont\npriority [pid] [value]\nrederr [-reset] fich\nentorno [-environ]\nmostrarvar VAR1\ncambiarvar [-a|-e|-p] VAR VALUE\nuid -get|-set [-l] id\nfork\nejec prog arg1 arg2 ...\nejecpri prio prog arg1 arg2 ...\nfg prog arg1 arg2 ...\nfgpri prio prog arg1 arg2 ...\nback prog arg1 arg2 ...\nbackpri prio prog arg1 arg2 ...\nejecas login prog arg1 arg2 ...\nfgas login prog arg1 arg2 ...\nbgas login prog arg1 arg2 ...\nlistjobs\njob [-fg] id\nborrarjobs -term|-sig|-all|-clear\n");
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
        else if (tr[0]!=NULL && !strcmp(tr[0], "crear"))
            printf("crear [-f ] name\nCreates a file or directory in the file system\nname is the name of the file (or directory) to be created.\ncreate -f name creates an empty file\ncreate name creates a new directory\nIf name is not given, the name of the current working directory will be printed\n");
        else if (tr[0]!=NULL && !strcmp(tr[0], "borrar"))
            printf("borrar name1 name2 ...\nDeletes files and/or empty directories\nIf no name is given, the name current working directory will be printed\n");
        else if (tr[0]!=NULL && !strcmp(tr[0], "borrarrec"))
            printf("borrarrec name1 name2 ...\nDeletes files and/or non empty directories with all of their content\nIf no name is given, the name of the current working directory will be printed\n");
        else if (tr[0]!=NULL && !strcmp(tr[0], "listfich"))
            printf("listfich [-long] [-link] [-acc] name1 name2 name3 ...\nGives info on files (or directories, or devices ... ) name1, name2 ... in ONE LINE per file\nIf no options are given, it prints the size and the name of each file.\nIf no name is given, the name current working directory will be printed\nlistfich -long stands for long listing\nlistfich -link is only meaningful for long listings: if the file is a symbolic link the name of the file it points to is also printed\nlistfich -acc last access time will be used instead of last modification time\n");
        else if (tr[0]!=NULL && !strcmp(tr[0], "listdir"))
            printf("listdir [-reca] [-recb] [-hid] [-long] [-link] [-acc] name1 name2 ...\nLists the contents of directories with names name1, name2 ...\nIf any of name1, name2 ... is not a directory, info on it will be printed as with command listfich\nIf no name is given, the name current working directory will be printed\nlistdir -long stands for long listing\nlistdir -link is only meaningful for long listings: if the directory is a symbolic link, the name of the directory it points to is also printed\nlistdir -acc last access time will be used instead of last modification time\nlistdir -hid hidden files and/or directories will also get listed\nlistdir -reca when listing a directory's contents, subdirectories will be listed recursively after all the files in the directory.\n(if the -hid option is also given, hidden subdirectories will also get listed)\nlistdir -recb when listing a directory's contents, subdirectories will be listed recursively before all the files in the directory.\n(if the -hid option is also given, hidden subdirectories will also get listed)\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "malloc"))
			printf("malloc [-free] [tam]\nThe shell allocates tam bytes using malloc and shows the memory address returned by malloc\nIf used as malloc -free [tam] The shell deallocates one of the blocks of size tam that has been allocated with the command malloc.\nIf no such block exists or if tam is not specified, the command will show the list of addresses allocated with the malloc command\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "mmap"))
			printf("mmap [-free] fich [perm]\nMaps in memory the file fich (all of its length starting at offset 0) and shows the memory address where the file has been mapped\nIf fich is not specified, the command will show the list of addresses allocated with the mmap command\nmmap -free fich Unmaps and closes the file fich and removes the address where it was mapped from the list\nIf fich has been mapped several times, only one of the mappings will be undone\nIf the file fich is not mapped by the process or if fich is not specified,\nthe command will show the list of addresses (and size, and time . . . ) allocated with the mmap command\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "shared"))
			printf("shared [-free|-create|-delkey ] cl [tam]\nGets shared memory of key cl, maps it in the proccess address space and shows\nthe memory address where the shared memory has been mapped\nIf -create is not given, it is assumed that key cl is in use in the system\nso a new block of shared memory is not created\nIf no cl is specified, the command will show the list of addresses (and size, and time . . . )\nallocated with the shared command\nshared -create cl tam Creates a shared memory block of key cl, and size tam, maps\nit in the proccess address space and shows the memory address where the shared memory has been mapped\nshared -free cl Detaches the shared memory block with key cl from the process’ address\nspace ad eliminates its address from the list. If shared memory block with key cl\nhas been attached several times, ONLY one of them is detached\nIf cl is not specified, the command will show the list of addresses\n(and size, and time . . . ) allocated with the shared command\nshared -delkey cl Removes the shared memory region of key\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "dealloc"))
			printf("dealloc [-malloc|-shared|-mmap] ...\ndeallocates one of the memory blocks allocated with the command malloc, mmap or shared and removes it\nfrom the list. If no arguments are given, it prints a list of the allocated memory blocks\ndealloc -malloc size Does exactly the same as malloc -free size\ndealloc -shared cl Does exactly the same as shared -free cl\ndealloc -mmap file Does exactly the same as mmap -free file\nThis does the same (albeit with a different parameter) as malloc -free,\nshared -free or mmap -free deppending on addr\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "memoria"))
			printf("memoria [-blocks] [-vars] [-funcs] [-all] [-pmap]\nShows addresses inside the process memory space. If no arguments are given, is equivalent to -all\nmemoria -blocks shows the list of addresses (and size, and time . . . ) allocated with the malloc, shared and mmap\nmemoria -vars Prints the memory addresses of nine variables of the shel:\nthree extern (global) initializad variables, three static initialized and three automatic (local) initialized variables\nmemoria -funcs Prints the memory addresses of three program functions of the shell and three C library functions\nused in the shell program\nmemoria -all does the equivalent to -blocks, -vars and -funcs together\nmemoria -pmap Calls the program pmap for the shell process\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "volcarmem"))
			printf("volcarmem addr [cont]\nShows the contents of cont bytes starting at memory address addr\nIf cont is not specified, it shows 25 bytes\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "llenarmem"))
			printf("llenarmem addr [cont] [byte]\nFills cont bytes of memory starting at address addr with the value ’byte’\nIf ’byte’ is not specified, the value of 65 is assumed\nIf cont is not specified, the value of 128 is assume\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "recursiva"))
			printf("recursiva n\nCalls a recursive function passing the integer n as its parameter\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "e-s"))
			printf("e-s\ne-s read fich addr cont Reads cont bytes from file fich into memory address addr. If\ncont is not specified ALL of fich is read onto memory address addr\ne-s write [-o] fich addr cont Writes (using ONE write system call ) cont bytes from memory address addr into\nfile fich. If file fich does not exist it gets created; if it already exists it is\nnot overwritten unless “-o” (overwrite) is specified\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "priority"))
			printf("priority [pid] [value]\nWhile pid and value are specified, it changes the value of process pid to value\nIf value is not given, it shows the priority of process pid\nIf neither pid nor value are specified, the priority of the process executing the shell is shown\n");
 		else if (tr[0]!=NULL && !strcmp(tr[0], "rederr"))  
			printf("rederr [-reset] fich\nrederr file Redirects the standard error of the shell to file fich\nrederr Shows where the standard error is currently going to\nrederr -reset Restores the standard error to what it was originally\n");
   		else if (tr[0]!=NULL && !strcmp(tr[0], "entorno"))
			printf("entorno [-environ]\nShows the environment of the shell process\nentorno Shows all the environment variables of the shell process.Access will be through the third argument of main\nentorno -environ Shows all the environment variables of the shell process. Access will be through the external variable environ\nentorno -addr Shows the value (as pointers) of environ and the third argument of main. Shows also the addresses at with they are stored\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "mostrarvar"))
			printf("mostrarvar VAR\nShows the value of environment variable VAR\nIf no var is specified, it shows all the environment variables of the shell process\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "cambiarvar"))
			printf("cambiarvar [-a|-e|-p] VAR VALUE\nChanges the value of the environment var VAR to value\n-a means access through main’s third argument\n-e means access through environ\n-p means access through the library function putenv\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "uid"))
			printf("uid -get|-set [-l] id\nuid -get ro simply uid Prints the real and effective user credentials of the process running the shell\nuid -set [-l] id Establishes the efective user id of the shell process. id represents the uid. f -l is given id represents the login. If no arguments are given to uid -set, this command behaves exactly as uid -get\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "fork"))
			printf("fork\nThe shell creates a child process with fork and waits for it to end\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "ejec"))
			printf("ejec prog arg1 arg2 ...\nExecutes, without creating a process, he program prog with its arguments\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "ejecpri"))
			printf("ejecpri prio prog arg1 arg2 ...\nDoes the same as ejec, but before executing prog it changes the priority of the proccess to prio\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "fg"))
			printf("fg prog arg1 arg2 ...\nThe shell creates a process that executes in foreground the program prog with its arguments\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "fgpri"))   
			printf("fgpri prio prog arg1 arg2 ...\nDoes the same as fg, but before executing prog it changes the priority of the proccess that executes prog to prio\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "back"))
			printf("back prog arg1 arg2 ...\nThe shell creates a process that executes in background the program prog with its arguments\n");
		else if (tr[0]!=NULL && !strcmp(tr[0], "backpri"))   
			printf("backpri prio prog arg1 arg2 ...\nDoes the same as back, but before executing prog it changes the priority of the proccess that executes prog to prio\n");
 		else if (tr[0]!=NULL && !strcmp(tr[0], "ejecas"))   
			printf("ejecas login prog arg1 arg2 ...\nTries to execute as user login the program and arguments prog arg1 arg2 ...\n");
 		else if (tr[0]!=NULL && !strcmp(tr[0], "fgas"))
			printf("fgas login prog arg1 arg2 ...\nCreates a process that tries to execute as user login the program and arguments prog arg1 arg2 ...\n");
 		else if (tr[0]!=NULL && !strcmp(tr[0], "bgas"))
			printf("bgas login prog arg1 arg2 ...\nCreates a process that tries to execute in the backgroud and as user login the program and arguments prog arg1 arg2 ...\n");
 		else if (tr[0]!=NULL && !strcmp(tr[0], "listjobs"))
			printf("listjobs\nShows the list of background processes of the shell\n");
 		else if (tr[0]!=NULL && !strcmp(tr[0], "job"))
			printf("job [-fg] id\nShows information on process pid\nIf pid is not given or if pid is not a background process from the shell, this comand does exactly the same as the comand listjobs\nIf we supply the argument -fg, process with pid pid must be brought to the foreground\n");
 		else if (tr[0]!=NULL && !strcmp(tr[0], "borrarjobs"))
			printf("borrarjobs -term|-sig|-all|-clear\nborrarjobs -term Removes from the list the processes that have exited normally\nborrarjobs -sig Removes from the list the processes that have been terminated by a signal\nborrarjobs -all Removes from the list all the processes that have finished (exited normally or by a signal)\nborrarjobs -clear Empties the list of background processes\n");
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

void cmd_crear(char *tr[]) 
{
    char dir[MAXLINEA];
    
    if ((tr[0] == NULL) || (!strcmp(tr[0], "-f") && (tr[1] == NULL))) 
        printf("%s\n", getcwd(dir, MAXLINEA));
    else if (!strcmp(tr[0], "-f")){
		if(open(tr[1], O_CREAT, S_IRWXU) == -1)
			printf("It is not possible to create %s: %s\n", tr[1], strerror(errno));
    }else{
		if(mkdir(tr[0], 0777) == -1)
			printf("It is not possible to create %s: %s\n", tr[0], strerror(errno));
	}
}

void cmd_borrar(char *tr[])
{
	int i;
    char dir[MAXLINEA];
    
    if (tr[0] == NULL)
        printf("%s\n", getcwd(dir, MAXLINEA));
    else{
    	for (i=0; tr[i] != NULL; i++){
    		delete(tr[i]);
    	}
	}
}

void cmd_borrarrec(char *tr[])
{
    char dir[MAXLINEA];
    struct stat buffer;
    int i;
    
    if (tr[0] == NULL) 
        printf("%s\n", getcwd(dir, MAXLINEA));
    else{ 
		for (i=0; tr[i] != NULL; i++){
			if(!strcmp(tr[i], ".") || !strcmp(tr[i], ".."))
				continue;
			else{
				if(lstat(tr[i], &buffer) == -1)
					printf("It is not possible to delete %s: %s\n", tr[i], strerror(errno));
				else if (!S_ISREG(buffer.st_mode) && !S_ISLNK(buffer.st_mode))
					deleteRec(tr[i]);
				else
					delete(tr[i]);  
			}    
		} 
    }
}

void cmd_listfich(char *tr[]){	
	char dir[MAXLINEA];
	struct stat buffer;
	int i, j, countTRUE = 0;
	bool linK = false, lonG = false, acC = false;	
	
	for(j = 0; tr[j] != NULL; j++){
		if(!strcmp(tr[j], "-link")){
			linK = true;
		 countTRUE ++;
		}if(!strcmp(tr[j], "-long")){
			lonG = true;
			 countTRUE++;
		}if(!strcmp(tr[j], "-acc")){
			acC = true;	
			 countTRUE++;
		 }	
	}	 
    if ((tr[0] == NULL) || ((lonG || acC) && (tr[1] == NULL))) 
        printf("%s\n", getcwd(dir, MAXLINEA));
    else if (!lonG && !acC){ 
		for (i=countTRUE; tr[i] != NULL; i++){
			if(lstat(tr[i], &buffer) == -1)
				printf("It is not possible to access %s: %s\n", tr[i], strerror(errno));
			else 
				printFILE(tr[i], linK, lonG, acC, false);
		} 
	}else if(lonG || acC){
		for (i=countTRUE; tr[i] != NULL; i++){
			printFILE(tr[i], linK, lonG, acC, false);
		}
	}	
}	

void cmd_listdir(char *tr[]){
	char dir[MAXLINEA];
	getcwd(dir, MAXLINEA);
	struct stat buffer;
	int i = 0, j = 0, countTRUE = 0;
	bool lonG = false, acC = false, hiD = false, linK = false, recA = false, recB = false;
	
	for(j = 0; tr[j] != NULL; j++){
		if(!strcmp(tr[j], "-long")){
			lonG = true;
			countTRUE++;
		 }else if(!strcmp(tr[j], "-link")){
			linK = true;
			countTRUE++;
		}else if(!strcmp(tr[j], "-acc")){
			acC = true;
			countTRUE++;
		 }else if(!strcmp(tr[j], "-hid")){
			hiD = true;
			countTRUE++;
		}else if(!strcmp(tr[j], "-reca")){
			recA = true;
			countTRUE++;
		}else if(!strcmp(tr[j], "-recb")){
			recB = true;
			countTRUE++;
		}
	}	
	if(recB){
		for (i=countTRUE; tr[i] != NULL; i++){
			lstat(tr[i], &buffer);
			if(S_ISDIR(buffer.st_mode)){
				printREC(tr[i], lonG, linK, acC, hiD, recA, recB);
				printLISTDIR(tr[i], lonG, linK, acC, hiD);
			}else
				printFILE(tr[i], linK, lonG, acC, hiD);
		}
	}else if(recA){
		for (i=countTRUE; tr[i] != NULL; i++){
			lstat(tr[i], &buffer);
			if(S_ISDIR(buffer.st_mode)){
				printLISTDIR(tr[i], lonG, linK, acC, hiD);
				printREC(tr[i], lonG, linK, acC, hiD, recA, recB);
			}else
				printFILE(tr[i], linK, lonG, acC, hiD);
		}
	}else if (tr[countTRUE] == NULL)
        printf("%s\n", dir);
    else{ 
		for (i=countTRUE; tr[i] != NULL; i++){
			lstat(tr[i], &buffer);
			if(S_ISDIR(buffer.st_mode))
				printLISTDIR(tr[i], lonG, linK, acC, hiD);
			else
				printFILE(tr[i], linK, lonG, acC, hiD);
		}
	}
}

void cmd_malloc(char *tr[]){
	size_t size;
	char *address = "";
	memItem item;
	item.memType = malloc(sizeof(char *));
	time_t t;
    struct tm * timeinfo;
    time(&t);
    timeinfo = localtime(&t);
    memItem freeItem;
	
	if((tr[0] == NULL) || (!strcmp(tr[0], "-free") && tr[1] == NULL )){
		printMemList("malloc", *memList);
	}else{
		if(!strcmp(tr[0], "-free")){
			size = atoi(tr[1]);
			if(findItemSize(size, *memList) != NULL){
				freeItem = getItem(findItemSize(size, *memList), *memList);
				address = freeItem.address;
				deleteAtPosition(findItemSize(size, *memList), memList);
				printf("deallocated %zd at %p\n", size, address);	
				free(address);	
			}else
				printMemList("malloc", *memList);
		}else{
			size = atoi(tr[0]);
			address = malloc(size);	
			item.address = address;
			item.memSize = size;
			item.memTime = *timeinfo;
			strcpy(item.memType, "malloc");
			item.otherInfo = NULL;
			item.df = 0;
			printf("allocated %zd at %p\n", size, address);
			insertItem(item, memList);
		}
	}	
}

void * MmapFichero (char *fichero, int protection){
	int df, map=MAP_PRIVATE,modo=O_RDONLY;
	struct stat s;
	void *p;
	memItem item;
	time_t t;
    struct tm * timeinfo;
    time(&t);
    timeinfo = localtime(&t);
    item.otherInfo = malloc(sizeof(char *));  
    item.memType = malloc(sizeof(char *));
	
	if (protection&PROT_WRITE) modo = O_RDWR;
	if (stat(fichero,&s)==-1 || (df=open(fichero, modo))==-1)
		return NULL;
	if ((p=mmap (NULL,s.st_size, protection,map,df,0)) == MAP_FAILED)
		return NULL;
	
	item.address = p;
	item.df = df;
	strcpy(item.otherInfo, fichero);
	item.memSize = s.st_size;
	item.memTime = *timeinfo;
	strcpy(item.memType, "mmap");
	
	insertItem(item, memList);
	
	return p;
}

void cmd_mmap(char *tr[]){
	char *perm;
	memItem freeItem;
	char *address = "";
	void *p;
	int protection=0;
	if((tr[0] == NULL) || (!strcmp(tr[0], "-free") && tr[1] == NULL )){
				printMemList("mmap", *memList);
		return;
	}else{
		if(!strcmp(tr[0], "-free")){
			if(findItemOtherInfo(tr[1], *memList) != NULL){
				freeItem = getItem(findItemOtherInfo(tr[1], *memList), *memList);
				address = freeItem.address;
				munmap(address, freeItem.memSize);
				deleteAtPosition(findItemOtherInfo(tr[1], *memList), memList);
			}else{
				printMemList("mmap", *memList);				
			}			
			return;
		}else if ((perm=tr[1])!= NULL && strlen(perm)<4) {
			if (strchr(perm,'r')!=NULL) protection|=PROT_READ;
			if (strchr(perm,'w')!=NULL) protection|=PROT_WRITE;
			if (strchr(perm,'x')!=NULL) protection|=PROT_EXEC;
		}
	}
	if ((p=MmapFichero(tr[0],protection))==NULL)
		perror ("mmap not possible");
	else{
		printf ("file %s mapped at %p\n", tr[0], p);
	}
}

void SharedDelkey (char *args[]) /*arg[0] points to a str containing the key*/
{
	key_t clave;
	int id;
	char *key=args[1];
	if (key==NULL || (clave=(key_t) strtoul(key,NULL,10))==IPC_PRIVATE){
		printf ("shared -delkey valid key\n");
		return;
	}
	if ((id=shmget(clave,0,0666))==-1){
		perror ("shmget: imposible to obtain shared memory");
		return;
	}
	if (shmctl(id,IPC_RMID,NULL)==-1)
		perror ("shmctl: imposible to delete shared memory");
}

void * ObtenerMemoriaShmget (key_t clave, size_t tam){
	void * p;
	int aux,id,flags = 0777;
	struct shmid_ds s;
	
	memItem item;
	time_t t;
    struct tm * timeinfo;
    time(&t);
    timeinfo = localtime(&t);  
    item.memType = malloc(sizeof(char *));
    
	if (tam)
		flags=flags | IPC_CREAT | IPC_EXCL;
	if (clave==IPC_PRIVATE)
		{errno=EINVAL; return NULL;}
	if ((id=shmget(clave, tam, flags))==-1)
		return (NULL);
	if ((p=shmat(id,NULL,0))==(void*) -1){
		aux=errno;
		if (tam)
			shmctl(id,IPC_RMID,NULL);
		errno=aux;
		return (NULL);
	}
	shmctl (id,IPC_STAT,&s);

	item.address = p;
	item.df = clave;
	item.otherInfo = NULL;
	item.memSize = s.shm_segsz;
	item.memTime = *timeinfo;
	strcpy(item.memType, "shared");
	
	insertItem(item, memList);

	return (p);
}

void cmd_shared(char *tr[]){
	key_t k;
	size_t tam = 0;
	void *p;
	char * address;
	memItem freeItem;
	tMemPos pos;
	if (tr[0]==NULL || ((!strcmp(tr[0], "-free") || !strcmp(tr[0], "-create")) && tr[1] == NULL) || (!strcmp(tr[0], "-create") && tr[1] != NULL && tr[2] == NULL)){
		printMemList("shared", *memList); 
		return;
	}else if(!strcmp(tr[0], "-delkey") && tr[1] == NULL){
		printf("\tshared -delkey needs valid key\n");
		return;
	}else{
		if(!strcmp(tr[0], "-free")){
			if((pos = findItemKey(atoi(tr[1]), *memList)) != NULL){
				freeItem = getItem(pos, *memList);
				address = freeItem.address;
				shmdt(address);
				deleteAtPosition(findItemKey(atoi(tr[1]), *memList), memList);
				printf("Shared memory block at %p (key %d) has been dealocated\n", address, freeItem.df);
			}else
				printMemList("shared", *memList); 
			return;
		}else if (!strcmp(tr[0], "-delkey")){
			SharedDelkey(tr);
			return;
		}	
	}	
	if(!strcmp(tr[0], "-create"))
		k=(key_t) atoi(tr[1]);
	else
		k=(key_t) atoi(tr[0]);
		
	if(!strcmp(tr[0], "-create") && tr[2] != NULL)
		tam=(size_t) atoll(tr[2]);
	else if(tr[1] != NULL)
		tam=(size_t) atoll(tr[1]);
	
	if ((p=ObtenerMemoriaShmget(k,tam))==NULL)
		perror ("Cannot allocate");
	else
		printf ("Allocated shared memory (key %d) at %p\n",k,p);	
}

void cmd_dealloc(char *tr[]){
	memItem freeItem;
	size_t size;
	tMemPos pos;
	char * address = malloc(sizeof(char *));
	if(tr[0]==NULL || ((!strcmp(tr[0], "-malloc") || !strcmp(tr[0], "-mmap") || !strcmp(tr[0], "-shared")) && tr[1] == NULL)){
		printMemList("all", *memList); 
		return;
	}else
	if((strcmp(tr[0], "-malloc") || strcmp(tr[0], "-mmap") || strcmp(tr[0], "-shared")) && tr[1] == NULL){
		if(findItem(tr[0], *memList) != NULL){
			freeItem = getItem((findItem(tr[0], *memList)), *memList);
			sprintf(address, "%p", freeItem.address);
			if(!strcmp(freeItem.memType, "malloc")){
				deleteAtPosition(findItem(address, *memList), memList);
				printf("block at address %s deallocated (malloc)\n", address);
				free(address);
			}else if(!strcmp(freeItem.memType, "mmap")){
				deleteAtPosition(findItem(address, *memList), memList);
				printf("block at address %s deallocated (mmap)\n", address);
				munmap(address, freeItem.memSize);
			}else if(!strcmp(freeItem.memType, "shared")){
				deleteAtPosition(findItem(address, *memList), memList);
				printf("block at address %s deallocated (shared)\n", address);
				shmdt(address);
			}			
		}else{
			printMemList("all", *memList);
			return;
		}		
	}else
	if(!strcmp(tr[0], "-malloc")){
			size = atoi(tr[1]);
			if(findItemSize(size, *memList) != NULL){
				freeItem = getItem(findItemSize(size, *memList), *memList);
				address = freeItem.address;
				deleteAtPosition(findItemSize(size, *memList), memList);
				printf("block at address %p deallocated (malloc)\n", address);	
				free(address);
			}else
				printMemList("malloc", *memList);
		}else
	if(!strcmp(tr[0], "-mmap")){
		if(findItemOtherInfo(tr[1], *memList) != NULL){
			freeItem = getItem(findItemOtherInfo(tr[1], *memList), *memList);
			address = freeItem.address;
			deleteAtPosition(findItemOtherInfo(tr[1], *memList), memList);
			printf("block at address %p deallocated (mmap)\n", address);
			munmap(address, freeItem.memSize);
			}else
				printMemList("mmap", *memList);				
	}else
	if(!strcmp(tr[0], "-shared")){
			if((pos = findItemKey(atoi(tr[1]), *memList)) != NULL){
				freeItem = getItem(pos, *memList);
				address = freeItem.address;
				shmdt(address);
				deleteAtPosition(findItemKey(atoi(tr[1]), *memList), memList);
				printf("Shared memory block at %p (key %d) has been dealocated\n", address, freeItem.df);
			}else
				printMemList("shared", *memList); 
		}
}

void dopmap (void){ 
	pid_t pid;
	char elpid[32];
	char *argv[3]={"pmap",elpid,NULL};
	sprintf (elpid,"%d", (int) getpid());
	if ((pid=fork())==-1){
		perror ("Imposible crear proceso");
		return;
	}
	if (pid==0){
		if (execvp(argv[0],argv)==-1)
		perror("cannot execute pmap");
		exit(1);
	}
	waitpid (pid,NULL,0);
}

void cmd_memoria(char *tr[]){
	double doubleExample = 0.5;
	float floatExample = 1.5;
	int intExample = 3;
	static int intStExample = 0;
	static float floatStExample = 5.5;
	static double doubleStExample = 7.8;
	if(tr[0] == NULL || !strcmp(tr[0], "-all")){
		printf("Local variables%6s%p,%5s%p,%5s%p\n", "", &doubleExample, "", &floatExample, "", &intExample);
		printf("Global variables%5s%p,%5s%p,%5s%p\n", "", &L, "", memList, "", &C);
		printf("Static variables%5s%p,%5s%p,%5s%p\n", "", &intStExample, "",&floatStExample, "",&doubleStExample);
		printf("Program functions%4s%p,%5s%p,%5s%p\n", "", &cmd_autores, "",&cmd_ayuda, "",&cmd_infosis);
		printf("Library functions%4s%p,%5s%p,%5s%p\n", "", &strcmp, "",&printf, "",&waitpid);
		printMemList("all", *memList);
	}else if (!strcmp(tr[0], "-blocks")){
		printMemList("all", *memList);
	}else if (!strcmp(tr[0], "-vars")){
		printf("Local variables%6s%p,%5s%p,%5s%p\n", "", &doubleExample, "", &floatExample, "", &intExample);
		printf("Global variables%5s%p,%5s%p,%5s%p\n", "", &L, "", memList, "", &C);
		printf("Static variables%5s%p,%5s%p,%5s%p\n", "", &intStExample, "",&floatStExample, "",&doubleStExample);
		
	}else if(!strcmp(tr[0], "-funcs")){
		printf("Program functions%4s%p,%5s%p,%5s%p\n", "", &cmd_autores, "",&cmd_ayuda, "",&cmd_infosis);
		printf("Library functions%4s%p,%5s%p,%5s%p\n", "", &strcmp, "",&printf, "",&waitpid);
	}else if (!strcmp(tr[0], "-pmap")){
		dopmap();
	}
	
}


void cmd_volcarmem(char *tr[]){

	int count, i, j, numLines, totalCounter1 = 0, totalCounter2 = 0;
	void *init = (void *) strtol(tr[0], NULL, 16);
	unsigned char * addr1 = (unsigned char *) (init);
	unsigned char * addr2 = (unsigned char *) (init);
  
	if(tr[1]==NULL)
		count = 25;
	else
		count = atoi(tr[1]);
    
	numLines = count/25;
	if(count%25 != 0)
		numLines ++;
	
	printf("Tipping over %d bytes from the direction %s\n", count, tr[0]);
	for(i = 0; i<numLines; i++){
		for(j = 0; j < 25 && totalCounter1 < count; j++){
			if(isprint(*addr1) != 0)
				printf("  %c ", *addr1);
			else
				printf("    ");
			addr1 = addr1 + 1;
			totalCounter1 ++;
		}
		printf("\n");
		for(j = 0; j < 25 && totalCounter2 < count; j++){
			printf(" %02X ", *addr2);
			addr2 = addr2 + 1;
			totalCounter2 ++;
		}
		printf("\n");
	}
    
}

int hextoAscii(char ch1, char ch2){
	int big, sml;
	
	big = ((ch1 / 16) - 3)*10 + (ch1 % 16);
	if(big > 9) big --;
	
	sml = ((ch2 / 16) - 3)*10 + (ch2 % 16);
	if(sml > 9) sml --;
	
	return (big * 16 + sml);
	
}

void cmd_llenarmem(char *tr[]){
  
  void *addr = (void *) strtol(tr[0], NULL, 16);
  size_t cont;
  int byte;
  char *byteChar;
  byteChar = malloc(sizeof(char *));
  char ch;
  if(tr[1]==NULL){
    cont = 128;
    byte = 65;
  }else if(tr[2]==NULL){
    cont =  atoi(tr[1]);
    byte = 65; 
  }else if(tr[2]!=NULL){
    cont =  atoi(tr[1]);
    if(((int)tr[2][0]) == 39){
		ch = tr[2][1];
    strtol(&ch, &byteChar, 10);
    printf("Filling %zd bytes of memory with byte %c(%d) from address %s\n", cont, *byteChar, (int)*byteChar, tr[0]);
    memset(addr, (int)*byteChar, cont);
    return;
    }else if(((tr[2][0]) == '0') && ((tr[2][1]) == 'x')){
		byte = hextoAscii(tr[2][2], tr[2][3]);
	}else{
		byte = atoi(tr[2]);
	}
  }
  printf("Filling %zd bytes of memory with byte %c(%d) from address %s\n", cont, byte, byte, tr[0]);
  memset(addr, byte, cont);  
  
}

void recursiveFunct (int n)
{
	char automaticArray[4096];
	
	static char staticArray[4096];
	
	printf ("n parameter: %d at %p\n",n,&n);
	
	printf ("Static Array at %p \n", staticArray);
	
	printf ("Automatic Array at %p\n",automaticArray);
	
	n--;
	if (n>0){
		printf("--------------------------------\n");
		recursiveFunct(n);
	}
}

void cmd_recursiva(char *tr[]){
	if(tr[0]!=NULL)
		recursiveFunct(atoi(tr[0]));
}

ssize_t LeerFichero (char *fich, void *p, ssize_t n){
	ssize_t nleidos,tam=n; 
	int df, aux;
	struct stat s;
	if (stat (fich,&s)==-1 || (df=open(fich,O_RDONLY))==-1)
		return ((ssize_t)-1);
	if (n==LEERCOMPLETO)
		tam=(ssize_t) s.st_size;
	if ((nleidos=read(df,p, tam))==-1){
		aux=errno;
		close(df);
		errno=aux;
		return ((ssize_t)-1);
	}
	close (df);
	return (nleidos);
}

void WriteFichero (bool overWrite, char *fich, void *p, ssize_t n){
	int df;
	//struct stat s;
	
	if(!overWrite){
		if((df=open(fich, O_WRONLY | O_EXCL | O_CREAT, S_IRGRP | S_IRUSR | S_IWUSR | S_IROTH |  S_IWGRP)) == -1){
			printf("Impossible to write the file: File exists\n");
			return;
		}
	}else{
		if(open(fich, O_WRONLY | O_EXCL | O_CREAT, S_IRGRP | S_IRUSR | S_IWUSR | S_IROTH |  S_IWGRP) == -1){
			if(overWrite)
				fclose(fopen(fich, "w"));
		}
		if((df=open(fich, O_CREAT | O_WRONLY, S_IRGRP | S_IRUSR | S_IWUSR | S_IROTH |  S_IWGRP)) == -1){
			printf("Impossible to write the file: File exists\n");
			return;
		}
	}
	if(write(df,p, n)==-1)
		printf("Impossible to write the file: File exists\n");
	
	 printf("%zd bytes write of %s in %p\n", n, fich, (char*)p);
	
	close (df);

}

void cmd_es(char *tr[]){
	char *fich; 
	void *init;
	ssize_t count;
	ssize_t total;
	bool overWrite = false;
	if(!strcmp(tr[0], "read")){
		if(tr[1] == NULL || tr[2] == NULL){
			printf("Parameters are needed\n");
			return;
		}else{
			fich = tr[1];
			init = (void *) strtol(tr[2], NULL, 16);
			if(tr[3] != NULL)
				count = atoi(tr[3]);
			else
				count = -1;
		}
	total = LeerFichero(fich, init, count);
	printf("%zd bytes read of %s in %s\n", total, tr[1], tr[2]);
		
	}else if(!strcmp(tr[0], "write")){
		if((tr[1] == NULL || tr[2] == NULL || tr[3] == NULL)){
			printf("Parameters are needed\n");
			return;
		}else if(!strcmp(tr[1], "-o") && (tr[2] == NULL || tr[3] == NULL || tr[4] == NULL)){
			printf("Parameters are needed\n");
			return;
		}
		
		if(!strcmp(tr[1], "-o")){
			overWrite = true;
			fich = tr[2];
			init = (void *) strtol(tr[3], NULL, 16);
			count = atoi(tr[4]);
			WriteFichero(overWrite, fich, init, count);
		}else{
			fich = tr[1];
			init = (void *) strtol(tr[2], NULL, 16);
			count = atoi(tr[3]);
			WriteFichero(overWrite, fich, init, count);
		}		
	}
}

void MostrarEntorno (char **entorno, char * nombre_entorno)
{
	int i=0;
	while (entorno[i]!=NULL) {
		printf ("%p->%s[%d]=(%p) %s\n", &entorno[i],
		nombre_entorno, i,entorno[i],entorno[i]);
		i++;
	}
}

int BuscarVariable (char * var, char *e[])
{
	int pos=0;
	char aux[MAXVAR];
	strcpy (aux,var);
	strcat (aux,"=");
	while (e[pos]!=NULL)
		if (!strncmp(e[pos],aux,strlen(aux)))
			return (pos);
		else
			pos++;
	errno=ENOENT;
	/*no hay tal variable*/
	return(-1);
}

int CambiarVariable(char * var, char * valor, char *e[])
{
	int pos;
	char *aux;
	if ((pos=BuscarVariable(var,e))==-1)
		return(-1);
	if ((aux=(char *)malloc(strlen(var)+strlen(valor)+2))==NULL)
		return -1;
	strcpy(aux,var);
	strcat(aux,"=");
	strcat(aux,valor);
	e[pos]=aux;
	return (pos);
}

void cmd_priority(char *tr[]){
	int which = PRIO_PROCESS;
	pid_t pid;
	pid = getpid();
	errno = 0;
	
	if(tr[0]==NULL)
		printf("Priority of process %d is %d\n", pid, getpriority(which,pid));
	else if((tr[0]!=NULL) && (tr[1]==NULL)){
		if ((getpriority(which, atoi(tr[0]))) == -1 && errno) 
			printf("It is not possible to obtain priority of process %d: %s\n", atoi(tr[0]), strerror(errno));
		else
			printf("Priority of process %d is %d\n", atoi(tr[0]), getpriority(which,atoi(tr[0])));
	}
	else if ((tr[0]!=NULL) && (tr[1]!=NULL)){
		if (((setpriority(which, atoi(tr[0]), atoi(tr[1])))) == -1 && errno)
			printf("It is not possible to change priority of process %d: %s\n", atoi(tr[0]), strerror(errno));
	}		
}

void cmd_rederr(char *tr[]){
	char *file = malloc(sizeof(char *));
	int df;
	if(tr[0]==NULL)
		printf("standard error in %s\n", currentFile);
	else if (!strcmp(tr[0], "-reset")){
		dup2(STDOUT_FILENO, STDERR_FILENO);
		sprintf(currentFile, "original configuration fich");
	}else{
		strcpy(file, tr[0]);
		if((df=open(file, O_WRONLY | O_EXCL | O_CREAT, S_IRGRP | S_IRUSR | S_IWUSR | S_IROTH |  S_IWGRP)) == -1){
			printf("Impossible to redirect: File exists\n");
			return;
		}
		dup2(df, STDERR_FILENO);
		strcpy(currentFile, file);
	}
	free(file);
}

void cmd_entorno(char *tr[]){
	extern char ** environ;
	char * env_name = "environ";
	char * main_name = "main arg3";
	
	if(tr[0]==NULL)
		MostrarEntorno(entorno_main, main_name); 
	else if(!strcmp(tr[0], "-environ"))
		MostrarEntorno(environ, env_name);
	else if(!strcmp(tr[0], "-addr")){
		printf("environ: %p (stored in %p)\n", *environ, environ);							
		printf("main arg3: %p (stored in %p)\n", *entorno_main, entorno_main);	
	}		
}

void cmd_mostrarvar(char *tr[]){
	extern char ** environ;
	char * main_name = "main arg3";
	int i;
	
	if(tr[0]==NULL)
		MostrarEntorno(entorno_main, main_name); 
	else if ((i = BuscarVariable(tr[0], entorno_main)) != -1) {
		printf("With arg3 main %s (%p) @%p\n", entorno_main[i], entorno_main[i], &entorno_main[i]);
		printf("With environ %s (%p) @%p\n", environ[i], environ[i], &environ[i]);
		printf("With getenv %s (%p)\n", getenv(tr[0]), getenv(tr[0]));
	}else if ((i = BuscarVariable(tr[0], environ)) != -1) {
		printf("With environ %s (%p) @%p\n", environ[i], environ[i], &environ[i]);
		printf("With getenv %s (%p)\n", getenv(tr[0]), getenv(tr[0]));
	}
}

void cmd_cambiarvar(char *tr[]){
	extern char ** environ;
	char *nameValue;
	nameValue=(char *)malloc(strlen(tr[1])+strlen(tr[2])+2);
	
	if(tr[0]==NULL)
		printf("Use: cambiarvar [-a|-e|-p] var value\n");
	else if ((tr[1]!=NULL) && (tr[2]!=NULL)){
		strcat(nameValue, tr[1]);
		strcat(nameValue, "=");
		strcat(nameValue, tr[2]);
		if(!strcmp(tr[0], "-a")){
			if (CambiarVariable(tr[1], tr[2], entorno_main) == -1)
				printf("Impossible to change variable: %s\n", strerror(errno));
		}else if(!strcmp(tr[0], "-e")){
			if (CambiarVariable(tr[1], tr[2], environ) == -1)
				printf("Impossible to change variable: %s\n", strerror(errno));
		}else if(!strcmp(tr[0], "-p")){
			putenv(nameValue);
		}
	}
}

char * NombreUsuario (uid_t uid){
	struct passwd *p;
	if ((p=getpwuid(uid))==NULL)
		return (" ??????");
	return p->pw_name;
}

uid_t UidUsuario (char * nombre){
	struct passwd *p;
	if ((p=getpwnam (nombre))==NULL)
		return (uid_t) -1;
	return p->pw_uid;
}

void MostrarUidsProceso (void){
	uid_t real=getuid(), efec=geteuid();
	printf ("Real credential: %d, (%s)\n", real, NombreUsuario (real));
	printf ("Effective credential: %d, (%s)\n", efec, NombreUsuario (efec));
}

int CambiarUidLogin (char * login){
	uid_t uid;
	if ((uid=UidUsuario(login))==(uid_t) -1){
		printf("login not valid: %s\n", login);
		return -1;
	}
	if (setuid(uid)==-1){
		printf ("Impossible to change the credential: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

void cmd_uid(char *tr[]){
	
	if(tr[0]==NULL || !strcmp(tr[0], "-get"))
		MostrarUidsProceso();
	else if(!strcmp(tr[0], "-set")){
		if(tr[1]==NULL)
			MostrarUidsProceso();
		else if(!strcmp(tr[1], "-l"))
			CambiarUidLogin(tr[2]);
		else{
			if (setuid(atoi(tr[1]))==-1)
				printf ("Impossible to change the credential: %s\n", strerror(errno));
		}
	}
}

void cmd_fork(char *tr[]){
	pid_t pidChild;
	
	if((pidChild = fork()) == -1){
		printf("It is not possible to use fork: %s\n", strerror(errno));
		return;
	}else if(pidChild != 0)
				printf("Executing process %d\n", pidChild);
	waitpid(pidChild, NULL, 0);
}

void cmd_ejec(char *tr[]){
	if(tr[0]!=NULL){								
		if (execvp(tr[0], tr)==-1){
			perror ("Cannot execute");
			return;
		}
		exit(255);
	}
}

void cmd_ejecpri(char *tr[]){
	if(tr[0]!=NULL && tr[1]!=NULL){
		int which = PRIO_PROCESS;
		setpriority(which, getpid(), atoi(tr[0]));
		if (execvp(tr[1], tr+1)==-1){
			perror ("Cannot execute");
			return;
		}
		exit(255);
	}
}

void cmd_fg(char *tr[]){
	int pid;
	if ((pid=fork())==0){
		if (execvp(tr[0], tr)==-1)	
			perror ("Cannot execute");
		exit(255);
	}
	waitpid (pid,NULL,0);
}

void cmd_fgpri(char *tr[]){
	int pid, which = PRIO_PROCESS;
	if ((pid=fork())==0){			
		setpriority(which, pid, atoi(tr[0]));
		if (execvp(tr[1], tr+1)==-1)					
			perror ("Cannot execute");
		exit(255);
	}
	waitpid (pid,NULL,0);
}


void concatComm(char *tr[], char *comm, int i){
	strcpy(comm, "");
	
	while(tr[i]!=NULL){
		strcat(comm, tr[i]);
		strcat(comm, " ");
		i++;
	}
	
}

int returnState(pid_t itemPid, char *stat){
	int state;
	waitpid(itemPid, &state, WUNTRACED | WCONTINUED);
	
	if(WIFCONTINUED(state)){
		strcpy(stat, "RUNNING");
		return 0;
	}else if(WIFEXITED(state)){
		strcpy(stat, "TERMINATED NORMALLY");
		return WEXITSTATUS(state);
	}else if(WIFSTOPPED(state)){
		strcpy(stat, "STOPPED");
		return WSTOPSIG(state);
	}else if(WIFSIGNALED(state)){
		strcpy(stat, "TERMINATED BY SIGNAL");
		return WTERMSIG(state);
	}
	return -1;
}

void createJob(pid_t itemPid, char *tr[], int i){
	jobItem newJob;
	newJob.pid = itemPid;
	newJob.priority = getpriority(PRIO_PROCESS, itemPid);
	newJob.user = malloc(sizeof(char *));
	strcpy(newJob.user, NombreUsuario(getuid()));
	newJob.comm = malloc(sizeof(char *));
	concatComm(tr, newJob.comm, i);
	newJob.state = malloc(sizeof(char *));
	strcpy(newJob.state, "NULL");
	newJob.time = time(NULL);
	newJob.retrn = malloc(sizeof(int));
	*newJob.retrn = 0;
	insertJobItem(newJob, jobLst);
}

void updateJob(tJobPos updateJobPos, pid_t itemPid){
	jobItem updatedJob = getJobItem(updateJobPos, *jobLst);
	updatedJob.priority = getpriority(PRIO_PROCESS, itemPid);
	strcpy(updatedJob.user, NombreUsuario(getuid()));
	*updatedJob.retrn = returnState(itemPid, updatedJob.state);
	updateJobItem(updatedJob, updateJobPos, jobLst);
}

void cmd_back(char *tr[]){
	if(tr[0]==NULL){
		printf("Introduce the process to execute\n");
		return;
	}
	int pid;
	if ((pid=fork())==0){
		if (execvp(tr[0], tr)==-1)
				perror ("Cannot execute");	
			exit(255);
	}
	createJob(pid, tr, 0);
}

void cmd_backpri(char *tr[]){
	if(tr[0]==NULL || tr[1]==NULL){
		printf("Introduce the process to execute\n");
		return;
	}
	int pid, which = PRIO_PROCESS;
	if ((pid=fork())==0){
		setpriority(which, pid, atoi(tr[0]));
		if (execvp(tr[1], tr+1)==-1)
			perror ("Cannot execute");
		exit(255);
		
	}
	createJob(pid, tr, 1);
}

void cmd_ejecas(char *tr[]){
	if(tr[0]!=NULL && tr[1]!=NULL){
		if(CambiarUidLogin(tr[0]) == 0){
			if (execvp(tr[1], tr+1)==-1){
				perror ("Cannot execute");
				return;
			}
			exit(255);
		}
	}
}

void cmd_fgas(char *tr[]){
	int pid;
	if ((pid=fork())==0){			
		if(CambiarUidLogin(tr[0]) == 0){
			if (execvp(tr[1], tr+1)==-1)					
				perror ("Cannot execute");
			exit(255);
		}
	}
	waitpid (pid,NULL,0);
}

void cmd_bgas(char *tr[]){
	if(tr[0]==NULL || tr[1]==NULL){
		printf("Introduce the process to execute\n");
		return;
	}
	int pid;
	if ((pid=fork())==0){
		if(CambiarUidLogin(tr[0]) == 0){
			if (execvp(tr[1], tr+1)==-1)
				perror ("Cannot execute");
			exit(255);
		}	
	}
	createJob(pid, tr, 1);
}

void lstJobs(){
	if(isEmptyJobList(*jobLst))
		return;
	tJobPos p;
	for( p=jobFirst(*jobLst); p!=NULL; p = jobNext(p, *jobLst) )
	{
		jobItem prntItem = getJobItem(p, *jobLst);
		if(strcmp(prntItem.state, "TERMINATED NORMALLY")){
			updateJob(p, prntItem.pid);
		}
		struct tm tm = *localtime(&prntItem.time);
		printf(" %d %s p=%d %04d/%02d/%02d %02d:%02d:%02d %s ", prntItem.pid, prntItem.user, prntItem.priority, (tm.tm_year + 1900), (tm.tm_mon + 1), tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, prntItem.state);
		
		if(!strcmp(prntItem.state, "TERMINATED BY SIGNAL"))
			printf("(%s) ", NombreSenal(*prntItem.retrn));
		else
			printf("(%03d) ", *prntItem.retrn);
		
		printf("%s\n", prntItem.comm);
	
	//faltan cousas por facer!!!
	}
}

void cmd_listjobs(char *tr[]){
	lstJobs();
}

void cmd_job(char *tr[]){
	if(tr[0] == NULL)
		lstJobs();
	else if(!strcmp(tr[0], "-fg")){
		//está sin facer
	}else{
		tJobPos p = findItemPid(atoi(tr[0]), *jobLst);
		if(p!=NULL){
			jobItem prntItem = getJobItem(p, *jobLst);
			if(!strcmp(prntItem.state, "NULL") || strcmp(prntItem.state, "TERMINATED NORMALLY")){
				*prntItem.retrn = returnState(prntItem.pid, prntItem.state);
			}
			struct tm tm = *localtime(&prntItem.time);
			printf(" %d %s p=%d %04d/%02d/%02d %02d:%02d:%02d %s (%03d) %s\n", prntItem.pid, prntItem.user, prntItem.priority, (tm.tm_year + 1900), (tm.tm_mon + 1), tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, prntItem.state, *prntItem.retrn, prntItem.comm);
		}else
			lstJobs();
	}
	
}

void cmd_borrarjobs(char *tr[]){
	if(tr[0]==NULL || isEmptyJobList(*jobLst))
		return;	
	tJobPos p, next;
	jobItem jbItm;
	
	if(!strcmp(tr[0], "-term")){
		p=jobFirst(*jobLst);
		while(p!=NULL){
			next = jobNext(p, *jobLst);
			jbItm = getJobItem(p, *jobLst);
			if(!strcmp(jbItm.state, "TERMINATED NORMALLY"))
				deleteAtJobPosition(p, jobLst);
			p = next;
		}	
	}else if(!strcmp(tr[0], "-sig")){
		p=jobFirst(*jobLst);
		while(p!=NULL){
			next = jobNext(p, *jobLst);
			jbItm = getJobItem(p, *jobLst);
			if(!strcmp(jbItm.state, "TERMINATED BY SIGNAL"))
				deleteAtJobPosition(p, jobLst);
			p = next;
		}
	}else if(!strcmp(tr[0], "-all")){
		p=jobFirst(*jobLst);
		while(p!=NULL){
			next = jobNext(p, *jobLst);
			jbItm = getJobItem(p, *jobLst);
			if(!strcmp(jbItm.state, "TERMINATED BY SIGNAL")||!strcmp(jbItm.state, "TERMINATED NORMALLY"))
				deleteAtJobPosition(p, jobLst);
			p = next;
		}
	}else if(!strcmp(tr[0], "-clear")){
		p=jobFirst(*jobLst);
		while(p!=NULL){
			next = jobNext(p, *jobLst);
			deleteAtJobPosition(p, jobLst);
			p = next;
		}	
	}
}

int trocearCadena (char *cadena, char *trozos[])
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

int main(int argc, char *argv[], char *envp[])
{
    char linea[MAXLINEA];
    char *tr[MAXLINEA/2];
    char aux[MAXLINEA];
    CreateCmdList(L);
    memList = malloc(sizeof(tMemList));
    jobLst = malloc(sizeof(tJobList));
    currentFile = malloc(MAXLINEA*sizeof(char));
    sprintf(currentFile, "original configuration fich");
    
    for (int i = 0; envp[i] != NULL; i++)
         entorno_main[i] = envp[i];
    
    createEmptyMemList(memList);
    createEmptyJobList(jobLst);

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
