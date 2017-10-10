CXX = g++
CXXFLAGS = -Wall

driver1.out: driver1.o Sally.o
        $(CXX) $(CXXFLAGS) Sally.o driver1.o -o driver1.out

Sally.o: Sally.cpp Sally.h
        $(CXX) $(CXXFLAGS) -c Sally.cpp

driver1.o: driver1.cpp
        $(CXX) $(CXXFLAGS) -c Driver.cpp

clean:
        rm *.o
        rm *~

run:
        ./driver1.out