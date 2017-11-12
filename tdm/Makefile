CC		 	= gcc
CXX		 	= g++
INCPATH		= -I../tbase/ -I../tnet/
LIBPATH		= -L../tbase/ -L../tnet/
CXXFLAGS 	= -O2 -g -Wall -fmessage-length=0 -Wno-unused-result -Wno-sizeof-pointer-memaccess $(INCPATH)
LDFLAGS  	= $(LIBPATH) -ltbase -ltnet -lpthread

CP_BINPATH	= ../output/bin

TARGET		= tdm
OBJS		= testdm.o testmng.o status.o jsoncpp.o

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $< 
	
$(TARGET):	$(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

all:	$(TARGET)
	@[ -d $(CP_BINPATH) ] || mkdir -p $(CP_BINPATH)
	cp -f $(TARGET) $(CP_BINPATH)

clean:
	rm -f $(OBJS) $(TARGET)

.PONY: all clean
