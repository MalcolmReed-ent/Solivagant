#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include "epub.h"

int toc_menu(WINDOW *stdscr, Epub *epub, int index);
void meta_display(WINDOW *stdscr, Epub *epub);
void help_display(WINDOW *stdscr);

#endif
