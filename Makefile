CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = crunner
SOURCES = main.c ini.c
OBJECTS = $(SOURCES:.c=.o)

PREFIX = $(HOME)
BINDIR = $(PREFIX)/bin
CONFIGDIR = $(PREFIX)/.config/crunner

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(TARGET)
	mkdir -p $(BINDIR)
	mkdir -p $(CONFIGDIR)
	cp $(TARGET) $(BINDIR)/$(TARGET)
	@echo "Installed $(TARGET) to $(BINDIR)/$(TARGET)"
	@echo "Config directory: $(CONFIGDIR)"
	@echo "Place your apps.ini in $(CONFIGDIR)/"

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean install