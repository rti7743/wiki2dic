CC=gcc
CXX=g++

#all cpp files
CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(notdir $(CPP_FILES:.cpp=.o))

CPPFLAGS+= \
$(CFLAGS) \
-std=gnu++0x \
-Wwrite-strings \
-Wno-unused-result \
-fpermissive \
-finput-charset=UTF-8 -fexec-charset=UTF-8 

LDFLAGS=

wiki2dic: $(OBJ_FILES)
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(OBJ) $(LDFLAGS)

#release build(default)
all: CFLAGS+=-O3
all: wiki2dic

#debug build
debug: CFLAGS+=-g -D_DEBUG -O0
debug: wiki2dic

#cleanp
clean:
	/bin/rm wiki2dic *.o *.bak *~ core -f 
