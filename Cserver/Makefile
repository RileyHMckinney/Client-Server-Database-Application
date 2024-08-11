CC = gcc
CFLAGS = -Wall -Werror -std=gnu99

.PHONY: all clean dbserver dbclient

main: dbserver dbclient

dbserver: dbserver.c
	$(CC) $(CFLAGS) -pthread $^ -o dbserver

dbclient: dbclient.c
	$(CC) $(CFLAGS) $^ -o dbclient

clean:
	rm -f dbserver dbclient

