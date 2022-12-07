CXXFLAGS+=-std=c++17
#CXX=g++
LDLIBS+=-lpthread
SOURCES = $(wildcard *.cpp */*.cpp)
HEADERS = $(wildcard *.hpp */*.hpp)
OBJ = db.o student.o common.o queries.o

.PHONY: main

main: smalldb client #check run

smalldb: smalldb.cpp ${OBJ}
	$(CXX) $(LDFLAGS) $^ -o $@ $(LOADLIBES) $(LDLIBS)

client: client.cpp $(OBJ)
	$(CXX) $(LDFLAGS) -o client client.cpp common.o

%.o: %.cpp %.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $^

.PHONY: clean

clean:
	-rm *.o
