CC = gcc
SC = glslc

SRC_DIR = source
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin
SHADER_SOURCE_DIR = shaders
SHADER_BIN_DIR = $(BIN_DIR)/shaders

CFLAGS = -std=c17 -g -Wall -O2 -I./$(INCLUDE_DIR) -lm

ifeq ($(OS), MACOS) 
	LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lm -lSDL2 -rpath /usr/local/lib
else
	LDFLAGS = -lm -lglfw3 -lvulkan-1 -lpthread
endif

# Source files
SOURCE_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJECT_FILES = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCE_FILES))
SHADER_SOURCE_FILES = $(wildcard $(SHADER_SOURCE_DIR)/*)
SHADER_FILES = $(patsubst $(SHADER_SOURCE_DIR)/%.frag, $(SHADER_BIN_DIR)/%_fragment.spv, $(SHADER_SOURCE_FILES)) $(patsubst $(SHADER_SOURCE_DIR)/%.vert, $(SHADER_BIN_DIR)/%_vertex.spv, $(SHADER_SOURCE_FILES)) $(patsubst $(SHADER_SOURCE_DIR)/%.comp, $(SHADER_BIN_DIR)/%_compute.spv, $(SHADER_SOURCE_FILES))

# Executable name
EXECUTABLE = $(BIN_DIR)/vulkan_app.exe

# Targets
all: $(BUILD_DIR) $(BIN_DIR) $(SHADER_BIN_DIR) $(SHADER_FILES) $(EXECUTABLE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(SHADER_BIN_DIR):
	mkdir -p $(SHADER_BIN_DIR)

$(EXECUTABLE): $(OBJECT_FILES)
	$(CC) $(OBJECT_FILES) -o $(EXECUTABLE) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SHADER_BIN_DIR)/%_fragment.spv: $(SHADER_SOURCE_DIR)/%.frag
	$(SC) $< -o $@

$(SHADER_BIN_DIR)/%_vertex.spv: $(SHADER_SOURCE_DIR)/%.vert
	$(SC) $< -o $@

$(SHADER_BIN_DIR)/%_compute.spv: $(SHADER_SOURCE_DIR)/%.comp
	$(SC) $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

shaders:
	$(SHADER_FILES)

test: all
	./$(EXECUTABLE)

debug: all
	gdb ./$(EXECUTABLE)

.PHONY: all clean shaders test