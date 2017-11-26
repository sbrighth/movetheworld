CC		 	= gcc
CXX		 	= g++
CXXFLAGS 	= -O2 -g -Wall -fmessage-length=0 -Wno-unused-result -I$(INCPATH)
LDFLAGS  	= -L$(LIBPATH) -ltbase -ltnet -lpthread

BINPATH		= ../output/bin
INCPATH		= ../output/include
LIBPATH		= ../output/lib

TARGET		= tdm
OBJS		= log.o testdm.o testmng.o status.o callback.o

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
