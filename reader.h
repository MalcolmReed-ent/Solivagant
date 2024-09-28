#ifndef READER_H
#define READER_H

#include <ncurses.h>
#include "epub.h"

void reader(WINDOW *stdscr, Epub *epub, int index, int y);

#endif
