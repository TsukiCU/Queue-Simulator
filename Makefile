PYTHON_INC = $(shell python3-config --includes)
NUMPY_INC = -I$(shell python3 -c 'import numpy; print(numpy.get_include())')
CXX = g++
CXXFLAGS = -O2 -Wall -g $(PYTHON_INC) $(NUMPY_INC) -lpython3.10
SRCS = mmcq.cc main.cc
TARGET = mmc

all:
	$(CXX) $(SRCS) $(CXXFLAGS) -o $(TARGET)

plot:
	$(CXX) $(SRCS) $(CXXFLAGS) -DENABLE_PLOT=1 -o $(TARGET)

