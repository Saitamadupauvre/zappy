
AI_BIN		=	zappy_ai
SRC_DIR		=	src
PYTHON		=	python3

all:	$(AI_BIN)

$(AI_BIN):
	@$(PYTHON) -m zipapp $(SRC_DIR) \
		--main "main:main" \
		--output ../$(AI_BIN) \
		--python "/usr/bin/env python3"
	@chmod +x ../$(AI_BIN)

clean:
	@find . -name "__pycache__" -type d -exec rm -rf {} + 2>/dev/null || true
	@find . -name "*.pyc" -delete 2>/dev/null || true

fclean:	clean
	@rm -f $(AI_BIN)

re:	fclean all

.PHONY:	all clean fclean re
