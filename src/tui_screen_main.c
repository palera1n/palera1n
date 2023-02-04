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

tui_screen_t tui_screen_main() {
    int ret = MAIN_SCREEN;
    bool can_start = false;
    newtCenteredWindow(60, 16, "palera1n version 2.0.0");
    newtComponent optionsButton = newtCompactButton(30, 14, "Options");
    newtComponent startButton = newtCompactButton(41, 14, "Start");
    newtComponent exitButton = newtCompactButton(50, 14, "Exit");
    newtComponent welcome = newtLabel(1, 0, "Welcome to palera1n!");
    newtComponent form = newtForm(NULL, NULL, 0);
    newtFormAddComponents(form, optionsButton, startButton, exitButton, welcome, NULL);
    newtRunForm(form);
    newtRefresh();
    newtComponent buttonPressed = newtFormGetCurrent(form);
    if (buttonPressed == optionsButton) {
        ret = OPTIONS_SCREEN;
    } else if (buttonPressed == startButton) {
        if (can_start)
            ret = ENTER_RECOVERY_SCREEN;
        else
            ret = MAIN_SCREEN;
    } else if (buttonPressed == exitButton) {
        ret = EXIT_SCREEN;
    }
    return ret;
}
