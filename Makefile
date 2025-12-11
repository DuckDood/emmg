all: obj/ build/ obj/main.cpp.o build/emmg 
OSMODE := l

obj/: 
ifeq (${OSMODE}, l)
	mkdir -p obj
else
	mkdir obj
endif

build/: 
ifeq (${OSMODE}, l)
	mkdir -p build
else
	mkdir build
endif

obj/main.cpp.o: src/main.cpp 
ifeq (${OSMODE}, l)
	${CXX} src/main.cpp -c -o obj/main.cpp.o
else
	${CXX} src/main.cpp -c -o obj/main.cpp.o
endif


build/emmg: obj/main.cpp.o 
ifeq (${OSMODE}, l)
	${CXX} obj/main.cpp.o -o build/emmg
else
	${CXX} obj/main.cpp.o -o build/emmg
endif


clean:
	rm -r obj
	rm -r build
.PHONY: clean

ifeq (${OSMODE}, l)
install:
	install build/emmg /usr/local/bin/
.PHONY: install
endif


gencommands:
	mkdir emmgtemp
ifeq (${OSMODE}, l)
	clang src/main.cpp -c  -MJ emmgtemp/0.json -fdriver-only
else
	clang src/main.cpp -c  -MJ emmgtemp/0.json -fdriver-only
endif
# not cross platform here sad i think
	echo [ > emmgtemp/[
	echo ] > emmgtemp/]
	cat emmgtemp/[ emmgtemp/*.json emmgtemp/] > compile_commands.json
	rm -r emmgtemp
