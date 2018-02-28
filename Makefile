BIN := bin
RPC_BIN := rpc/bin

CFLAGS := -Wall
# Ignore incompatible-pointer-types warnings from `rpcgen`
# Ignore unused-command-line-argument stemming from "'linker' input unused"
CIGNORE := -Wno-incompatible-pointer-types -Wno-unused-command-line-argument
CC := gcc $(CFLAGS) $(CIGNORE)

all: clean .master .dbms

clean:
	@echo "Cleaning..."
	@if test -d $(BIN); then rm -rf $(BIN)/*; else mkdir $(BIN); fi

$(BIN)/tree_map.o:
	@echo "Compiling tree map"
	@$(CC) -c -o $(BIN)/tree_map.o \
		consistent-hash/ring/src/tree_map.c

.rpc:
	@echo "Compiling RPC modules"
	@cd rpc && make

.master: .rpc $(BIN)/tree_map.o
	@echo "Compiling Master"
	@$(CC) -c -o $(BIN)/master_rq.o \
		$(RPC_BIN)/rq_xdr.o $(RPC_BIN)/rq_clnt.o \
		master/master_rq.c
	@echo "Compiling master main"
	@$(CC) -o $(BIN)/master \
		$(BIN)/tree_map.o $(BIN)/master_rq.o \
		master/master.c \
		-lssl -lcrypto -lm

.dbms:
	@echo "Compiling DBMS"
	@$(CC) -o $(BIN)/dbms dbms/dbms.c

.start_dbms:
	@$(bin)/dbms
