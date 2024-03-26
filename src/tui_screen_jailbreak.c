#ifdef TUI
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

bool tui_is_jailbreaking = false;

void tui_screen_jailbreak_nav(void) {
    if (tui_is_jailbreaking) {
        SETCOLOR(FG_BRIGHT_BLACK, BG_BLACK);
    } else {
        SETCOLOR(FG_BLACK, BG_BRIGHT_WHITE);
    }
    PRINTATT(80 - 11, 23, "[  Done  ]");
    SETCOLOR(FG_WHITE, BG_BLACK);
}

pthread_t tui_jailbreak_thread_id;
int tui_jailbreak_stage = 0;
char *tui_jailbreak_status = NULL;

void tui_jailbreak_status_changed(void) {
    tui_last_event = TUI_EVENT_JAILBREAK_STATUS_CHANGED;
    sem_post(tui_event_semaphore);
}

void *tui_jailbreak_thread(void *arg) {
    tui_jailbreak_stage = 1;
    tui_jailbreak_status = "About to execute checkra1n";

    tui_jailbreak_status_changed();

    if (!tui_connected_devices || tui_connected_devices->next || tui_connected_devices->mode != TUI_DEVICE_MODE_DFU) {
        tui_jailbreak_status = "ERROR: No single device in DFU mode found";
        tui_jailbreak_status_changed();
        pthread_exit(NULL);
        return NULL;
    }

    tui_jailbreak_stage = 2;
    tui_jailbreak_status = "Running exploit";
    tui_jailbreak_status_changed();

    if (exec_checkra1n() != 0) {
        tui_jailbreak_status = "ERROR: Failed to run exploit";
        tui_jailbreak_status_changed();
        pthread_exit(NULL);
        return NULL;
    }

    tui_jailbreak_stage = 3;
    tui_jailbreak_status = "Waiting for PongoOS";
    tui_jailbreak_status_changed();

    sleep(2);
    pthread_create(&pongo_thread, NULL, pongo_helper, NULL);

    // pongo helper will take care of the rest

    pthread_exit(NULL);
}

void tui_jailbreak(void) {
    tui_is_jailbreaking = true;
    if (tui_options_verbose_boot) {
        palerain_flags |= palerain_option_verbose_boot;
    } else {
        palerain_flags &= ~palerain_option_verbose_boot;
    }

    if (tui_options_safe_mode) {
        palerain_flags |= palerain_option_safemode;
    } else {
        palerain_flags &= ~palerain_option_safemode;
    }

    if (tui_options_force_revert) {
        palerain_flags |= palerain_option_force_revert;
    } else {
        palerain_flags &= ~palerain_option_force_revert;
    }

    snprintf(palerain_flags_cmd, 0x30, "palera1n_flags 0x%" PRIx64, palerain_flags);
    snprintf(xargs_cmd, sizeof(xargs_cmd), "xargs %s", tui_options_boot_args);

    pthread_create(&tui_jailbreak_thread_id, NULL, tui_jailbreak_thread, NULL);
}

void tui_screen_jailbreak_redraw(void) {
    PRINTATT(3, 3, "Installing jailbreak, this will take a moment. If the device asks for a");
    PRINTATT(3, 4, "passcode please enter it. Do not disconnect the device until finished.");

    if (tui_jailbreak_status) {
        SETCOLOR(FG_YELLOW, BG_BLACK);
        PRINTATT(3, 6, tui_jailbreak_status);
        SETCOLOR(FG_WHITE, BG_BLACK);
    }

    tui_draw_rectangle(5, 9, 80 - 6, 9 + 2);
    MOVETOT(6 + 1, 10 + 1);
    SETCOLOR(FG_BLACK, BG_WHITE);
    for (int i = 0; i < ((80 - 12) * ((float)tui_jailbreak_stage / 9)); i++) {
        putchar(' ');
    }
    SETCOLOR(FG_WHITE, BG_BLACK);

    tui_screen_jailbreak_nav();
}

int tui_jailbreak_nav_mouse_select = -1;

tui_screen_t tui_screen_jailbreak(void) {
    redraw_screen();

    while(1) {
        int event = tui_get_event();
        switch (event) {
        case TUI_EVENT_INPUT: {
            int input = tui_last_input;
            if (input == TUI_INPUT_NONE) continue;

            switch (input) {
            case TUI_INPUT_SELECT:
                if (!tui_is_jailbreaking) {
                    tui_is_restarting = true;
                    tui_terminate(0);
                }
                break;
            case TUI_INPUT_MOUSE_DOWN:
                if (tui_mouse_y == tui_y_offset + 24 - 2 &&
                    tui_mouse_x >= tui_x_offset + 80 - 12 &&
                    tui_mouse_x <= tui_x_offset + 80 - 12 + 10) {
                    tui_jailbreak_nav_mouse_select = 0;
                    redraw_screen();
                }
                break;
            case TUI_INPUT_MOUSE_UP:
                if (tui_jailbreak_nav_mouse_select == 0) {
                    tui_jailbreak_nav_mouse_select = -1;
                    if (!tui_is_jailbreaking) {
                        tui_is_restarting = true;
                        tui_terminate(0);
                    }
                }
                break;
            }
            break;
        }
        case TUI_EVENT_JAILBREAK_STATUS_CHANGED:
            redraw_screen();
            break;
        }
    }

    return EXIT_SCREEN;
}

#else
/* ISO C forbids an empty translation unit */
extern char** environ;
#endif
