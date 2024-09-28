# solivagant - cli epub reader
# See LICENSE file for copyright and license details.

CC = gcc
CFLAGS = -Wall -g -Os $(shell pkg-config --cflags libzip libxml-2.0 json-c)
LIBS = $(shell pkg-config --libs libzip libxml-2.0 json-c) -lncurses

# Non pkg-config use compile method
#CFLAGS = -Wall -g -Os -I/usr/include/libzip -I/usr/include/libxml2 -I/usr/include/json-c
#LIBS = -lzip -lxml2 -ljson-c -lncurses

SRC = main.c epub.c html_parser.c reader.c ui.c utils.c
HEADERS = config.h epub.h html_parser.h reader.h ui.h utils.h
OBJ = $(SRC:.c=.o)
EXEC = solivagant

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1
DESKTOP_FILE = solivagant.desktop
DESKTOP_DIR = $(PREFIX)/share/applications
ICON_FILE = solivagant.png
ICON_DIR = $(PREFIX)/share/icons/hicolor/256x256/apps
MAN_FILE = solivagant.1

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LIBS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += -DDEBUG -g
debug: all

clean:
	rm -f $(OBJ) $(EXEC)

install: $(EXEC)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(EXEC) $(DESTDIR)$(BINDIR)
	install -d $(DESTDIR)$(DESKTOP_DIR)
	install -m 644 $(DESKTOP_FILE) $(DESTDIR)$(DESKTOP_DIR)
	install -d $(DESTDIR)$(ICON_DIR)
	install -m 644 $(ICON_FILE) $(DESTDIR)$(ICON_DIR)
	install -d $(DESTDIR)$(MANDIR)
	install -m 644 $(MAN_FILE) $(DESTDIR)$(MANDIR)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(EXEC)
	rm -f $(DESTDIR)$(DESKTOP_DIR)/$(DESKTOP_FILE)
	rm -f $(DESTDIR)$(ICON_DIR)/$(ICON_FILE)
	rm -f $(DESTDIR)$(MANDIR)/$(MAN_FILE)

dist: clean
	mkdir -p $(EXEC)-1.0
	cp $(SRC) $(HEADERS) Makefile README.md $(DESKTOP_FILE) $(ICON_FILE) $(MAN_FILE) $(EXEC)-1.0
	tar -czf $(EXEC)-1.0.tar.gz $(EXEC)-1.0
	rm -rf $(EXEC)-1.0

.PHONY: all debug clean install uninstall dist
