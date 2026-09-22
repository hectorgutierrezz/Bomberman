# ==============================================================================
# Makefile para 2DGame / Bomberman (Linux)
# ==============================================================================

CXX      ?= g++
CC       ?= gcc
CXXFLAGS ?= -std=c++11 -Wall -Wextra -O2
CFLAGS   ?= -Wall -Wextra -O2

# Directorios relativos
SRC_DIR   := 02-Bubble
LIBS_DIR  := ../../libs
BUILD_DIR := build
TARGET    := bomberman

# Enlace sin espacios para SOIL si no existe
$(shell test -e $(LIBS_DIR)/SOIL || ln -sf "Simple OpenGL Image Library" $(LIBS_DIR)/SOIL)

SOIL_DIR  := $(LIBS_DIR)/SOIL/src
GLEW_DIR  := $(LIBS_DIR)/glew-1.13.0

# Flags de librerías e includes
GLFW_CFLAGS := $(shell pkg-config --cflags glfw3 2>/dev/null)
GLFW_LIBS   := $(shell pkg-config --libs glfw3 2>/dev/null || echo "-lglfw")

INCLUDES := -I$(SRC_DIR) \
            -I$(GLEW_DIR)/include \
            -I$(LIBS_DIR)/glm \
            -I$(SOIL_DIR) \
            $(GLFW_CFLAGS)

LIBS := $(GLFW_LIBS) -lGL -lm -lpthread -ldl

# Archivos fuente del juego (C++)
CPP_SRCS := $(wildcard $(SRC_DIR)/*.cpp)
CPP_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/game_%.o,$(CPP_SRCS))

# Archivos fuente de SOIL (C)
SOIL_SRCS := $(SOIL_DIR)/SOIL.c \
             $(SOIL_DIR)/image_DXT.c \
             $(SOIL_DIR)/image_helper.c \
             $(SOIL_DIR)/stb_image_aug.c
SOIL_OBJS := $(patsubst $(SOIL_DIR)/%.c,$(BUILD_DIR)/soil_%.o,$(SOIL_SRCS))

# Archivo fuente de GLEW (C)
GLEW_SRC  := $(GLEW_DIR)/src/glew.c
GLEW_OBJ  := $(BUILD_DIR)/glew.o

# Todos los objetos
ALL_OBJS := $(CPP_OBJS) $(SOIL_OBJS) $(GLEW_OBJ)

.PHONY: all run clean help assets

all: assets $(TARGET)
	@echo "=================================================="
	@echo " Compilacion completada con exito!"
	@echo " Ejecutable: ./$(TARGET)"
	@echo " Ejecutar con: make run"
	@echo "=================================================="

# Enlaces a recursos en 2DGame/02-Bubble para que se puedan cargar directamente desde aquí
assets:
	@if [ ! -e images ]; then ln -s $(SRC_DIR)/images images; fi
	@if [ ! -e shaders ]; then ln -s $(SRC_DIR)/shaders shaders; fi
	@if [ ! -e levels ]; then ln -s $(SRC_DIR)/levels levels; fi

$(TARGET): $(ALL_OBJS)
	@echo "Enlazando ejecutable $@..."
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@
	@ln -sf ../$(TARGET) $(SRC_DIR)/$(TARGET)

# Regla de compilacion para fuentes C++ del juego
$(BUILD_DIR)/game_%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo "Compilando C++ $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Regla de compilacion para SOIL (C)
$(BUILD_DIR)/soil_%.o: $(SOIL_DIR)/%.c | $(BUILD_DIR)
	@echo "Compilando SOIL $<..."
	$(CC) $(CFLAGS) -I$(SOIL_DIR) -c $< -o $@

# Regla de compilacion para GLEW (C)
$(BUILD_DIR)/glew.o: $(GLEW_SRC) | $(BUILD_DIR)
	@echo "Compilando GLEW $<..."
	$(CC) $(CFLAGS) -I$(GLEW_DIR)/include -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

run: all
	@echo "Iniciando $(TARGET)..."
	./$(TARGET)

clean:
	@echo "Limpiando archivos de compilacion..."
	@rm -rf $(BUILD_DIR) $(TARGET) $(SRC_DIR)/$(TARGET)
	@rm -f images shaders levels
	@echo "Limpieza completada."

help:
	@echo "Opciones disponibles:"
	@echo "  make        - Compilar el proyecto"
	@echo "  make run    - Compilar y ejecutar el juego"
	@echo "  make clean  - Eliminar binarios y archivos objeto"
	@echo "  make help   - Mostrar esta ayuda"
