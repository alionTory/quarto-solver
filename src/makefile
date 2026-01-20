OPTIONS = -O3 -std=c++17
all: main.o Board.o negamax.o MonteCarlo.o
	g++ $(OPTIONS) -o QuartoCppCode.out main.o Board.o negamax.o MonteCarlo.o
main.o: main.cpp
	g++ $(OPTIONS) -c main.cpp
Board.o: Board.cpp
	g++ $(OPTIONS) -c Board.cpp
negamax.o: negamax.cpp
	g++ $(OPTIONS) -c negamax.cpp
MonteCarlo.o: MonteCarlo.cpp
	g++ $(OPTIONS) -c MonteCarlo.cpp
clean:
	rm *.o QuartoCppCode.out
