ssu_backup: main.o
	gcc -o ssu_backup main.o -lpthread

main.o: ssu_backup.h ssu_backup.c
	gcc -c -o main.o ssu_backup.c
