BIN := bin
RPC_BIN := rpc/bin

all: clean .master .dbms

.start_dbms:
	@$(bin)/dbms

tree_map.o:
	@echo "Compiling tree map"
	@gcc -c -Wall consistent-hash/ring/src/tree_map.c -o $(BIN)/tree_map.o

clean:
	@if test -d $(BIN); then rm -rf $(BIN)/*; else mkdir $(BIN); fi

.dbms:
	@echo "Compiling DBMS"
	@gcc dbms/dbms.c -o $(BIN)/dbms

.rpc:
	@echo "Compiling RPC modules"
	@cd rpc && make

.master: tree_map.o .rpc
	@echo "Compiling Master"
	@gcc -c -Wall $(RPC_BIN)/rq_xdr.o $(RPC_BIN)/rq_clnt.o master/master_rq.c -o $(BIN)/master_rq.o
	@echo "compiling master main"
	@gcc -Wall $(BIN)/tree_map.o $(BIN)/master_rq.o master/master.c -o $(BIN)/master -lssl -lcrypto -lm
