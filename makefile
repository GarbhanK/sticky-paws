# ============================================================================
# Sticky Paws - Makefile
# ============================================================================
# Alternative build system using Make instead of CMake
# Useful for quick local builds and development iteration
#
# Prerequisites:
#   - Clang or GCC compiler
#   - Raylib library (pre-built in lib/ directory)
#   - make utility
#
# Quick start:
#   make              # Build for current platform (macOS default)
#   make run          # Build and run
#   make format       # Format code with clang-format
#   make help         # Show all available targets
# ============================================================================

# === Configuration ===
COMPILER_OSX     := clang
COMPILER_LINUX   := clang
COMPILER_WINDOWS := x86_64-w64-mingw32-gcc   # Cross-compiler for Windows

CFILES       := $(wildcard src/*.c)
INCLUDES     := -Ilib/
LIBRARY_PATH := -Llib/
CFLAGS       := -std=c99 -Wall -Werror $(INCLUDES)

# macOS libraries
LIBRARIES_OSX    := -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL -lraylib
# Linux libraries
LIBRARIES_LINUX  := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
# Windows libraries
LIBRARIES_WIN    := -lraylib -lopengl32 -lgdi32 -lwinmm

OUTPUT_DIR := bin
OUTPUT_OSX := $(OUTPUT_DIR)/sticky_paws
OUTPUT_LINUX := $(OUTPUT_DIR)/sticky_paws
OUTPUT_WIN := $(OUTPUT_DIR)/sticky_paws.exe

# === Targets ===
build: build_osx   ## Default build is macOS

build_osx: $(CFILES)
	@mkdir -p $(OUTPUT_DIR)
	$(COMPILER_OSX) $(CFLAGS) $(CFILES) $(LIBRARY_PATH) $(LIBRARIES_OSX) -o $(OUTPUT_OSX)
	@echo "Built for macOS: $(OUTPUT_OSX)"

build_linux: $(CFILES)
	@mkdir -p $(OUTPUT_DIR)
	$(COMPILER_LINUX) $(CFLAGS) $(CFILES) $(LIBRARY_PATH) $(LIBRARIES_LINUX) -o $(OUTPUT_LINUX)
	@echo "Built for Linux: $(OUTPUT_LINUX)"

build_windows: $(CFILES)
	@mkdir -p $(OUTPUT_DIR)
	$(COMPILER_WINDOWS) $(CFLAGS) $(CFILES) $(LIBRARY_PATH) $(LIBRARIES_WIN) -o $(OUTPUT_WIN)
	@echo "Built for Windows: $(OUTPUT_WIN)"

run: build_osx
	@./$(OUTPUT_OSX)

clean:
	@rm -rf $(OUTPUT_DIR)/*
	@echo "Cleaned build output."

rebuild: clean build_osx

format:
	@find src -name "*.c" -o -name "*.h" | xargs clang-format -i
	@echo "Formatted all source files."

lint:
	@clang -fsyntax-only $(CFLAGS) $(CFILES)
	@echo "Lint check passed."

help:
	@echo "Sticky Paws - Build System"
	@echo ""
	@echo "Usage: make [target]"
	@echo ""
	@echo "Build Targets:"
	@echo "  build          Build the app for macOS (default)"
	@echo "  build_osx      Build for macOS"
	@echo "  build_linux    Build for Linux"
	@echo "  build_windows  Build for Windows (cross-compile)"
	@echo "  run            Build and run (macOS)"
	@echo ""
	@echo "Development Targets:"
	@echo "  clean          Remove build artifacts"
	@echo "  rebuild        Clean and rebuild"
	@echo "  format         Format C source files with clang-format"
	@echo "  lint           Run syntax check"
	@echo "  help           Show this help message"
	@echo ""
	@echo "For CMake builds (recommended), see README.md"

.PHONY: build build_osx build_linux build_windows run clean rebuild format lint help
