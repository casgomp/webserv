CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
NAME = webserv

SRC_DIR = src
OBJ_DIR	= objs

SRCS = main.cpp \
		server4.cpp

OBJS = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRCS))

vpath %.cpp $(SRC_DIR)

all: $(NAME)

valgrind: CXXFLAGS += -g
valgrind: fclean $(NAME)

asan: CXXFLAGS += -g -fsanitize=address
asan: fclean $(NAME)

gdb: CXXFLAGS += -g
gdb: fclean $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
