#CXX = g++
CXX = mpicxx
CXXFLAGS = -Wall -W -s -O3 -march=native -std=c++11
LIBRARIES = -lpthread
.PHONY: default run
default: run

run:
	${CXX} ${CXXFLAGS} *.cpp ${LIBRARIES} -o program 
	
clean:
	rm -f *.o program
