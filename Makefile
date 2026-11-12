CXX = mpic++
CXXFLAGS = -std=c++2a -Wall

BOOST_INC = /usr/include
BOOST_LIB = /usr/lib


BOOST_LIBS = -L$(BOOST_LIB) -lboost_graph

all: cftp

cftp: cftp.o glauber_dynamics.o random_trees.o
	$(CXX) $(CXXFLAGS) -I$(BOOST_INC) -o cftp cftp.o glauber_dynamics.o random_trees.o $(BOOST_LIBS)

# Object file compilation
cftp.o: cftp.cpp glauber_dynamics.h random_trees.h
	$(CXX) $(CXXFLAGS) -I$(BOOST_INC) -c cftp.cpp

test.o: test.cpp glauber_dynamics.h random_trees.h
	$(CXX) $(CXXFLAGS) -I$(BOOST_INC) -c test.cpp

glauber_dynamics.o: glauber_dynamics.cpp glauber_dynamics.h
	$(CXX) $(CXXFLAGS) -I$(BOOST_INC) -c glauber_dynamics.cpp

random_trees.o: random_trees.cpp random_trees.h
	$(CXX) $(CXXFLAGS) -I$(BOOST_INC) -c random_trees.cpp

clean:
	rm -f *.o cftp