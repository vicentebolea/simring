CXXFLAGS = -Wall -g -I.
sources := $(wildcard *.cpp) 
objects := $(patsubst %.cpp, %.o, $(sources))
.SUFFIXES: .o .cpp

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(OPTIONS)

all: $(objects) 
	-rm libuniDQP.a
	$(AR) -rv libuniDQP.a $(objects)
	-rm $(objects)

$(objects) : $(sources)
