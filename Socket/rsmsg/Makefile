PHNOY:all
all:client sever
client:client.c
	gcc -o $@ $^
sever:sever.c
	gcc -o $@ $^
.PHNOY:clean
clean:
	rm -f client sever
