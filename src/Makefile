DIR_PATH=.
CXX=g++
FLAGS=-std=c++17 -Wall -Wextra -Wpedantic -D_GNU_SOURCE -Werror=all -lpthread
LDLIBS=-lpthread
CLIENT=$(wildcard ./client/*.cpp)
SERVER=$(wildcard ./server/*.cpp)
COBJECTS=$(CLIENT:.cpp=.o)
SOBJECTS=$(SERVER:.cpp=.o)
COMOBJECTS=common.o

all: sdbsh smalldb

sdbsh: $(COBJECTS) $(COMOBJECTS)
	$(CXX) -o $@ $(COBJECTS) $(COMOBJECTS) $(FLAGS)

smalldb: $(SOBJECTS) $(COMOBJECTS)
	$(CXX) -o $@ $(SOBJECTS) $(COMOBJECTS) $(FLAGS)

	
/%.o: %.cpp %.hpp
	$(CXX) -c $<

.PHONY: clean

clean:
	-rm $(wildcard */*.o)