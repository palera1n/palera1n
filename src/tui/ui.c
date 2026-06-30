#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "ui.h"
#include "../globals.h"
#include "../paleinfo.h"

#define WIDTH 80
#define HEIGHT 24

typedef enum {
    MODE_NONE = 0,
    MODE_NORMAL,
    MODE_RECOVERY,
    MODE_DFU
} DeviceMode;

typedef struct {
    int connected;
    int is_supported;
    DeviceMode mode;
    char product_type[32];
    char product_version[32];
    unsigned long long ecid;
} DeviceState;

static DeviceState current_device = {0, 0, MODE_NONE, "", "", 0};
static pthread_mutex_t device_mutex = PTHREAD_MUTEX_INITIALIZER;

void update_tui_device_state(const DeviceState* new_state) {
    pthread_mutex_lock(&device_mutex);
    current_device = *new_state;
    pthread_mutex_unlock(&device_mutex);
}

typedef enum {
    PANEL_MAIN = 0,
    PANEL_OPTIONS,
    PANEL_START_1,
    PANEL_START_2,
    PANEL_FINAL,
    PANEL_COUNT
} PanelState;

static PanelState current_panel = PANEL_MAIN;
static int selected = 1;
static int running = 1;

static const char *buttons_main[]    = { "[ Options ]", "[  Start  ]", "[  Quit   ]" };
static const char *buttons_options[] = { "[  Back   ]" };
static const char *buttons_start_1[] = { "[  Back   ]", "[  Next   ]" };
static const char *buttons_start_2[] = { "[  Back   ]", "[  Start  ]" };
static const char *buttons_final[]   = { "[  Back   ]" };

static void draw_panel_main(int sy, int sx) {
    mvprintw(sy + 2, sx + 2, "");

    pthread_mutex_lock(&device_mutex);
    DeviceState state = current_device;
    pthread_mutex_unlock(&device_mutex);
}

static void draw_panel_options(int sy, int sx, int sel) {
    mvprintw(sy + 2, sx + 2, "You may set the following options. If you don't know what they mean you'll");
    mvprintw(sy + 3, sx + 2, "probably have no reason to set them.");

    int states[] = {
        !!(palerain_flags & palerain_option_safemode),
        !!(palerain_flags & palerain_option_verbose_boot),
        !!(palerain_flags & palerain_option_force_revert),
        !!(palerain_flags & palerain_option_flower_chain)
    };

    const char *labels[] = {
        "Safe Mode",
        "Verbose Boot",
        "Restore System",
        "Dark Blockchain"
    };

    mvprintw(sy + 6, sx + 2, "[%c] ", states[0] ? 'x' : ' ');
    if (sel == 0) attron(A_REVERSE);
    printw("%s", labels[0]);
    if (sel == 0) attroff(A_REVERSE);

    mvprintw(sy + 7, sx + 2, "[%c] ", states[1] ? 'x' : ' ');
    if (sel == 1) attron(A_REVERSE);
    printw("%s", labels[1]);
    if (sel == 1) attroff(A_REVERSE);

    mvprintw(sy + 8, sx + 6, "Boot Arguments:");
    if (sel == 2) attron(A_REVERSE);
    mvprintw(sy + 9, sx + 6, "%-66s", boot_args);
    if (sel == 2) attroff(A_REVERSE);

    mvprintw(sy + 10, sx + 2, "[%c] ", states[2] ? 'x' : ' ');
    if (sel == 3) attron(A_REVERSE);
    printw("%s", labels[2]);
    if (sel == 3) attroff(A_REVERSE);

    mvprintw(sy + 11, sx + 2, "[%c] ", states[3] ? 'x' : ' ');
    if (sel == 4) attron(A_REVERSE);
    printw("%s", labels[3]);
    if (sel == 4) attroff(A_REVERSE);
}

static void draw_panel_start_1(int sy, int sx) {
    mvprintw(sy + 2, sx + 4, "Recovery");
}

static void draw_panel_start_2(int sy, int sx) {
    mvprintw(sy + 2, sx + 4, "DFU");
}

static void draw_panel_final(int sy, int sx) {
    mvprintw(sy + 2, sx + 4, "Exploit");
}

static void draw_ui(int total_items, int btn_cnt, const char **panel_buttons) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    erase();

    if (rows < HEIGHT || cols < WIDTH) {
        mvprintw(rows / 2, (cols - 34) / 2, "Terminal too small! Need min 80x24.");
        refresh();
        return;
    }

    int start_y = (rows - HEIGHT) / 2;
    int start_x = (cols - WIDTH) / 2;

    mvhline(start_y, start_x, ACS_HLINE, WIDTH);
    mvhline(start_y + HEIGHT - 1, start_x, ACS_HLINE, WIDTH);
    mvvline(start_y, start_x, ACS_VLINE, HEIGHT);
    mvvline(start_y, start_x + WIDTH - 1, ACS_VLINE, HEIGHT);

    mvaddch(start_y, start_x, ACS_ULCORNER);
    mvaddch(start_y, start_x + WIDTH - 1, ACS_URCORNER);
    mvaddch(start_y + HEIGHT - 1, start_x, ACS_LLCORNER);
    mvaddch(start_y + HEIGHT - 1, start_x + WIDTH - 1, ACS_LRCORNER);

    switch (current_panel) {
        case PANEL_MAIN:    draw_panel_main(start_y, start_x); break;
        case PANEL_OPTIONS: draw_panel_options(start_y, start_x, selected); break;
        case PANEL_START_1: draw_panel_start_1(start_y, start_x); break;
        case PANEL_START_2: draw_panel_start_2(start_y, start_x); break;
        case PANEL_FINAL:   draw_panel_final(start_y, start_x); break;
    }

    mvhline(start_y + 21, start_x + 1, ACS_HLINE, WIDTH - 2);

    int y = start_y + 22;
    int x = start_x + WIDTH - 2;

    for (int i = btn_cnt - 1; i >= 0; i--) {
        int len = (int)strlen(panel_buttons[i]);
        x -= len;

        int btn_idx = total_items - btn_cnt + i;

        int is_start_btn = (current_panel == PANEL_MAIN && btn_idx == 1);
        int can_start = 0;

        if (is_start_btn) {
            pthread_mutex_lock(&device_mutex);
            can_start = (current_device.connected && current_device.is_supported && current_device.mode != MODE_DFU);
            pthread_mutex_unlock(&device_mutex);
        }

        if (is_start_btn && !can_start) {
            attron(A_DIM);
            if (btn_idx == selected) attron(A_REVERSE);
            mvprintw(y, x, "%s", panel_buttons[i]);
            if (btn_idx == selected) attroff(A_REVERSE);
            attroff(A_DIM);
        } else {
            if (btn_idx == selected) attron(A_REVERSE);
            mvprintw(y, x, "%s", panel_buttons[i]);
            if (btn_idx == selected) attroff(A_REVERSE);
        }

        x -= 3;
    }

    refresh();
}

static void edit_boot_args(int start_y, int start_x) {
    curs_set(1);
    timeout(-1);

    int field_y = start_y + 9;
    int field_x = start_x + 6;
    int max_len = 51;

    char temp_buf[128];
    snprintf(temp_buf, sizeof(temp_buf), "%s", boot_args);
    int len = (int)strlen(temp_buf);

    while (1) {
        mvprintw(field_y, field_x, "%-66s", "");
        mvprintw(field_y, field_x, "%s", temp_buf);
        move(field_y, field_x + len);
        refresh();

        int input_ch = getch();

        if (input_ch == KEY_ENTER || input_ch == 10 || input_ch == 13) {
            break;
        } else if (input_ch == KEY_BACKSPACE || input_ch == 127 || input_ch == '\b') {
            if (len > 0) temp_buf[--len] = '\0';
        } else if (input_ch >= 32 && input_ch <= 126) {
            if (len < max_len) {
                temp_buf[len++] = (char)input_ch;
                temp_buf[len] = '\0';
            }
        }
    }

    snprintf(boot_args, sizeof(boot_args), "%s", temp_buf);
    curs_set(0);
    timeout(100);
}

static void handle_enter_options(int start_y, int start_x) {
    switch(selected) {
        case 0: palerain_flags ^= palerain_option_safemode; break;
        case 1: palerain_flags ^= palerain_option_verbose_boot; break;
        case 2: edit_boot_args(start_y, start_x); break;
        case 3: palerain_flags ^= palerain_option_force_revert; break;
        case 4: palerain_flags ^= palerain_option_flower_chain; break;
        case 5: current_panel = PANEL_MAIN; selected = 0; break;
    }
}

static void handle_enter_main() {
    switch(selected) {
        case 0:
            current_panel = PANEL_OPTIONS;
            selected = 5;
            break;
        case 1:
            pthread_mutex_lock(&device_mutex);
            int can_start = (current_device.connected && current_device.is_supported && current_device.mode != MODE_DFU);
            pthread_mutex_unlock(&device_mutex);

            if (can_start) {
                current_panel = PANEL_START_1;
                selected = 1;
            }
            break;
        case 2:
            running = 0;
            break;
    }
}

static void handle_enter_start_1() {
    switch(selected) {
        case 0: current_panel = PANEL_MAIN; selected = 1; break;
        case 1: current_panel = PANEL_START_2; selected = 1; break;
    }
}

static void handle_enter_start_2() {
    switch(selected) {
        case 0: current_panel = PANEL_START_1; selected = 1; break;
        case 1: current_panel = PANEL_FINAL; selected = 0; break;
    }
}

static void handle_enter_final() {
    if (selected == 0) {
        current_panel = PANEL_MAIN;
        selected = 1;
    }
}

static void handle_input(int ch, int total_items, int start_y, int start_x) {
    if (ch == ERR) return;

    switch(ch) {
        case KEY_UP:
        case KEY_LEFT:
            selected = (selected - 1 + total_items) % total_items;
            break;

        case KEY_DOWN:
        case KEY_RIGHT:
            selected = (selected + 1) % total_items;
            break;

        case KEY_ENTER:
        case 10:
        case 13:
            switch(current_panel) {
                case PANEL_MAIN:    handle_enter_main(); break;
                case PANEL_OPTIONS: handle_enter_options(start_y, start_x); break;
                case PANEL_START_1: handle_enter_start_1(); break;
                case PANEL_START_2: handle_enter_start_2(); break;
                case PANEL_FINAL:   handle_enter_final(); break;
                default: break;
            }
            break;
    }
}

void ui_run(void) {
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(100);

    int ch;

    while (running) {
        const char **panel_buttons = NULL;
        int btn_cnt = 0;
        int total_items = 0;

        switch (current_panel) {
            case PANEL_MAIN:    panel_buttons = buttons_main;    btn_cnt = 3; total_items = 3; break;
            case PANEL_OPTIONS: panel_buttons = buttons_options; btn_cnt = 1; total_items = 6; break;
            case PANEL_START_1: panel_buttons = buttons_start_1; btn_cnt = 2; total_items = 2; break;
            case PANEL_START_2: panel_buttons = buttons_start_2; btn_cnt = 2; total_items = 2; break;
            case PANEL_FINAL:   panel_buttons = buttons_final;   btn_cnt = 1; total_items = 1; break;
        }

        draw_ui(total_items, btn_cnt, panel_buttons);
        ch = getch();

        if (ch == 'q' || ch == 'Q') break;

        int rows, cols;
        getmaxyx(stdscr, rows, cols);
        if (rows >= HEIGHT && cols >= WIDTH) {
            int start_y = (rows - HEIGHT) / 2;
            int start_x = (cols - WIDTH) / 2;
            handle_input(ch, total_items, start_y, start_x);
        }
    }

    endwin();
}
