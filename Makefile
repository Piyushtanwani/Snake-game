CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
LIBS = -lncursesw

TARGET = snake
TARGET_SIMPLE = snake_simple
SOURCE = snake.cpp
SOURCE_SIMPLE = snake_simple.cpp

all: $(TARGET_SIMPLE)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE) $(LIBS)

$(TARGET_SIMPLE): $(SOURCE_SIMPLE)
	$(CXX) $(CXXFLAGS) -o $(TARGET_SIMPLE) $(SOURCE_SIMPLE)

clean:
	rm -f $(TARGET)

install:
	sudo apt-get update && sudo apt-get install -y libncurses-dev libncursesw5-dev

.PHONY: all clean install