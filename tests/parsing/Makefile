CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

OBJ_DIR = objs

# Sources for THIS tester. TestSuite.cpp lives one directory up (shared by
# every tester); ConfigParserTests.cpp lives right here. main_parsing_tests.cpp
# is added later once it actually exists.

SRCS = ../TestSuite.cpp \
		ConfigParserTests.cpp

OBJS = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(SRCS)))

vpath %.cpp . ..

# Default target: compile-only for now. No main() exists yet, so there is
# nothing to link into a runnable binary until main_parsing_tests.cpp
# is added — at that point, add a $(NAME): $(OBJS) link rule here, same
# pattern as the root Makefile.

all: $(OBJS)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJ_DIR)

fclean: clean

re: fclean all

.PHONY: all clean fclean re