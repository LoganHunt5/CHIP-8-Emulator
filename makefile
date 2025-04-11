#CC specifies which compiler we're using
CC = g++
CFLAGS = -w

#TARGET specifies the name of our exectuable
TARGET = chip8

OBJ = chip8.o

LIB = -lSDL2

GAME = Rocket.ch8

OUTPUT = chip8

#This is the target that compiles our executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LIB) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) $(OUTPUT)

run: $(TARGET)
	./$(TARGET) $(GAME)
