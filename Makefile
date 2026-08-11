CC = clang
CFLAGS = -Wall -Wextra 
TARGET = dino_terminal
SRC = dino_terminal.c

#Default Target: Build Excecutables
all: $(TARGET)

#Compile Rule
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

#shortcut to compile and run in one command
run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)