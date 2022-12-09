DIR_PATH=.
CXX=g++
LDLIBS=-lpthread
CLIENT=$(wildcard ./client/*.cpp)
SERVER=$(wildcard ./server/*.cpp)
COBJECTS=$(CLIENT:.cpp=.o)
SOBJECTS=$(SERVER:.cpp=.o)

all: client_cpp smalldb_cpp

client_cpp: $(COBJECTS)
	$(CXX) -o $@ $(COBJECTS)

smalldb_cpp: $(SOBJECTS)
	$(CXX) -o $@ $(SOBJECTS)

	
/%.o: %.cpp %.hpp
	$(CXX) -c $<

.PHONY: clean

clean:
	-rm $(wildcard */*.o)