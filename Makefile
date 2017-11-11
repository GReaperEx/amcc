SRCS := $(wildcard sources/*.cpp)
OBJS := $(patsubst sources/%.cpp,obj/%.o,$(SRCS))

CXXFLAGS := -DGLM_FORCE_RADIANS -Wall -O2 -std=c++11
LDFLAGS := -s -lSDL2_image -lSDL2 -lGLEW -lGL -lpthread -lz

all : $(OBJS) chunks/
	g++ -o amcc $(OBJS) $(LDFLAGS)

obj/%.o : sources/%.cpp
	g++ $(CXXFLAGS) -c -o $@ $<

chunks/ :
	mkdir chunks/

.PHONY : clean
clean : 
	rm $(OBJS)
	rm chunks/*

