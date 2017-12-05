MAJOR		= 1
MINOR		= 1

CC			= gcc
CXX			= g++
CXXFLAGS	= -o2 -g -Wall -fPIC -I$(INCPATH)
LDFLAGS		= -shared -Wl,-soname,$(TARGET_SO) -L$(LIBPATH) -ltbase

INCPATH		= ../output/include
LIBPATH		= ../output/lib

OBJS		= msgbox.o msgqthread.o msgsend.o socketserver.o socketclient.o
TARGET_LINK	= libtnet.so
TARGET_SO	= $(TARGET_LINK).$(MAJOR)
TARGET_REAL	= $(TARGET_LINK).$(MAJOR).$(MINOR)

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $< 
	
$(TARGET_REAL): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

all: $(TARGET_REAL)
	@[ -d $(INCPATH) ] || mkdir -p $(INCPATH)
	@[ -d $(LIBPATH) ] || mkdir -p $(LIBPATH)

	@[ -e $(LIBPATH)/$(TARGET_SO) ]		|| ln -s $(TARGET_REAL) $(LIBPATH)/$(TARGET_SO)
	@[ -e $(LIBPATH)/$(TARGET_LINK) ]	|| ln -s $(TARGET_SO) $(LIBPATH)/$(TARGET_LINK)

	cp -f *.h $(INCPATH)
	cp -f *.so* $(LIBPATH)

clean:
	rm -f *.o *.so*

.PONY: all clean
