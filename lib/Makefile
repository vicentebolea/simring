CXXFLAGS = -Wall -g -I.
sources := $(wildcard *.cc) 
objects := $(patsubst %.cc, %.o, $(sources))
.SUFFIXES: .o .cc

.cc.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(OPTIONS)

all: $(objects) 
	-rm libsimring.a
	$(AR) -rv libsimring.a $(objects)
	-rm $(objects)

$(objects) : $(sources)
