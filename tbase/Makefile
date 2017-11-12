CC			= gcc
CXX			= g++
CXXFLAGS	= -o2 -g -Wall -fPIC
LDFLAGS		= -shared

CP_INCPATH  = ../output/include
CP_LIBPATH  = ../output/lib

TARGET		= libtbase.so
OBJS		= base.o

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<
	
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

all: $(TARGET)
	@[ -d $(CP_INCPATH) ] || mkdir -p $(CP_INCPATH)
	@[ -d $(CP_LIBPATH) ] || mkdir -p $(CP_LIBPATH)
	cp -f *.h $(CP_INCPATH)
	cp -f *.so $(CP_LIBPATH)

clean:
	rm -f *.o $(TARGET)

.PONY: all clean
