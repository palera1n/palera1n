#ifndef GUI_H
#define GUI_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define WINDOW_X 0
#define WINDOW_Y 0
#define WINDOW_SIZE_X 550
#define WINDOW_SIZE_Y 300

#define WINDOW_TITLE "palera1n"
#define WINDOW_RESIZE false
#define WINDOW_RESTORE true
#define WINDOW_ALLOW_FULLSCREEN false
#define WINDOW_BORDERLESS false
#define WINDOW_DECORATIONS true

#define IDLE false
#define TARGET_FPS 60
#define CONFIG_FILE "palera1n.ini"
#define EMBED_ASSETS true
#define ASSETS_DIRECTORY "assets"

#define NORMAL_MODE 1
#define RECOVERY_MODE 2
#define DFU_MODE 3
#define NONE 4
#define PONGO_MODE 5

int palera1n(int argc, char *argv[]);

#endif
