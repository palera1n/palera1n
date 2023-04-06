#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <gui.h>

// 50/50 chance this will be removed for in-window alerts
#if __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#define IDOK 1
#define IDCANCEL 2
#define AL kCFUserNotificationNoteAlertLevel
#elif _WIN32
#include <windows.h>
#include <winuser.h>
#elif __linux__
// #include <gtk/gtk.h>
#elif BSD
#elif __ANDROID__
#else
#endif

void send_alert(const char *title, const char *msg) {
#if __APPLE__
    CFStringRef header_ref = CFStringCreateWithCString(NULL, title, strlen(title));
    CFStringRef message_ref = CFStringCreateWithCString(NULL, msg, strlen(msg));
    // CFStringRef url_ref = CFStringCreateWithCString( NULL, "",  strlen("") );
    CFURLRef icon_ref = CFURLCreateWithString(kCFAllocatorDefault, NULL, NULL);
    CFOptionFlags result;

    CFUserNotificationDisplayAlert(0, AL, icon_ref, NULL, NULL, header_ref, message_ref, NULL, NULL, NULL, &result);
    CFRelease(header_ref);
    CFRelease(message_ref);
#elif _WIN32
    MessageBox(0, msg, title, MB_OK);
#elif __linux__
    char line[256];
    FILE *fp;

    fp=popen("zenity --version","r");
    if (fp==NULL) {
        perror("Pipe returned a error");
    } else {
        while (fgets(line,sizeof(line),fp)) printf("%s",line);
        pclose(fp);
    }

    char *str_msg = (char*)malloc(40+strlen(title)+strlen(msg)+1);
    sprintf(str_msg, "zenity --info --title '%s' --text '%s'", title, msg);
    fp=popen(str_msg, "r");

    if (fp==NULL) {
        perror("Pipe returned a error");
    } else {
        printf("Exit code: %i\n",WEXITSTATUS(pclose(fp)));
    }
#elif BSD
#elif __ANDROID__
#else
#endif
}
