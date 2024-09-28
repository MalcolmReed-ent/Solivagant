#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ncurses.h>
#include "config.h"
#include "epub.h"
#include "reader.h"
#include "utils.h"

#define MAX_PATH 1024

// Global state structure
typedef struct {
    char last_read_file[MAX_PATH];
    int last_index;
    int last_width;
    int last_y;
} GlobalState;

GlobalState g_state = {0};

// Function to load global state
void load_global_state() {
    char *home = getenv("HOME");
    if (!home) return;

    char state_file[MAX_PATH];
    snprintf(state_file, sizeof(state_file), "%s/.epr_state", home);

    FILE *f = fopen(state_file, "r");
    if (!f) return;

    fscanf(f, "%s %d %d %d", 
           g_state.last_read_file, 
           &g_state.last_index, 
           &g_state.last_width, 
           &g_state.last_y);

    fclose(f);
}

// Function to save global state
void save_global_state() {
    char *home = getenv("HOME");
    if (!home) return;

    char state_file[MAX_PATH];
    snprintf(state_file, sizeof(state_file), "%s/.epr_state", home);

    FILE *f = fopen(state_file, "w");
    if (!f) return;

    fprintf(f, "%s %d %d %d", 
            g_state.last_read_file, 
            g_state.last_index, 
            g_state.last_width, 
            g_state.last_y);

    fclose(f);
}

// Function to print usage
void print_usage(const char *program_name) {
    printf("Usage: %s [options] [epub_file]\n", program_name);
    printf("Options:\n");
    printf("  -h, --help     Display this help message\n");
    printf("  -v, --version  Display version information\n");
    printf("  -d, --dump     Dump the contents of the epub file\n");
}

// Function to print version
void print_version() {
    printf("epr version %s\n", VERSION);
    printf("%s License\n", LICENSE);
    printf("Copyright (c) 2023 %s\n", AUTHOR);
    printf("%s\n", PROJECT_URL);
}

// Main function
int main(int argc, char *argv[]) {
    int opt;
    int dump_flag = 0;
    char *epub_file = NULL;

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"dump", no_argument, 0, 'd'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "hvd", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 'v':
                print_version();
                return 0;
            case 'd':
                dump_flag = 1;
                break;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    if (optind < argc) {
        epub_file = argv[optind];
    }

    load_global_state();

    if (!epub_file) {
        if (g_state.last_read_file[0] != '\0') {
            epub_file = g_state.last_read_file;
        } else {
            fprintf(stderr, "No epub file specified and no last read file found.\n");
            print_usage(argv[0]);
            return 1;
        }
    }

    Epub *epub = epub_new(epub_file);
    if (!epub) {
        fprintf(stderr, "Failed to open epub file: %s\n", epub_file);
        return 1;
    }

    epub_initialize(epub);
    epub_parse_toc(epub);

    if (dump_flag) {
        // Dump epub contents
        for (int i = 0; i < epub->contents_count; i++) {
            char *content = read_file_from_zip(epub->file, epub->contents[i]);
            if (content) {
                printf("%s\n\n", content);
                free(content);
            }
        }
    } else {
        // Start ncurses mode
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);

        setup_colors();

        int index = g_state.last_index;
        int y = g_state.last_y;

        reader(stdscr, epub, index, y);

        // End ncurses mode
        endwin();
    }

    // Save state and clean up
    snprintf(g_state.last_read_file, sizeof(g_state.last_read_file), "%s", epub_file);
    save_global_state();

    epub_free(epub);

    return 0;
}
