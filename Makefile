CPPFLAGS  = -Wall -Wextra -std=c++98 -g3 -ggdb3
SRC       = $(shell find src/ -name '*.cpp') $(MAIN_PATH)
OBJDIR    = builds
SRC_OBJ   = $(patsubst %.cpp,$(OBJDIR)/%.o, $(SRC))
CXX       = g++
NAME      = runme

MAIN_PATH ?= ./tests/config_file_tests/main.cpp

all: $(NAME)

$(NAME): $(SRC_OBJ)
	$(CXX) $(SRC_OBJ) -o $(NAME)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) -c  $< -o $@

clean:
	$(RM) $(SRC_OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
.SECONDARY: $(SRC_OBJ)
