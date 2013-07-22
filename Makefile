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

CXX = g++
MAKE = make
AR = ar
CXXFLAGS = -I./lib/ -L./lib/ -luniDQP -Wall -g -std=gnu++98
OPTIONS = -D__STDC_FORMAT_MACROS

#Experiments parameters
OPTIONS += 	-DALPHA=0.03f
OPTIONS += 	-DCACHESIZE=10000
OPTIONS += 	-DDATA_MIGRATION=1

#others parameters
OPTIONS += 	-DLOT=1024

POLICY = -DDATA_MIGRATION

export POLICY CXX CXXFLAGS MAKE AR OPTIONS
.PHONY: lib dist node docs

all: lib src
	$(MAKE) -C src/

lib:
	@echo building a static library!!
	$(MAKE) -C lib/ -j16

src:
	$(MAKE) -C src/

clean:
	-$(MAKE) -C lib/ clean
	-$(MAKE) -C src/ clean

dist: clean
	tar -cvzf simulator_simple_`date +"%d-%m-%y"`.tar.gz ./*

docs:
	cd docs; doxygen
