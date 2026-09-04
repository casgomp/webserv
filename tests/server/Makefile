CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
NAME = run_server_tests

OBJ_DIR = objs

SRCS = main_server_tests.cpp \
		../TestSuite.cpp \
		ServerInitTests.cpp \
		../../src/serverInit.cpp

OBJS = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(SRCS)))

vpath %.cpp . .. ../../src

all: $(NAME)

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