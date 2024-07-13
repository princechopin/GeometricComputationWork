all : FindEar.o
	g++ FindEar.cpp -std=c++11 -o FindEar
clean : 
	rm FindEar
