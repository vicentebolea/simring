include Makefile.vars

.PHONY: lib dist node docs

all: lib node
	$(CXX) src/scheduler.cpp -o bin/scheduler_bdema $(CXXFLAGS) $(OPTIONS) -DBDEMA
	$(CXX) src/scheduler.cpp -o bin/scheduler_hash $(CXXFLAGS) $(OPTIONS) -DHASH 

lib:
	@echo building a static library!!
	$(MAKE) -C lib/ 

node:
	-$(CXX) src/appServer.cpp -o bin/appserver -lpthread $(CXXFLAGS) $(OPTIONS)

clean:
	-rm lib/*.o bin/scheduler_{bdema,hash} bin/appserver lib/libuniDQP.a

dist: clean
	tar -cvzf simulator_simple_`date +"%d-%m-%y"`.tar.gz ./*

docs:
	cd docs; doxygen
