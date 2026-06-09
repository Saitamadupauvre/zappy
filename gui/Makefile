BINARY := zappy_gui
BUILD_DIR := build
CMAKE := cmake

.PHONY: all clean fclean re debug release info

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

clean:
	@rm -rf $(BUILD_DIR)

fclean: clean
	@rm -f $(BINARY)

re: fclean all

%:
	@:
	