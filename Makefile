export TOP_PATH=$(shell dirname $(abspath $(lastword $(MAKEFILE_LIST))))

export ANTLR_OUTPUT=$(TOP_PATH)/src/parser/antlr
ANTLR_INC=$(TOP_PATH)/lib/LIBANTLR4-4.9.1-Linux/include/antlr4-runtime/

export OUTPUT=$(TOP_PATH)/output

export CXX=clang++-13
export LLVM_CONFIG=llvm-config-13
export CXXARGS=-g -c -I$(ANTLR_INC) -I$(ANTLR_OUTPUT) -I$(TOP_PATH)/src -std=c++14
export LIBS=$(TOP_PATH)/lib/LIBANTLR4-4.9.1-Linux/lib/libantlr4-runtime.a
export LLVM_LDFLAGS=$(shell $(LLVM_CONFIG) --ldflags --libs core native)
BIN=decaf

all:
	mkdir -p $(OUTPUT)
	$(MAKE) -C src
	$(CXX) $(OUTPUT)/*.o $(LIBS) $(LLVM_LDFLAGS) -o $(BIN)

clean:
	@$(MAKE) -C $(TOP_PATH)/src clean
	rm -rf $(OUTPUT)
	rm -rf $(TOP_PATH)/$(BIN)
	rm -rf $(TOP_PATH)/test/PA1/output
	rm -rf $(TOP_PATH)/test/PA2/output
	rm -rf $(TOP_PATH)/test/PA3/output
