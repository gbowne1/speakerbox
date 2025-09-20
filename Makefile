CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O3 -D_XOPEN_SOURCE=700 -D_GNU_SOURCE -DPI=3.14159265358979323846
DEBUGFLAGS = -g -DDEBUG
INCLUDES = -Iinclude
LIBS = -lglog -lpthread

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

release: CXXFLAGS += -O3
release: build_dir speakerbox

debug: CXXFLAGS += $(DEBUGFLAGS)
debug: build_dir speakerbox

speakerbox: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o build/$@ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

build_dir:
	mkdir -p build data docs

clean:
	rm -f src/*.o build/speakerbox
