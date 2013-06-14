INCLUDE := -I./../../lib/ -I./
LIBDIR  := -L./../../lib/
LIB     := -l simring

all: clean compile
	g++ -g -Wall -o scheduler scheduler.o $(INCLUDE) $(LIBDIR) $(LIB) 

compile:
	g++ -c scheduler.cc -g -Wall -std=gnu++98 $(INCLUDE) -D_DEBUG

clean:
	-rm *.o node
