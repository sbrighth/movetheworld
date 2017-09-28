CC			= gcc
CXX			= g++
CXXFLAGS	= -o2 -g -Wall -fPIC
LDFLAGS		= -shared

TARGET		= libtbase.so
OBJS		= tbase.o

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<
	
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

all: $(TARGET)

clean:
	rm -f *.o $(TARGET)

.PONY: all clean
