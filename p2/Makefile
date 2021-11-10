
CFLAGS=-Wall -g

p2: p2.o CmdList.o MemList.o
	gcc $(CFLAGS) p2.o CmdList.o MemList.o -o p2
	
p2.o: p2.c CmdList.h MemList.h
	gcc $(CFLAGS) -c p2.c
	
CmdList.o: CmdList.c CmdList.h
	gcc $(CFLAGS) -c CmdList.c
	
MemList.o: MemList.c MemList.h
	gcc $(CFLAGS) -c MemList.c
	
clean:
	rm p2 *.o
