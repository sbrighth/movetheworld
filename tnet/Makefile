CC			= gcc
CXX			= g++
CXXFLAGS	= -o2 -g -Wall -fPIC
LDFLAGS		= -shared -L../tbase -ltbase

TARGET		= libtnet.so
OBJS		= msgproc.o

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $< 
	
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

all: $(TARGET)

clean:
	rm -f *.o $(TARGET)

.PONY: all clean
