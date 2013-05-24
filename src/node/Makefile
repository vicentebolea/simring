INCLUDE := -I./../../lib/
LIBDIR  := -L./../../lib/
LIB     := -l uniDQP -l pthread

all: clean compile
	g++ -g -Wall -o node node.o main.o $(INCLUDE) $(LIBDIR) $(LIB) 

compile:
	g++ -c node.cc -g -Wall -std=gnu++98 $(INCLUDE)
	g++ -c main.cc -g -Wall -std=gnu++98 $(INCLUDE)

clean:
	-rm *.o node
