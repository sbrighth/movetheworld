CC		 	= gcc
CXX		 	= g++
CXXFLAGS 	= -O2 -g -Wall -fmessage-length=0 -Wno-unused-result -Wno-sizeof-pointer-memaccess -I$(INCPATH)
LDFLAGS  	= -L$(LIBPATH) -ltbase -ltnet -lpthread

BINPATH		= ../output/bin
INCPATH		= ../output/include
LIBPATH		= ../output/lib

OBJS		= scriptexec.o
TARGET		= texec

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
