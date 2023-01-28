#ifndef TUI_H
#define TUI_H
#define NCURSES_WIDECHAR 1

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>

struct button {
    char name[0x10];
};

struct button_list {
    uint8_t count;
    uint8_t current;
    uint8_t height;
    struct button buttons[];
};

#endif
