#ifdef TUI
#ifdef __APPLE__
#define _DARWIN_C_SOURCE
#else
#define _DEFAULT_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <inttypes.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>

#include <palerain.h>
#include <tui.h>

#ifdef __APPLE__
int proc_pidpath(int pid, void * buffer, uint32_t buffersize);
char random_sem_name[sizeof("palera1n.tui_event_semaphore") + 16 + 1];
#endif

int tui_state = 0;
int tui_x_offset = 0;
int tui_y_offset = 0;

bool supports_bright_colors = true;

struct termios saved_termios;

int redraw_screen(void) {
    SETCOLOR(FG_BRIGHT_WHITE, BG_BLACK);
    CLEAR_SCREEN;
    MOVETO(0, 0);
    
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (w.ws_col < 80 || w.ws_row < 24) {
        LOG(LOG_FATAL, "Terminal size must be at least 80x24");
        printf("\033[37;41mERROR: Terminal size must be at least 80x24!");
        fflush(stdout);
        return -1;
    }
    tui_y_offset = (w.ws_row - 24) / 2;
    for (int i = 0; i < tui_y_offset; i++) {
        printf("\n");
    }
    tui_x_offset = (w.ws_col - 80) / 2;
    
    const char* tui_version = TUI_VERSION;
    if (tui_version[0] == 'v') tui_version++;

    tui_draw_rectangle(0, 0, 79, 23);
    MOVETOT((int)(80 - (strlen("[palera1n - Version ]") + strlen(tui_version) ) - 1), 1);
    printf("[palera1n - Version %s]", tui_version);
    SETCOLOR(FG_WHITE, BG_BLACK);
    switch(tui_state) {
    case MAIN_SCREEN:
        tui_screen_main_redraw();
        break;
    case OPTIONS_SCREEN:
        tui_screen_options_redraw();
        break;
    case ENTER_RECOVERY_SCREEN:
        tui_screen_enter_recovery_redraw();
        break;
    case ENTER_DFU_SCREEN:
        tui_screen_enter_dfu_redraw();
        break;
    case JAILBREAK_SCREEN:
        tui_screen_jailbreak_redraw();
        break;
    }
    fflush(stdout);
    return 0;
}

int destroy_window(void) {
    if (!tui_started) return 0;
    tui_started = false;
    tcsetattr(STDIN_FILENO, 0, &saved_termios);
    MOUSEOFF;
    CLEAR_SCREEN;

    MOVETO(0, 0);
    RMCUP;
    CNORM;
    
    return 0;
}

int init_window(void) {
    setlocale(LC_ALL, NULL);

    tui_started = true;
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (w.ws_col < 80 || w.ws_row < 24) {
        tui_started = false;
        LOG(LOG_FATAL, "Terminal size must be at least 80x24");
        return -1;
    }
    LOG(LOG_VERBOSE3, "cols: %d, rows: %d\n", w.ws_col, w.ws_row);
    SMCUP;
    CIVIS;
    fflush(stdout);
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    tcgetattr(STDIN_FILENO, &saved_termios);
    cfmakeraw(&term);
    term.c_lflag |= ISIG;
    tcsetattr(STDIN_FILENO, 0, &term);

    MOUSEON;

    return 0;
}

bool tui_is_restarting = false;

extern int saved_argc;
extern char** saved_argv;
extern char** saved_envp;

void tui_terminate(int sig) {
    destroy_window();
    if (tui_is_restarting) {
#ifdef __APPLE__
    sem_close(tui_event_semaphore);
    sem_unlink(random_sem_name);
    char pathbuf[4096];
    if (proc_pidpath(getpid(), pathbuf, sizeof(pathbuf)) <= 0) {
        LOG(LOG_FATAL, "Failed to get path of running executable");
        exit(1);
    }
    
    execve(pathbuf, saved_argv, saved_envp);
#elif defined(__linux__)
    execve("/proc/self/exe", saved_argv, saved_envp);
#endif
    } else {
        exit(0);
    }
}

void resize_handler(int sig) {
    redraw_screen();
}

int tui(void) {
    if (strncmp(getenv("TERM"), "xterm", 5) != 0) {
        supports_bright_colors = false;
    }
    srand(time(NULL));
    int ret = 0;
    if ((ret = init_window())) return ret;
    signal(SIGINT, tui_terminate);
    signal(SIGTERM, tui_terminate);
    signal(SIGQUIT, tui_terminate);
    signal(SIGWINCH, resize_handler);

#ifdef __APPLE__
    // generate a 16 character random hex string from urandom
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        LOG(LOG_FATAL, "Failed to open /dev/urandom");
        return -1;
    }
    uint8_t random[8];
    if (read(fd, random, 8) != 8) {
        LOG(LOG_FATAL, "Failed to read from /dev/urandom");
        return -1;
    }
    close(fd);
    for (int i = 0; i < 8; i++) {
        sprintf(&random_sem_name[i * 2], "%02x", random[i]);
    }
    sem_unlink(random_sem_name);
    tui_event_semaphore = sem_open(random_sem_name, O_CREAT, 0644, 0);
#else
    tui_event_semaphore = malloc(sizeof(sem_t));
    sem_init(tui_event_semaphore, 0, 0);
#endif
    pthread_t input_thread;
    pthread_create(&input_thread, NULL, tui_input_thread, NULL);

    tui_devhelper();

    tui_state = MAIN_SCREEN;
    while (1) {
        switch (tui_state) {
        case ERROR_SCREEN:
        case EXIT_SCREEN:
            goto out;
            break;
        case MAIN_SCREEN:
            tui_state = tui_screen_main();
            break;
        case OPTIONS_SCREEN:
            tui_state = tui_screen_options();
            break;
        case ENTER_RECOVERY_SCREEN:
            tui_state = tui_screen_enter_recovery();
            break;
        case ENTER_DFU_SCREEN:
            tui_state = tui_screen_enter_dfu();
            break;
        case JAILBREAK_SCREEN:
            tui_state = tui_screen_jailbreak();
            break;
        default:
            assert(0);
            goto out;
        }
    }
out:
    destroy_window();
    return 0;
}

void tui_draw_rectangle(int x1, int y1, int x2, int y2) {
    if (x1 == x2 || y1 == y2) return;
    DSGON;
    MOVETOT(x1 + 1, y1 + 1);
    printf("%c", 0x6C);
    for (int x = x1 + 1; x < x2; x++) {
        putchar(0x71);
    }
    putchar(0x6B);
    MOVETOT(x1 + 1, y2 + 1);
    putchar(0x6D);
    for (int x = x1 + 1; x < x2; x++) {
        putchar(0x71);
    }
    putchar(0x6A);
    for (int y = y1 + 1; y < y2; y++) {
        PRINTATT(x1 + 1, y + 1, "\x78");
        PRINTATT(x2 + 1, y + 1, "\x78");
    }
    DSGOFF;
    fflush(stdout);
}

#else
/* ISO C forbids an empty translation unit */
extern char** environ;
#endif
