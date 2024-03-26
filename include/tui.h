#ifndef TUI_H
#define TUI_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <paleinfo.h>
#include <pthread.h>
#include <semaphore.h>

#define WIDTH 76
#define HEIGHT 19

#define CHECKBOX_STATE(flags, opt) (flags & opt) != 0 ? '*' : ' '

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define CLEAR_SCREEN printf("\033[2J")
#define RESETFONT printf("\x1b[0m")
#define PRINTAT(x, y, str) printf("\x1b[%d;%dH%s", y, x, str)
#define PRINTATT(x, y, str) PRINTAT(tui_x_offset + x, tui_y_offset + y, str)
#define PRINTATLEN(x, y, str, len) printf("\x1b[%d;%dH%." STR(len) "s", y, x, str)
#define PRINTATLENT(x, y, str, len) PRINTATLEN(tui_x_offset + x, tui_y_offset + y, str, len)
#define MOVETO(x, y) printf("\x1b[%d;%dH", y, x)
#define MOVETOT(x, y) MOVETO(tui_x_offset + x, tui_y_offset + y)
#define SETCOLOR(fg, bg) printf("\x1b[%d;%dm", fg, bg)
#define SETCOLORA(fg, bg, a) printf("\x1b[" STR(fg) ";" STR(bg) ";" STR(a) "m")
#define FG_BLACK 30
#define FG_YELLOW 33
#define FG_WHITE 37
#define BG_BLACK 40
#define BG_YELLOW 43
#define BG_WHITE 47
#define NONE 0
#define BOLD 1
#define FG_BRIGHT_BLACK 90
#define FG_BRIGHT_WHITE 97
#define BG_BRIGHT_BLACK 100
#define BG_BRIGHT_WHITE 107
#define G1D4(set) printf("\x1b\x29%c", set)
#define DSGON putchar('\x0e'); G1D4('0')
#define DSGOFF putchar('\x0f'); G1D4('B')
#define MOUSEON printf("\x1b[?1003h")
#define MOUSEOFF printf("\x1b[?1003l")
#define CIVIS printf("\x1b[?25l")
#define CNORM printf("\x1b[?25h")
#define SMCUP printf("\x1b[?1049h")
#define RMCUP printf("\x1b[?1049l")
#define DRAWLINE(x, y, len) \
    MOVETO(x, y); \
    DSGON; \
    for (int i = 0; i < len; i++) { \
        putchar(0x71); \
    } \
    DSGOFF;
#define DRAWLINET(x, y, len) DRAWLINE(tui_x_offset + x, tui_y_offset + y, len)

typedef enum tui_screens {
    ERROR_SCREEN = -1,
    EXIT_SCREEN = 0,
    MAIN_SCREEN = 1,
    OPTIONS_SCREEN = 2,
    ENTER_DFU_SCREEN = 3,
    JAILBREAK_SCREEN = 4,
    ENTER_RECOVERY_SCREEN = 5
} tui_screen_t;

typedef struct bitfield_checkbox_info {
    uint64_t* flags_p;
    uint64_t opt;
} tui_bit_info_t;

tui_screen_t tui_screen_main(void);
void tui_screen_main_redraw(void);
tui_screen_t tui_screen_options(void);
void tui_screen_options_redraw(void);
tui_screen_t tui_screen_enter_recovery(void);
void tui_screen_enter_recovery_redraw(void);
tui_screen_t tui_screen_enter_dfu(void);
void tui_screen_enter_dfu_redraw(void);
tui_screen_t tui_screen_jailbreak(void);
void tui_screen_jailbreak_redraw(void);

int redraw_screen(void);

void tui_draw_rectangle(int x1, int y1, int x2, int y2);

#ifdef BUILD_TAG
#define TUI_VERSION BUILD_TAG
#else
#define TUI_VERSION "Unknown Version"
#endif

extern int tui_state;
extern int tui_x_offset;
extern int tui_y_offset;

extern bool tui_can_start;

extern int tui_mouse_x;
extern int tui_mouse_y;

enum {
    TUI_INPUT_NONE,
    TUI_INPUT_LEFT,
    TUI_INPUT_RIGHT,
    TUI_INPUT_UP,
    TUI_INPUT_DOWN,
    TUI_INPUT_MOUSE_MOVE,
    TUI_INPUT_MOUSE_DOWN,
    TUI_INPUT_MOUSE_UP,
    TUI_INPUT_SELECT,
    TUI_INPUT_TAB,
    TUI_INPUT_TAB_BACK,
    TUI_INPUT_BACKSPACE
};

enum {
    TUI_EVENT_INPUT,
    TUI_EVENT_CONNECTED_DEVICES_CHANGED,
    TUI_EVENT_JAILBREAK_STATUS_CHANGED
};

int tui_get_input(void);
void *tui_input_thread(void *arg);
int tui_get_event(void);
int tui_try_get_event(void);

extern int tui_last_event;
extern int tui_last_input;
extern sem_t *tui_event_semaphore;
extern char tui_last_key;

void tui_devhelper(void);

enum tui_device_mode {
    TUI_DEVICE_MODE_NORMAL,
    TUI_DEVICE_MODE_RECOVERY,
    TUI_DEVICE_MODE_DFU
};

struct tui_connected_device {
    uint64_t ecid;
    char udid[44];
    char version[0x20];
    bool arm64;
    enum tui_device_mode mode;
    char product_type[0x20];
	char display_name[0x20];
    unsigned int cpid;
    bool requires_passcode_disabled;
    bool passcode_state;
    struct tui_connected_device *next;
};

extern struct tui_connected_device *tui_connected_devices;

int tui_compare_versions(const char *firstVersion, const char *secondVersion);

void tui_jailbreak(void);
extern bool tui_is_jailbreaking;
extern int tui_jailbreak_stage;
extern char *tui_jailbreak_status;

void tui_jailbreak_status_changed(void);

extern bool tui_is_restarting;
void tui_terminate(int sig);

extern bool tui_options_allow_untested;
extern bool tui_options_safe_mode;
extern bool tui_options_verbose_boot;
extern bool tui_options_force_revert;
extern bool tui_options_flower_chain;

extern char tui_options_boot_args[0x1e0 + 1];

#endif
