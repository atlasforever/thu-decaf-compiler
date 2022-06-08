SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:%.cpp=%.o)

LLVM_FLAGS=$(shell $(LLVM_CONFIG) --cflags) -DLLVM_ENABLE_DUMP

.PHONY:all clean

all: $(OBJS)
	cp *.o $(OUTPUT)

%.o: %.cpp
	$(CXX) $(CXXARGS) $(LLVM_FLAGS) $< -o $@

clean:
	rm -rf *.o