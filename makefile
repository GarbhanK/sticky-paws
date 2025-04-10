# === Configuration ===
COMPILER     := clang
CFILES       := $(wildcard src/*.c)
INCLUDES     := -Ilib/
LIBRARY_PATH := -Llib/
LIBRARIES    := -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL -lraylib
CFLAGS       := -std=c99 -Wall -Werror $(INCLUDES)
LDFLAGS      := $(LIBRARY_PATH) $(LIBRARIES)
OUTPUT       := bin/sticky_paws

# === Targets ===

# Default build
build: build_osx

# Build for macOS
build_osx: $(CFILES)
	@mkdir -p $(dir $(OUTPUT))
	$(COMPILER) $(CFLAGS) $^ $(LDFLAGS) -o $(OUTPUT)

# Run the app
run: build_osx
	@./$(OUTPUT)

# Clean build artifacts
clean:
	@rm -rf bin/*
	@echo "Cleaned build output."

# Rebuild from scratch
rebuild: clean build_osx

# Format source code
format:
	@clang-format -i $(CFILES)
	@echo "Formatted source files."

# Basic lint check
lint:
	@clang -fsyntax-only $(CFLAGS) $(CFILES)
	@echo "Lint check passed."

# Help message
help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  build       Build the app (default)"
	@echo "  build_osx   Build the app for macOS"
	@echo "  run         Build and run the app"
	@echo "  clean       Remove build artifacts"
	@echo "  rebuild     Clean and rebuild"
	@echo "  format      Format C source files with clang-format"
	@echo "  lint        Run syntax check with clang"

.PHONY: build build_osx run clean rebuild format lint help


