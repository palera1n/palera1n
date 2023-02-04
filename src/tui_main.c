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

int redraw_screen() {
    return 0;
}

int destroy_window() {
    return 0;
}

int init_window() {
    setlocale(LC_ALL, NULL);
    if (getenv("NEWT_COLORS") == NULL)
        putenv(
            "NEWT_COLORS=root=,black\n"
            "border=white,black\n"
            "title=white,black\n"
            "roottext=red,black\n"
            "window=red,black\n"
            "textbox=white,black\n"
            "button=black,green\n"
            "compactbutton=white,black\n"
            "listbox=white,black\n"
            "actlistbox=black,white\n"
            "actsellistbox=black,green\n"
            "checkbox=green,black\n"
            "actcheckbox=black,green\n");

    newtInit();
    newtCls();
    newtDrawRootText(0, 0, "Package Configuration");
    newtPushHelpLine(NULL);
    return 0;
}

int tui() {
    init_window();
    int ret = MAIN_SCREEN;
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
    return 0;

}
