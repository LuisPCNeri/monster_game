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
	LIBS_DIR = win_libs
	TARGET = main.exe
	SDL_CFLAGS = -I$(LIBS_DIR)/include -Dmain=SDL_main
	SDL_LDFLAGS = -L$(LIBS_DIR)/lib -lmingw32 -lSDL2main -lSDL2
	
	FixPath = $(subst /,\,$1)
	MKDIR_P = if not exist "$(call FixPath,$(@D))" mkdir "$(call FixPath,$(@D))"
	CLEAN_CMD = if exist "$(call FixPath,$(BUILD_DIR))" rmdir /S /Q "$(call FixPath,$(BUILD_DIR))" & if exist "$(call FixPath,$(TARGET))" del /Q "$(call FixPath,$(TARGET))"
else
	LIBS_DIR = libs
	TARGET = main
	# Check for sdl2-config, fallback to pkg-config, then default
  ifneq ($(shell command -v sdl2-config 2> /dev/null),)
	SDL_CFLAGS := -I$(LIBS_DIR)/include $(shell sdl2-config --cflags)
	SDL_LDFLAGS := -L$(LIBS_DIR)/lib $(shell sdl2-config --libs) -Wl,-rpath,'$$ORIGIN/$(LIBS_DIR)/lib'
  else ifneq ($(shell command -v pkg-config 2> /dev/null),)
	SDL_CFLAGS := -I$(LIBS_DIR)/include $(shell pkg-config --cflags sdl2)
	SDL_LDFLAGS := -L$(LIBS_DIR)/lib $(shell pkg-config --libs sdl2) -Wl,-rpath,'$$ORIGIN/$(LIBS_DIR)/lib'
  else
	SDL_CFLAGS := -I$(LIBS_DIR)/include -D_REENTRANT
	SDL_LDFLAGS := -L$(LIBS_DIR)/lib -lSDL2 -Wl,-rpath,'$$ORIGIN/$(LIBS_DIR)/lib'
  endif
	MKDIR_P = mkdir -p $(@D)
	CLEAN_CMD = rm -rf $(BUILD_DIR) $(TARGET)
endif

# Recursive wildcard function to find all .c files
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
SRCS = $(call rwildcard,$(SRC_DIR),*.c) $(call rwildcard,$(LIBS_DIR),*.c)

OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(filter $(SRC_DIR)/%, $(SRCS))) \
       $(patsubst $(LIBS_DIR)/%.c, $(BUILD_DIR)/libs/%.o, $(filter $(LIBS_DIR)/%, $(SRCS)))

CFLAGS = -Wall -Wextra -std=c99 -I$(SRC_DIR) -I$(LIBS_DIR) $(SDL_CFLAGS)
LDFLAGS = $(SDL_LDFLAGS) -lSDL2_image -lSDL2_ttf

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/libs/%.o: $(LIBS_DIR)/%.c
	@$(MKDIR_P)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@$(MKDIR_P)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(CLEAN_CMD)

.PHONY: all clean