
CFLAGS=-Wall -g

p2: p2.o CmdList.o
	gcc $(CFLAGS) p2.o CmdList.o -o p2
	
p2.o: p2.c CmdList.h
	gcc $(CFLAGS) -c p2.c
	
CmdList.o: CmdList.c CmdList.h
	gcc $(CFLAGS) -c CmdList.c
	
clean:
	rm p2 *.o
