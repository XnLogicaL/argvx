CXX_STANDARD=c++23

test: test.cpp
	g++ -std=$(CXX_STANDARD) -I./include test.cpp -o test
