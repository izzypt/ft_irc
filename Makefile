NAME = ircserv
CXX = c++
CXXFLAGS = -std=c++98 -Wall -Werror -Wextra
SRC = src/Config.cpp src/Log.cpp src/EpollSocketServer.cpp src/Controller.cpp src/SigHandler.cpp main.cpp
OBJ = Config.o Log.o EpollSocketServer.o Controller.o SigHandler.o main.o
DEPS = headers/Config.hpp headers/Log.hpp headers/EpollSocketServer.hpp headers/Controller.hpp headers/SigHandler.hpp
	
all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)

$(OBJ): $(SRC) $(DEPS)
	$(CXX) $(CXXFLAGS) -c $(SRC)
	
clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean fclean re all