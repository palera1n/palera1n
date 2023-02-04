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
    newtCenteredWindow(WIDTH, HEIGHT, NULL);
    newtComponent optionsButton = newtCompactButton(46, 17, "Options");
    newtComponent startButton = newtCompactButton(57, 17, "Start");
    newtComponent exitButton = newtCompactButton(66, 17, "Exit");
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
    newtFormDestroy(form);
    return ret;
}
