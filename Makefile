CXX=g++
CXXOPTS=-std=c++11 -Wall -Wextra -Werror -pedantic -static-libgcc -static-libstdc++
OPTOPTS=-fno-rtti -O3 -DNDEBUG
LINKOPTS=-lodbc32

test.exe: test.cpp odbcpp.o
	$(CXX) $(CXXOPTS) $(OPTOPTS) -o $@ $^ $(LINKOPTS) 

%.o: %.cpp %.hpp
	$(CXX) $(CXXOPTS) $(OPTOPTS) -c -o $@ $<

clean:
	-rm *.o *.exe
