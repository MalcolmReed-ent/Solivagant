#include "epub.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

// Epub functions implementation
Epub *epub_new(const char *fileepub) {
    Epub *epub = malloc(sizeof(Epub));
    epub->path = strdup(fileepub);
    epub->file = zip_open(fileepub, 0, NULL);
    if (!epub->file) {
        fprintf(stderr, "Failed to open epub file: %s\n", fileepub);
        free(epub->path);
        free(epub);
        return NULL;
    }
    epub->rootfile = NULL;
    epub->rootdir = NULL;
    epub->toc = NULL;
    epub->version = NULL;
    epub->contents = NULL;
    epub->contents_count = 0;
    epub->toc_entries = NULL;
    epub->toc_entries_count = 0;
    return epub;
}

void epub_free(Epub *epub) {
    free(epub->path);
    zip_close(epub->file);
    free(epub->rootfile);
    free(epub->rootdir);
    free(epub->toc);
    free(epub->version);
    for (int i = 0; i < epub->contents_count; i++) {
        free(epub->contents[i]);
    }
    free(epub->contents);
    for (int i = 0; i < epub->toc_entries_count; i++) {
        free(epub->toc_entries[i]);
    }
    free(epub->toc_entries);
    free(epub);
}

void epub_initialize(Epub *epub) {
    // Read and parse container.xml
    char *container_xml = read_file_from_zip(epub->file, "META-INF/container.xml");
    if (!container_xml) {
        fprintf(stderr, "Failed to read container.xml\n");
        return;
    }

    xmlDocPtr doc = xmlReadMemory(container_xml, strlen(container_xml), "container.xml", NULL, 0);
    if (!doc) {
        fprintf(stderr, "Failed to parse container.xml\n");
        free(container_xml);
        return;
    }

    xmlXPathContextPtr context = xmlXPathNewContext(doc);
    xmlXPathRegisterNs(context, (xmlChar *)"cont", (xmlChar *)"urn:oasis:names:tc:opendocument:xmlns:container");
    xmlXPathObjectPtr result = xmlXPathEvalExpression((xmlChar *)"//cont:rootfile/@full-path", context);

    if (result && result->nodesetval && result->nodesetval->nodeNr > 0) {
        epub->rootfile = strdup((char *)result->nodesetval->nodeTab[0]->children->content);
    }

    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    xmlFreeDoc(doc);
    free(container_xml);

    // Set rootdir
    char *last_slash = strrchr(epub->rootfile, '/');
    if (last_slash) {
        epub->rootdir = strndup(epub->rootfile, last_slash - epub->rootfile + 1);
    } else {
        epub->rootdir = strdup("");
    }

    // Read and parse the rootfile (OPF)
    char *opf_content = read_file_from_zip(epub->file, epub->rootfile);
    if (!opf_content) {
        fprintf(stderr, "Failed to read OPF file\n");
        return;
    }

    doc = xmlReadMemory(opf_content, strlen(opf_content), "content.opf", NULL, 0);
    if (!doc) {
        fprintf(stderr, "Failed to parse OPF file\n");
        free(opf_content);
        return;
    }

    context = xmlXPathNewContext(doc);
    xmlXPathRegisterNs(context, (xmlChar *)"opf", (xmlChar *)"http://www.idpf.org/2007/opf");

    // Get version
    result = xmlXPathEvalExpression((xmlChar *)"//opf:package/@version", context);
    if (result && result->nodesetval && result->nodesetval->nodeNr > 0) {
        epub->version = strdup((char *)result->nodesetval->nodeTab[0]->children->content);
    }
    xmlXPathFreeObject(result);

    // Get TOC
    const char *toc_xpath = (strcmp(epub->version, "2.0") == 0) ?
        "//opf:manifest/*[@media-type='application/x-dtbncx+xml']/@href" :
        "//opf:manifest/*[@properties='nav']/@href";
    
    result = xmlXPathEvalExpression((xmlChar *)toc_xpath, context);
    if (result && result->nodesetval && result->nodesetval->nodeNr > 0) {
        char *toc_path = (char *)result->nodesetval->nodeTab[0]->children->content;
        epub->toc = malloc(strlen(epub->rootdir) + strlen(toc_path) + 1);
        sprintf(epub->toc, "%s%s", epub->rootdir, toc_path);
    }
    xmlXPathFreeObject(result);

    // Get spine and manifest items
    xmlXPathObjectPtr spine_result = xmlXPathEvalExpression((xmlChar *)"//opf:spine/opf:itemref/@idref", context);
    xmlXPathObjectPtr manifest_result = xmlXPathEvalExpression((xmlChar *)"//opf:manifest/opf:item", context);

    if (spine_result && spine_result->nodesetval && manifest_result && manifest_result->nodesetval) {
        epub->contents_count = spine_result->nodesetval->nodeNr;
        epub->contents = malloc(epub->contents_count * sizeof(char *));

        for (int i = 0; i < epub->contents_count; i++) {
            const char *idref = (char *)spine_result->nodesetval->nodeTab[i]->children->content;
            for (int j = 0; j < manifest_result->nodesetval->nodeNr; j++) {
                xmlNodePtr item = manifest_result->nodesetval->nodeTab[j];
                char *id = get_attribute(item, "id");
                if (id && strcmp(id, idref) == 0) {
                    char *href = get_attribute(item, "href");
                    if (href) {
                        epub->contents[i] = malloc(strlen(epub->rootdir) + strlen(href) + 1);
                        sprintf(epub->contents[i], "%s%s", epub->rootdir, href);
                        free(href);
                    }
                    free(id);
                    break;
                }
                free(id);
            }
        }
    }

    xmlXPathFreeObject(spine_result);
    xmlXPathFreeObject(manifest_result);
    xmlXPathFreeContext(context);
    xmlFreeDoc(doc);
    free(opf_content);

    // TODO: Implement TOC parsing
}

void epub_parse_toc(Epub *epub) {
    // Free existing TOC entries if any
    if (epub->toc_entries) {
        for (int i = 0; i < epub->toc_entries_count; i++) {
            free(epub->toc_entries[i]);
        }
        free(epub->toc_entries);
        epub->toc_entries = NULL;
        epub->toc_entries_count = 0;
    }

    // Try to parse the TOC file first
    char *toc_content = read_file_from_zip(epub->file, epub->toc);
    if (toc_content) {
        xmlDocPtr doc = xmlReadMemory(toc_content, strlen(toc_content), "toc.xml", NULL, 0);
        if (doc) {
            xmlXPathContextPtr context = xmlXPathNewContext(doc);
            xmlXPathRegisterNs(context, (xmlChar *)"epub", (xmlChar *)"http://www.idpf.org/2007/ops");
            xmlXPathRegisterNs(context, (xmlChar *)"xhtml", (xmlChar *)"http://www.w3.org/1999/xhtml");

            const char *nav_xpath = (strcmp(epub->version, "2.0") == 0) ?
                "//navMap/navPoint" :
                "//xhtml:nav[@epub:type='toc']//xhtml:a";

            xmlXPathObjectPtr result = xmlXPathEvalExpression((xmlChar *)nav_xpath, context);

            if (result && result->nodesetval && result->nodesetval->nodeNr > 0) {
                epub->toc_entries_count = result->nodesetval->nodeNr;
                epub->toc_entries = malloc(epub->toc_entries_count * sizeof(char *));

                if (epub->toc_entries) {
                    for (int i = 0; i < epub->toc_entries_count; i++) {
                        xmlNodePtr node = result->nodesetval->nodeTab[i];
                        xmlChar *text;
                        if (strcmp(epub->version, "2.0") == 0) {
                            text = xmlNodeGetContent(xmlFirstElementChild(node));
                        } else {
                            text = xmlNodeGetContent(node);
                        }
                        epub->toc_entries[i] = strdup((char *)text);
                        xmlFree(text);
                    }
                    xmlXPathFreeObject(result);
                    xmlXPathFreeContext(context);
                    xmlFreeDoc(doc);
                    free(toc_content);
                    return;  // Successfully parsed TOC
                }
            }

            if (result) xmlXPathFreeObject(result);
            xmlXPathFreeContext(context);
            xmlFreeDoc(doc);
        }
        free(toc_content);
    }

    // Fallback: Use HTML file count for chapters
    epub->toc_entries_count = epub->contents_count;
    epub->toc_entries = malloc(epub->toc_entries_count * sizeof(char *));

    if (!epub->toc_entries) {
        fprintf(stderr, "Failed to allocate memory for TOC entries\n");
        return;
    }

    for (int i = 0; i < epub->toc_entries_count; i++) {
        char chapter_name[20];
        snprintf(chapter_name, sizeof(chapter_name), "Chapter %d", i + 1);
        epub->toc_entries[i] = strdup(chapter_name);
        if (!epub->toc_entries[i]) {
            fprintf(stderr, "Failed to allocate memory for TOC entry %d\n", i);
            // Free previously allocated entries
            for (int j = 0; j < i; j++) {
                free(epub->toc_entries[j]);
            }
            free(epub->toc_entries);
            epub->toc_entries = NULL;
            epub->toc_entries_count = 0;
            return;
        }
    }
}

// Helper function to read a file from the zip archive
char *read_file_from_zip(struct zip *z, const char *filename) {
    struct zip_file *f = zip_fopen(z, filename, 0);
    if (!f) {
        return NULL;
    }
    
    struct zip_stat st;
    zip_stat(z, filename, 0, &st);
    char *contents = malloc(st.size + 1);
    zip_fread(f, contents, st.size);
    contents[st.size] = '\0';
    zip_fclose(f);
    return contents;
}

char *get_attribute(xmlNode *node, const char *attr_name) {
    xmlChar *value = xmlGetProp(node, (xmlChar *)attr_name);
    if (value) {
        char *result = strdup((char *)value);
        xmlFree(value);
        return result;
    }
    return NULL;
}
