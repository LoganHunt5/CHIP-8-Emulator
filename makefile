#OBJS specifies which files to compile as part of the project
OBJS = chip8.cpp

#CC specifies which compiler we're using
CC = g++

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
COMPILER_FLAGS = -w

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = chip8

GAME = IBMLogo.ch8

#This is the target that compiles our executable
$(OBJ_NAME): $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) -o $(OBJ_NAME)

run: $(OBJ_NAME)
	./$(OBJ_NAME) $(GAME)

clean:
	rm $(OBJ_NAME)
