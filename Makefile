CXXFLAGS  = -Wall -Wextra -Werror -std=c++98
CPPFLAGS  = -MMD -MP
SRC       = src/Request/Request.cpp \
            src/Request/Request_utils.cpp \
	    src/config_file_parser/lexer/lexer.cpp \
	    src/config_file_parser/parser/interpreter.cpp \
            src/config_file_parser/parser/directive.cpp \
	    src/config_file_parser/parser/parser.cpp \
            src/config_file_parser/parser/configfile.cpp src/utils/utils.cpp \
	    src/core/utils.cpp src/core/Connections.cpp src/core/Multiplexer.cpp \
	    src/core/ListenerAddrInfo.cpp src/core/Listeners.cpp src/CGI/CGIResponse.cpp \
	    src/CGI/CGI.cpp src/Router/RouterResource.cpp src/Router/Router.cpp \
	    src/Router/Directory_listing.cpp src/Response/ResponseHelpers.cpp \
	    src/Response/Response.cpp $(MAIN_PATH)

OBJDIR    = builds
SRC_OBJ   = $(patsubst %.cpp,$(OBJDIR)/%.o, $(SRC))
DEP_FILES = $(patsubst %.cpp,$(OBJDIR)/%.d, $(SRC))
CXX       = c++
NAME      = webserv

MAIN_PATH ?= ./tests/main.cpp

all: $(NAME)

$(NAME): $(SRC_OBJ)
	$(CXX) $(SRC_OBJ) -o $(NAME)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c  $< -o $@

clean:
	$(RM) $(SRC_OBJ) $(DEP_FILES)

fclean: clean
	$(RM) $(NAME)

re: fclean all

-include $(DEP_FILES)

.PHONY: all clean fclean re
.SECONDARY: $(SRC_OBJ)
