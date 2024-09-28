#include "reader.h"
#include "ui.h"
#include "html_parser.h"
#include "utils.h"
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void reader(WINDOW *stdscr, Epub *epub, int index, int y) {
    int k = 0;
    int rows, cols;
    int marked_y = -1;
    int color_scheme = 0;
    int need_redraw = 1;
    HTMLtoLines *htl = NULL;

    while (1) {
        getmaxyx(stdscr, rows, cols);
        int max_width = cols * 2 / 3; // Use 2/3 of the screen width
        int x = (cols - max_width) / 2; // Center the text

        if (index < 0) index = 0;
        if (index >= epub->contents_count) index = epub->contents_count - 1;

        if (!htl) {
            char *content = read_file_from_zip(epub->file, epub->contents[index]);
            if (!content) {
                mvprintw(rows - 1, 0, "Failed to read content file for chapter %d", index + 1);
                refresh();
                getch();
                return;
            }

            htl = htmltolines_new();
            if (!htl) {
                free(content);
                mvprintw(rows - 1, 0, "Memory allocation failed");
                refresh();
                getch();
                return;
            }

            parse_html(content, htl);
            free(content);

            if (htl->text_count == 0) {
                mvprintw(rows - 1, 0, "No readable content found in chapter %d", index + 1);
                refresh();
                getch();
                htmltolines_free(htl);
                htl = NULL;
                index++;
                continue;
            }
        }

        if (need_redraw) {
            clear();
            int current_y = 0;
            int visible_lines = 0;

            // Draw and wrap text
            for (int i = y; i < htl->text_count && current_y < rows - 1; i++) {
                char *line = htl->text[i];
                int line_x = x;
                char word[256] = {0}; // Buffer for current word
                int word_len = 0;

                while (*line && current_y < rows - 1) {
                    if (isspace(*line)) {
                        // Print the word if it fits
                        if (line_x + word_len <= x + max_width) {
                            mvprintw(current_y, line_x, "%s", word);
                            line_x += word_len;
                            mvaddch(current_y, line_x, *line);
                            line_x++;
                        } else {
                            // Word doesn't fit, move to next line
                            current_y++;
                            line_x = x;
                            if (current_y < rows - 1) {
                                mvprintw(current_y, line_x, "%s", word);
                                line_x += word_len;
                            }
                        }
                        word[0] = '\0';
                        word_len = 0;
                    } else {
                        // Add character to current word
                        if (word_len < sizeof(word) - 1) {
                            word[word_len++] = *line;
                            word[word_len] = '\0';
                        }
                    }

                    if (line_x >= x + max_width) {
                        current_y++;
                        line_x = x;
                    }

                    line++;
                }

                // Print any remaining word
                if (word_len > 0 && current_y < rows - 1) {
                    if (line_x + word_len > x + max_width) {
                        current_y++;
                        line_x = x;
                    }
                    if (current_y < rows - 1) {
                        mvprintw(current_y, line_x, "%s", word);
                    }
                }

                current_y++;
                visible_lines++;
            }

            mvprintw(rows - 1, 0, "Chapter %d/%d | %d%% | Press '?' for help", 
                     index + 1, epub->contents_count, (int)((float)y / htl->text_count * 100));
            refresh();
            need_redraw = 0;
        }

        k = getch();

        switch (k) {
            case KEY_DOWN:
            case 'j':
                if (y < htl->text_count - 1) {
                    y++;
                    need_redraw = 1;
                }
                break;
            case KEY_UP:
            case 'k':
                if (y > 0) {
                    y--;
                    need_redraw = 1;
                }
                break;
            case KEY_NPAGE:
            case ' ':
                y += rows - 2;
                if (y >= htl->text_count) y = htl->text_count - 1;
                need_redraw = 1;
                break;
            case KEY_PPAGE:
                y -= rows - 2;
                if (y < 0) y = 0;
                need_redraw = 1;
                break;
            case 'n':
                if (index < epub->contents_count - 1) {
                    index++;
                    y = 0;
                    htmltolines_free(htl);
                    htl = NULL;
                    need_redraw = 1;
                }
                break;
            case 'p':
                if (index > 0) {
                    index--;
                    y = 0;
                    htmltolines_free(htl);
                    htl = NULL;
                    need_redraw = 1;
                }
                break;
            case 't':
                {
                    int new_index = toc_menu(stdscr, epub, index);
                    if (new_index != index) {
                        index = new_index;
                        y = 0;
                        htmltolines_free(htl);
                        htl = NULL;
                        need_redraw = 1;
                    }
                }
                break;
            case 'q':
                if (htl) htmltolines_free(htl);
                return;
            case META:
                meta_display(stdscr, epub);
                need_redraw = 1;
                break;
            case HELP:
                help_display(stdscr);
                need_redraw = 1;
                break;
            case MARKPOS:
                marked_y = y;
                break;
            case JUMPTOPOS:
                if (marked_y >= 0) {
                    y = marked_y;
                    need_redraw = 1;
                }
                break;
            case COLORSWITCH:
                color_scheme = !color_scheme;
                if (color_scheme) {
                    init_pair(1, LIGHT_FG, LIGHT_BG);
                } else {
                    init_pair(1, DARK_FG, DARK_BG);
                }
                wbkgd(stdscr, COLOR_PAIR(1));
                need_redraw = 1;
                break;
            case KEY_RESIZE:
                getmaxyx(stdscr, rows, cols);
                need_redraw = 1;
                break;
        }
    }
}
