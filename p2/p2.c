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
#include <errno.h>

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
void cmd_crear(char *tr[]);
void cmd_borrar(char *tr[]);
void cmd_borrarrec(char *tr[]);
void cmd_listfich(char *tr[]);
void cmd_listdir(char *tr[]);
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
        {"crear", cmd_crear},
        {"borrar", cmd_borrar},
        {"borrarrec", cmd_borrarrec},
        {"listfich",cmd_listfich},
        {"listdir", cmd_listdir},

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
        printf("autores [-l|-n]\npid [-p]\ncarpeta [direct]\nfecha [-d|-h]\nhist [-c|-N]\ncomando N\ninfosis\nayuda [cmd]\nfin\nsalir\nbye\ncrear [-f ] name\nborrar name1 name2 ...\nborrarrec name1 name2 ...\nlistfich [-long] [-link] [-acc] name1 name2 name3 ...\nlistdir [-reca] [-recb] [-hid] [-long] [-link] [-acc] name1 name2 ...\n");
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
