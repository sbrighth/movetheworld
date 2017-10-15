CC		 	= gcc
CXX		 	= g++
BINPATH		= /tmp/exicon/bin
INCPATH		= /tmp/exicon/include
LIBPATH		= /tmp/exicon/lib
CXXFLAGS 	= -O2 -g -Wall -fmessage-length=0 -Wno-unused-result -I$(INCPATH)
LDFLAGS  	=  -L$(LIBPATH) -ltbase -ltnet -lpthread 

TARGET =	tdm
OBJS =		testdm.o testmng.o

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $< 
	
$(TARGET):	$(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

all:	$(TARGET)
	@[ -d $(BINPATH) ] || mkdir -p $(BINPATH)
	cp -f $(TARGET) $(BINPATH)

clean:
	rm -f $(OBJS) $(TARGET)

.PONY: all clean