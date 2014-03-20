CXX=g++
CXXOPTS=-std=c++11 -Wall -Wextra -Werror -pedantic -static-libgcc -static-libstdc++
#OPTOPTS=-s -fno-rtti -O3 -DNDEBUG
OPTOPTS=-g
LINKOPTS=-L. -lodbcpp -lodbc32
AR=ar
AROPTS=rcs

test.exe: test.cpp libodbcpp.a
	$(CXX) $(CXXOPTS) $(OPTOPTS) -o $@ $< $(LINKOPTS) 

libodbcpp.a: odbcpp.o odbcpp_streams.o
	$(AR) $(AROPTS) $@ $^

%.o: %.cpp %.hpp pointer_types.def nonpointer_types.def
	$(CXX) $(CXXOPTS) $(OPTOPTS) -c -o $@ $<

clean:
	-rm *.a *.o *.exe
