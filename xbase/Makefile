UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	EX_SYS_DIR := /ssd
	EX_SERVER_DIR := /ssd
	CROSS_COMPILE	= $(EX_SERVER_DIR)/cross/linux/bin/arm-linux-gnueabihf-
else
	EX_SYS_DIR := D:/ssd
	EX_SERVER_DIR := D:/ssd/tools
	CROSS_COMPILE	= $(EX_SERVER_DIR)/cross/win/bin/arm-linux-gnueabihf-
endif	

CXX	:= g++
CC	:= gcc

CROSS_CXX	:= $(CROSS_COMPILE)g++
CROSS_CC	:= $(CROSS_COMPILE)gcc

EX_SYS_LIB_DIR := $(EX_SERVER_DIR)/lib
EX_SYS_INC_DIR := $(EX_SERVER_DIR)/include
EX_SYS_BIN_DIR := $(EX_SERVER_DIR)/bin

EX_SYS_SHA_DIR := $(EX_SYS_DIR)/athost/update
EX_SYS_SHA_LIB_DIR := $(EX_SYS_SHA_DIR)/lib
EX_SYS_SHA_INC_DIR := $(EX_SYS_SHA_DIR)/include
EX_SYS_SHA_BIN_DIR := $(EX_SYS_SHA_DIR)/bin
 
EX_BASE_DIR := ../../include

CFLAGS=-Wall -O2
CFLAGS+=-fno-strict-aliasing
CFLAGS+=-Wno-pointer-sign -Wno-sign-compare
CFLAGS+=-g -W -Wall -D__NCURSES $(EX_SYS_INC_DIR)

TARGET_WIN_LIB := xbase.dll
TARGET_LINUX_LIB := libxbase.so

ifeq ($(UNAME), Linux)
	TARGET_LIB := $(TARGET_LINUX_LIB)
	EX_LIBS        += -lm -fPIC -pthread
else
	TARGET_LIB := $(TARGET_WIN_LIB)
	EX_LIBS        += -lm -lWs2_32 -pthread
endif

CROSS_LIB := libxbase_c.so

EX_LIB_OBJS += base.o

all: $(CROSS_LIB)

$(TARGET_LIB): $(EX_LIB_OBJS)
	$(CXX) -I$(EX_SYS_INC_DIR) -c base.cpp $(EX_LIBS)
	$(CXX) -shared -Wl,-soname,$@ -o $@ $^ $(EX_LIBS)
	cp -rf $(TARGET_LIB) $(EX_SYS_LIB_DIR)
	cp -rf *.h $(EX_SYS_INC_DIR)
	
$(CROSS_LIB): $(EX_LIB_OBJS)
	$(CROSS_CXX) -I$(EX_SYS_INC_DIR) -c base.cpp -lm -fPIC
	$(CROSS_CXX) -shared -Wl,-soname,$@ -o $@ $^ -lm -fPIC	
	cp -rf $(CROSS_LIB) $(EX_SYS_LIB_DIR)
	cp -rf $(CROSS_LIB) $(EX_SYS_SHA_LIB_DIR)/$(CROSS_LIB)
	cp -rf *.h D:/ssd/athost/rack_001/tester001/update/include
	cp -rf $(CROSS_LIB) D:/ssd/athost/rack_001/tester001/update/lib
	cp -rf *.h $(EX_SYS_INC_DIR)
	cp -rf *.h $(EX_SYS_SHA_INC_DIR)
	
clean:
	rm -rf $(TARGET_LIB) *~ *.o *.a *.so* *.out *.exe
	rm -rf $(CROSS_LIB) *~ *.o *.a *.so* *.out *.exe

.PHONY: all clean
