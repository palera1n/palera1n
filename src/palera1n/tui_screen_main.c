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

#include <palerain.h>
#include <tui.h>

tui_screen_t tui_screen_main() {
    int ret = MAIN_SCREEN;
    bool can_start = false;
    newtCenteredWindow(WIDTH, HEIGHT, NULL);
    newtComponent optionsButton = newtCompactButton(46, 18, "Options");
    newtComponent startButton = newtCompactButton(57, 18, "Start");
    newtComponent exitButton = newtCompactButton(66, 18, "Exit");
    newtComponent welcome = newtLabel(1, 0, "Welcome to palera1n!");
    newtComponent deviceMsg = newtTextbox(1, 2, 40, 3, NEWT_FLAG_WRAP);
    set_tui_log(deviceMsg);
    LOG(LOG_INFO, "Connect your iPhone, iPad or iPod Touch to begin.");
    newtTextboxSetColors(deviceMsg, PI_LOG_COLOR, PI_LOG_COLOR);
    set_tui_log(deviceMsg);
    newtComponent form = newtForm(NULL, NULL, 0);
    newtFormAddComponents(form, optionsButton, startButton, exitButton, welcome,
    deviceMsg, NULL);
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
    set_tui_log(NULL);
    newtFormDestroy(form);
    return ret;
}
