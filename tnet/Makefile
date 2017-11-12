CC			= gcc
CXX			= g++
INCPATH		= -I../tbase/
LIBPATH		= -L../tbase/
CXXFLAGS	= -o2 -g -Wall -fPIC $(INCPATH)
LDFLAGS		= -shared $(LIBPATH) -ltbase

CP_INCPATH  = ../output/include
CP_LIBPATH  = ../output/lib

TARGET		= libtnet.so
OBJS		= msgbox.o msgqthread.o msgsend.o socketserver.o socketclient.o

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
