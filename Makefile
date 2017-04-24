CC=gcc
LIBSOCKET=-lnsl
CCFLAGS=-Wall -g
SRV=server
SEL_SRV=selectserver
CLT=client
RCLT=run-client
RSRV=run-srv
PORT=2000
ADR=127.0.0.1

all: $(SEL_SRV) $(CLT)

$(SEL_SRV):$(SEL_SRV).c
	$(CC) -o $(SEL_SRV) $(LIBSOCKET) $(SEL_SRV).c

$(CLT):	$(CLT).c
	$(CC) -o $(CLT) $(LIBSOCKET) $(CLT).c

run-client: $(CLT)
	./$(CLT) $(ADR) $(PORT)

run-server: $(SEL_SRV)
	./$(SEL_SRV) $(PORT)

clean:
	rm -f *.o *~
	rm -f $(SEL_SRV) $(CLT)
	rm -f *.log


