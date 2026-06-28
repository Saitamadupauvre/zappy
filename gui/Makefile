BINARY := zappy_gui
BUILD_DIR := build
TEST_BUILD_DIR := build_tests
CMAKE := cmake

.PHONY: all clean fclean re debug release info tests_run coverage

all:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) ..
	@$(CMAKE) --build $(BUILD_DIR) -- -j$(shell nproc)

debug:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) -DCMAKE_BUILD_TYPE=Debug -DGUI_CONSOLE_LOG_LEVEL="zappy::LogLevel::DEBUG" -DGUI_FILE_LOG_LEVEL="zappy::LogLevel::TRACE" -DGUI_LOG_FILE_PATH="\"network.log\"" ..
	@$(CMAKE) --build $(BUILD_DIR) -- -j$(shell nproc)

release:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) -DCMAKE_BUILD_TYPE=Release -DGUI_CONSOLE_LOG_LEVEL="zappy::LogLevel::NONE" -DGUI_FILE_LOG_LEVEL="zappy::LogLevel::NONE" -DGUI_LOG_FILE_PATH="\"\"" ..
	@$(CMAKE) --build $(BUILD_DIR) -- -j$(shell nproc)

info:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) -DCMAKE_BUILD_TYPE=Release -DGUI_CONSOLE_LOG_LEVEL="zappy::LogLevel::INFO" -DGUI_FILE_LOG_LEVEL="zappy::LogLevel::NONE" -DGUI_LOG_FILE_PATH="\"\"" ..
	@$(CMAKE) --build $(BUILD_DIR) -- -j$(shell nproc)

tests_run:
	@mkdir -p $(TEST_BUILD_DIR)
	@cd $(TEST_BUILD_DIR) && $(CMAKE) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DGUI_CONSOLE_LOG_LEVEL="zappy::LogLevel::NONE" \
		-DGUI_FILE_LOG_LEVEL="zappy::LogLevel::NONE" \
		-DGUI_LOG_FILE_PATH="\"\"" \
		..
	@$(CMAKE) --build $(TEST_BUILD_DIR) --target unit_tests -- -j$(shell nproc)
	@$(TEST_BUILD_DIR)/unit_tests --verbose || true
	@echo ""
	@echo "=== Coverage Report ==="
	@gcovr --root . \
		--filter src/world/ \
		--filter src/parser/ \
		--object-directory $(TEST_BUILD_DIR) \
		--print-summary \
		--html-details coverage.html 2>/dev/null || \
	gcovr --root . \
		--filter src/world/ \
		--filter src/parser/ \
		--object-directory $(TEST_BUILD_DIR) \
		--print-summary
	@echo "Coverage HTML → coverage.html"

coverage: tests_run

clean:
	@rm -rf $(BUILD_DIR) $(TEST_BUILD_DIR)
	@rm -f coverage*.html coverage*.css

fclean: clean
	@rm -f $(BINARY)

re: fclean all

%:
	@:
	