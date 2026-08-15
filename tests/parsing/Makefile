CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
NAME = run_parsing_tests

OBJ_DIR = objs

# Sources for THIS tester. TestSuite.cpp lives one directory up (shared by
# every tester); ConfigParserTests.cpp lives right here. main_parsing_tests.cpp
# is added later once it actually exists.

SRCS = main_parsing_tests.cpp \
		../TestSuite.cpp \
		ConfigParserTests.cpp \
		HttpRequestParserTests.cpp

OBJS = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(SRCS)))

vpath %.cpp . ..

all: $(NAME)

# Default target: compile-only for now. No main() exists yet, so there is
# nothing to link into a runnable binary until main_parsing_tests.cpp
# is added — at that point, add a $(NAME): $(OBJS) link rule here, same
# pattern as the root Makefile.

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