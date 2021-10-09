
CFLAGS=-Wall -g

p1: p1.o CmdList.o
	gcc $(CFLAGS) p1.o CmdList.o -o p1
	
p1.o: p1.c CmdList.h
	gcc $(CFLAGS) -c p1.c
	
CmdList.o: CmdList.c CmdList.h
	gcc $(CFLAGS) -c CmdList.c
	
clean:
	rm p1 *.o
