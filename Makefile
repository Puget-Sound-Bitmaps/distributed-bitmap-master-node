BIN := bin

all: clean .master .dbms

.start_dbms:
	@cd bin && ./dbms

tree_map.o:
	@echo "Compiling tree map"
	@cd consistent-hash/ring/src && gcc -c -Wall tree_map.c -o ../../../$(BIN)/tree_map.o

clean:
	@if test -d $(BIN); then rm -rf $(BIN)/*; else mkdir $(BIN); fi

.dbms:
	@echo "Compiling DBMS"
	@cd dbms && gcc dbms.c -o ../$(BIN)/dbms

.rpc:
	@echo "Compiling RPC modules"
	@cd rpc && make

.master: tree_map.o .rpc
	@echo "Compiling Master"
	@cd master && gcc -c -Wall ../rpc/bin/rq_xdr.o ../rpc/bin/rq_clnt.o master_rq.c -o ../$(BIN)/master_rq.o
	@echo "compiling master main"
	@cd master && gcc -Wall ../$(BIN)/tree_map.o ../$(BIN)/master_rq.o master.c -o ../bin/master -lssl -lcrypto -lm
