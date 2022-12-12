DIR_PATH=.
CXX=g++
FLAGS=-std=c++17 -Wall -Wextra -Wpedantic -D_GNU_SOURCE -Werror=all -lpthread
LDLIBS=-lpthread
CLIENT=$(wildcard ./client/*.cpp)
SERVER=$(wildcard ./server/*.cpp)
COBJECTS=$(CLIENT:.cpp=.o)
SOBJECTS=$(SERVER:.cpp=.o)

all: sdbsh smalldb

sdbsh: $(COBJECTS)
	$(CXX) -o $@ $(COBJECTS) $(FLAGS)

smalldb: $(SOBJECTS)
	$(CXX) -o $@ $(SOBJECTS) $(FLAGS)

	
/%.o: %.cpp %.hpp
	$(CXX) -c $<

.PHONY: clean

clean:
	-rm $(wildcard */*.o)