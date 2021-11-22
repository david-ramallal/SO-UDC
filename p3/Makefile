
CFLAGS=-Wall -g

shell: shell.o CmdList.o MemList.o
	gcc $(CFLAGS) shell.o CmdList.o MemList.o -o shell
	
shell.o: shell.c CmdList.h MemList.h
	gcc $(CFLAGS) -c shell.c
	
CmdList.o: CmdList.c CmdList.h
	gcc $(CFLAGS) -c CmdList.c
	
MemList.o: MemList.c MemList.h
	gcc $(CFLAGS) -c MemList.c
	
clean:
	rm shell *.o
