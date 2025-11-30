SRC_DIR := .
OBJ_DIR := .
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

CXX := g++
CXXFLAGS := -IC:/SFML/include -g -Wall -fpermissive -std=c++17
LDFLAGS := -LC:/SFML/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

TARGET := Mandelbrot.exe

$(TARGET): $(OBJ_FILES)
	g++ -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	g++ $(CXXFLAGS) -c -o $@ $<

run:
	./$(TARGET)

clean:
	rm $(TARGET) *.o