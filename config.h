/* See LICENSE file for copyright and license details. */
#ifndef CONFIG_H
#define CONFIG_H

#define VERSION "1.0.00"
#define LICENSE "MIT"
#define AUTHOR "Malcolm Reed"
#define EMAIL "your.email@example.com"
#define PROJECT_URL "https://github.com/malcolmreed-ent/solivagant"

// Key bindings
#define SCROLL_DOWN KEY_DOWN
#define SCROLL_DOWN_J 'j'
#define SCROLL_UP KEY_UP
#define SCROLL_UP_K 'k'
#define HALF_DOWN 4
#define HALF_UP 21
#define PAGE_DOWN (KEY_NPAGE | 'l' | ' ' | KEY_RIGHT)
#define PAGE_UP (KEY_PPAGE | 'h' | KEY_LEFT)
#define CH_NEXT 'n'
#define CH_PREV 'p'
#define CH_HOME (KEY_HOME | 'g')
#define CH_END (KEY_END | 'G')
#define SHRINK '-'
#define WIDEN '+'
#define WIDTH '='
#define META 'm'
#define TOC ('\t' | 't')
#define FOLLOW 10
#define QUIT ('q' | 3 | 27 | 304)
#define HELP '?'
#define MARKPOS 'b'
#define JUMPTOPOS '`'
#define COLORSWITCH 'c'

#define LINEPRSRV 2  // You may need to adjust this value based on your requirements

// Colorscheme
#define DARK_FG 252
#define DARK_BG 235
#define LIGHT_FG 239
#define LIGHT_BG 223

#endif
