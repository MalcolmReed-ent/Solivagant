#include "utils.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <ncurses.h>
#include "config.h"

// Helper function to trim whitespace
char* trim(char *str) {
    char *end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

char* clean_text(const char *text) {
    char *cleaned = malloc(strlen(text) * 2 + 1);  // Allocate more space to be safe
    int i = 0, j = 0;
    int capitalize_next = 1;  // Capitalize the first character

    while (text[i]) {
        if ((text[i] >= 'a' && text[i] <= 'z') ||
            (text[i] >= 'A' && text[i] <= 'Z') ||
            (text[i] >= '0' && text[i] <= '9') ||
            text[i] == ' ' || text[i] == '.' || text[i] == ',' ||
            text[i] == '!' || text[i] == '?' || text[i] == '\'' ||
            text[i] == '"' || text[i] == '-' || text[i] == ';' ||
            text[i] == ':' || text[i] == '(' || text[i] == ')') {
            
            if (capitalize_next && isalpha(text[i])) {
                cleaned[j++] = toupper(text[i]);
                capitalize_next = 0;
            } else {
                cleaned[j++] = text[i];
            }

            // Set flag to capitalize next character after sentence-ending punctuation
            if (text[i] == '.' || text[i] == '!' || text[i] == '?') {
                capitalize_next = 1;
            }
        } else if (text[i] == '\n' || text[i] == '\r') {
            // Replace newlines with spaces to avoid word cutoffs
            cleaned[j++] = ' ';
        }
        i++;
    }
    cleaned[j] = '\0';

    // Trim leading and trailing whitespace
    char *final = trim(cleaned);
    if (final != cleaned) {
        char *temp = strdup(final);
        free(cleaned);
        return temp;
    }
    return cleaned;
}

// Text wrapping function
char **wrap_text(const char *text, int width, int *line_count) {
    int text_len = strlen(text);
    int max_lines = text_len / width + 1;
    char **lines = malloc(max_lines * sizeof(char *));
    *line_count = 0;

    int line_start = 0;
    int line_end = width;

    while (line_start < text_len) {
        if (line_end >= text_len) {
            line_end = text_len;
        } else {
            while (line_end > line_start && !isspace(text[line_end])) {
                line_end--;
            }
            if (line_end == line_start) {
                line_end = line_start + width;
            }
        }

        int line_length = line_end - line_start;
        lines[*line_count] = malloc(line_length + 1);
        strncpy(lines[*line_count], text + line_start, line_length);
        lines[*line_count][line_length] = '\0';
        (*line_count)++;

        line_start = line_end;
        while (line_start < text_len && isspace(text[line_start])) {
            line_start++;
        }
        line_end = line_start + width;
    }

    return lines;
}

// Color support function
void setup_colors() {
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);  // Default
        init_pair(2, COLOR_BLACK, COLOR_WHITE);  // Inverted
        init_pair(3, COLOR_YELLOW, COLOR_BLACK); // Highlight
    }
}

// Utility functions for pagination
int pgup(int pos, int winhi, int preservedline, int c) {
    if (pos >= (winhi - preservedline) * c) {
        return pos - (winhi - preservedline) * c;
    } else {
        return 0;
    }
}

int pgdn(int pos, int tot, int winhi, int preservedline, int c) {
    (void)preservedline;  // Mark as unused

    if (pos + (winhi * c) <= tot - winhi) {
        return pos + (winhi * c);
    } else {
        pos = tot - winhi;
        if (pos < 0) {
            return 0;
        }
        return pos;
    }
}

int pgend(int tot, int winhi) {
    if (tot - winhi >= 0) {
        return tot - winhi;
    } else {
        return 0;
    }
}
