#ifndef LOG_H
#define LOG_H

#define RESET_COLOR     "\033[0m"
#define RED_COLOR       "\033[31m"
#define GREEN_COLOR     "\033[32m"
#define YELLOW_COLOR    "\033[33m"
#define BLUE_COLOR      "\033[34m"
#define MAGENTA_COLOR   "\033[35m"
#define CYAN_COLOR      "\033[36m"
#define WHITE_COLOR     "\033[37m"
#define GRAY_COLOR      "\033[90m"

#define LOG(fmt, ...) \
printf("(%s:%d) => " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#endif // LOG_H
