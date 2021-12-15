all:
	gcc client.c -o client -lpthread
	gcc server.c -o server -lpthread

clean:
	rm -f client server *.txt