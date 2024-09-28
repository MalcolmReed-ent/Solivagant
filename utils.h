#ifndef UTILS_H
#define UTILS_H

char *trim(char *str);
char *clean_text(const char *text);
char **wrap_text(const char *text, int width, int *line_count);
void setup_colors();
int pgup(int pos, int winhi, int preservedline, int c);
int pgdn(int pos, int tot, int winhi, int preservedline, int c);
int pgend(int tot, int winhi);

#endif
