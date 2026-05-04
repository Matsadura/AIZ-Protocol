CPP=c++
CPPFLAGS= -Wall -Wextra -Werror -std=c++98
NAME = webserv
SRC= $(shell find . -name "*.cpp")
OBJ= $(SRC:%.cpp=%.o)
RM = rm -f

%.o : %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

all : $(NAME)

$(NAME) : $(OBJ)
	$(CPP) $(CPPFLAGS) $(OBJ) -o $@

clean :
	$(RM) $(OBJ)

fclean : clean
	$(RM) $(NAME)

re : fclean all

