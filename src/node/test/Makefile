INCLUDE := -I./../../../lib/
INCLUDE += -I./../
LIBDIR  := -L./../../../lib/
LIB     := -l uniDQP -l pthread
OBJECT  := common.cc
OBJECT  += main.cc

all: main common random

main:
	g++ -c main.cc -g -Wall -std=gnu++98 $(INCLUDE)

common:
	g++ -c common.cc -g -Wall -std=gnu++98 $(INCLUDE)

random:
	g++ -c random_number.cc -g -Wall -std=gnu++98 $(INCLUDE)
	g++ -g -Wall -o node_test_rn ../node.o common.o random_number.o main.o $(INCLUDE) $(LIBDIR) $(LIB) 

same:
	g++ -c same_number.cc -g -Wall -std=gnu++98 $(INCLUDE)
	g++ -g -Wall -o node_test_sn ../node.o common.o same_number.o main.o $(INCLUDE) $(LIBDIR) $(LIB) 

clean:
	-rm *.o node_test
