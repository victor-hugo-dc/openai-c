# CC = gcc
# CFLAGS = -g -Wall

# BIN = main
# SRC = main.c openai.c  # Add openai.c here

# all: $(BIN)

# $(BIN): $(SRC)
# 	$(CC) $(CFLAGS) $^ -o $@ -lcurl

# clean:
# 	$(RM) -rf $(BIN) *.dSYM

CC = gcc
CFLAGS = -g -Wall
LIBS = -lcurl

BIN = main
SRC = main.c openai.c cJSON.c # Add openai.c here
OBJ = $(SRC:.c=.o)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -rf $(BIN) $(OBJ)

