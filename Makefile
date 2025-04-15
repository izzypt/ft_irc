NAME = ircserv
CXX = c++
CXXFLAGS = -std=c++98 -Wall -Werror -Wextra
SRC = src/Config.cpp src/Log.cpp src/EpollSocketServer.cpp src/Controller.cpp src/SigHandler.cpp src/Client.cpp src/Channel.cpp main.cpp
OBJ = Config.o Log.o EpollSocketServer.o Controller.o SigHandler.o  Client.o Channel.o main.o
DEPS = headers/Config.hpp headers/Log.hpp headers/EpollSocketServer.hpp headers/Controller.hpp headers/SigHandler.hpp  headers/Client.hpp headers/Channel.hpp
	
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