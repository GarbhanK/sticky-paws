
COMPILER = clang
SOURCE_LIBS = -Ilib/
MACOS_OPT = -std=c99 -Wall -Werror -Llib/ -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL lib/libraylib.a
MACOS_OUT = -o "bin/sticky_paws"
CFILES = src/*.c

build_osx:
	$(COMPILER) $(CFILES) $(SOURCE_LIBS) $(MACOS_OUT) $(MACOS_OPT)

