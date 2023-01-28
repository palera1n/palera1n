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

#define MAIN_OPTIONS "Options"
#define MAIN_START "Start"
#define MAIN_EXIT "Exit"

enum tui_screens {
    ERROR_SCREEN = -1,
    EXIT_SCREEN = 0,
    MAIN_SCREEN = 1,
    OPTIONS_SCREEN = 2,
    ENTER_DFU_SCREEN = 3,
    JAILBREAKING_SCREEN = 4,
    ENTER_RECOVERY_SCREEN = 5
};

#endif
