CC			= gcc
CXX			= g++
INCPATH		= /tmp/exicon/include/
LIBPATH		= /tmp/exicon/lib/
CXXFLAGS	= -o2 -g -Wall -fPIC -I$(INCPATH)
LDFLAGS		= -shared -L$(LIBPATH) -ltbase

TARGET		= libtnet.so
OBJS		= msgbox.o msgqthread.o msgsend.o

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $< 
	
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

all: $(TARGET)
	@[ -d $(INCPATH) ] || mkdir -p $(INCPATH)
	@[ -d $(LIBPATH) ] || mkdir -p $(LIBPATH)
	cp -f *.h $(INCPATH)
	cp -f *.so $(LIBPATH) 


clean:
	rm -f *.o $(TARGET)

.PONY: all clean
