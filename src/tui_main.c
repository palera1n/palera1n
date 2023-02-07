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

int redraw_screen() {
    return 0;
}

int destroy_window() {
    return 0;
}

int init_window() {
    setlocale(LC_ALL, NULL);
    setenv(
        "NEWT_COLORS",
        "root=,black\n"
        "border=lightgray,black\n"
        "title=lightgray,black\n"
        "roottext=red,black\n"
        "label=lightgray,black\n"
        "entry=lightgray,black\n"
        "window=red,black\n"
        "textbox=lightgray,black\n"
        "button=black,lightgray\n"
        "compactbutton=lightgray,black\n"
        "listbox=lightgray,black\n"
        "actlistbox=black,lightgray\n"
        "actsellistbox=black,lightgray\n"
        "checkbox=lightgray,black\n"
        "actcheckbox=black,lightgray\n",
        0);

    tui_started = true;
    newtInit();
    int cols, rows;
    newtGetScreenSize(&cols, &rows);
    if (cols < 80 || rows < 24) {
        newtFinished();
        tui_started = false;
        LOG(LOG_FATAL, "Terminal size must be at least 80x24");
        return -1;
    }
    LOG(LOG_VERBOSE3, "cols: %d, rows: %d\n", cols, rows);
    newtCls();
    newtSetColor(PI_LOG_COLOR, "yellow", "black");
    newtSetColor(PI_STEP_CURRENT, "lightgray", "black");
    newtSetColor(PI_STEP_OTHER, "gray", "black");
    newtDrawRootText(0, 0, "palera1n version 2.0.0");
    newtPushHelpLine(NULL);
    return 0;
}

int tui() {
    int ret = 0;
    if ((ret = init_window())) return ret;
    ret = MAIN_SCREEN;
    while (1) {
        switch (ret) {
        case ERROR_SCREEN:
            goto out;
            break;
        case EXIT_SCREEN:
            goto out;
            break;
        case MAIN_SCREEN:
            ret = tui_screen_main();
            break;
        case OPTIONS_SCREEN:
            ret = tui_screen_options();
            break;
        case ENTER_RECOVERY_SCREEN:
            ret = tui_screen_enter_recovery();
            break;
        case ENTER_DFU_SCREEN:
            ret = tui_screen_enter_dfu();
            break;
        case JAILBREAKING_SCREEN:
            ret = tui_screen_jailbreak();
            break;
        default:
            assert(0);
            goto out;
        }
        newtPopWindow();
    }
out:
    newtFinished();
    tui_started = false;
    return 0;
}
