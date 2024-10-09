# building_demo: building_demo.c rotation_demo.o
#	gcc -g -Wall -Wextra -o0 building_demo.c rotation_demo.o -lcurses -lm -o building_demo

# rotation_demo.o: rotation_demo.c rotation_demo.h
#	gcc -g -o0 -Wall -Wextra -o0 -c rotation_demo.c -o rotation_demo.o

# clean:
#	rm -f *.o building_demo

# --------------------------------------------------------------------------- #

PROJECT := tetris

# Variables for paths of source, header, and test files
INC_DIR := ./include
SRC_DIR := ./src
SRCMODULES := $(wildcard $(SRC_DIR)/*.c)

# Variables for paths of object files and binary targets
BUILD_DIR := ./build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
EXECUTABLE := $(BIN_DIR)/$(PROJECT)
BUILD_DIRS := $(OBJ_DIR) $(BIN_DIR)
OBJMODULES := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCMODULES))

# C compiler configuration
CC = gcc # using gcc compiler
CFLAGS = -Wall -Wextra -g3 -O0 -Iinclude -fsanitize=address,undefined
# CFLAGS options:
# -Wall		Warnings: all - display every single warning;
# -Wextra	Warnings: extra - enable some extra warning flags that are not
#		enabled by -Wall;
# -g3		Compile with verbose debugging information, including
#		preprocessor macros and additional metadata;
# -O0		Disable compilation optimizations;
# -Iinclude	Add the directory /include to the list of directories to be
#  		searched for header files during preprocessing;
# -fsanitize=	Enable sanitizers, which inject extra checks into the code
#		compile time, preparing it to catch potential issues at runtime;
#	address
#		This sanitizer detects memory-related errors such as buffer
#		overflows, heap-use-after-free and stack-use-after-return;
#	undefined
#		This sanitizer detects undefined behavior.

all: $(EXECUTABLE)

# Display useful goals in this Makefile
help:
	@echo "Try one of the following make goals:"
	@echo " make             - compile the game"
	@echo " make readme      - project's documentation"
	@echo " make run         - start the game"
	@echo " make debug       - begin a gdb process for the executable"
	@echo " make leak_search - run the project under valgrind"
	@echo " make clean       - delete build files in project"
	@echo " make variables   - print Makefile's variables"

# Build the project by combining all object files
$(EXECUTABLE): $(OBJMODULES) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -lcurses -lm -o $@

# Build object files from sources in a template pattern
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -lm -o $@

ifeq ('', $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRCMODULES)
	$(CC) -MM -Iinclude -D $(D) $^ > $@

readme:
ifdef EDITOR
	@$(EDITOR) README.txt
else
	@xdg-open README.txt
endif

run: $(EXECUTABLE)
	@$(EXECUTABLE)

debug:
	gdb $(EXECUTABLE)

leak_search:
	valgrind --tool=memcheck --leak-check=full --errors-for-leak-kinds=definite,indirect,possible --show-leak-kinds=definite,indirect,possible $(EXECUTABLE)

clean:
	rm -f $(OBJ_DIR)/* $(EXECUTABLE)

variables:
	@echo "PROJECT =" $(PROJECT)
	@echo
	@echo "# Variables for paths of source, header, and test files"
	@echo "INC_DIR =" $(INC_DIR)
	@echo "SRC_DIR =" $(SRC_DIR)
	@echo "SRCMODULES =" $(SRCMODULES)
	@echo
	@echo "# Variables for paths of object files and binary targets"
	@echo "BUILD_DIR =" $(BUILD_DIR)
	@echo "OBJ_DIR =" $(OBJ_DIR)
	@echo "BIN_DIR =" $(BIN_DIR)
	@echo "EXECUTABLE =" $(EXECUTABLE)
	@echo "BUILD_DIRS =" $(BUILD_DIRS)
	@echo "OBJMODULES =" $(OBJMODULES)
	@echo
	@echo "# C compiler configuration"
	@echo "CC =" $(CC)
	@echo "CFLAGS =" $(CFLAGS)
