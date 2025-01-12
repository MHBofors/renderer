CC = gcc

OS = MACOS
SRC_DIR = source
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin
SHADER_DIR = shaders

CFLAGS = -std=c17 -g -Wall -O2 -I./$(INCLUDE_DIR)

ifeq ($(OS), WINDOWS) 
	LDFLAGS = -lglfw3 -lvulkan-1 -lpthread -lm
else
	LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lm -lSDL2 -rpath /usr/local/lib
endif

# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

# Executable name
EXECUTABLE = $(BIN_DIR)/vulkan_app.exe

# Targets
all: $(BUILD_DIR) $(BIN_DIR) $(EXECUTABLE) shaders

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(EXECUTABLE): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(EXECUTABLE) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

test: all
	./$(EXECUTABLE)

debug: all
	gdb ./$(EXECUTABLE)

.PHONY: all clean shaders test shaders