all: clean cspp 

cspp: csp.cpp
	g++ -g -O2 -Wall -pedantic -std=c++11 -o cspp csp.cpp -lelf 

clean:
	rm -f *o cspp
