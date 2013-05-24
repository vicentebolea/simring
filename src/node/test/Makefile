INCLUDE := -I./../../../lib/
INCLUDE += -I./../
LIBDIR  := -L./../../../lib/
LIB     := -l uniDQP -l pthread

all: main random same

main:
	g++ -c main.cc -g -Wall -std=gnu++98 $(INCLUDE)

random:
	g++ -c random_number.cc -g -Wall -std=gnu++98 $(INCLUDE)
	g++ -g -Wall -o node_test_rn ../random_number.o main.o $(INCLUDE) $(LIBDIR) $(LIB) 

same:
	g++ -c same_number.cc -g -Wall -std=gnu++98 $(INCLUDE)
	g++ -g -Wall -o node_test_sn ../same_number.o main.o $(INCLUDE) $(LIBDIR) $(LIB) 

clean:
	-rm *.o node_test
