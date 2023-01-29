#define NCURSES_WIDECHAR 1

#include <ncurses.h>
#include <form.h>
#include <menu.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <assert.h>

#include <common.h>
#include <tui.h>

tui_screen_t tui_screen_jailbreak() { return EXIT_SCREEN; }
