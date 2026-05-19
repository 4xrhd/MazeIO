CXX = g++
CXXFLAGS = -fdiagnostics-color=always -Wall
INCLUDES = -I./include
SRCS = ./src/main.cpp ./src/glad.c

# Detect OS
ifeq ($(OS),Windows_NT)
    TARGET = build/main.exe
    LDFLAGS = -Llib -lglfw3 -lopengl32 -lgdi32
    RM = del /Q /F
    MKDIR = if not exist build mkdir build
    EXEC = build\main.exe
    CLEAN_TARGET = build\main.exe
else
    TARGET = build/main
    LDFLAGS = -Llib -lglfw -lGL -lXrandr -lX11 -lrt -ldl
    RM = rm -f
    MKDIR = mkdir -p build
    EXEC = ./$(TARGET)
    CLEAN_TARGET = $(TARGET)
endif

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(SRCS)
	@$(MKDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(TARGET) $(LDFLAGS)

# Run target
run: all
	$(EXEC)

# Clean target
clean:
	-$(RM) $(CLEAN_TARGET)

.PHONY: all run clean