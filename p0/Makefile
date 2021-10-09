
CFLAGS=-Wall -g

p0: p0.o CmdList.o
	gcc $(CFLAGS) p0.o CmdList.o -o p0
	
p0.o: p0.c CmdList.h
	gcc $(CFLAGS) -c p0.c
	
CmdList.o: CmdList.c CmdList.h
	gcc $(CFLAGS) -c CmdList.c
	
clean:
	rm p0 *.o
