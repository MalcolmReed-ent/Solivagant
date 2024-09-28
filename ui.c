#include "ui.h"
#include "config.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

// TOC menu function
int toc_menu(WINDOW *stdscr, Epub *epub, int index) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    int hi = rows - 4, wi = cols - 4;
    int Y = 2, X = 2;
    int oldindex = index;

    WINDOW *toc = newwin(hi, wi, Y, X);
    keypad(toc, TRUE);
    box(toc, 0, 0);
    mvwprintw(toc, 1, 2, "Table of Contents");
    mvwprintw(toc, 2, 2, "-----------------");
    wrefresh(toc);

    WINDOW *pad = newpad(epub->toc_entries_count, wi - 2);
    keypad(pad, TRUE);

    int padhi = rows - 5 - Y - 4 + 1;
    int y = 0;
    if (index >= padhi / 2 && index < epub->toc_entries_count - padhi / 2) {
        y = index - padhi / 2 + 1;
    }

    for (int n = 0; n < epub->toc_entries_count; n++) {
        mvwprintw(pad, n, 0, "  %s", epub->toc_entries[n]);
    }

    int key_toc;
    do {
        for (int n = 0; n < epub->toc_entries_count; n++) {
            int attr = (index == n) ? A_REVERSE : A_NORMAL;
            mvwprintw(pad, n, 0, index == n ? ">>" : "  ");
            mvwchgat(pad, n, 0, -1, attr, 0, NULL);
        }

        prefresh(pad, y, 0, Y + 4, X + 4, rows - 5, cols - 6);
        key_toc = wgetch(toc);

        switch (key_toc) {
            case KEY_UP:
            case 'k':
                index = (index > 0) ? index - 1 : 0;
                break;
            case KEY_DOWN:
            case 'j':
                index = (index < epub->toc_entries_count - 1) ? index + 1 : epub->toc_entries_count - 1;
                break;
            case KEY_PPAGE:
                index = pgup(index, padhi, 0, 1);
                break;
            case KEY_NPAGE:
                index = pgdn(index, epub->toc_entries_count, padhi, 0, 1);
                break;
            case KEY_HOME:
            case 'g':
                index = 0;
                break;
            case KEY_END:
            case 'G':
                index = epub->toc_entries_count - 1;
                break;
        }

        while (index < y || index >= y + padhi) {
            if (index < y) {
                y--;
            } else {
                y++;
            }
        }
    } while (key_toc != '\n' && key_toc != 'q' && key_toc != 't');

    delwin(pad);
    delwin(toc);

    return (key_toc == '\n' && index != oldindex) ? index : oldindex;
}

// Metadata display function
void meta_display(WINDOW *stdscr, Epub *epub) {
    (void)epub;  // Mark epub as unused for now

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    int hi = rows - 4, wi = cols - 4;
    int Y = 2, X = 2;

    WINDOW *meta = newwin(hi, wi, Y, X);
    keypad(meta, TRUE);
    box(meta, 0, 0);
    mvwprintw(meta, 1, 2, "Metadata");
    mvwprintw(meta, 2, 2, "--------");
    wrefresh(meta);

    // TODO: Implement metadata extraction from epub
    // For now, we'll just display some dummy metadata
    char *dummy_metadata[] = {
        "Title: Sample EPUB",
        "Author: John Doe",
        "Publisher: EPUB Publishers",
        "Publication Date: 2023-05-01",
        "ISBN: 1234567890",
        "Language: English"
    };
    int meta_count = sizeof(dummy_metadata) / sizeof(dummy_metadata[0]);

    WINDOW *pad = newpad(meta_count, wi - 2);
    keypad(pad, TRUE);

    for (int n = 0; n < meta_count; n++) {
        mvwprintw(pad, n, 0, "%s", dummy_metadata[n]);
    }

    int y = 0;
    int key_meta;
    do {
        prefresh(pad, y, 0, Y + 4, X + 4, rows - 5, cols - 6);
        key_meta = wgetch(meta);

        switch (key_meta) {
            case KEY_UP:
            case 'k':
                y = (y > 0) ? y - 1 : 0;
                break;
            case KEY_DOWN:
            case 'j':
                y = (y < meta_count - hi + 6) ? y + 1 : meta_count - hi + 6;
                break;
        }
    } while (key_meta != 'q' && key_meta != 'm');

    delwin(pad);
    delwin(meta);
}

// Help display function
void help_display(WINDOW *stdscr) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    int hi = rows - 4, wi = cols - 4;
    int Y = 2, X = 2;

    WINDOW *help = newwin(hi, wi, Y, X);
    keypad(help, TRUE);
    box(help, 0, 0);
    mvwprintw(help, 1, 2, "Help");
    mvwprintw(help, 2, 2, "----");
    wrefresh(help);

    char *help_text[] = {
        "Key Bindings:",
        "q - Quit",
        "j/Down - Scroll down",
        "k/Up - Scroll up",
        "PgDn/Space - Page down",
        "PgUp - Page up",
        "n - Next chapter",
        "p - Previous chapter",
        "g/Home - Go to beginning",
        "G/End - Go to end",
        "t - Table of Contents",
        "m - Metadata",
        "? - This help screen"
    };
    int help_count = sizeof(help_text) / sizeof(help_text[0]);

    WINDOW *pad = newpad(help_count, wi - 2);
    keypad(pad, TRUE);

    for (int n = 0; n < help_count; n++) {
        mvwprintw(pad, n, 0, "%s", help_text[n]);
    }

    int y = 0;
    int key_help;
    do {
        prefresh(pad, y, 0, Y + 4, X + 4, rows - 5, cols - 6);
        key_help = wgetch(help);

        switch (key_help) {
            case KEY_UP:
            case 'k':
                y = (y > 0) ? y - 1 : 0;
                break;
            case KEY_DOWN:
            case 'j':
                y = (y < help_count - hi + 6) ? y + 1 : help_count - hi + 6;
                break;
        }
    } while (key_help != 'q' && key_help != '?');

    delwin(pad);
    delwin(help);
}
