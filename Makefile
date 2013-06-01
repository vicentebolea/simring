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

all: lib node
	$(CXX) src/scheduler.cc -o bin/scheduler_bdema $(CXXFLAGS) $(OPTIONS) -DBDEMA
	$(CXX) src/scheduler.cc -o bin/scheduler_hash $(CXXFLAGS) $(OPTIONS) -DHASH 

lib:
	@echo building a static library!!
	$(MAKE) -C lib/ 

node:
	-$(CXX) src/node.cc -o bin/node -lpthread $(CXXFLAGS) $(OPTIONS)

clean:
	-rm lib/*.o bin/scheduler_{bdema,hash} bin/node lib/libuniDQP.a

dist: clean
	tar -cvzf simulator_simple_`date +"%d-%m-%y"`.tar.gz ./*

docs:
	cd docs; doxygen
