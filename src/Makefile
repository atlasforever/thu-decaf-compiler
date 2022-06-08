all: subdirs main.o

pwd=$(shell pwd)
export LLVM_INCDIR=$(shell $(LLVM_CONFIG) --includedir)
CXXARGS+= -I$(pwd)/utils -I$(pwd) -I$(pwd)/parser -I$(pwd)/semantic -I$(LLVM_INCDIR)
export

.PHONY: subdirs
subdirs:
	$(MAKE) -C parser
	$(MAKE) -C semantic
	$(MAKE) -C codegen
	$(MAKE) -C utils

main.o: main.cpp
	$(CXX) $(CXXARGS) $^ -o $@
	cp $@ $(OUTPUT)/

clean:
	@$(MAKE) -C parser/ clean
	@$(MAKE) -C semantic/ clean
	@$(MAKE) -C codegen/ clean
	@$(MAKE) -C utils/ clean
	rm -rf *.o

