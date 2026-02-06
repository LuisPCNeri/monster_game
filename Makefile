# Detect OS
ifeq ($(OS),Windows_NT)
	detected_OS := Windows
else
	detected_OS := $(shell uname -s)
endif

CC = gcc

SRC_DIR = src
BUILD_DIR = build

# Platform-specific settings
ifeq ($(detected_OS),Windows)
	TARGET = main.exe
	SDL_CFLAGS = -Dmain=SDL_main
	SDL_LDFLAGS = -lmingw32 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2
	
	FixPath = $(subst /,\,$1)
	MKDIR_P = if not exist "$(call FixPath,$(@D))" mkdir "$(call FixPath,$(@D))"
	CLEAN_CMD = if exist "$(call FixPath,$(BUILD_DIR))" rmdir /S /Q "$(call FixPath,$(BUILD_DIR))" & if exist "$(call FixPath,$(TARGET))" del /Q "$(call FixPath,$(TARGET))"
else
	TARGET = main
	SDL_CFLAGS := $(shell sdl2-config --cflags)
	SDL_LDFLAGS := $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm
	MKDIR_P = mkdir -p $(@D)
	CLEAN_CMD = rm -rf $(BUILD_DIR) $(TARGET)
endif

# Recursive wildcard function to find all .c files
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
SRCS = $(call rwildcard,$(SRC_DIR),*.c)

OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(filter $(SRC_DIR)/%, $(SRCS)))

CFLAGS = -Wall -Wextra -std=c99 -I$(SRC_DIR) $(SDL_CFLAGS)
LDFLAGS = $(SDL_LDFLAGS)

release: CFLAGS += -O3
release: all

debug: CFLAGS += -g -O0
debug: all

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@$(MKDIR_P)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(CLEAN_CMD)

.PHONY: all clean release debug