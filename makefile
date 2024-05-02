CC = gcc
CFLAGS = -g -Wall
LIBS = -lcurl

BIN = main
SRC = main.c openai.c cJSON.c
OBJ = $(SRC:.c=.o)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -rf $(BIN) $(OBJ)

