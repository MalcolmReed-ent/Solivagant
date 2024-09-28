#ifndef HTML_PARSER_H
#define HTML_PARSER_H

#define IN_TEXT 0
#define IN_TAG 1
#define IN_ATTR 2

typedef struct {
    char **text;
    int text_count;
    char **imgs;
    int imgs_count;
    int ishead;
    int isinde;
    int isbull;
    int ispref;
    int ishidden;
    int *idhead;
    int idhead_count;
    int *idinde;
    int idinde_count;
    int *idbull;
    int idbull_count;
    int *idpref;
    int idpref_count;
} HTMLtoLines;

HTMLtoLines *htmltolines_new();
void htmltolines_free(HTMLtoLines *htl);
void htmltolines_add_text(HTMLtoLines *htl, const char *text);
void parse_html(const char *content, HTMLtoLines *htl);

#endif
