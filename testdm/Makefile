CXXFLAGS =	-O2 -g -Wall -fmessage-length=0 

OBJS =		testdm.o msgctrl.o

LIBS = 		-lpthread

TARGET =	testdm

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
