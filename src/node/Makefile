INCLUDE := -I./../../lib/
LIB     := -L./../../lib/

all: compile
	g++ -g -Wall -o node *.o $(INCLUDE) $(LIB) -l uniDQP,pthread

compile:
	g++ -c node.cc -g -Wall -std=gnu++98 $(INCLUDE) $(LIB)
	g++ -c main.cc -g -Wall -std=gnu++98 $(INCLUDE) $(LIB)

