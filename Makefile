########################
# Policies List        #
# ===================  #
# - DATA_MIGRATION     #
# - LRU POP POLICY     #
# - PUSH POLICY        #
# - BDEMA              #
# - ROUND_ROBIN        #
#                      #
########################

CXX = gcc
MAKE = make
AR = ar

CXXFLAGS = -Wall -g -std=gnu++98 -rdynamic
INCLUDE = -I./lib/ -I./src/common/ -L./lib/ 
BINLIB  = -lstdc++ -lsimring

#Experiments parameters
OPTIONS = -D__STDC_FORMAT_MACROS
OPTIONS += 	-DALPHA=0.03f
OPTIONS += 	-DCACHESIZE=1000
OPTIONS += 	-DDATA_MIGRATION

POLICY = -DDATA_MIGRATION

export POLICY CXX CXXFLAGS MAKE AR OPTIONS INCLUDE BINLIB
.PHONY: lib dist node docs

all: src
	$(MAKE) -C src/

lib:
	@echo building a static library!!
	-$(MAKE) -C lib/ -j16

src:
	-$(MAKE) -C src/

clean:
	-$(MAKE) -C lib/ clean
	-$(MAKE) -C src/ clean

dist: clean
	tar -cvzf simulator_simple_`date +"%d-%m-%y"`.tar.gz ./*

docs:
	cd docs; doxygen
