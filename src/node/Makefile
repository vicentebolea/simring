INCLUDE := -I./../../lib/ -I./
INCLUDE += -I./../common/
LIBDIR  := -L./../../lib/
LIBDIR  += -L./../common/
LIB     := -l simring -l pthread

all: clean compile
	g++ -g -Wall -o node node.o main.o $(INCLUDE) $(LIBDIR) $(LIB) 

compile:
	g++ -c node.cc -g -Wall -std=gnu++98 $(INCLUDE)
	g++ -c main.cc -g -Wall -std=gnu++98 $(INCLUDE) $(OPTIONS)

clean:
	-rm *.o node
