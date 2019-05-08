gcc cronk.c -o cronk.o -c
gcc help.c -o help.o -c

gcc -o ./cronk cronk.o help.o

