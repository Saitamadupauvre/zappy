##
## EPITECH PROJECT, 2026
## Zappy
## File description:
## Makefile
##

NAME	=	zappy_server

SRC	=	src/main.c

OBJ	=	$(SRC:.c=.o)

CFLAGS	+=	-Wall -Wextra -Werror
CFLAGS	+=	-Iinclude

all:	$(NAME)

$(NAME):	$(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(CFLAGS)

clean:
	$(RM) $(OBJ)

fclean:	clean
	$(RM) $(NAME)

re:	fclean all

.PHONY:	all clean fclean re
