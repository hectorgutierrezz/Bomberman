# ==============================================================================
# Makefile para 2DGame / Bomberman (Linux)
# ==============================================================================

CXX      ?= g++
CC       ?= gcc
CXXFLAGS ?= -std=c++11 -Wall -Wextra -O2 -g -DGLEW_NO_GLU -MMD -MP
CFLAGS   ?= -Wall -Wextra -O2 -g -DGLEW_NO_GLU -MMD -MP

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

.PHONY: all run clean help assets test web

all: assets $(TARGET)
	@echo "=================================================="
	@echo " Compilacion completada con exito!"
	@echo " Ejecutable: ./$(TARGET)"
	@echo " Ejecutar con: make run"
	@echo "=================================================="

TEST_TARGET := test_features
TEST_SRC    := ../../tests/test_features.cpp
TEST_OBJS   := $(filter-out $(BUILD_DIR)/game_main.o,$(ALL_OBJS))
WEB_BUILD_DIR := web-build
WEB_TARGET := $(WEB_BUILD_DIR)/index.html
WEB_CPP_SRCS := $(wildcard $(SRC_DIR)/*.cpp)
WEB_IMAGE_LIB := $(SOIL_DIR)/stb_image_aug.c
WEB_IMAGE_OBJ := $(WEB_BUILD_DIR)/stb_image_aug.o
WEB_CFLAGS := -std=c++11 -O2 -I$(SRC_DIR) -I$(LIBS_DIR)/glm -I$(SOIL_DIR)
WEB_LDFLAGS := -O2 -sUSE_GLFW=3 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 \
               -sALLOW_MEMORY_GROWTH=1 -sFORCE_FILESYSTEM=1 -sEXIT_RUNTIME=0
WEB_ASSETS := --preload-file $(SRC_DIR)/images@/images \
              --preload-file $(SRC_DIR)/levels@/levels \
              --preload-file $(SRC_DIR)/sound@/sound \
              --preload-file $(SRC_DIR)/shaders@/shaders

test: assets $(TARGET)
	@echo "Compilando tests..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $(TEST_SRC) $(TEST_OBJS) $(LIBS) -o $(TEST_TARGET)
	@echo "Ejecutando tests de funcionalidades..."
	@./$(TEST_TARGET)
	@rm -f $(TEST_TARGET)

web: $(WEB_TARGET)
	@echo "Browser build ready: $(WEB_TARGET)"

$(WEB_BUILD_DIR):
	@mkdir -p $@

$(WEB_TARGET): $(WEB_CPP_SRCS) $(WEB_IMAGE_OBJ) $(wildcard $(SRC_DIR)/*.h) $(wildcard $(SRC_DIR)/shaders/*) ../../web/shell.html | $(WEB_BUILD_DIR)
	em++ $(WEB_CFLAGS) $(WEB_CPP_SRCS) $(WEB_IMAGE_OBJ) $(WEB_LDFLAGS) \
		$(WEB_ASSETS) --shell-file ../../web/shell.html -o $@

$(WEB_IMAGE_OBJ): $(WEB_IMAGE_LIB) | $(WEB_BUILD_DIR)
	emcc -O2 -I$(SOIL_DIR) -c $< -o $@

# Enlaces a recursos en 2DGame/02-Bubble para que se puedan cargar directamente desde aquí
assets:
	@if [ ! -e images ]; then ln -s $(SRC_DIR)/images images; fi
	@if [ ! -e shaders ]; then ln -s $(SRC_DIR)/shaders shaders; fi
	@if [ ! -e levels ]; then ln -s $(SRC_DIR)/levels levels; fi
	@if [ ! -e sound ]; then ln -s $(SRC_DIR)/sound sound; fi

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

-include $(wildcard $(BUILD_DIR)/*.d)

run: all
	@echo "Iniciando $(TARGET)..."
	./$(TARGET)

clean:
	@echo "Limpiando archivos de compilacion..."
	@rm -rf $(BUILD_DIR) $(TARGET) $(SRC_DIR)/$(TARGET) $(WEB_BUILD_DIR)
	@rm -f images shaders levels sound
	@echo "Limpieza completada."

help:
	@echo "Opciones disponibles:"
	@echo "  make        - Compilar el proyecto"
	@echo "  make run    - Compilar y ejecutar el juego"
	@echo "  make clean  - Eliminar binarios y archivos objeto"
	@echo "  make web    - Crear la version para navegador (Emscripten)"
	@echo "  make help   - Mostrar esta ayuda"
