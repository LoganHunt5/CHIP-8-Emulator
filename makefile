#CC specifies which compiler we're using
CC = g++
CFLAGS = -w

#TARGET specifies the name of our exectuable
TARGET = chip8

OBJ = SDL_lib.o chip8.o

GAME = IBMLogo.ch8

OUTPUT = chip8

#This is the target that compiles our executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) $(OUTPUT)

run: $(TARGET)
	./$(TARGET) $(GAME)
