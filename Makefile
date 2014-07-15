
TOOLBOX = $(HOME)/Toolbox/c
CFLAGS = -c -Wall -I$(TOOLBOX)/include -I/opt/local/include
LDFLAGS = -L$(TOOLBOX)/lib -L/opt/local/lib -lcplcore -lcfitsio -lfftw3 -lm -lgsl -lgslcblas
CC = gcc
CXX = g++
INCLUDES = *.h
MODULES = cppgvspc_gen_p2vm cppgvspc

all: $(MODULES)

cppgvspc: cppgvspc.o gvspcSensor.o gvspcPix.o gvspcCsv.o gvspcV2PM.o gvspcCsv_c.o
	$(CXX) $(LDFLAGS) $^ -o $@

cppgvspc_gen_p2vm: cppgvspc_gen_p2vm.o gvspcSensor.o gvspcPix.o gvspcCsv.o gvspcCsv_c.o gvspcV2PM.o
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.c $(INCLUDES)
	$(CXX) $(CFLAGS) $< -o $@

%.o: %.cpp $(INCLUDES)
	$(CXX) $(CFLAGS) $< -o $@

clean:
	rm *.o
	rm $(MODULES)
