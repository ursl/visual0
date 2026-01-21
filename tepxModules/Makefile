# Assumes OpenCV is available via pkg-config as "opencv4".

CXX      = g++
CXXFLAGS = -O2 -Wall -std=c++17
OPENCV_CFLAGS := $(shell pkg-config --cflags opencv4)
OPENCV_LIBS   := $(shell pkg-config --libs opencv4)

SRC_DIR  = .
OBJ_DIR  = obj
BIN_DIR  = bin

TARGET   = $(BIN_DIR)/ocvMarkersFromJPG
SRC      = $(SRC_DIR)/ocvMarkersFromJPG.cc
OBJ      = $(OBJ_DIR)/ocvMarkersFromJPG.o

all: dirs $(TARGET)

dirs:
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET) $(OPENCV_LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) $(OPENCV_CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET)

.PHONY: all dirs clean
