CXX = g++
MAKE = make
AR = ar
CXXFLAGS = -I./lib/ -L./lib/ -luniDQP -Wall -g 
OPTIONS = -D__STDC_FORMAT_MACROS

#Experiments parameters
OPTIONS += 	-DALPHA=0.03f
OPTIONS += 	-DCACHESIZE=100000

#others parameters
OPTIONS += 	-DLOT=1024

.PHONY: lib dist node docs

all: lib src

lib:
	@echo building a static library!!
	$(MAKE) -C lib/ 

src:
	$(MAKE) -C src/

clean:
	-rm lib/*.o bin/scheduler_{bdema,hash} bin/node lib/libuniDQP.a

dist: clean
	tar -cvzf simulator_simple_`date +"%d-%m-%y"`.tar.gz ./*

docs:
	cd docs; doxygen
