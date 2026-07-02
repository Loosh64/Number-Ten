CC       ?= cc
CFLAGS   ?= -O2 -Wall -Wextra -std=c11
LDLIBS   := -lgmp
TARGET   := digit_fraction

.PHONY: all clean

all: $(TARGET)

$(TARGET): digit_fraction.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f $(TARGET)
