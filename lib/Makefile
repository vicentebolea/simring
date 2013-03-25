CXXFLAGS = -Wall -g -I.
sources := $(wildcard *.cc) 
objects := $(patsubst %.cc, %.o, $(sources))
.SUFFIXES: .o .cc

.cc.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(OPTIONS)

all: $(objects) 
	-rm libuniDQP.a
	$(AR) -rv libuniDQP.a $(objects)
	-rm $(objects)

$(objects) : $(sources)
