NAME = zappy_server 

CC = gcc
CFLAGS = -Wall -Wextra
INCLUDES = -I ./include/
DEPFLAGS = -MMD -MP

SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
TEST_OBJ_DIR = $(BUILD_DIR)/test_obj

SRC = $(shell find $(SRC_DIR) -name "*.c")
OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))
DEPS = $(OBJ:.o=.d)

TEST_SRC = $(shell find $(TEST_DIR) -name "*.c" 2>/dev/null)
TEST_OBJ = $(patsubst $(TEST_DIR)/%.c, $(TEST_OBJ_DIR)/%.o, $(TEST_SRC))
TEST_DEPS = $(TEST_OBJ:.o=.d)

TARGET = $(BIN_DIR)/$(NAME)
ROOT_TARGET = $(NAME)
TEST_TARGET = unit_tests

all: $(TARGET) $(ROOT_TARGET)

$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CC) $(OBJ) $(CFLAGS) -o $(TARGET)

$(ROOT_TARGET): $(TARGET)
	cp $(TARGET) $(ROOT_TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEPFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	@echo "Compiling Test $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEPFLAGS) -c $< -o $@

tests_run: $(TEST_OBJ) $(filter-out $(OBJ_DIR)/main.o, $(OBJ))
	$(CC) $(TEST_OBJ) $(filter-out $(OBJ_DIR)/main.o, $(OBJ)) $(CFLAGS) -o $(TEST_TARGET)
	./$(TEST_TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(ROOT_TARGET)
	rm -rf $(TEST_TARGET)

fclean: clean

compile_commands.json: fclean
	bear -- make
re: fclean all

-include $(DEPS)
-include $(TEST_DEPS)

.PHONY: clean fclean re
