CFLAGS = -I. -I../

CC = gcc -std=gnu99
OBJDIR = ./build

.PHONY: clean

$(shell mkdir -p $(OBJDIR))

SRC = ./main.c

OBJECTS = $(patsubst %.c,./$(OBJDIR)/%.o,$(notdir $(SRC)))

test : $(OBJECTS)
	${CC} $^ -g -o $@

./$(OBJDIR)/%.o : ./%.c
	${CC} -g -c $< -o $@ ${CFLAGS}

clean :
	-rm -f ./build/*.o test
