BIN := bin
GEN := gen
SRC := src
TST := tst

CFLAGS := -w
CC := gcc $(CFLAGS)
RPCGEN := rpcgen -N

all: clean compile

################
# Remote Query #
################

RQ_CLNT = $(BIN)/rq_clnt
RQ_SVC = $(BIN)/rq_svc
RQ_XDR = $(BIN)/rq_xdr

RQ_BIN = $(RQ_CLNT) $(RQ_SVC) $(RQ_XDR)
RQ_GEN = $(GEN)/rq_clnt.c $(GEN)/rq_svc.c $(GEN)/rq_xdr.c $(GEN)/rq.h

$(RQ_GEN): $(SRC)/rq.x
	@cd $(GEN) && $(RPCGEN) ../$(SRC)/rq.x

$(RQ_CLNT): $(GEN)/rq_clnt.c $(GEN)/rq.h
	@echo "Compiling Remote-Query Client"
	@$(CC) -c -o $(RQ_CLNT) $(GEN)/rq_clnt.c

$(RQ_SVC): $(GEN)/rq_svc.c $(GEN)/rq.h
	@echo "Compiling Remote-Query Service"
	@$(CC) -c -o $(RQ_SVC) $(GEN)/rq_clnt.c

$(RQ_XDR): $(GEN)/rq_xdr.c $(GEN)/rq.h
	@echo "Compiling Remote-Query External Data Representation"
	@$(CC) -c -o $(RQ_XDR) $(GEN)/rq_clnt.c

####################
# Two-Phase Commit #
####################

TPC_CLNT = $(BIN)/tpc_clnt
TPC_SVC = $(BIN)/tpc_svc
TPC_XDR = $(BIN)/tpc_xdr

TPC_BIN = $(TPC_CLNT) $(TPC_SVC) $(TPC_XDR)
TPC_GEN = $(GEN)/tpc_clnt.c $(GEN)/tpc_svc.c $(GEN)/tpc_xdr.c $(GEN)/tpc.h

$(TPC_GEN): $(SRC)/tpc.x
	@cd $(GEN) && $(RPCGEN) ../$(SRC)/tpc.x

$(TPC_CLNT): $(GEN)/tpc_clnt.c $(GEN)/tpc.h
	@echo "Compiling Remote-Query Client"
	@$(CC) -c -o $(TPC_CLNT) $(GEN)/tpc_clnt.c

$(TPC_SVC): $(GEN)/tpc_svc.c $(GEN)/tpc.h
	@echo "Compiling Remote-Query Service"
	@$(CC) -c -o $(TPC_SVC) $(GEN)/tpc_clnt.c

$(TPC_XDR): $(GEN)/tpc_xdr.c $(GEN)/tpc.h
	@echo "Compiling Remote-Query External Data Representation"
	@$(CC) -c -o $(TPC_XDR) $(GEN)/tpc_clnt.c

###########
# General #
###########

compile: $(RQ_BIN) $(TPC_BIN)

clean:
	@echo "Cleaning..."
	@if test -d $(BIN); then rm -rf $(BIN)/*; else mkdir $(BIN); fi
	@if test -d $(GEN); then rm -rf $(GEN)/*; else mkdir $(GEN); fi
