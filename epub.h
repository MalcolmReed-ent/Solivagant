#ifndef EPUB_H
#define EPUB_H

#include <zip.h>
#include <libxml/tree.h>

typedef struct {
    char *path;
    struct zip *file;
    char *rootfile;
    char *rootdir;
    char *toc;
    char *version;
    char **contents;
    int contents_count;
    char **toc_entries;
    int toc_entries_count;
} Epub;

Epub *epub_new(const char *fileepub);
void epub_free(Epub *epub);
void epub_initialize(Epub *epub);
void epub_parse_toc(Epub *epub);
char *read_file_from_zip(struct zip *z, const char *filename);
char *get_attribute(xmlNode *node, const char *attr_name);

#endif
