# Makefile for LLD_gbemu_wasm: Install Emscripten, Build, and Serve

# Variables
EMSDK_DIR := emsdk
EMSDK_REPO := https://github.com/emscripten-core/emsdk.git
EMSDK_VERSION := latest
EMCC_FLAGS := -I ./include -O3 -s USE_SDL=2 -s USE_SDL_GFX=2 --preload-file LG.gb -s ALLOW_MEMORY_GROWTH -s ASYNCIFY -pthread -Wpthreads-mem-growth
SRC_FILES := main.c \
             ./lib/cpu_fetch.c \
             ./lib/interrupts.c \
             ./lib/ppu_pipeline.c \
             ./lib/timer.c \
             ./lib/dma.c \
             ./lib/bus.c \
             ./lib/cart.c \
             ./lib/cpu_proc.c \
             ./lib/emu.c \
             ./lib/io.c \
             ./lib/ppu_sm.c \
             ./lib/ui.c \
             ./lib/cpu_util.c \
             ./lib/gamepad.c \
             ./lib/lcd.c \
             ./lib/ram.c \
             ./lib/cpu.c \
             ./lib/dbg.c \
             ./lib/instructions.c \
             ./lib/ppu.c \
			 ./lib/audio.c \
             ./lib/stack.c
OUTPUT := foo.html
PORT := 5545

.PHONY: all setup build serve clean

# Default target: setup, build, and serve
all: setup build serve

# Setup Emscripten SDK
setup:
	@if [ ! -d $(EMSDK_DIR) ]; then \
		echo "Cloning emsdk repository..."; \
		git clone $(EMSDK_REPO) $(EMSDK_DIR); \
	else \
		echo "emsdk already cloned."; \
	fi
	cd $(EMSDK_DIR) && \
	./emsdk install $(EMSDK_VERSION) && \
	./emsdk activate $(EMSDK_VERSION)
	@echo "Emscripten SDK setup complete."

# Build the project using emcc
# @bash -c "source $(EMSDK_DIR)/emsdk_env.sh"
#fish
#@bash -c "source $(EMSDK_DIR)/emsdk_env.fish"
build:
	@fish -c "source $(EMSDK_DIR)/emsdk_env.fish"
	@echo "Building the project..."
	emcc $(SRC_FILES) $(EMCC_FLAGS) -o $(OUTPUT)
	@echo "Build complete."

# Serve the project with emrun
#bash 
# @bash -c "source $(EMSDK_DIR)/emsdk_env.sh"
#fish
#@bash -c "source $(EMSDK_DIR)/emsdk_env.fish"
serve:
	@fish -c "source $(EMSDK_DIR)/emsdk_env.fish"
	@echo "Starting local server..."
	emrun --no_browser --port $(PORT)  ./
	@echo "Server started at http://localhost:$(PORT)/$(OUTPUT)"
	@echo "Press Ctrl+C to stop the server."

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(OUTPUT) foo.js foo.wasm
	@echo "Clean complete."
