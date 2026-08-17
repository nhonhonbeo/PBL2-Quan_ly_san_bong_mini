CXX = g++
CXXFLAGS = -std=c++17 -Wall -I include

SRC_DIRS = src src/Core src/Account src/Field src/Booking src/Customer src/Tournament src/Storage src/UI
SOURCES = $(wildcard src/*.cpp) $(wildcard src/Core/*.cpp) $(wildcard src/Account/*.cpp) \
          $(wildcard src/Field/*.cpp) $(wildcard src/Booking/*.cpp) $(wildcard src/Customer/*.cpp) \
          $(wildcard src/Tournament/*.cpp) $(wildcard src/Storage/*.cpp) $(wildcard src/UI/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = build/QuanLySanBong

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: all
	./$(TARGET)

.PHONY: all clean run
