CXX=g++
CXXOPTS=-std=c++11 -Wall -Wextra -Werror -pedantic -static-libgcc -static-libstdc++
#OPTOPTS=-fno-rtti -O3 -DNDEBUG
OPTOPTS=-g
LINKOPTS=-lodbc32

test.exe: test.cpp odbcpp.o odbcpp_streams.o
	$(CXX) $(CXXOPTS) $(OPTOPTS) -o $@ $^ $(LINKOPTS) 

%.o: %.cpp %.hpp pointer_types.def nonpointer_types.def
	$(CXX) $(CXXOPTS) $(OPTOPTS) -c -o $@ $<

clean:
	-rm *.o *.exe
