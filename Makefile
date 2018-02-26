BIN := bin

all: ring_consistent_hash dbms master

tree_map.o:
	@cd consistent-hash/ring/src && gcc -c -Wall tree_map.c -o ../../../$(BIN)/tree_map.o

clean:
	@if test -d $(BIN); then rm -rf $(BIN)/*; else mkdir $(BIN); fi

dbms: dbms.c
	@echo "Compiling DBMS"
	@cd dbms && gcc dbms.c -o $(BIN)/dbms

master: tree_map.o master.c
	@echo "Compiling Master"
	@cd master
	@gcc $(BIN)/tree_map.o master.c -o $(BIN)/master
	#@gcc master_tpc_vector.c -o $(BIN)/master.o
