CC = gcc
CFLAGS = -Wall -Wextra -O2
SRC = main.c core.c network.c deps.c sync.c
OBJ = $(SRC:.c=.o)
TARGET = fnf

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
