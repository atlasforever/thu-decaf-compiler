SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:%.cpp=%.o)

.PHONY:all clean

all: $(OBJS)
	cp *.o $(OUTPUT)

%.o: %.cpp
	$(CXX) $(CXXARGS) $< -o $@

clean:
	rm -rf *.o