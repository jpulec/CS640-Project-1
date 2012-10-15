FLAGS=
COMP=gcc


sender: sender.c
	$(COMP) sender.c -o sender

clean:
	rm *.o
