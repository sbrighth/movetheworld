CC		 =	gcc
CXX		 =	g++
CXXFLAGS =	-O2 -g -Wall -fmessage-length=0 -std=c++11
LDFLAGS  = 	-L../tbase -ltbase -lpthread 

TARGET =	tdm
OBJS =		tdm.o msgctrl.o

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $< 
	
$(TARGET):	$(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PONY: all clean