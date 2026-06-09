##
## EPITECH PROJECT, 2026
## Zappy
## File description:
## Makefile
##

SERVER_DIR	=	api
GUI_DIR		=	gui
AI_DIR		=	ai

SERVER_BIN	=	zappy_server
GUI_BIN		=	zappy_gui
AI_BIN		=	zappy_ai

all:	zappy_server zappy_gui zappy_ai

zappy_server:
	$(MAKE) -C $(SERVER_DIR)
	mv $(SERVER_DIR)/$(SERVER_BIN) .

zappy_gui:
	$(MAKE) -C $(GUI_DIR)
	mv $(GUI_DIR)/$(GUI_BIN) .

zappy_ai:
	$(MAKE) -C $(AI_DIR)
	mv $(AI_DIR)/$(AI_BIN) .

clean:
	$(MAKE) -C $(SERVER_DIR) clean
	$(MAKE) -C $(GUI_DIR) clean
	$(MAKE) -C $(AI_DIR) clean

fclean:
	$(MAKE) -C $(SERVER_DIR) fclean
	$(MAKE) -C $(GUI_DIR) fclean
	$(MAKE) -C $(AI_DIR) fclean

re:	fclean all

.PHONY:	all zappy_server zappy_gui zappy_ai clean fclean re
