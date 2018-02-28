BIN := bin
RPC_BIN := rpc/bin

CFLAGS := -Wall
# Ignore incompatible-pointer-types warnings from `rpcgen`
# Ignore unused-command-line-argument stemming from "'linker' input unused"
CIGNORE := -Wno-incompatible-pointer-types -Wno-unused-command-line-argument
CC := gcc $(CFLAGS) $(CIGNORE)

all: clean .master .slave .dbms

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

.engine:
	@echo "Compiling Bitmap Engine Pieces"
	@$(CC) -c -o $(BIN)/WAHQuery.o \
		../bitmap-engine/BitmapEngine/src/wah/WAHQuery.c
	@$(CC) -c -o $(BIN)/SegUtil.o \
		../bitmap-engine/BitmapEngine/src/seg-util/SegUtil.c

.slave: .rpc .engine
	@echo "Compiling Slave"
	@$(CC) -c -o $(BIN)/slave_rq.o slave/slave_rq.c
	@echo "Compiling Slave Main"
	@$(CC) -o $(BIN)/slave \
		$(RPC_BIN)/rq_svc.o \
		$(RPC_BIN)/rq_xdr.o \
		$(BIN)/slave_rq.o \
		$(BIN)/WAHQuery.o \
		$(BIN)/SegUtil.o \
		-lssl -lcrypto -lm -lpthread

.dbms:
	@echo "Compiling DBMS"
	@$(CC) -o $(BIN)/dbms dbms/dbms.c

.start_dbms:
	@$(bin)/dbms
