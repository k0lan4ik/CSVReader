CC = gcc
CFLAGS = -Wall -Wextra -g -I./include

TARGET_NAME = csvreader

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

SRC_FILES = $(wildcard src/*.c)
TEST_FILES = $(wildcard tests/*.c)

MODULES = $(filter-out src/main.c, $(SRC_FILES))

MODULE_OBJS = $(patsubst src/%.c, $(OBJ_DIR)/%.o, $(MODULES))
MAIN_OBJ = $(OBJ_DIR)/main.o

TEST_BINS = $(patsubst tests/%.c, $(BIN_DIR)/%.bin, $(TEST_FILES))

TARGET = $(BIN_DIR)/$(TARGET_NAME)

all: setup $(TARGET) run_tests

setup:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)


$(TARGET): $(MODULE_OBJS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: src/%.c | setup
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.bin: tests/%.c $(MODULE_OBJS) | setup
	$(CC) $(CFLAGS) $< $(MODULE_OBJS) -o $@

run_tests: $(TEST_BINS)
	@for test in $(TEST_BINS); do \
		echo "Running $$test"; \
		./$$test || exit 1; \
	done

clean:
	rm -rf $(BUILD_DIR)