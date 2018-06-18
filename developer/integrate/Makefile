################################################################################
# Test Project Makefile
################################################################################

TARGET=test

CXX=g++
CXXFLAGS=-g -O0 -std=c++14 -I. -I/home/bergen/packages/flecsi-mpi/include

OBJECTS=main.o

all: ${TARGET}

%.o: %.cc control.h context.h dag.h specialization.h
	${CXX} ${CXXFLAGS} -c $< -o $@

${TARGET}: ${OBJECTS}
	${CXX} ${CXXFLAGS} -o ${TARGET} ${OBJECTS}

clean:
	rm -rf ${OBJECTS} ${TARGET}
