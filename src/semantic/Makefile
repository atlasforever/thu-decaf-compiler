.PHONY:all clean subdirs

pwd = $(shell pwd)

CXXARGS+= -I$(pwd) -I$(pwd)/scope/ -I$(pwd)/symbol/ -I$(pwd)/type/
export

all: subdirs SymbolChecker.o TypeChecker.o
	cp *.o $(OUTPUT)

subdirs:
	$(MAKE) -C scope/
	$(MAKE) -C symbol/
	$(MAKE) -C type/

SymbolChecker.o: SymbolChecker.cpp SymbolChecker.h
	$(CXX) $(CXXARGS) $< -o $@

TypeChecker.o: TypeChecker.cpp TypeChecker.h
	$(CXX) $(CXXARGS) $< -o $@

clean:
	rm -rf *.o
	$(MAKE) -C scope/ clean
	$(MAKE) -C symbol/ clean
	$(MAKE) -C type/ clean
