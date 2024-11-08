EXE 		= converter

SRC 		= $(wildcard src/*.cpp)
OBJ 		= $(subst src, build, $(patsubst %.cpp, %.o, $(SRC)))

DBG 		= 

INCLUDE 	= -I ./include
LIB			= 
EXTRA		= -Wall -Werror -O3
STATIC  	= -static
FIX			= -Wl,-V


all: link

remake: clean all

run:
	$(EXE)

run!:
	$(EXE) --verbose

clean:
	rm $(OBJ)

build/%.o: src/%.cpp
	g++ $(INCLUDE) -c src/$*.cpp -o build/$*.o $(DBG) $(EXTRA)

link: $(OBJ)
	g++ $(OBJ) -o $(EXE) $(LIB) $(STATIC) $(DBG) $(EXTRA) $(FIX)

test:
	g++ test.cpp -o test $(LIB) $(STATIC) $(DBG) $(EXTRA) $(FIX)

