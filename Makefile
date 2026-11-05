CXX = mpic++
CXXFLAGS = -std=c++2a -Wall

BOOST_INC = /usr/include
BOOST_LIB = /usr/lib


BOOST_LIBS = -L$(BOOST_LIB) -lboost_graph

all: cftp

cftp: cftp.o dynamic_programming.o glauber_dynamics.o random_trees.o
	$(CXX) $(CXXFLAGS) -I$(BOOST_INC) -o cftp cftp.o dynamic_programming.o glauber_dynamics.o random_trees.o $(BOOST_LIBS)

test: test.o dynamic_programming.o glauber_dynamics.o random_trees.o
	$(CXX) $(CXXFLAGS) -I$(BOOST_INC) -o test test.o dynamic_programming.o glauber_dynamics.o random_trees.o $(BOOST_LIBS)

# Object file compilation
cftp.o: cftp.cpp dynamic_programming.h glauber_dynamics.h random_trees.h
	$(CXX) $(CXXFLAGS) -I$(BOOST_INC) -c cftp.cpp

test.o: test.cpp dynamic_programming.h glauber_dynamics.h random_trees.h
	$(CXX) $(CXXFLAGS) -I$(BOOST_INC) -c test.cpp

dynamic_programming.o: dynamic_programming.cpp dynamic_programming.h
	$(CXX) $(CXXFLAGS) -I$(BOOST_INC) -c dynamic_programming.cpp
glauber_dynamics.o: glauber_dynamics.cpp glauber_dynamics.h
	$(CXX) $(CXXFLAGS) -I$(BOOST_INC) -c glauber_dynamics.cpp
random_trees.o: random_trees.cpp random_trees.h
	$(CXX) $(CXXFLAGS) -I$(BOOST_INC) -c random_trees.cpp


clean:
	rm -f *.o test