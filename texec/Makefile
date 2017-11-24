CC		 	= gcc
CXX		 	= g++
INCPATH		= -I../tbase/ -I../tnet/
LIBPATH		= -L../tbase/ -L../tnet/
CXXFLAGS 	= -O2 -g -Wall -fmessage-length=0 -Wno-unused-result -Wno-sizeof-pointer-memaccess $(INCPATH)
LDFLAGS  	= $(LIBPATH) -ltbase -ltnet -lpthread

CP_BINPATH	= ../output/bin
CP_INCPATH  = ../output/include

TARGET		= texec
OBJS		= scriptexec.o

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $< 
	
$(TARGET):	$(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

all:	$(TARGET)
	@[ -d $(CP_BINPATH) ] || mkdir -p $(CP_BINPATH)
	cp -f *.h $(CP_INCPATH)
	cp -f $(TARGET) $(CP_BINPATH)

clean:
	rm -f $(OBJS) $(TARGET)

.PONY: all clean
