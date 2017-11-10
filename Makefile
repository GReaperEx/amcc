SRCS := $(wildcard sources/*.cpp)
OBJS := $(patsubst sources/%.cpp,obj/%.o,$(SRCS))

CXXFLAGS := -DGLM_FORCE_RADIANS -Wall -O2 -std=c++11
LDFLAGS := -s -lSDL2_image -lSDL2 -lGLEW -lGL -lpthread

all : $(OBJS)
	g++ -o amcc $(OBJS) $(LDFLAGS)

obj/%.o : sources/%.cpp
	g++ $(CXXFLAGS) -c -o $@ $<

