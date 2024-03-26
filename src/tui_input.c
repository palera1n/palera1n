#ifdef TUI

#include <tui.h>

int tui_mouse_x = 0;
int tui_mouse_y = 0;
int tui_mouse_button = -1;

int tui_last_event = -1;
int tui_last_input = -1;
sem_t *tui_event_semaphore;
char tui_last_key = 0;

int tui_get_input(void) {
    char c = getc(stdin);
    if (c == '\x1b' && getc(stdin) == '[') {
        char c3 = getc(stdin);
        switch (c3) {
        case 'D':
            return TUI_INPUT_LEFT;
        case 'C':
            return TUI_INPUT_RIGHT;
        case 'A':
            return TUI_INPUT_UP;
        case 'B':
            return TUI_INPUT_DOWN;
        case 'Z':
            return TUI_INPUT_TAB_BACK;
        case 'M': {
            char c4 = getc(stdin);
            char c5 = getc(stdin);
            char c6 = getc(stdin);

            int new_mouse_x = c5 - 33;
            if (new_mouse_x < 0) new_mouse_x += 256;
            int new_mouse_y = c6 - 33;
            if (new_mouse_y < 0) new_mouse_y += 256;
            int mouse_button = c4 & 3;

            if (tui_mouse_x != new_mouse_x || tui_mouse_y != new_mouse_y) {
                tui_mouse_x = new_mouse_x;
                tui_mouse_y = new_mouse_y;
                return TUI_INPUT_MOUSE_MOVE;
            }

            if (tui_mouse_button != (c4 & 1)) {
                tui_mouse_button = c4 & 1;
                if (tui_mouse_button == 0) {
                    return TUI_INPUT_MOUSE_DOWN;
                } else {
                    return TUI_INPUT_MOUSE_UP;
                }
            }

            break;
        }
        }
    }
    
    tui_last_key = c;

    if (c == '\x0d' || c == ' ') {
        return TUI_INPUT_SELECT;
    }

    if (c == '\t') {
        return TUI_INPUT_TAB;
    }

    if (c == '\x7f') {
        return TUI_INPUT_BACKSPACE;
    }

    return TUI_INPUT_NONE;
}

void *tui_input_thread(void *arg) {
    while (1) {
        int input = tui_get_input();
        tui_last_event = TUI_EVENT_INPUT;
        tui_last_input = input;
        sem_post(tui_event_semaphore);
    }
    return 0;
}

int tui_get_event(void) {
    sem_wait(tui_event_semaphore);
    int last_event = tui_last_event;
    tui_last_event = -1;
    return last_event;
}

int tui_try_get_event(void) {
    int ret = sem_trywait(tui_event_semaphore);
    if (ret == 0) {
        int last_event = tui_last_event;
        tui_last_event = -1;
        return last_event;
    }
    return -1;
}

#endif
