APP_NAME=atpg

OBJS=main.o cframe.o

CXX = g++
CXXFLAGS = -Wall -O3 -std=c++20 -m64 -I. -fopenmp -Wno-unknown-pragmas
MACCXX = g++-14
MACFLAGS = -Wall -O3 -std=c++20 -arch arm64 -I. -fopenmp -Wno-unknown-pragmas

all: $(APP_NAME)

$(APP_NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

mac: CXXFLAGS = $(MACFLAGS)
mac: CXX = $(MACCXX)
mac: $(APP_NAME)

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $<

clean:
	/bin/rm -rf *~ *.o $(APP_NAME) *.class
