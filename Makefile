# CPPFLAGS = -Wall -Wextra -Werror -std=c++98 -Iincludes -g -g3 -ggdb3
# Just tell me that I have warnings and SHUT UP AND LET ME COMPILE!
CPPFLAGS = -Wall -Wextra -std=c++98 -g3 -ggdb3
SRC      = $(wildcard src/*.cpp src/**/*.cpp)
OBJDIR   = builds
SRC_OBJ  = $(patsubst %.cpp,$(OBJDIR)/%.o, $(SRC))
CXX      = g++ # g++ is better! update to c++ later
NAME     = runme

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
