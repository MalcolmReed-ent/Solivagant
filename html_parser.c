#include "html_parser.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

HTMLtoLines *htmltolines_new() {
    HTMLtoLines *htl = malloc(sizeof(HTMLtoLines));
    htl->text = NULL;
    htl->text_count = 0;
    htl->imgs = NULL;
    htl->imgs_count = 0;
    htl->ishead = 0;
    htl->isinde = 0;
    htl->isbull = 0;
    htl->ispref = 0;
    htl->ishidden = 0;
    htl->idhead = NULL;
    htl->idhead_count = 0;
    htl->idinde = NULL;
    htl->idinde_count = 0;
    htl->idbull = NULL;
    htl->idbull_count = 0;
    htl->idpref = NULL;
    htl->idpref_count = 0;
    return htl;
}

void htmltolines_free(HTMLtoLines *htl) {
    for (int i = 0; i < htl->text_count; i++) {
        free(htl->text[i]);
    }
    free(htl->text);
    for (int i = 0; i < htl->imgs_count; i++) {
        free(htl->imgs[i]);
    }
    free(htl->imgs);
    free(htl->idhead);
    free(htl->idinde);
    free(htl->idbull);
    free(htl->idpref);
    free(htl);
}

void htmltolines_add_text(HTMLtoLines *htl, const char *text) {
    htl->text = realloc(htl->text, (htl->text_count + 1) * sizeof(char *));
    htl->text[htl->text_count] = strdup(text);
    htl->text_count++;
}

void parse_html(const char *content, HTMLtoLines *htl) {
    int state = IN_TEXT;
    char buffer[4096] = {0};  // Increased buffer size
    int buffer_index = 0;
    int in_style = 0;

    for (const char *p = content; *p; p++) {
        switch (state) {
            case IN_TEXT:
                if (*p == '<') {
                    if (buffer_index > 0) {
                        buffer[buffer_index] = '\0';
                        char *trimmed = trim(buffer);
                        if (strlen(trimmed) > 0 && !strstr(trimmed, "@page") && !strstr(trimmed, "body{")) {
                            char *cleaned = clean_text(trimmed);
                            if (strlen(cleaned) > 0) {
                                // Split cleaned text into lines of appropriate width
                                int width = 80;  // Adjust this value as needed
                                int len = strlen(cleaned);
                                for (int i = 0; i < len; i += width) {
                                    char line[width + 1];
                                    int line_len = (len - i < width) ? len - i : width;
                                    strncpy(line, cleaned + i, line_len);
                                    line[line_len] = '\0';
                                    
                                    // Adjust line break to not cut words
                                    if (line_len == width && i + width < len && !isspace(cleaned[i + width])) {
                                        int j = line_len - 1;
                                        while (j > 0 && !isspace(line[j])) j--;
                                        if (j > 0) {
                                            line[j] = '\0';
                                            i -= (line_len - j - 1);
                                        }
                                    }
                                    
                                    htmltolines_add_text(htl, trim(line));
                                }
                            }
                            free(cleaned);
                        }
                        buffer_index = 0;
                    }
                    state = IN_TAG;
                } else if (!in_style) {
                    if (buffer_index < 4095) {
                        buffer[buffer_index++] = *p;
                    }
                }
                break;
            case IN_TAG:
                if (*p == '>') {
                    buffer[buffer_index] = '\0';
                    if (strcmp(buffer, "style") == 0) {
                        in_style = 1;
                    } else if (strcmp(buffer, "/style") == 0) {
                        in_style = 0;
                    } else if (strcmp(buffer, "p") == 0 || strcmp(buffer, "div") == 0 || 
                               strcmp(buffer, "/p") == 0 || strcmp(buffer, "/div") == 0 ||
                               strcmp(buffer, "br") == 0) {
                        htmltolines_add_text(htl, "\n");
                    }
                    buffer_index = 0;
                    state = IN_TEXT;
                } else {
                    if (buffer_index < 4095) {
                        buffer[buffer_index++] = *p;
                    }
                }
                break;
        }
    }
}
