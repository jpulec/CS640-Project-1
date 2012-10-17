FLAGS=-g -Wall
COMP=gcc


sender: sender.c
	$(COMP) sender.c -o sender $(FLAGS)

clean:
	rm *.o
