PHNOY:all
all: sever client
client:client.c
	gcc -o $@ $^
sever:sever.c
	gcc -o $@ $^
.PHNOY:clean
clean:
	rm -f sever client
